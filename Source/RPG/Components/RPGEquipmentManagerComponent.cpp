// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#include "RPG.h"
#include "../RPGCharacter.h"
#include "RPGEquipmentManagerComponent.h"
#include "../RPGPlayerController.h"


URPGEquipmentManagerComponent::URPGEquipmentManagerComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	
}
void URPGEquipmentManagerComponent::SetCharacterStats(ARPGItem* item, ARPGCharacter* character)
{
	if(item && character)
	{
		if(EquipedItems.Num() > 0)
		{
			if(item->Attributes.Constitution != 0) //add checks for the rest of the stats.
			{
				EquipedItems.Sort(URPGEquipmentManagerComponent::ConstPredicate); //sort array so the item with highest stat currently equiped is first

				if(item->Attributes.Constitution > EquipedItems[0]->Attributes.Constitution)
				{
					character->BaseAttributes.Constitution = character->BaseAttributes.Constitution - EquipedItems[0]->Attributes.Constitution;
					character->BaseAttributes.Constitution = character->BaseAttributes.Constitution + item->Attributes.Constitution;
				}						
			}
		}
		else if (EquipedItems.Num() == 0)
		{
			if(item->Attributes.Constitution != 0) 
			{
				character->BaseAttributes.Constitution = character->BaseAttributes.Constitution + item->Attributes.Constitution;				
			}
		}
	}
}

void URPGEquipmentManagerComponent::EquipChestItem(ARPGItem* item, ARPGCharacter* character)
{
	if(item && character)
	{
		if(item->ItemType == EItemType::Item_Chest) //make sure item is of right type. Just in case. Could be removed later as this check is redundant
		{
			if(!ChestSlot) //check if something is equiped in target slot
			{
					if(EquipedItems.Num() == 0) //if there is no item equiped
					{
						//we can just assign stats from item to the character
						SetCharacterStats(item, character);
						ChestSlot = item; //the assign item to the slot
						character->ChestMesh->SetSkeletalMesh(item->ChestMesh);
						EquipedItems.AddUnique(ChestSlot); //and add to array
						return;
					}
					else //otherwise
					{
						SetCharacterStats(item, character); //we check if the current item have higher stat that current equiped
						ChestSlot = item; //we just assign our item to the slot
						character->ChestMesh->SetSkeletalMesh(item->ChestMesh);
						EquipedItems.AddUnique(ChestSlot); //and add to array
					}
			}
			else //something is equiped in chest slot!
			{
				if(EquipedItems.Num() > 0) //so we assume that array  contain more than 0 elements
				{
					SetCharacterStats(item, character); //check if the current item have better stat than old ones
					EquipedItems.RemoveSingle(ChestSlot); //remove our old item from array
					ChestSlot = item; //asign new item
					character->ChestMesh->SetSkeletalMesh(item->ChestMesh);
					EquipedItems.AddUnique(ChestSlot); //and add it to array
				}
			}
		}
	}
}

void URPGEquipmentManagerComponent::EquipFootItem(ARPGItem* item, ARPGCharacter* character)
{
	if(item && character)
	{
		if(item->ItemType == EItemType::Item_Foot) //make sure item is of right type. Just in case. Could be removed later as this check is redundant
		{
			if(!FootSlot) //check if something is equiped in target slot
			{
					if(EquipedItems.Num() == 0) //if there is no item equiped
					{
						//we can just assign stats from item to the character
						SetCharacterStats(item, character);
						FootSlot = item; //the assign item to the slot
						EquipedItems.AddUnique(FootSlot); //and add to array
						return;
					}
					else //otherwise
					{
						SetCharacterStats(item, character); //we check if the current item have higher stat that current equiped
						FootSlot = item; //we just assign our item to the slot
						EquipedItems.AddUnique(FootSlot); //and add to array
					}
			}
			else //something is equiped in chest slot!
			{
				if(EquipedItems.Num() > 0) //so we assume that array  contain more than 0 elements
				{
					SetCharacterStats(item, character); //check if the current item have better stat than old ones
					EquipedItems.RemoveSingle(FootSlot); //remove our old item from array
					FootSlot = item; //asign new item
					EquipedItems.AddUnique(FootSlot); //and add it to array
				}
			}
		}
	}
}

void URPGEquipmentManagerComponent::EquipItem(TSubclassOf<class ARPGItem> item, ARPGCharacter* character)
{
	if(item)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bNoCollisionFail = true;
		ARPGItem* itemBase = GetWorld()->SpawnActor<ARPGItem>(item, SpawnInfo);
		itemBase->ItemOwner = character; //we assign current character as owner to item, it might be better to use GetOwner()! as item is either eequiped on character or not.
		itemBase->InitializeItem();

		//note this currently assume that stats NEVER stack
		//actualy negative with positive stats could stack. If there +9 con and -3 con, the end result stat will be +6 con;
		//note add UnEquipItem method to handle stat subtraction!
		if(itemBase->ItemType == EItemType::Item_Chest)
		{
			EquipChestItem(itemBase, character);
		}
		if(itemBase->ItemType == EItemType::Item_Foot)
		{
			EquipFootItem(itemBase, character);
		}
	}
}

void URPGEquipmentManagerComponent::UnEquipItem(ARPGItem* item, ARPGCharacter* character)
{
	if(item)
	{
		if(item->Attributes.Constitution != 0)
		{
			character->BaseAttributes.Constitution = character->BaseAttributes.Constitution - item->Attributes.Constitution;
			if(EquipedItems.Num() > 0)
			{
				for(ARPGItem* itemInArray : EquipedItems)
				{

				}
			}
		}
		EquipedItems.RemoveSingle(item);
	}
}

void URPGEquipmentManagerComponent::EquipWeapon(TSubclassOf<class ARPGWeaponBase> weapon, ARPGCharacter* characterToAttach, FName SocketName)
{
	if(weapon)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bNoCollisionFail = true;
		ARPGWeaponBase* weaponBase = GetWorld()->SpawnActor<ARPGWeaponBase>(weapon, SpawnInfo);
		MainWeapon = weaponBase;
		USkeletalMeshComponent* PawnMesh = characterToAttach->Mesh;
		USkeletalMeshComponent* weaponMesh = MainWeapon->WeaponMesh;
		//		USkeletalMesh* weaponMesh = MainWeapon->WeaponMesh;
		//characterToAttach->SomeMesh->SetSkeletalMesh(weaponMesh);
		weaponMesh->AttachTo(PawnMesh, SocketName);
		weaponMesh->SetHiddenInGame(false);
	}
}

