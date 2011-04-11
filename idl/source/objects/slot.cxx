/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idl.hxx"

#include <ctype.h>
#include <stdio.h>
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
    if( aGroupId.Len() )        nMask |= 0x0002;
    if( aHasCoreId.IsSet() )    nMask |= 0x0004;
    if( aConfigId.Len() )       nMask |= 0x0008;
    if( aExecMethod.Len() )     nMask |= 0x0010;
    if( aStateMethod.Len() )    nMask |= 0x0020;
    if( aDefault.Len() )        nMask |= 0x0040;
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
    if( aPseudoPrefix.Len() )     nMask |= 0x0020;
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

ByteString SvMetaSlot::GetMangleName( sal_Bool bVariable ) const
{
    if( !bVariable )
    {
        SvMetaAttribute * pMeth = GetMethod();
        if( pMeth )
            return pMeth->GetName();
    }
    return GetName();
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
const ByteString & SvMetaSlot::GetGroupId() const
{
    if( aGroupId.Len() || !GetRef() ) return aGroupId;
    return ((SvMetaSlot *)GetRef())->GetGroupId();
}
const ByteString & SvMetaSlot::GetDisableFlags() const
{
    if( aDisableFlags.Len() || !GetRef() ) return aDisableFlags;
    return ((SvMetaSlot *)GetRef())->GetDisableFlags();
}
const ByteString & SvMetaSlot::GetConfigId() const
{
    if( aConfigId.Len() || !GetRef() ) return aConfigId;
    return ((SvMetaSlot *)GetRef())->GetConfigId();
}
const ByteString & SvMetaSlot::GetExecMethod() const
{
    if( aExecMethod.Len() || !GetRef() ) return aExecMethod;
    return ((SvMetaSlot *)GetRef())->GetExecMethod();
}
const ByteString & SvMetaSlot::GetStateMethod() const
{
    if( aStateMethod.Len() || !GetRef() ) return aStateMethod;
    return ((SvMetaSlot *)GetRef())->GetStateMethod();
}
const ByteString & SvMetaSlot::GetDefault() const
{
    if( aDefault.Len() || !GetRef() ) return aDefault;
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
const ByteString & SvMetaSlot::GetPseudoPrefix() const
{
    if( aPseudoPrefix.Len() || !GetRef() ) return aPseudoPrefix;
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

const ByteString& SvMetaSlot::GetUnoName() const
{
    if( aUnoName.IsSet() || !GetRef() ) return aUnoName;
    return ((SvMetaSlot *)GetRef())->GetUnoName();
}

#ifdef IDL_COMPILER
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
        rBase.WriteError( "warning", ByteString( rInStm.GetFileName(), RTL_TEXTENCODING_UTF8 ),
                    "<Get> old style, use Readonly",
                    rInStm.GetToken()->GetLine(),
                    rInStm.GetToken()->GetColumn() );
    }
    if( aSet.ReadSvIdl( SvHash_Set(), rInStm ) )
    {
        rBase.WriteError( "warning", ByteString( rInStm.GetFileName(), RTL_TEXTENCODING_UTF8 ),
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
        rOutStm << SvHash_SlotType()->GetName().GetBuffer() << '(';
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
    if( aGroupId.Len() )
    {
        WriteTab( rOutStm, nTab );
        aGroupId.WriteSvIdl( SvHash_GroupId(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }
    if( aExecMethod.Len() )
    {
        WriteTab( rOutStm, nTab );
        aExecMethod.WriteSvIdl( SvHash_ExecMethod(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }
    if( aStateMethod.Len() )
    {
        WriteTab( rOutStm, nTab );
        aStateMethod.WriteSvIdl( SvHash_StateMethod(), rOutStm, nTab +1);
        rOutStm << ';' << endl;
    }

    if( aDisableFlags.Len() )
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

    ByteString aDel( ", " );
    ByteString aOut;
    if( aVolatile )
        aOut += aVolatile.GetSvIdlString( SvHash_Volatile() );
    else if( !aCachable )
        // because of Default == TRUE, only when no other is set
        aOut += aCachable.GetSvIdlString( SvHash_Cachable() );
    else
        aDel.Erase();

    if( aToggle )
    {
        ( aOut += aDel ) += aToggle.GetSvIdlString( SvHash_Toggle() );
        aDel = ", ";
    }
    if( aAutoUpdate )
    {
        (aOut += aDel ) += aAutoUpdate.GetSvIdlString( SvHash_AutoUpdate() );
        aDel = ", ";
    }

    ByteString aDel1( ", " );
    if( aAsynchron )
        ( aOut += aDel ) += aAsynchron.GetSvIdlString( SvHash_Asynchron() );
    else if( !aSynchron )
        // because of Default == TRUE, only when no other is set
        ( aOut += aDel ) += aSynchron.GetSvIdlString( SvHash_Synchron() );
    else
        aDel1 = aDel;

    aDel = ", ";
    if( aRecordManual )
        ( aOut += aDel1 ) += aRecordManual.GetSvIdlString( SvHash_RecordManual() );
    else if( aNoRecord )
        ( aOut += aDel1 ) += aNoRecord.GetSvIdlString( SvHash_NoRecord() );
    else if( !aRecordPerSet )
        // because of Default == TRUE, only when no other is set
        ( aOut += aDel1 ) += aRecordPerSet.GetSvIdlString( SvHash_RecordPerSet() );
    else if( aRecordPerItem )
        ( aOut += aDel1 ) += aRecordPerItem.GetSvIdlString( SvHash_RecordPerItem() );
    else
        aDel = aDel1;

    if( aRecordAbsolute )
    {
        ( aOut += aDel ) += aRecordAbsolute.GetSvIdlString( SvHash_RecordAbsolute() );
        aDel = ", ";
    }
    if( aHasDialog )
    {
        ( aOut += aDel ) += aHasDialog.GetSvIdlString( SvHash_HasDialog() );
        aDel = ", ";
    }
    if( aMenuConfig )
    {
        ( aOut += aDel ) += aMenuConfig.GetSvIdlString( SvHash_MenuConfig() );
        aDel = ", ";
    }
    if( aToolBoxConfig )
    {
        ( aOut += aDel ) += aToolBoxConfig.GetSvIdlString( SvHash_ToolBoxConfig() );
        aDel = ", ";
    }
    if( aStatusBarConfig )
    {
        ( aOut += aDel ) += aStatusBarConfig.GetSvIdlString( SvHash_StatusBarConfig() );
        aDel = ", ";
    }
    if( aAccelConfig )
    {
        ( aOut += aDel ) += aAccelConfig.GetSvIdlString( SvHash_AccelConfig() );
        aDel = ", ";
    }
    if( aFastCall )
    {
        ( aOut += aDel ) += aFastCall.GetSvIdlString( SvHash_FastCall() );
        aDel = ", ";
    }
    if( aContainer )
    {
        ( aOut += aDel ) += aContainer.GetSvIdlString( SvHash_Container() );
        aDel = ", ";
    }
    if( aImageRotation )
    {
        ( aOut += aDel ) += aImageRotation.GetSvIdlString( SvHash_ImageRotation() );
        aDel = ", ";
    }

    if( aImageReflection )
    {
        ( aOut += aDel ) += aImageReflection.GetSvIdlString( SvHash_ImageReflection() );
        aDel = ", ";
    }

    if( aOut.Len() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << aOut.GetBuffer() << endl;
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
            SetName( pKnownSlot->GetName(), &rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        else
        {
            ByteString aStr( "attribute " );
            aStr += pAttr->GetName();
            aStr += " is method or variable but not a slot";
            rBase.SetError( aStr, rInStm.GetToken() );
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
                if ( pKnownSlot->GetName() != GetName() )
                {
                    OSL_FAIL("Illegal definition!");
                    rInStm.Seek( nTokPos );
                    return sal_False;
                }

                  SetName( pKnownSlot->GetName(), &rBase );
            }
            else
            {
                ByteString aStr( "attribute " );
                aStr += pAttr2->GetName();
                aStr += " is method or variable but not a slot";
                rBase.SetError( aStr, rInStm.GetToken() );
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
            xM->SetDescription( GetDescription() );
            xM->Write( rBase, rOutStm, nTab, nT, nA );
            return;
        }
    }

    SvMetaAttribute::Write( rBase, rOutStm, nTab, nT, nA );
}


void SvMetaSlot::Insert( SvSlotElementList& rList, const ByteString & rPrefix,
                        SvIdlDataBase& rBase)
{
    // get insert position through binary search in slotlist
    sal_uInt16 nId = (sal_uInt16) GetSlotId().GetValue();
    sal_uInt16 nListCount = (sal_uInt16) rList.Count();
    sal_uInt16 nPos;
    sal_uLong m;  // for inner "for" loop

    if ( !nListCount )
        nPos = 0;
    else if ( nListCount == 1 )
        nPos = rList.GetObject(0)->xSlot->GetSlotId().GetValue() >= nId ? 0 : 1;
    else
    {
        sal_uInt16 nMid = 0, nLow = 0;
        sal_uInt16 nHigh = nListCount - 1;
        sal_Bool bFound = sal_False;
        while ( !bFound && nLow <= nHigh )
        {
            nMid = (nLow + nHigh) >> 1;
            DBG_ASSERT( nMid < nListCount, "bsearch ist buggy" );
            int nDiff = (int) nId - (int) rList.GetObject(nMid)->xSlot->GetSlotId().GetValue();
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
        (sal_uInt16) rList.GetObject(nPos)->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );
    DBG_ASSERT( nPos == 0 || nId >
        (sal_uInt16) rList.GetObject(nPos-1)->xSlot->GetSlotId().GetValue(),
        "Predecessor has higher SlotId" );
    DBG_ASSERT( nPos+1 >= nListCount || nId <
        (sal_uInt16) rList.GetObject(nPos+1)->xSlot->GetSlotId().GetValue(),
        "Successor has lower SlotId" );

    rList.Insert( new SvSlotElement( this, rPrefix ), nPos );

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
            ByteString aValName = enumValue->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            xEnumSlot = NULL;
            for( m=0; m<rBase.GetAttrList().Count(); m++ )
            {
                SvMetaAttribute * pAttr = rBase.GetAttrList().GetObject( m );
                if( pAttr->GetSlotId() == aSId )
                {
                    SvMetaSlot* pSlot = PTR_CAST( SvMetaSlot, pAttr );
                    xEnumSlot = pSlot->Clone();
                    break;
                }
            }

            if ( m == rBase.GetAttrList().Count() )
            {
                OSL_FAIL("Invalid EnumSlot!");
                xEnumSlot = Clone();
                sal_uLong nValue;
                if ( rBase.FindId(aSId , &nValue) )
                {
                    SvNumberIdentifier aId;
                    *((SvIdentifier*)&aId) = aSId;
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
        rList.Seek((sal_uLong)0);
        xEnumSlot = pFirstEnumSlot;
        SvSlotElement *pEle;
        do
        {
            pEle = rList.Next();
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


static ByteString MakeSlotName( SvStringHashEntry * pEntry )
{
    ByteString aName( "SFX_SLOT_" );
    aName += pEntry->GetName();
    return aName.ToUpperAscii();
};

void SvMetaSlot::WriteSlotStubs( const ByteString & rShellName,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    ByteString aMethodName( GetExecMethod() );
    if ( aMethodName.Len() && aMethodName != "NoExec" )
    {
        sal_Bool bIn = sal_False;
        for( size_t n = 0; n < rList.size(); n++ )
        {
            if( *(rList[ n ]) == aMethodName )
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new ByteString(aMethodName) );
            rOutStm << "SFX_EXEC_STUB("
                    << rShellName.GetBuffer()
                    << ','
                    << aMethodName.GetBuffer()
                    << ')' << endl;
        }
    }

    aMethodName = GetStateMethod();
    if ( aMethodName.Len() && aMethodName != "NoState" )
    {
        sal_Bool bIn = sal_False;
        for ( size_t n=0; n < rList.size(); n++ )
        {
            if ( *(rList[ n ]) == aMethodName )
            {
                bIn=sal_True;
                break;
            }
        }

        if ( !bIn )
        {
            rList.push_back( new ByteString(aMethodName) );
            rOutStm << "SFX_STATE_STUB("
                    << rShellName.GetBuffer()
                    << ','
                    << aMethodName.GetBuffer()
                    << ')' << endl;
        }
    }
}

void SvMetaSlot::WriteSlot( const ByteString & rShellName, sal_uInt16 nCount,
                            const ByteString & rSlotId,
                            SvSlotElementList& rSlotList,
                            const ByteString & rPrefix,
                            SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

    sal_Bool bIsEnumSlot = 0 != pEnumValue;

    rOutStm << "// Slot Nr. " << ByteString::CreateFromInt32(nListPos).GetBuffer() << " : ";
    ByteString aSlotIdValue( ByteString::CreateFromInt32( GetSlotId().GetValue() ) );
    rOutStm << aSlotIdValue.GetBuffer() << endl;
    WriteTab( rOutStm, 1 );
    if( bIsEnumSlot )
        rOutStm << "SFX_NEW_SLOT_ENUM( ";
    else
        rOutStm << "SFX_NEW_SLOT_ARG( " << rShellName.GetBuffer() << ',' ;

    rOutStm << rSlotId.GetBuffer() << ',';
    const SvHelpContext& rHlpCtx = GetHelpContext();
    if( rHlpCtx.IsSet() )
        rOutStm << rHlpCtx.GetBuffer() << ',';
    else
        rOutStm << rSlotId.GetBuffer() << ',';

    // GroupId
    if( GetGroupId().Len() )
        rOutStm << GetGroupId().GetBuffer();
    else
        rOutStm << '0';
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 4 );

    if( bIsEnumSlot )
    {
        rOutStm << "&a" << rShellName.GetBuffer() << "Slots_Impl["
                << ByteString::CreateFromInt32(pLinkedSlot->GetListPos()).GetBuffer()
                << "] /*Offset Master*/, " << endl;
        WriteTab( rOutStm, 4 );
        rOutStm << "&a" << rShellName.GetBuffer() << "Slots_Impl["
                << ByteString::CreateFromInt32(pNextSlot->GetListPos()).GetBuffer()
                << "] /*Offset Next*/, " << endl;

        WriteTab( rOutStm, 4 );

        // SlotId
        if( GetSlotId().Len() )
            rOutStm << pLinkedSlot->GetSlotId().GetBuffer();
        else
            rOutStm << '0';
        rOutStm << ',';
        rOutStm << pEnumValue->GetName().GetBuffer();
    }
    else
    {
        // look for the next slot with the same StateMethod like me
        // the slotlist is set to the current slot
        SvSlotElement * pEle = rSlotList.Next();
        pNextSlot = pEle ? &pEle->xSlot : NULL;
        while ( pNextSlot )
        {
            if ( !pNextSlot->pNextSlot &&
                pNextSlot->GetStateMethod() == GetStateMethod() )
                break;
            pEle = rSlotList.Next();
            pNextSlot = pEle ? &pEle->xSlot : NULL;
        }

        if ( !pNextSlot )
        {
            // There is no slot behind me that has the same ExecMethod.
            // So I search for the first slot with it (could be myself).
            pEle = rSlotList.First();
            pNextSlot = pEle ? &pEle->xSlot : NULL;
            while ( pNextSlot != this )
            {
                if ( !pNextSlot->pEnumValue &&
                    pNextSlot->GetStateMethod() == GetStateMethod() )
                    break;
                pEle = rSlotList.Next();
                pNextSlot = pEle ? &pEle->xSlot : NULL;
            }
        }

        if ( !pLinkedSlot )
        {
            rOutStm << "0 ,";
        }
        else
        {
            rOutStm << "&a" << rShellName.GetBuffer() << "Slots_Impl["
                << ByteString::CreateFromInt32(pLinkedSlot->GetListPos()).GetBuffer()
                << "] /*Offset Linked*/, " << endl;
            WriteTab( rOutStm, 4 );
        }

        rOutStm << "&a" << rShellName.GetBuffer() << "Slots_Impl["
                << ByteString::CreateFromInt32(pNextSlot->GetListPos()).GetBuffer()
                << "] /*Offset Next*/, " << endl;

        WriteTab( rOutStm, 4 );

        // write ExecMethod, with standard name if not specified
        if( GetExecMethod().Len() && GetExecMethod() != "NoExec")
            rOutStm << "SFX_STUB_PTR(" << rShellName.GetBuffer() << ','
                    << GetExecMethod().GetBuffer() << ')';
        else
            rOutStm << "SFX_STUB_PTR_EXEC_NONE";
        rOutStm << ',';

        // write StateMethod, with standard name if not specified
        if( GetStateMethod().Len() && GetStateMethod() != "NoState")
            rOutStm << "SFX_STUB_PTR(" << rShellName.GetBuffer() << ','
                    << GetStateMethod().GetBuffer() << ')';
        else
            rOutStm << "SFX_STUB_PTR_STATE_NONE";
    }
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 4 );

    // write flags
    if( GetHasCoreId() )
        rOutStm << MakeSlotName( SvHash_HasCoreId() ).GetBuffer() << '|';
    if( GetCachable() )
        rOutStm << MakeSlotName( SvHash_Cachable() ).GetBuffer() << '|';
    if( GetVolatile() )
        rOutStm << MakeSlotName( SvHash_Volatile() ).GetBuffer() << '|';
    if( GetToggle() )
        rOutStm << MakeSlotName( SvHash_Toggle() ).GetBuffer() << '|';
    if( GetAutoUpdate() )
        rOutStm << MakeSlotName( SvHash_AutoUpdate() ).GetBuffer() << '|';
    if( GetSynchron() )
        rOutStm << MakeSlotName( SvHash_Synchron() ).GetBuffer() << '|';
    if( GetAsynchron() )
        rOutStm << MakeSlotName( SvHash_Asynchron() ).GetBuffer() << '|';
    if( GetRecordPerItem() )
        rOutStm << MakeSlotName( SvHash_RecordPerItem() ).GetBuffer() << '|';
    if( GetRecordPerSet() )
        rOutStm << MakeSlotName( SvHash_RecordPerSet() ).GetBuffer() << '|';
    if( GetRecordManual() )
        rOutStm << MakeSlotName( SvHash_RecordManual() ).GetBuffer() << '|';
    if( GetNoRecord() )
        rOutStm << MakeSlotName( SvHash_NoRecord() ).GetBuffer() << '|';
    if( GetRecordAbsolute() )
        rOutStm << MakeSlotName( SvHash_RecordAbsolute() ).GetBuffer() << '|';
    if( GetHasDialog() )
        rOutStm << MakeSlotName( SvHash_HasDialog() ).GetBuffer() << '|';
    if( GetMenuConfig() )
        rOutStm << MakeSlotName( SvHash_MenuConfig() ).GetBuffer() << '|';
    if( GetToolBoxConfig() )
        rOutStm << MakeSlotName( SvHash_ToolBoxConfig() ).GetBuffer() << '|';
    if( GetStatusBarConfig() )
        rOutStm << MakeSlotName( SvHash_StatusBarConfig() ).GetBuffer() << '|';
    if( GetAccelConfig() )
        rOutStm << MakeSlotName( SvHash_AccelConfig() ).GetBuffer() << '|';
    if( GetFastCall() )
        rOutStm << MakeSlotName( SvHash_FastCall() ).GetBuffer() << '|';
    if( GetContainer() )
        rOutStm << MakeSlotName( SvHash_Container() ).GetBuffer() << '|';
    if ( GetReadOnlyDoc() )
        rOutStm << MakeSlotName( SvHash_ReadOnlyDoc() ).GetBuffer() << '|';
    if( GetImageRotation() )
        rOutStm << MakeSlotName( SvHash_ImageRotation() ).GetBuffer() << '|';
    if( GetImageReflection() )
        rOutStm << MakeSlotName( SvHash_ImageReflection() ).GetBuffer() << '|';
    rOutStm << '0';

    rOutStm << ',' << endl;
       WriteTab( rOutStm, 4 );
    if ( !GetDisableFlags().Len() )
        rOutStm << "0";
    else
        rOutStm << GetDisableFlags().GetBuffer();

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
            rOutStm << pT->GetName().GetBuffer();
            if( !rBase.FindType( pT, rBase.aUsedTypes ) )
                rBase.aUsedTypes.Append( pT );
        }
        else
            rOutStm << "SfxVoidItem not defined";
    }
    else
    {
        SvMetaType *pT = rBase.FindType( "SfxBoolItem" );
        if ( pT && !rBase.FindType( pT, rBase.aUsedTypes ) )
            rBase.aUsedTypes.Append( pT );
    }

    if( !bIsEnumSlot )
    {
        rOutStm << ',' << endl;
        WriteTab( rOutStm, 4 );
        rOutStm << ByteString::CreateFromInt32( nCount ).GetBuffer() << "/*Offset*/, ";

        if( IsMethod() )
        {
            SvMetaAttribute * pMethod = GetMethod();
            SvMetaType * pType;
            if( pMethod )
                pType = pMethod->GetType();
            else
                pType = GetType();
            sal_uLong nSCount = pType->GetAttrCount();
            rOutStm << ByteString::CreateFromInt32( nSCount ).GetBuffer() << "/*Count*/";
        }
        else
            rOutStm << '0';

        // name for recording
        if ( GetExport() )
        {
            rOutStm << ",\"";
            if( rPrefix.Len() )
                rOutStm << rPrefix.GetBuffer();
            rOutStm << '.';
            if ( !IsVariable() || !GetType() ||
                 GetType()->GetBaseType()->GetType() != TYPE_STRUCT )
                rOutStm << GetMangleName( sal_False ).GetBuffer();
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
        rOutStm << GetMangleName( sal_False ).GetBuffer();
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
            rBase.aUsedTypes.Append( pType );

        const SvMetaAttributeMemberList & rList =
                    pType->GetAttrList();
        for( sal_uLong n = 0; n < rList.Count(); n++ )
        {
            SvMetaAttribute * pPar  = rList.GetObject( n );
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm << "SFX_ARGUMENT("
                << pPar->GetSlotId().GetBuffer() << ',' // SlodId
                // parameter name
                << "\"" << pPar->GetName().GetBuffer() << "\","
                // item name
                << pPType->GetName().GetBuffer() << ")," << endl;
            if( !rBase.FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.Append( pPType );
        }
        return (sal_uInt16)rList.Count();
    }
    return 0;
}

sal_uInt16 SvMetaSlot::WriteSlotMap( const ByteString & rShellName, sal_uInt16 nCount,
                                SvSlotElementList& rSlotList,
                                const ByteString & rPrefix,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    // SlotId, if not specified generate from name
    ByteString slotId = GetSlotId();

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

    WriteSlot( rShellName, nCount, slotId, rSlotList, rPrefix, rBase, rOutStm );
    return nSCount;
}

void SvMetaSlot::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    if (!GetToolBoxConfig() && !GetAccelConfig() && !GetMenuConfig() && !GetStatusBarConfig() )
        return;

    sal_uLong nSId = GetSlotId().GetValue();
    if( !pTable->IsKeyValid( nSId ) )
    {
        pTable->Insert( nSId, this );
        rOutStm << "SfxSlotInfo " << ByteString::CreateFromInt32( nSId ).GetBuffer()
                << endl << '{' << endl;

        WriteTab( rOutStm, 1 );
        ByteString aStr = GetConfigName();
        if( !aStr.Len() )
            aStr = GetName();

        rOutStm << "SlotName = \"" << aStr.GetBuffer() << "\";" << endl;

        aStr = GetHelpText();
        if( aStr.Len() )
        {
            WriteTab( rOutStm, 1 );
            rOutStm << "HelpText = \"" << aStr.GetBuffer() << "\";" << endl;
        }

        rOutStm << "};" << endl;
    }

    SvMetaTypeEnum * pEnum = PTR_CAST( SvMetaTypeEnum, GetType() );
    if( GetPseudoSlots() && pEnum )
    {
        for( sal_uLong n = 0; n < pEnum->Count(); n++ )
        {
            ByteString aValName = pEnum->GetObject( n )->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            sal_uLong nSId2;
            sal_Bool bIdOk = sal_False;
            if( rBase.FindId( aSId, &nSId2 ) )
            {
                aSId = ByteString::CreateFromInt32( nSId2 );
                bIdOk = sal_True;
            }

            // if id not found, write always
            if( !bIdOk || !pTable->IsKeyValid( nSId2 ) )
            {
                pTable->Insert( nSId2, this );
                rOutStm << "SfxSlotInfo " << aSId.GetBuffer()
                        << endl << '{' << endl;

                WriteTab( rOutStm, 1 );
                rOutStm << "SlotName = \"" << aValName.GetBuffer() << "\";" << endl;

                ByteString aStr = GetHelpText();
                if( aStr.Len() )
                {
                    WriteTab( rOutStm, 1 );
                    rOutStm << "HelpText = \"" << aStr.GetBuffer() << "\";" << endl;
                }
                rOutStm << "};" << endl;
            }
        }
    }
}

void SvMetaSlot::WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    sal_uLong nSId = GetSlotId().GetValue();
    if( !pTable->IsKeyValid( nSId ) )
    {
        pTable->Insert( nSId, this );
        rOutStm << "#define " << GetSlotId().GetBuffer() << '\t' << ByteString::CreateFromInt32( nSId ).GetBuffer() << endl;
    }

    SvMetaTypeEnum * pEnum = PTR_CAST( SvMetaTypeEnum, GetType() );
    if( GetPseudoSlots() && pEnum )
    {
        for( sal_uLong n = 0; n < pEnum->Count(); n++ )
        {
            ByteString aValName = pEnum->GetObject( n )->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            sal_uLong nSId2;
            sal_Bool bIdOk = sal_False;
            if( rBase.FindId( aSId, &nSId2 ) )
            {
                aSId = ByteString::CreateFromInt32( nSId2 );
                bIdOk = sal_True;
            }

            // if id not found, write always
            if( !bIdOk || !pTable->IsKeyValid( nSId2 ) )
            {
                pTable->Insert( nSId2, this );

                rOutStm << "#define " << aSId.GetBuffer() << '\t'
                        << ByteString::CreateFromInt32( nSId2 ).GetBuffer() << endl;
            }
        }
    }
}

void WriteBool( sal_Bool bSet, SvStream& rStream )
{
    if ( bSet )
        rStream << "sal_True" << ',';
    else
        rStream << "sal_False" << ',';
}

void SvMetaSlot::WriteCSV( SvIdlDataBase& rBase, SvStream& rStrm )
{
    rStrm << "PROJECT,";
    rStrm << GetSlotId().GetBuffer() << ',';
    rStrm << ByteString::CreateFromInt32( GetSlotId().GetValue() ).GetBuffer() << ',';

    if ( GetPseudoPrefix().Len() )
        rStrm << GetPseudoPrefix().GetBuffer() << ',';
    else
        rStrm << ',';

    rStrm << GetGroupId().GetBuffer() << ',';

    WriteBool( GetAccelConfig(), rStrm );
    WriteBool( GetMenuConfig(), rStrm );
    WriteBool( GetStatusBarConfig(), rStrm );
    WriteBool( GetToolBoxConfig(), rStrm );

    if ( GetSlotType() )
        rStrm << GetSlotType()->GetName().GetBuffer() << ',';
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
    rStrm << GetDisableFlags().GetBuffer() << ',';

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
        rStrm << GetMethod()->GetType()->GetReturnType()->GetName().GetBuffer() << ',';
        rStrm << GetMethod()->GetName().GetBuffer() << ',';
    }
    else
    {
        rStrm << ",,";
    }

    rStrm << GetType()->GetSvName().GetBuffer() << ',';
    rStrm << GetName().GetBuffer() << ',';

    if ( GetType()->GetType() == TYPE_METHOD || GetMethod() )
    {
        SvMetaAttributeMemberList *pList = &GetType()->GetAttrList();
           if ( GetMethod() )
            pList = &GetMethod()->GetType()->GetAttrList();

        if( pList && pList->Count() )
        {
            rStrm << "\"(";
            SvMetaAttribute* pAttr = pList->First();
            while( pAttr )
            {
                pAttr->WriteCSV( rBase, rStrm );
                pAttr = pList->Next();
                if( pAttr )
                    rStrm << ',';
            }
            rStrm << ")\"";
        }
        else
            rStrm << "()";
    }

    rStrm << endl;
}

#endif // IDL_COMPILER


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
