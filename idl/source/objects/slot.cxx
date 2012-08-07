/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <ctype.h>
#include <stdio.h>
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <slot.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaSlot, SvMetaAttribute );

SvMetaObject *SvMetaSlot::MakeClone() const
{
        return new SvMetaSlot( *this );
}

SvMetaSlot::SvMetaSlot()
    : aCachable( sal_True, sal_False )
    , aSynchron( sal_True, sal_False )
    , aRecordPerSet( sal_True, sal_False )
    , aRecordAbsolute( sal_False, sal_False )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , pEnumValue(0)
{
}

SvMetaSlot::SvMetaSlot( SvMetaType * pType )
    : SvMetaAttribute( pType )
    , aCachable( sal_True, sal_False )
    , aSynchron( sal_True, sal_False )
    , aRecordPerSet( sal_True, sal_False )
    , aRecordAbsolute( sal_False, sal_False )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , pEnumValue(0)
{
}

#define TEST_READ
#define TEST_WRITE

void SvMetaSlot::Load( SvPersistStream & rStm )
{
    SvMetaAttribute::Load( rStm );

    sal_uInt16 nMask;
    rStm >> nMask;

    TEST_READ
    if( nMask & 0x0001 )
    {
        SvMetaAttribute * pMeth;
        rStm >> pMeth;
        aMethod = pMeth;
    }

    TEST_READ
    if( nMask & 0x0002 ) rStm >> aGroupId;
    TEST_READ
    if( nMask & 0x0004 ) rStm >> aHasCoreId;
    TEST_READ
    if( nMask & 0x0008 ) rStm >> aConfigId;
    TEST_READ
    if( nMask & 0x0010 ) rStm >> aExecMethod;
    TEST_READ
    if( nMask & 0x0020 ) rStm >> aStateMethod;
    TEST_READ
    if( nMask & 0x0040 ) rStm >> aDefault;
    TEST_READ
    if( nMask & 0x0080 ) rStm >> aPseudoSlots;
    TEST_READ
    if( nMask & 0x0100 ) rStm >> aGet;
    TEST_READ
    if( nMask & 0x0200 ) rStm >> aSet;
    TEST_READ
    if( nMask & 0x0400 ) rStm >> aCachable;
    TEST_READ
    if( nMask & 0x0800 ) rStm >> aVolatile;
    TEST_READ
    if( nMask & 0x1000 ) rStm >> aToggle;
    TEST_READ
    if( nMask & 0x2000 ) rStm >> aAutoUpdate;
    TEST_READ
    if( nMask & 0x4000 ) rStm >> aSynchron;
    TEST_READ
    if( nMask & 0x8000 ) rStm >> aAsynchron;

    nMask = 0;
    rStm >> nMask;

    TEST_READ
    if( nMask & 0x0001 ) rStm >> aRecordPerItem;
    TEST_READ
    if( nMask & 0x0002 ) rStm >> aRecordManual;
    TEST_READ
    if( nMask & 0x0004 ) rStm >> aNoRecord;
    TEST_READ
    if( nMask & 0x0008 ) rStm >> aHasDialog;
    TEST_READ
    if( nMask & 0x0010 ) rStm >> aDisableFlags;
    TEST_READ
    if( nMask & 0x0020 ) rStm >> aPseudoPrefix;
    TEST_READ
    if( nMask & 0x0040 ) rStm >> aRecordPerSet;
    TEST_READ
    if( nMask & 0x0080 ) rStm >> aMenuConfig;
    TEST_READ
    if( nMask & 0x0100 ) rStm >> aToolBoxConfig;
    TEST_READ
    if( nMask & 0x0200 ) rStm >> aStatusBarConfig;
    TEST_READ
    if( nMask & 0x0400 ) rStm >> aAccelConfig;
    TEST_READ
    if( nMask & 0x0800 ) rStm >> aFastCall;
    TEST_READ
    if( nMask & 0x1000 ) rStm >> aContainer;
    TEST_READ

    if( nMask & 0x2000 )
    {
        SvMetaType * pT;
        rStm >> pT;
        aSlotType = pT;
    }

    TEST_READ
    if( nMask & 0x4000 ) rStm >> aRecordAbsolute;
    TEST_READ
    if( nMask & 0x8000 ) rStm >> aImageRotation;

    nMask = 0;
    rStm >> nMask;

    TEST_READ
    if( nMask & 0x0001 ) rStm >> aUnoName;
    if( nMask & 0x0002 ) rStm >> aImageReflection;
}

void SvMetaSlot::Save( SvPersistStream & rStm )
{
    SvMetaAttribute::Save( rStm );

    // create mask
    sal_uInt16 nMask = 0;
    if( aMethod.Is() )          nMask |= 0x0001;
    if( !aGroupId.getString().isEmpty() )       nMask |= 0x0002;
    if( aHasCoreId.IsSet() )    nMask |= 0x0004;
    if( !aConfigId.getString().isEmpty() )      nMask |= 0x0008;
    if( !aExecMethod.getString().isEmpty() )    nMask |= 0x0010;
    if( !aStateMethod.getString().isEmpty() )   nMask |= 0x0020;
    if( !aDefault.getString().isEmpty() )       nMask |= 0x0040;
    if( aPseudoSlots.IsSet() )  nMask |= 0x0080;
    if( aGet.IsSet() )          nMask |= 0x0100;
    if( aSet.IsSet() )          nMask |= 0x0200;
    if( aCachable.IsSet() )     nMask |= 0x0400;
    if( aVolatile.IsSet() )     nMask |= 0x0800;
    if( aToggle.IsSet() )       nMask |= 0x1000;
    if( aAutoUpdate.IsSet() )   nMask |= 0x2000;
    if( aSynchron.IsSet() )     nMask |= 0x4000;
    if( aAsynchron.IsSet() )    nMask |= 0x8000;

    // write data
    rStm << nMask;
    TEST_WRITE
    if( nMask & 0x0001 ) rStm << aMethod;
    TEST_WRITE
    if( nMask & 0x0002 ) rStm << aGroupId;
    TEST_WRITE
    if( nMask & 0x0004 ) rStm << aHasCoreId;
    TEST_WRITE
    if( nMask & 0x0008 ) rStm << aConfigId;
    TEST_WRITE
    if( nMask & 0x0010 ) rStm << aExecMethod;
    TEST_WRITE
    if( nMask & 0x0020 ) rStm << aStateMethod;
    TEST_WRITE
    if( nMask & 0x0040 ) rStm << aDefault;
    TEST_WRITE
    if( nMask & 0x0080 ) rStm << aPseudoSlots;
    TEST_WRITE
    if( nMask & 0x0100 ) rStm << aGet;
    TEST_WRITE
    if( nMask & 0x0200 ) rStm << aSet;
    TEST_WRITE
    if( nMask & 0x0400 ) rStm << aCachable;
    TEST_WRITE
    if( nMask & 0x0800 ) rStm << aVolatile;
    TEST_WRITE
    if( nMask & 0x1000 ) rStm << aToggle;
    TEST_WRITE
    if( nMask & 0x2000 ) rStm << aAutoUpdate;
    TEST_WRITE
    if( nMask & 0x4000 ) rStm << aSynchron;
    TEST_WRITE
    if( nMask & 0x8000 ) rStm << aAsynchron;

    // write next bunch
    // create mask
    nMask = 0;
    if( aRecordPerItem.IsSet() )  nMask |= 0x0001;
    if( aRecordManual.IsSet() )   nMask |= 0x0002;
    if( aNoRecord.IsSet() )       nMask |= 0x0004;
    if( aHasDialog.IsSet() )      nMask |= 0x0008;
    if ( aDisableFlags.IsSet() )      nMask |= 0x0010;
    if( !aPseudoPrefix.getString().isEmpty() )    nMask |= 0x0020;
    if( aRecordPerSet.IsSet() )   nMask |= 0x0040;
    if( aMenuConfig.IsSet() )     nMask |= 0x0080;
    if( aToolBoxConfig.IsSet() )  nMask |= 0x0100;
    if( aStatusBarConfig.IsSet() )nMask |= 0x0200;
    if( aAccelConfig.IsSet() )    nMask |= 0x0400;
    if( aFastCall.IsSet() )       nMask |= 0x0800;
    if( aContainer.IsSet() )      nMask |= 0x1000;
    if( aSlotType.Is() )          nMask |= 0x2000;
    if( aRecordAbsolute.IsSet() ) nMask |= 0x4000;
    if( aImageRotation.IsSet() )       nMask |= 0x8000;

    // write data
    rStm << nMask;
    TEST_WRITE
    if( nMask & 0x0001 ) rStm << aRecordPerItem;
    TEST_WRITE
    if( nMask & 0x0002 ) rStm << aRecordManual;
    TEST_WRITE
    if( nMask & 0x0004 ) rStm << aNoRecord;
    TEST_WRITE
    if( nMask & 0x0008 ) rStm << aHasDialog;
    TEST_WRITE
    if( nMask & 0x0010 ) rStm << aDisableFlags;
    TEST_WRITE
    if( nMask & 0x0020 ) rStm << aPseudoPrefix;
    TEST_WRITE
    if( nMask & 0x0040 ) rStm << aRecordPerSet;
    TEST_WRITE
    if( nMask & 0x0080 ) rStm << aMenuConfig;
    TEST_WRITE
    if( nMask & 0x0100 ) rStm << aToolBoxConfig;
    TEST_WRITE
    if( nMask & 0x0200 ) rStm << aStatusBarConfig;
    TEST_WRITE
    if( nMask & 0x0400 ) rStm << aAccelConfig;
    TEST_WRITE
    if( nMask & 0x0800 ) rStm << aFastCall;
    TEST_WRITE
    if( nMask & 0x1000 ) rStm << aContainer;
    TEST_WRITE
    if( nMask & 0x2000 ) rStm << aSlotType;
    TEST_WRITE
    if( nMask & 0x4000 ) rStm << aRecordAbsolute;
    TEST_WRITE
    if( nMask & 0x8000 ) rStm << aImageRotation;

    nMask = 0;
    if( aUnoName.IsSet() )          nMask |= 0x0001;
    if( aImageReflection.IsSet() )  nMask |= 0x0002;
    rStm << nMask;
    TEST_WRITE
    if( nMask & 0x0001 ) rStm << aUnoName;
    TEST_WRITE
    if( nMask & 0x0002 ) rStm << aImageReflection;
}

sal_Bool SvMetaSlot::IsVariable() const
{
    return SvMetaAttribute::IsVariable();
}

sal_Bool SvMetaSlot::IsMethod() const
{
    sal_Bool b = SvMetaAttribute::IsMethod();
    b |= NULL != GetMethod();
    return b;
}

rtl::OString SvMetaSlot::GetMangleName( sal_Bool bVariable ) const
{
    if( !bVariable )
    {
        SvMetaAttribute * pMeth = GetMethod();
        if( pMeth )
            return pMeth->GetName().getString();
    }
    return GetName().getString();
}

/*************************************************************************
|*    reference
|*
|*    description       Second FALSE in the SvBOOL-Objects means
|*                      IsSet() provides FALSE (default initialization).
*************************************************************************/
/** reference disbandment **/
SvMetaType * SvMetaSlot::GetSlotType() const
{
    if( aSlotType.Is() || !GetRef() ) return aSlotType;
    return ((SvMetaSlot *)GetRef())->GetSlotType();
}
SvMetaAttribute * SvMetaSlot::GetMethod() const
{
    if( aMethod.Is() || !GetRef() ) return aMethod;
    return ((SvMetaSlot *)GetRef())->GetMethod();
}
sal_Bool SvMetaSlot::GetHasCoreId() const
{
    if( aHasCoreId.IsSet() || !GetRef() ) return aHasCoreId;
    return ((SvMetaSlot *)GetRef())->GetHasCoreId();
}
const rtl::OString& SvMetaSlot::GetGroupId() const
{
    if( !aGroupId.getString().isEmpty() || !GetRef() ) return aGroupId.getString();
    return ((SvMetaSlot *)GetRef())->GetGroupId();
}
const rtl::OString& SvMetaSlot::GetDisableFlags() const
{
    if( !aDisableFlags.getString().isEmpty() || !GetRef() ) return aDisableFlags.getString();
    return ((SvMetaSlot *)GetRef())->GetDisableFlags();
}
const rtl::OString& SvMetaSlot::GetConfigId() const
{
    if( !aConfigId.getString().isEmpty() || !GetRef() ) return aConfigId.getString();
    return ((SvMetaSlot *)GetRef())->GetConfigId();
}
const rtl::OString& SvMetaSlot::GetExecMethod() const
{
    if( !aExecMethod.getString().isEmpty() || !GetRef() ) return aExecMethod.getString();
    return ((SvMetaSlot *)GetRef())->GetExecMethod();
}
const rtl::OString& SvMetaSlot::GetStateMethod() const
{
    if( !aStateMethod.getString().isEmpty() || !GetRef() ) return aStateMethod.getString();
    return ((SvMetaSlot *)GetRef())->GetStateMethod();
}
const rtl::OString& SvMetaSlot::GetDefault() const
{
    if( !aDefault.getString().isEmpty() || !GetRef() ) return aDefault.getString();
    return ((SvMetaSlot *)GetRef())->GetDefault();
}
sal_Bool SvMetaSlot::GetPseudoSlots() const
{
    if( aPseudoSlots.IsSet() || !GetRef() ) return aPseudoSlots;
    return ((SvMetaSlot *)GetRef())->GetPseudoSlots();
}
sal_Bool SvMetaSlot::GetCachable() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aCachable.IsSet() || aVolatile.IsSet() )
         return aCachable;
    return ((SvMetaSlot *)GetRef())->GetCachable();
}
sal_Bool SvMetaSlot::GetVolatile() const
{
    // Cachable and Volatile are exclusive
    if( !GetRef() || aVolatile.IsSet() || aCachable.IsSet() )
        return aVolatile;
    return ((SvMetaSlot *)GetRef())->GetVolatile();
}
sal_Bool SvMetaSlot::GetToggle() const
{
    if( aToggle.IsSet() || !GetRef() ) return aToggle;
    return ((SvMetaSlot *)GetRef())->GetToggle();
}
sal_Bool SvMetaSlot::GetAutoUpdate() const
{
    if( aAutoUpdate.IsSet() || !GetRef() ) return aAutoUpdate;
    return ((SvMetaSlot *)GetRef())->GetAutoUpdate();
}
sal_Bool SvMetaSlot::GetSynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aSynchron.IsSet() || aAsynchron.IsSet() )
        return aSynchron;
    return ((SvMetaSlot *)GetRef())->GetSynchron();
}
sal_Bool SvMetaSlot::GetAsynchron() const
{
    // Synchron and Asynchron are exclusive
    if( !GetRef() || aAsynchron.IsSet() || aSynchron.IsSet() )
        return aAsynchron;
    return ((SvMetaSlot *)GetRef())->GetAsynchron();
}
sal_Bool SvMetaSlot::GetRecordPerItem() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerItem;
    return ((SvMetaSlot *)GetRef())->GetRecordPerItem();
}
sal_Bool SvMetaSlot::GetRecordPerSet() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerSet;
    return ((SvMetaSlot *)GetRef())->GetRecordPerSet();
}
sal_Bool SvMetaSlot::GetRecordManual() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordManual;
    return ((SvMetaSlot *)GetRef())->GetRecordManual();
}
sal_Bool SvMetaSlot::GetNoRecord() const
{
    // Record- PerItem, No, PerSet and Manual are exclusive
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aNoRecord;
    return ((SvMetaSlot *)GetRef())->GetNoRecord();
}
sal_Bool SvMetaSlot::GetRecordAbsolute() const
{
    if( !GetRef() || aRecordAbsolute.IsSet() )
        return aRecordAbsolute;
    return ((SvMetaSlot *)GetRef())->GetRecordAbsolute();
}
sal_Bool SvMetaSlot::GetHasDialog() const
{
    if( aHasDialog.IsSet() || !GetRef() ) return aHasDialog;
    return ((SvMetaSlot *)GetRef())->GetHasDialog();
}
const rtl::OString& SvMetaSlot::GetPseudoPrefix() const
{
    if( !aPseudoPrefix.getString().isEmpty() || !GetRef() ) return aPseudoPrefix.getString();
    return ((SvMetaSlot *)GetRef())->GetPseudoPrefix();
}
sal_Bool SvMetaSlot::GetMenuConfig() const
{
    if( aMenuConfig.IsSet() || !GetRef() ) return aMenuConfig;
    return ((SvMetaSlot *)GetRef())->GetMenuConfig();
}
sal_Bool SvMetaSlot::GetToolBoxConfig() const
{
    if( aToolBoxConfig.IsSet() || !GetRef() ) return aToolBoxConfig;
    return ((SvMetaSlot *)GetRef())->GetToolBoxConfig();
}
sal_Bool SvMetaSlot::GetStatusBarConfig() const
{
    if( aStatusBarConfig.IsSet() || !GetRef() ) return aStatusBarConfig;
    return ((SvMetaSlot *)GetRef())->GetStatusBarConfig();
}
sal_Bool SvMetaSlot::GetAccelConfig() const
{
    if( aAccelConfig.IsSet() || !GetRef() ) return aAccelConfig;
    return ((SvMetaSlot *)GetRef())->GetAccelConfig();
}
sal_Bool SvMetaSlot::GetFastCall() const
{
    if( aFastCall.IsSet() || !GetRef() ) return aFastCall;
    return ((SvMetaSlot *)GetRef())->GetFastCall();
}
sal_Bool SvMetaSlot::GetContainer() const
{
    if( aContainer.IsSet() || !GetRef() ) return aContainer;
    return ((SvMetaSlot *)GetRef())->GetContainer();
}

sal_Bool SvMetaSlot::GetImageRotation() const
{
    if( aImageRotation.IsSet() || !GetRef() ) return aImageRotation;
    return ((SvMetaSlot *)GetRef())->GetImageRotation();
}

sal_Bool SvMetaSlot::GetImageReflection() const
{
    if( aImageReflection.IsSet() || !GetRef() ) return aImageReflection;
    return ((SvMetaSlot *)GetRef())->GetImageReflection();
}

const rtl::OString& SvMetaSlot::GetUnoName() const
{
    if( aUnoName.IsSet() || !GetRef() ) return aUnoName.getString();
    return ((SvMetaSlot *)GetRef())->GetUnoName();
}

void SvMetaSlot::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    SvMetaAttribute::ReadAttributesSvIdl( rBase, rInStm );

    sal_Bool bOk = sal_False;
    bOk |= aDefault.ReadSvIdl( SvHash_Default(), rInStm );
    bOk |= aPseudoSlots.ReadSvIdl( SvHash_PseudoSlots(), rInStm );
    bOk |= aHasCoreId.ReadSvIdl( SvHash_HasCoreId(), rInStm );
    bOk |= aGroupId.ReadSvIdl( SvHash_GroupId(), rInStm );
    bOk |= aExecMethod.ReadSvIdl( SvHash_ExecMethod(), rInStm );
    bOk |= aStateMethod.ReadSvIdl( SvHash_StateMethod(), rInStm );
    bOk |= aDisableFlags.ReadSvIdl( SvHash_DisableFlags(), rInStm );
    if( aGet.ReadSvIdl( SvHash_Get(), rInStm ) )
    {
        rBase.WriteError( "warning", rtl::OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Get> old style, use Readonly",
                    rInStm.GetToken()->GetLine(),
                    rInStm.GetToken()->GetColumn() );
    }
    if( aSet.ReadSvIdl( SvHash_Set(), rInStm ) )
    {
        rBase.WriteError( "warning", rtl::OUStringToOString(rInStm.GetFileName(), RTL_TEXTENCODING_UTF8),
                    "<Set> old style, use method declaration",
                    rInStm.GetToken()->GetLine(),
                    rInStm.GetToken()->GetColumn() );
    }

    if( aCachable.ReadSvIdl( SvHash_Cachable(), rInStm ) )
        SetCachable( aCachable ), bOk = sal_True;
    if( aVolatile.ReadSvIdl( SvHash_Volatile(), rInStm ) )
        SetVolatile( aVolatile ), bOk = sal_True;
    if( aToggle.ReadSvIdl( SvHash_Toggle(), rInStm ) )
        SetToggle( aToggle ), bOk = sal_True;
    if( aAutoUpdate.ReadSvIdl( SvHash_AutoUpdate(), rInStm ) )
        SetAutoUpdate( aAutoUpdate ), bOk = sal_True;

    if( aSynchron.ReadSvIdl( SvHash_Synchron(), rInStm ) )
        SetSynchron( aSynchron ), bOk = sal_True;
    if( aAsynchron.ReadSvIdl( SvHash_Asynchron(), rInStm ) )
        SetAsynchron( aAsynchron ), bOk = sal_True;

    if( aRecordAbsolute.ReadSvIdl( SvHash_RecordAbsolute(), rInStm ) )
        SetRecordAbsolute( aRecordAbsolute), bOk = sal_True;
    if( aRecordPerItem.ReadSvIdl( SvHash_RecordPerItem(), rInStm ) )
        SetRecordPerItem( aRecordPerItem ), bOk = sal_True;
    if( aRecordPerSet.ReadSvIdl( SvHash_RecordPerSet(), rInStm ) )
        SetRecordPerSet( aRecordPerSet ), bOk = sal_True;
    if( aRecordManual.ReadSvIdl( SvHash_RecordManual(), rInStm ) )
        SetRecordManual( aRecordManual ), bOk = sal_True;
    if( aNoRecord.ReadSvIdl( SvHash_NoRecord(), rInStm ) )
        SetNoRecord( aNoRecord ), bOk = sal_True;

    bOk |= aHasDialog.ReadSvIdl( SvHash_HasDialog(), rInStm );
    bOk |= aPseudoPrefix.ReadSvIdl( SvHash_PseudoPrefix(), rInStm );
    bOk |= aMenuConfig.ReadSvIdl( SvHash_MenuConfig(), rInStm );
    bOk |= aToolBoxConfig.ReadSvIdl( SvHash_ToolBoxConfig(), rInStm );
    bOk |= aStatusBarConfig.ReadSvIdl( SvHash_StatusBarConfig(), rInStm );
    bOk |= aAccelConfig.ReadSvIdl( SvHash_AccelConfig(), rInStm );

    SvBOOL aAllConfig;
    if( aAllConfig.ReadSvIdl( SvHash_AllConfig(), rInStm ) )
        SetAllConfig( aAllConfig ), bOk = sal_True;
    bOk |= aFastCall.ReadSvIdl( SvHash_FastCall(), rInStm );
    bOk |= aContainer.ReadSvIdl( SvHash_Container(), rInStm );
    bOk |= aImageRotation.ReadSvIdl( SvHash_ImageRotation(), rInStm );
    bOk |= aImageReflection.ReadSvIdl( SvHash_ImageReflection(), rInStm );
    bOk |= aUnoName.ReadSvIdl( SvHash_UnoName(), rInStm );

    if( !bOk )
    {
        if( !aSlotType.Is() )
        {
            sal_uInt32 nTokPos = rInStm.Tell();
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->Is( SvHash_SlotType() ) )
            {
                sal_Bool bBraket = rInStm.Read( '(' );
                if( bBraket || rInStm.Read( '=' ) )
                {
                    aSlotType = rBase.ReadKnownType( rInStm );
                    if( aSlotType.Is() )
                    {
                        if( aSlotType->IsItem() )
                        {
                            if( bBraket )
                            {
                                if( rInStm.Read( ')' ) )
                                    return;
                            }
                            else
                                return;
                        }
                        rBase.SetError( "the SlotType is not a item", rInStm.GetToken() );
                        rBase.WriteError( rInStm );
                    }
                    rBase.SetError( "SlotType with unknown item type", rInStm.GetToken() );
                    rBase.WriteError( rInStm );
                }
            }
            rInStm.Seek( nTokPos );

        }
        if( !aMethod.Is() )
        {
            SvToken * pTok = rInStm.GetToken();
            if( pTok->IsIdentifier() )
            {
                aMethod = new SvMetaSlot();
                sal_uInt32 nTokPos = rInStm.Tell();
                if( aMethod->ReadSvIdl( rBase, rInStm ) )
                {
                    if( aMethod->IsMethod() )
                    {
                        aMethod->SetSlotId( GetSlotId() );
                        if( aMethod->Test( rBase, rInStm ) )
                            return;
                    }
                    rInStm.Seek( nTokPos );
                }
                aMethod.Clear();
            }
        }
    }
}

void SvMetaSlot::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                       SvStream & rOutStm,
                       sal_uInt16 nTab )
{
    SvMetaAttribute::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( aSlotType.Is() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << SvHash_SlotType()->GetName().getStr() << '(';
        aSlotType->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
        rOutStm << ");" << endl;
    }
    if( aMethod.Is() )
    {
        WriteTab( rOutStm, nTab );
        aMethod->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    if( aHasCoreId )
    {
        aHasCoreId.WriteSvIdl( SvHash_HasCoreId(), rOutStm );
        rOutStm << ';' << endl;
    }
    if( !aGroupId.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aGroupId.WriteSvIdl( SvHash_GroupId(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }
    if( !aExecMethod.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aExecMethod.WriteSvIdl( SvHash_ExecMethod(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }
    if( !aStateMethod.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aStateMethod.WriteSvIdl( SvHash_StateMethod(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }

    if( !aDisableFlags.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        aDisableFlags.WriteSvIdl( SvHash_DisableFlags(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }

    if( aSet || aGet || aPseudoSlots )
    {
        WriteTab( rOutStm, nTab );
        char const * p = "";
        if( aPseudoSlots )
        {
            aPseudoSlots.WriteSvIdl( SvHash_PseudoSlots(), rOutStm );
            p = ", ";
        }
        if( aGet )
        {
            rOutStm << p;
            aGet.WriteSvIdl( SvHash_Get(), rOutStm );
            p = ", ";
        }
        if( aSet )
        {
            rOutStm << p;
            aSet.WriteSvIdl( SvHash_Set(), rOutStm );
        }
        rOutStm << ';' << endl;
    }

    rtl::OString aDel(", ");
    rtl::OStringBuffer aOut;
    if( aVolatile )
        aOut.append(aVolatile.GetSvIdlString( SvHash_Volatile() ));
    else if( !aCachable )
        // because of Default == TRUE, only when no other is set
        aOut.append(aCachable.GetSvIdlString( SvHash_Cachable() ));
    else
        aDel = rtl::OString();

    if( aToggle )
    {
        aOut.append(aDel).append(aToggle.GetSvIdlString( SvHash_Toggle() ));
        aDel = ", ";
    }
    if( aAutoUpdate )
    {
        aOut.append(aDel).append(aAutoUpdate.GetSvIdlString( SvHash_AutoUpdate() ));
        aDel = ", ";
    }

    rtl::OString aDel1(", ");
    if( aAsynchron )
        aOut.append(aDel).append(aAsynchron.GetSvIdlString( SvHash_Asynchron() ));
    else if( !aSynchron )
    {
        // because of Default == TRUE, only when no other is set
        aOut.append(aDel).append(aSynchron.GetSvIdlString( SvHash_Synchron() ));
    }
    else
        aDel1 = aDel;

    aDel = ", ";
    if( aRecordManual )
        aOut.append(aDel1).append(aRecordManual.GetSvIdlString( SvHash_RecordManual() ));
    else if( aNoRecord )
        aOut.append(aDel1).append(aNoRecord.GetSvIdlString( SvHash_NoRecord() ));
    else if( !aRecordPerSet )
        // because of Default == TRUE, only when no other is set
        aOut.append(aDel1).append(aRecordPerSet.GetSvIdlString( SvHash_RecordPerSet() ));
    else if( aRecordPerItem )
        aOut.append(aDel1).append(aRecordPerItem.GetSvIdlString( SvHash_RecordPerItem() ));
    else
        aDel = aDel1;

    if( aRecordAbsolute )
    {
        aOut.append(aDel).append(aRecordAbsolute.GetSvIdlString( SvHash_RecordAbsolute() ));
        aDel = ", ";
    }
    if( aHasDialog )
    {
        aOut.append(aDel).append(aHasDialog.GetSvIdlString( SvHash_HasDialog() ));
        aDel = ", ";
    }
    if( aMenuConfig )
    {
        aOut.append(aDel).append(aMenuConfig.GetSvIdlString( SvHash_MenuConfig() ));
        aDel = ", ";
    }
    if( aToolBoxConfig )
    {
        aOut.append(aDel).append(aToolBoxConfig.GetSvIdlString( SvHash_ToolBoxConfig() ));
        aDel = ", ";
    }
    if( aStatusBarConfig )
    {
        aOut.append(aDel).append(aStatusBarConfig.GetSvIdlString( SvHash_StatusBarConfig() ));
        aDel = ", ";
    }
    if( aAccelConfig )
    {
        aOut.append(aDel).append(aAccelConfig.GetSvIdlString( SvHash_AccelConfig() ));
        aDel = ", ";
    }
    if( aFastCall )
    {
        aOut.append(aDel).append(aFastCall.GetSvIdlString( SvHash_FastCall() ));
        aDel = ", ";
    }
    if( aContainer )
    {
        aOut.append(aDel).append(aContainer.GetSvIdlString( SvHash_Container() ));
        aDel = ", ";
    }
    if( aImageRotation )
    {
        aOut.append(aDel).append(aImageRotation.GetSvIdlString( SvHash_ImageRotation() ));
        aDel = ", ";
    }

    if( aImageReflection )
    {
        aOut.append(aDel).append(aImageReflection.GetSvIdlString( SvHash_ImageReflection() ));
        aDel = ", ";
    }

    if( aOut.getLength() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << aOut.getStr() << endl;
    }
}


sal_Bool SvMetaSlot::Test( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_Bool bOk = SvMetaAttribute::Test( rBase, rInStm );
    if( bOk )
    {
        SvMetaType * pType = GetType();
        if( pType->GetType() == TYPE_METHOD )
            pType = pType->GetReturnType();
        if( !pType->IsItem() )
        {
            rBase.SetError( "this attribute is not a slot", rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = sal_False;
        }
    }

    return bOk;
}

sal_Bool SvMetaSlot::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    sal_Bool bOk = sal_True;

    SvMetaAttribute * pAttr = rBase.ReadKnownAttr( rInStm, GetType() );
    if( pAttr )
    {
        // c
        SvMetaSlot * pKnownSlot = PTR_CAST( SvMetaSlot, pAttr );
        if( pKnownSlot )
        {
            SetRef( pKnownSlot );
            SetName( pKnownSlot->GetName().getString(), &rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        else
        {
            rtl::OStringBuffer aStr( "attribute " );
            aStr.append(pAttr->GetName().getString());
            aStr.append(" is method or variable but not a slot");
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = sal_False;
        }
    }
    else
    {
        bOk = SvMetaAttribute::ReadSvIdl( rBase, rInStm );

        SvMetaAttribute *pAttr2 = rBase.SearchKnownAttr( GetSlotId() );
        if( pAttr2 )
        {
            // for testing purposes: reference in case of complete definition
            SvMetaSlot * pKnownSlot = PTR_CAST( SvMetaSlot, pAttr2 );
            if( pKnownSlot )
            {
                SetRef( pKnownSlot );

                  // names may differ, because explicitly given
                if ( pKnownSlot->GetName().getString() != GetName().getString() )
                {
                    OSL_FAIL("Illegal definition!");
                    rInStm.Seek( nTokPos );
                    return sal_False;
                }

                  SetName( pKnownSlot->GetName().getString(), &rBase );
            }
            else
            {
                rtl::OStringBuffer aStr("attribute ");
                aStr.append(pAttr2->GetName().getString());
                aStr.append(" is method or variable but not a slot");
                rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
                rBase.WriteError( rInStm );
                bOk = sal_False;
            }
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
}

void SvMetaSlot::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab )
{
    SvMetaAttribute::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaSlot::Write( SvIdlDataBase & rBase,
                        SvStream & rOutStm, sal_uInt16 nTab,
                        WriteType nT, WriteAttribute nA )
{
    if ( nT == WRITE_DOCU )
    {
        if ( GetHidden() )
            return;
    }
    else
    {
        // no attribute for Automation
        if( !GetAutomation() || !GetExport() )
            return;
    }

    if( !(nA & WA_VARIABLE) )
    {
        SvMetaAttributeRef xM = GetMethod();
        if( xM.Is() )
        {
            xM->SetSlotId( GetSlotId() );
            xM->SetDescription( GetDescription().getString() );
            xM->Write( rBase, rOutStm, nTab, nT, nA );
            return;
        }
    }

    SvMetaAttribute::Write( rBase, rOutStm, nTab, nT, nA );
}


void SvMetaSlot::Insert( SvSlotElementList& rList, const rtl::OString& rPrefix,
                        SvIdlDataBase& rBase)
{
    // get insert position through binary search in slotlist
    sal_uInt16 nId = (sal_uInt16) GetSlotId().GetValue();
    sal_uInt16 nListCount = (sal_uInt16) rList.size();
    sal_uInt16 nPos;
    sal_uLong m;  // for inner "for" loop

    if ( !nListCount )
        nPos = 0;
    else if ( nListCount == 1 )
        nPos = rList[ 0 ]->xSlot->GetSlotId().GetValue() >= nId ? 0 : 1;
    else
    {
        sal_uInt16 nMid = 0, nLow = 0;
        sal_uInt16 nHigh = nListCount - 1;
        sal_Bool bFound = sal_False;
        while ( !bFound && nLow <= nHigh )
        {
            nMid = (nLow + nHigh) >> 1;
            DBG_ASSERT( nMid < nListCount, "bsearch ist buggy" );
            int nDiff = (int) nId - (int) rList[ nMid ]->xSlot->GetSlotId().GetValue();
            if ( nDiff < 0)
            {
                if ( nMid == 0 )
                    break;
                nHigh = nMid - 1;
            }
            else if ( nDiff > 0 )
            {
                nLow = nMid + 1;
                if ( nLow == 0 )
                break;
            }
            else
                bFound = sal_True;
        }

        DBG_ASSERT(!bFound, "Duplicate SlotId!");
        nPos = bFound ? nMid : nLow;
    }

    DBG_ASSERT( nPos <= nListCount,
        "nPos too large" );
    DBG_ASSERT( nPos == nListCount || nId <=
        (sal_uInt16) rList[ nPos ]->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );
    DBG_ASSERT( nPos == 0 || nId >
        (sal_uInt16) rList[ nPos-1 ]->xSlot->GetSlotId().GetValue(),
        "Predecessor has higher SlotId" );
    DBG_ASSERT( nPos+1 >= nListCount || nId <
        (sal_uInt16) rList[ nPos+1 ]->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );

    if ( nPos < rList.size() )
    {
        SvSlotElementList::iterator it = rList.begin();
        std::advance( it, nPos );
        rList.insert( it, new SvSlotElement( this, rPrefix ) );
    }
    else
    {
        rList.push_back( new SvSlotElement( this, rPrefix ) );
    }

    // iron out EnumSlots
    SvMetaTypeEnum * pEnum = NULL;
    SvMetaType * pBType = GetType()->GetBaseType();
    pEnum = PTR_CAST( SvMetaTypeEnum, pBType );
    if( GetPseudoSlots() && pEnum && pEnum->Count() )
    {
        // clone the MasterSlot
        SvMetaSlotRef xEnumSlot;
        SvMetaSlot *pFirstEnumSlot = NULL;
        for( sal_uLong n = 0; n < pEnum->Count(); n++ )
        {
            // create SlotId
            SvMetaEnumValue *enumValue = pEnum->GetObject(n);
            rtl::OString aValName = enumValue->GetName().getString();
            rtl::OStringBuffer aBuf;
            if( !GetPseudoPrefix().isEmpty() )
                aBuf.append(GetPseudoPrefix());
            else
                aBuf.append(GetSlotId().getString());
            aBuf.append('_');
            aBuf.append(aValName.copy(pEnum->GetPrefix().getLength()));

            rtl::OString aSId = aBuf.makeStringAndClear();

            xEnumSlot = NULL;
            for( m=0; m<rBase.GetAttrList().size(); m++ )
            {
                SvMetaAttribute * pAttr = rBase.GetAttrList()[m];
                if (aSId.equals(pAttr->GetSlotId().getString()))
                {
                    SvMetaSlot* pSlot = PTR_CAST( SvMetaSlot, pAttr );
                    xEnumSlot = pSlot->Clone();
                    break;
                }
            }

            if ( m == rBase.GetAttrList().size() )
            {
                OSL_FAIL("Invalid EnumSlot!");
                xEnumSlot = Clone();
                sal_uLong nValue;
                if ( rBase.FindId(aSId , &nValue) )
                {
                    SvNumberIdentifier aId;
                    aId.setString(aSId);
                    aId.SetValue(nValue);
                    xEnumSlot->SetSlotId(aId);
                }
            }

            // The slaves are no master!
            xEnumSlot->aPseudoSlots = sal_False;
            xEnumSlot->SetEnumValue(enumValue);

            if ( !pFirstEnumSlot || xEnumSlot->GetSlotId().GetValue() < pFirstEnumSlot->GetSlotId().GetValue() )
                pFirstEnumSlot = xEnumSlot;

            // insert the created slave as well
            xEnumSlot->Insert( rList, rPrefix, rBase);

            // concatenate the EnumSlots with the master
            xEnumSlot->pLinkedSlot = this;
        }

        // master points to the first slave
        pLinkedSlot = pFirstEnumSlot;

        // concatenate slaves among themselves
        xEnumSlot = pFirstEnumSlot;
        size_t i = 0;
        SvSlotElement *pEle;
        do
        {
            pEle = ( ++i < rList.size() ) ? rList[ i ] : NULL;
            if ( pEle && pEle->xSlot->pLinkedSlot == this )
            {
                xEnumSlot->pNextSlot = pEle->xSlot;
                xEnumSlot = pEle->xSlot;
            }
        }
        while ( pEle );
        xEnumSlot->pNextSlot = pFirstEnumSlot;
    }
}


static rtl::OString MakeSlotName( SvStringHashEntry * pEntry )
{
    rtl::OStringBuffer aName(RTL_CONSTASCII_STRINGPARAM("SFX_SLOT_"));
    aName.append(pEntry->GetName());
    return aName.makeStringAndClear().toAsciiUpperCase();
};

void SvMetaSlot::WriteSlotStubs( const rtl::OString& rShellName,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    rtl::OString aMethodName( GetExecMethod() );
    if ( !aMethodName.isEmpty() &&
         !aMethodName.equalsL(RTL_CONSTASCII_STRINGPARAM("NoExec")) )
    {
        sal_Bool bIn = sal_False;
        for( size_t n = 0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new rtl::OString(aMethodName) );
            rOutStm << "SFX_EXEC_STUB("
                    << rShellName.getStr()
                    << ','
                    << aMethodName.getStr()
                    << ')' << endl;
        }
    }

    aMethodName = GetStateMethod();
    if (!aMethodName.isEmpty() &&
        !aMethodName.equalsL(RTL_CONSTASCII_STRINGPARAM("NoState")))
    {
        sal_Bool bIn = sal_False;
        for ( size_t n=0; n < rList.size(); n++ )
        {
            if (rList[n]->equals(aMethodName))
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new rtl::OString(aMethodName) );
            rOutStm << "SFX_STATE_STUB("
                    << rShellName.getStr()
                    << ','
                    << aMethodName.getStr()
                    << ')' << endl;
        }
    }
}

void SvMetaSlot::WriteSlot( const rtl::OString& rShellName, sal_uInt16 nCount,
                            const rtl::OString& rSlotId,
                            SvSlotElementList& rSlotList,
                            size_t nStart,
                            const rtl::OString& rPrefix,
                            SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    sal_Bool bIsEnumSlot = 0 != pEnumValue;

    rOutStm << "// Slot Nr. "
        << rtl::OString::valueOf(static_cast<sal_Int32>(nListPos)).getStr()
        << " : ";
    rtl::OString aSlotIdValue(rtl::OString::valueOf(static_cast<sal_Int32>(
        GetSlotId().GetValue())));
    rOutStm << aSlotIdValue.getStr() << endl;
    WriteTab( rOutStm, 1 );
    if( bIsEnumSlot )
        rOutStm << "SFX_NEW_SLOT_ENUM( ";
    else
        rOutStm << "SFX_NEW_SLOT_ARG( " << rShellName.getStr() << ',' ;

    rOutStm << rSlotId.getStr() << ',';
    const SvHelpContext& rHlpCtx = GetHelpContext();
    if( rHlpCtx.IsSet() )
        rOutStm << rHlpCtx.getString().getStr() << ',';
    else
        rOutStm << rSlotId.getStr() << ',';

    // GroupId
    if( !GetGroupId().isEmpty() )
        rOutStm << GetGroupId().getStr();
    else
        rOutStm << '0';
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 4 );

    if( bIsEnumSlot )
    {
        rOutStm << "&a" << rShellName.getStr() << "Slots_Impl["
            << rtl::OString::valueOf(static_cast<sal_Int32>(pLinkedSlot->GetListPos())).getStr()
            << "] /*Offset Master*/, " << endl;
        WriteTab( rOutStm, 4 );
        rOutStm << "&a" << rShellName.getStr() << "Slots_Impl["
            << rtl::OString::valueOf(static_cast<sal_Int32>(pNextSlot->GetListPos())).getStr()
            << "] /*Offset Next*/, " << endl;

        WriteTab( rOutStm, 4 );

        // SlotId
        if( !GetSlotId().getString().isEmpty() )
            rOutStm << pLinkedSlot->GetSlotId().getString().getStr();
        else
            rOutStm << '0';
        rOutStm << ',';
        rOutStm << pEnumValue->GetName().getString().getStr();
    }
    else
    {
        // look for the next slot with the same StateMethod like me
        // the slotlist is set to the current slot
        size_t i = nStart;
        SvSlotElement* pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
        pNextSlot = pEle ? &pEle->xSlot : NULL;
        while ( pNextSlot )
        {
            if ( !pNextSlot->pNextSlot &&
                pNextSlot->GetStateMethod() == GetStateMethod()
            ) {
                break;
            }
            pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
            pNextSlot = pEle ? &pEle->xSlot : NULL;
        }

        if ( !pNextSlot )
        {
            // There is no slot behind me that has the same ExecMethod.
            // So I search for the first slot with it (could be myself).
            i = 0;
            pEle = rSlotList.empty() ? NULL : rSlotList[ i ];
            pNextSlot = pEle ? &pEle->xSlot : NULL;
            while ( pNextSlot != this )
            {
                if ( !pNextSlot->pEnumValue &&
                    pNextSlot->GetStateMethod() == GetStateMethod() )
                    break;
                pEle = ( ++i < rSlotList.size() ) ? rSlotList[ i ] : NULL;
                pNextSlot = pEle ? &pEle->xSlot : NULL;
            }
        }

        if ( !pLinkedSlot )
        {
            rOutStm << "0 ,";
        }
        else
        {
            rOutStm << "&a" << rShellName.getStr() << "Slots_Impl["
                << rtl::OString::valueOf(static_cast<sal_Int32>(pLinkedSlot->GetListPos())).getStr()
                << "] /*Offset Linked*/, " << endl;
            WriteTab( rOutStm, 4 );
        }

        rOutStm << "&a" << rShellName.getStr() << "Slots_Impl["
            << rtl::OString::valueOf(static_cast<sal_Int32>(pNextSlot->GetListPos())).getStr()
            << "] /*Offset Next*/, " << endl;

        WriteTab( rOutStm, 4 );

        // write ExecMethod, with standard name if not specified
        if( !GetExecMethod().isEmpty() &&
            !GetExecMethod().equalsL(RTL_CONSTASCII_STRINGPARAM("NoExec")))
        {
            rOutStm << "SFX_STUB_PTR(" << rShellName.getStr() << ','
                    << GetExecMethod().getStr() << ')';
        }
        else
            rOutStm << "SFX_STUB_PTR_EXEC_NONE";
        rOutStm << ',';

        // write StateMethod, with standard name if not specified
        if( !GetStateMethod().isEmpty() &&
            !GetStateMethod().equalsL(RTL_CONSTASCII_STRINGPARAM("NoState")))
        {
            rOutStm << "SFX_STUB_PTR(" << rShellName.getStr() << ','
                    << GetStateMethod().getStr() << ')';
        }
        else
            rOutStm << "SFX_STUB_PTR_STATE_NONE";
    }
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 4 );

    // write flags
    if( GetHasCoreId() )
        rOutStm << MakeSlotName( SvHash_HasCoreId() ).getStr() << '|';
    if( GetCachable() )
        rOutStm << MakeSlotName( SvHash_Cachable() ).getStr() << '|';
    if( GetVolatile() )
        rOutStm << MakeSlotName( SvHash_Volatile() ).getStr() << '|';
    if( GetToggle() )
        rOutStm << MakeSlotName( SvHash_Toggle() ).getStr() << '|';
    if( GetAutoUpdate() )
        rOutStm << MakeSlotName( SvHash_AutoUpdate() ).getStr() << '|';
    if( GetSynchron() )
        rOutStm << MakeSlotName( SvHash_Synchron() ).getStr() << '|';
    if( GetAsynchron() )
        rOutStm << MakeSlotName( SvHash_Asynchron() ).getStr() << '|';
    if( GetRecordPerItem() )
        rOutStm << MakeSlotName( SvHash_RecordPerItem() ).getStr() << '|';
    if( GetRecordPerSet() )
        rOutStm << MakeSlotName( SvHash_RecordPerSet() ).getStr() << '|';
    if( GetRecordManual() )
        rOutStm << MakeSlotName( SvHash_RecordManual() ).getStr() << '|';
    if( GetNoRecord() )
        rOutStm << MakeSlotName( SvHash_NoRecord() ).getStr() << '|';
    if( GetRecordAbsolute() )
        rOutStm << MakeSlotName( SvHash_RecordAbsolute() ).getStr() << '|';
    if( GetHasDialog() )
        rOutStm << MakeSlotName( SvHash_HasDialog() ).getStr() << '|';
    if( GetMenuConfig() )
        rOutStm << MakeSlotName( SvHash_MenuConfig() ).getStr() << '|';
    if( GetToolBoxConfig() )
        rOutStm << MakeSlotName( SvHash_ToolBoxConfig() ).getStr() << '|';
    if( GetStatusBarConfig() )
        rOutStm << MakeSlotName( SvHash_StatusBarConfig() ).getStr() << '|';
    if( GetAccelConfig() )
        rOutStm << MakeSlotName( SvHash_AccelConfig() ).getStr() << '|';
    if( GetFastCall() )
        rOutStm << MakeSlotName( SvHash_FastCall() ).getStr() << '|';
    if( GetContainer() )
        rOutStm << MakeSlotName( SvHash_Container() ).getStr() << '|';
    if ( GetReadOnlyDoc() )
        rOutStm << MakeSlotName( SvHash_ReadOnlyDoc() ).getStr() << '|';
    if( GetImageRotation() )
        rOutStm << MakeSlotName( SvHash_ImageRotation() ).getStr() << '|';
    if( GetImageReflection() )
        rOutStm << MakeSlotName( SvHash_ImageReflection() ).getStr() << '|';
    rOutStm << '0';

    rOutStm << ',' << endl;
       WriteTab( rOutStm, 4 );
    if ( GetDisableFlags().isEmpty() )
        rOutStm << "0";
    else
        rOutStm << GetDisableFlags().getStr();

    // write attribute type
    if( !bIsEnumSlot )
    {
        rOutStm << ',' << endl;
        WriteTab( rOutStm, 4 );

        SvMetaType * pT = GetSlotType();
        if( !pT )
        {
            if( !IsVariable() )
                pT = rBase.FindType( "SfxVoidItem" );
            else
                pT = GetType();
        }
        if( pT )
        {
            rOutStm << pT->GetName().getString().getStr();
            if( !rBase.FindType( pT, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pT );
        }
        else
            rOutStm << "SfxVoidItem not defined";
    }
    else
    {
        SvMetaType *pT = rBase.FindType( "SfxBoolItem" );
        if ( pT && !rBase.FindType( pT, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pT );
    }

    if( !bIsEnumSlot )
    {
        rOutStm << ',' << endl;
        WriteTab( rOutStm, 4 );
        rOutStm
            << rtl::OString::valueOf(static_cast<sal_Int32>(nCount)).getStr()
            << "/*Offset*/, ";

        if( IsMethod() )
        {
            SvMetaAttribute * pMethod = GetMethod();
            SvMetaType * pType;
            if( pMethod )
                pType = pMethod->GetType();
            else
                pType = GetType();
            sal_uLong nSCount = pType->GetAttrCount();
            rOutStm
                << rtl::OString::valueOf(static_cast<sal_Int32>(
                    nSCount)).getStr()
                << "/*Count*/";
        }
        else
            rOutStm << '0';

        // name for recording
        if ( GetExport() )
        {
            rOutStm << ",\"";
            if (!rPrefix.isEmpty())
                rOutStm << rPrefix.getStr();
            rOutStm << '.';
            if ( !IsVariable() || !GetType() ||
                 GetType()->GetBaseType()->GetType() != TYPE_STRUCT )
                rOutStm << GetMangleName( sal_False ).getStr();
            rOutStm << "\",";
        }
        else
            rOutStm << ", 0, ";

        // Method/Property flags
        if( IsMethod() )
            rOutStm << "SFX_SLOT_METHOD|";
        if( IsVariable() )
        {
            rOutStm << "SFX_SLOT_PROPGET|";
            if( !GetReadonly() )
                rOutStm << "SFX_SLOT_PROPSET|";
        }

        rOutStm << '0';
    }

    {
        rOutStm << ",\"";
        rOutStm << GetMangleName( sal_False ).getStr();
        rOutStm << "\"";
    }

    rOutStm << " )," << endl;
}

sal_uInt16 SvMetaSlot::WriteSlotParamArray( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return 0;

    SvMetaAttribute * pMethod = GetMethod();
    if( IsMethod() )
    {
        SvMetaType * pType;
        if( pMethod )
            pType = pMethod->GetType();
        else
            pType = GetType();

        if( !rBase.FindType( pType, rBase.aUsedTypes ) )
            rBase.aUsedTypes.push_back( pType );

        const SvMetaAttributeMemberList & rList =
                    pType->GetAttrList();
        for( sal_uLong n = 0; n < rList.size(); n++ )
        {
            SvMetaAttribute * pPar  = rList[n];
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm << "SFX_ARGUMENT("
                << pPar->GetSlotId().getString().getStr() << ',' // SlodId
                // parameter name
                << "\"" << pPar->GetName().getString().getStr() << "\","
                // item name
                << pPType->GetName().getString().getStr() << ")," << endl;
            if( !rBase.FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.push_back( pPType );
        }
        return (sal_uInt16)rList.size();
    }
    return 0;
}

sal_uInt16 SvMetaSlot::WriteSlotMap( const rtl::OString& rShellName, sal_uInt16 nCount,
                                SvSlotElementList& rSlotList,
                                size_t nStart,
                                const rtl::OString& rPrefix,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    // SlotId, if not specified generate from name
    rtl::OString slotId = GetSlotId().getString();

    sal_uInt16 nSCount = 0;
    if( IsMethod() )
    {
        SvMetaType * pType;
        SvMetaAttribute * pMethod = GetMethod();
        if( pMethod )
            pType = pMethod->GetType();
        else
            pType = GetType();

        nSCount = (sal_uInt16)pType->GetAttrCount();
    }

    WriteSlot( rShellName, nCount, slotId, rSlotList, nStart, rPrefix, rBase, rOutStm );
    return nSCount;
}

void SvMetaSlot::WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                             HelpIdTable&  rTable )
{
    sal_uLong nSId = GetSlotId().GetValue();
    if( rTable.find( nSId ) == rTable.end() )
    {
        rTable[ nSId ] = this;
        rOutStm << "#define " << GetSlotId().getString().getStr() << '\t'
            << rtl::OString::valueOf(static_cast<sal_Int32>(nSId)).getStr()
            << endl;
    }

    SvMetaTypeEnum * pEnum = PTR_CAST( SvMetaTypeEnum, GetType() );
    if( GetPseudoSlots() && pEnum )
    {
        for( sal_uLong n = 0; n < pEnum->Count(); ++n )
        {
            rtl::OString aValName = pEnum->GetObject( n )->GetName().getString();

            rtl::OStringBuffer aBuf;
            if( !GetPseudoPrefix().isEmpty() )
                aBuf.append(GetPseudoPrefix());
            else
                aBuf.append(GetSlotId().getString());
            aBuf.append('_');
            aBuf.append(aValName.copy(pEnum->GetPrefix().getLength()));

            rtl::OString aSId = aBuf.makeStringAndClear();

            sal_uLong nSId2;
            sal_Bool bIdOk = sal_False;
            if( rBase.FindId( aSId, &nSId2 ) )
            {
                aSId = rtl::OString::valueOf(static_cast<sal_Int32>(nSId2));
                bIdOk = sal_True;
            }

            // if id not found, write always
            if( !bIdOk || rTable.find( nSId2 ) == rTable.end() )
            {
                rTable[ nSId2 ] = this;

                rOutStm << "#define " << aSId.getStr() << '\t'
                    << rtl::OString::valueOf(
                        static_cast<sal_Int32>(nSId2)).getStr()
                    << endl;
            }
        }
    }
}

void WriteBool( sal_Bool bSet, SvStream& rStream )
{
    if ( bSet )
        rStream << "TRUE" << ',';
    else
        rStream << "FALSE" << ',';
}

void SvMetaSlot::WriteCSV( SvIdlDataBase& rBase, SvStream& rStrm )
{
    rStrm << "PROJECT,";
    rStrm << GetSlotId().getString().getStr() << ',';
    rStrm
        << rtl::OString::valueOf(
            static_cast<sal_Int32>(GetSlotId().GetValue())).getStr()
        << ',';

    if ( !GetPseudoPrefix().isEmpty() )
        rStrm << GetPseudoPrefix().getStr() << ',';
    else
        rStrm << ',';

    rStrm << GetGroupId().getStr() << ',';

    WriteBool( GetAccelConfig(), rStrm );
    WriteBool( GetMenuConfig(), rStrm );
    WriteBool( GetStatusBarConfig(), rStrm );
    WriteBool( GetToolBoxConfig(), rStrm );

    if ( GetSlotType() )
        rStrm << GetSlotType()->GetName().getString().getStr() << ',';
    else
        rStrm << ',';

    WriteBool( GetAutoUpdate(), rStrm );
    if ( GetCachable() )
        rStrm << "Cachable" << ',';
    else
        rStrm << "Volatile" << ',';

    WriteBool( GetContainer(), rStrm );
    WriteBool( GetFastCall(), rStrm );
    WriteBool( GetHasCoreId(), rStrm );
    WriteBool( GetHasDialog(), rStrm );
    WriteBool( GetReadOnlyDoc(), rStrm );
    WriteBool( GetImageRotation(), rStrm );
    WriteBool( GetImageReflection(), rStrm );
    rStrm << GetDisableFlags().getStr() << ',';

    if( GetSynchron() )
        rStrm << "Synchron" << ',';
    else
        rStrm << "Asynchron" << ',';

    WriteBool( GetToggle(), rStrm );
    WriteBool( GetReadonly(), rStrm );
    WriteBool( GetExport(), rStrm );
    if( GetRecordPerItem() )
        rStrm << "RecordPerItem" << ',';
    else if ( GetNoRecord() )
        rStrm << "NoRecord" << ',';
    else if ( GetRecordManual() )
        rStrm << "RecordManual" << ',';
    else
        rStrm << "RecordPerSet" << ',';

    WriteBool( GetRecordAbsolute(), rStrm );

    if ( GetType()->GetType() != TYPE_METHOD && GetMethod() )
    {
        rStrm << GetMethod()->GetType()->GetReturnType()->GetName().getString().getStr() << ',';
        rStrm << GetMethod()->GetName().getString().getStr() << ',';
    }
    else
    {
        rStrm << ",,";
    }

    rStrm << GetType()->GetSvName().getStr() << ',';
    rStrm << GetName().getString().getStr() << ',';

    if ( GetType()->GetType() == TYPE_METHOD || GetMethod() )
    {
        SvMetaAttributeMemberList *pList = &GetType()->GetAttrList();
           if ( GetMethod() )
            pList = &GetMethod()->GetType()->GetAttrList();

        if( pList && !pList->empty() )
        {
            rStrm << "\"(";
            SvMetaAttributeMemberList::const_iterator it = pList->begin();
            while( it != pList->end() )
            {
                SvMetaAttribute* pAttr = *it;
                pAttr->WriteCSV( rBase, rStrm );
                ++it;
                if( it != pList->end() )
                    rStrm << ',';
            }
            rStrm << ")\"";
        }
        else
            rStrm << "()";
    }

    rStrm << endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
