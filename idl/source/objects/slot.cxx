/*************************************************************************
 *
 *  $RCSfile: slot.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <tools/debug.hxx>
#include <attrib.hxx>
#include <slot.hxx>
#include <globals.hxx>
#include <database.hxx>
#pragma hdrstop


/****************** SvMetaSlot *****************************************/
SV_IMPL_META_FACTORY1( SvMetaSlot, SvMetaAttribute );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaSlot::GetAttributeList()
{
    if( !pAttribList )
    {
        pAttribList = new SvAttributeList();
    }
    return *pAttribList;
}
#endif

/*************************************************************************
|*    SvMetaSlot::SvMetaSlot()
|*
|*    Beschreibung      Zweites FALSE bei den SvBOOL-Objekten bedeutet,
|*                      IsSet() liefert FALSE (Defaultinitialisierung).
*************************************************************************/
SvMetaSlot::SvMetaSlot()
    : aCachable( TRUE, FALSE )
    , aSynchron( TRUE, FALSE )
    , aRecordPerSet( TRUE, FALSE )
    , aRecordAbsolute( FALSE, FALSE )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , pEnumValue(0)
{
}

SvMetaSlot::SvMetaSlot( SvMetaType * pType )
    : SvMetaAttribute( pType )
    , aCachable( TRUE, FALSE )
    , aSynchron( TRUE, FALSE )
    , aRecordPerSet( TRUE, FALSE )
    , aRecordAbsolute( FALSE, FALSE )
    , pLinkedSlot(0)
    , pNextSlot(0)
    , pEnumValue(0)
{
}

/*
#define TEST_READ                                               \
{                                                               \
    UINT32  nPos;                                               \
    rStm >> nPos;                                               \
    DBG_ASSERT( nPos +4 == rStm.Tell(), "stream pos error" );   \
}

#define TEST_WRITE                                              \
    rStm << (UINT32)rStm.Tell();
*/
#define TEST_READ
#define TEST_WRITE

void SvMetaSlot::Load( SvPersistStream & rStm )
{
    SvMetaAttribute::Load( rStm );

    USHORT nMask;
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
    if( nMask & 0x8000 ) rStm >> aPlugComm;

    nMask = 0;
    rStm >> nMask;

    TEST_READ
    if( nMask & 0x0001 ) rStm >> aUnoName;
}

void SvMetaSlot::Save( SvPersistStream & rStm )
{
    SvMetaAttribute::Save( rStm );

    // Maske erstellen
    USHORT nMask = 0;
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

    // Daten schreiben
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

    // naechste Fuhre schreiben
    // Maske erstellen
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
    if( aPlugComm.IsSet() )       nMask |= 0x8000;

    // Daten schreiben
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
    if( nMask & 0x8000 ) rStm << aPlugComm;

    nMask = 0;
    if( aUnoName.IsSet() )  nMask |= 0x0001;

    rStm << nMask;
    TEST_WRITE
    if( nMask & 0x0001 ) rStm << aUnoName;
}

/*************************************************************************
|*    SvMetaSlot::IsVariable()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaSlot::IsVariable() const
{
    return SvMetaAttribute::IsVariable();
}

/*************************************************************************
|*    SvMetaSlot::IsMethod()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaSlot::IsMethod() const
{
    BOOL b = SvMetaAttribute::IsMethod();
    b |= NULL != GetMethod();
    return b;
}

/*************************************************************************
|*    SvMetaSlot::HasMethods()
|*
|*    Beschreibung
*************************************************************************/
ByteString SvMetaSlot::GetMangleName( BOOL bVariable ) const
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
|*    Referenz
|*
|*    Beschreibung      Zweites FALSE bei den SvBOOL-Objekten bedeutet,
|*                      IsSet() liefert FALSE (Defaultinitialisierung).
*************************************************************************/
/** Referenz Aufloesung **/
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
BOOL SvMetaSlot::GetHasCoreId() const
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
BOOL SvMetaSlot::GetPseudoSlots() const
{
    if( aPseudoSlots.IsSet() || !GetRef() ) return aPseudoSlots;
    return ((SvMetaSlot *)GetRef())->GetPseudoSlots();
}
/*
BOOL SvMetaSlot::GetGet() const
{
    if( aGet.IsSet() || !GetRef() ) return aGet;
    return ((SvMetaSlot *)GetRef())->GetGet();
}
BOOL SvMetaSlot::GetSet() const
{
    if( aSet.IsSet() || !GetRef() ) return aSet;
    return ((SvMetaSlot *)GetRef())->GetSet();
}
*/
BOOL SvMetaSlot::GetCachable() const
{
    // Cachable und Volatile sind exclusiv
    if( !GetRef() || aCachable.IsSet() || aVolatile.IsSet() )
         return aCachable;
    return ((SvMetaSlot *)GetRef())->GetCachable();
}
BOOL SvMetaSlot::GetVolatile() const
{
    // Cachable und Volatile sind exclusiv
    if( !GetRef() || aVolatile.IsSet() || aCachable.IsSet() )
        return aVolatile;
    return ((SvMetaSlot *)GetRef())->GetVolatile();
}
BOOL SvMetaSlot::GetToggle() const
{
    if( aToggle.IsSet() || !GetRef() ) return aToggle;
    return ((SvMetaSlot *)GetRef())->GetToggle();
}
BOOL SvMetaSlot::GetAutoUpdate() const
{
    if( aAutoUpdate.IsSet() || !GetRef() ) return aAutoUpdate;
    return ((SvMetaSlot *)GetRef())->GetAutoUpdate();
}
BOOL SvMetaSlot::GetSynchron() const
{
    // Synchron und Asynchron sind exclusiv
    if( !GetRef() || aSynchron.IsSet() || aAsynchron.IsSet() )
        return aSynchron;
    return ((SvMetaSlot *)GetRef())->GetSynchron();
}
BOOL SvMetaSlot::GetAsynchron() const
{
    // Synchron und Asynchron sind exclusiv
    if( !GetRef() || aAsynchron.IsSet() || aSynchron.IsSet() )
        return aAsynchron;
    return ((SvMetaSlot *)GetRef())->GetAsynchron();
}
BOOL SvMetaSlot::GetRecordPerItem() const
{
    // Record- PerItem, No, PerSet und Manual sind exclusiv
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerItem;
    return ((SvMetaSlot *)GetRef())->GetRecordPerItem();
}
BOOL SvMetaSlot::GetRecordPerSet() const
{
    // Record- PerItem, No, PerSet und Manual sind exclusiv
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordPerSet;
    return ((SvMetaSlot *)GetRef())->GetRecordPerSet();
}
BOOL SvMetaSlot::GetRecordManual() const
{
    // Record- PerItem, No, PerSet und Manual sind exclusiv
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aRecordManual;
    return ((SvMetaSlot *)GetRef())->GetRecordManual();
}
BOOL SvMetaSlot::GetNoRecord() const
{
    // Record- PerItem, No, PerSet und Manual sind exclusiv
    if( !GetRef() || aRecordPerItem.IsSet() || aNoRecord.IsSet()
      || aRecordPerSet.IsSet() || aRecordManual.IsSet() )
        return aNoRecord;
    return ((SvMetaSlot *)GetRef())->GetNoRecord();
}
BOOL SvMetaSlot::GetRecordAbsolute() const
{
    if( !GetRef() || aRecordAbsolute.IsSet() )
        return aRecordAbsolute;
    return ((SvMetaSlot *)GetRef())->GetRecordAbsolute();
}
BOOL SvMetaSlot::GetHasDialog() const
{
    if( aHasDialog.IsSet() || !GetRef() ) return aHasDialog;
    return ((SvMetaSlot *)GetRef())->GetHasDialog();
}
const ByteString & SvMetaSlot::GetPseudoPrefix() const
{
    if( aPseudoPrefix.Len() || !GetRef() ) return aPseudoPrefix;
    return ((SvMetaSlot *)GetRef())->GetPseudoPrefix();
}
BOOL SvMetaSlot::GetMenuConfig() const
{
    if( aMenuConfig.IsSet() || !GetRef() ) return aMenuConfig;
    return ((SvMetaSlot *)GetRef())->GetMenuConfig();
}
BOOL SvMetaSlot::GetToolBoxConfig() const
{
    if( aToolBoxConfig.IsSet() || !GetRef() ) return aToolBoxConfig;
    return ((SvMetaSlot *)GetRef())->GetToolBoxConfig();
}
BOOL SvMetaSlot::GetStatusBarConfig() const
{
    if( aStatusBarConfig.IsSet() || !GetRef() ) return aStatusBarConfig;
    return ((SvMetaSlot *)GetRef())->GetStatusBarConfig();
}
BOOL SvMetaSlot::GetAccelConfig() const
{
    if( aAccelConfig.IsSet() || !GetRef() ) return aAccelConfig;
    return ((SvMetaSlot *)GetRef())->GetAccelConfig();
}
BOOL SvMetaSlot::GetFastCall() const
{
    if( aFastCall.IsSet() || !GetRef() ) return aFastCall;
    return ((SvMetaSlot *)GetRef())->GetFastCall();
}
BOOL SvMetaSlot::GetContainer() const
{
    if( aContainer.IsSet() || !GetRef() ) return aContainer;
    return ((SvMetaSlot *)GetRef())->GetContainer();
}

BOOL SvMetaSlot::GetPlugComm() const
{
    if( aPlugComm.IsSet() || !GetRef() ) return aPlugComm;
    return ((SvMetaSlot *)GetRef())->GetPlugComm();
}

const ByteString& SvMetaSlot::GetUnoName() const
{
    if( aUnoName.IsSet() || !GetRef() ) return aUnoName;
    return ((SvMetaSlot *)GetRef())->GetUnoName();
}

/*************************************************************************
|*    SvMetaSlot::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaSlot::FillSbxObject( SvIdlDataBase & rBase, SbxObject * pObj,
                                BOOL bVariable )
{
    // keine Attribut fuer Automation
    if( !GetAutomation() || !GetExport() )
        return;

    if( !bVariable )
    {
        SvMetaAttributeRef xM = GetMethod();
        if( xM.Is() )
        {
            SvMetaType *    pType = xM->GetType();
            SvMetaType *    pRetBaseType = pType->GetReturnType()->GetBaseType();
            ByteString          aName = xM->GetName();

            SbxMethodRef xMeth = new SbxMethod( aName,
                                    pRetBaseType->GetSbxDataType() );
            pType->FillSbxObject( xMeth, bVariable );
            xMeth->SetUserData( MakeSlotValue(rBase, FALSE) );

            pObj->Insert( &xMeth );
            return;
        }
    }

    SvMetaAttribute::FillSbxObject( rBase, pObj, bVariable );
}
 */

#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaSlot::ReadAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaSlot::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    SvMetaAttribute::ReadAttributesSvIdl( rBase, rInStm );

    BOOL bOk = FALSE;
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
        SetCachable( aCachable ), bOk = TRUE;
    if( aVolatile.ReadSvIdl( SvHash_Volatile(), rInStm ) )
        SetVolatile( aVolatile ), bOk = TRUE;
    if( aToggle.ReadSvIdl( SvHash_Toggle(), rInStm ) )
        SetToggle( aToggle ), bOk = TRUE;
    if( aAutoUpdate.ReadSvIdl( SvHash_AutoUpdate(), rInStm ) )
        SetAutoUpdate( aAutoUpdate ), bOk = TRUE;

    if( aSynchron.ReadSvIdl( SvHash_Synchron(), rInStm ) )
        SetSynchron( aSynchron ), bOk = TRUE;
    if( aAsynchron.ReadSvIdl( SvHash_Asynchron(), rInStm ) )
        SetAsynchron( aAsynchron ), bOk = TRUE;

    if( aRecordAbsolute.ReadSvIdl( SvHash_RecordAbsolute(), rInStm ) )
        SetRecordAbsolute( aRecordAbsolute), bOk = TRUE;
    if( aRecordPerItem.ReadSvIdl( SvHash_RecordPerItem(), rInStm ) )
        SetRecordPerItem( aRecordPerItem ), bOk = TRUE;
    if( aRecordPerSet.ReadSvIdl( SvHash_RecordPerSet(), rInStm ) )
        SetRecordPerSet( aRecordPerSet ), bOk = TRUE;
    if( aRecordManual.ReadSvIdl( SvHash_RecordManual(), rInStm ) )
        SetRecordManual( aRecordManual ), bOk = TRUE;
    if( aNoRecord.ReadSvIdl( SvHash_NoRecord(), rInStm ) )
        SetNoRecord( aNoRecord ), bOk = TRUE;

    bOk |= aHasDialog.ReadSvIdl( SvHash_HasDialog(), rInStm );
    bOk |= aPseudoPrefix.ReadSvIdl( SvHash_PseudoPrefix(), rInStm );
    bOk |= aMenuConfig.ReadSvIdl( SvHash_MenuConfig(), rInStm );
    bOk |= aToolBoxConfig.ReadSvIdl( SvHash_ToolBoxConfig(), rInStm );
    bOk |= aStatusBarConfig.ReadSvIdl( SvHash_StatusBarConfig(), rInStm );
    bOk |= aAccelConfig.ReadSvIdl( SvHash_AccelConfig(), rInStm );

    SvBOOL aAllConfig;
    if( aAllConfig.ReadSvIdl( SvHash_AllConfig(), rInStm ) )
        SetAllConfig( aAllConfig ), bOk = TRUE;
    bOk |= aFastCall.ReadSvIdl( SvHash_FastCall(), rInStm );
    bOk |= aContainer.ReadSvIdl( SvHash_Container(), rInStm );
    bOk |= aPlugComm.ReadSvIdl( SvHash_PlugComm(), rInStm );
    bOk |= aUnoName.ReadSvIdl( SvHash_UnoName(), rInStm );

    if( !bOk )
    {
        if( !aSlotType.Is() )
        {
            UINT32 nTokPos = rInStm.Tell();
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->Is( SvHash_SlotType() ) )
            {
                BOOL bBraket = rInStm.Read( '(' );
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
                UINT32 nTokPos = rInStm.Tell();
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

/*************************************************************************
|*    SvMetaSlot::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaSlot::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                       SvStream & rOutStm,
                       USHORT nTab )
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
        char * p = "";
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
        // wegen Default == TRUE, nur wenn kein anderer gesetzt
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
        // wegen Default == TRUE, nur wenn kein anderer gesetzt
        ( aOut += aDel ) += aSynchron.GetSvIdlString( SvHash_Synchron() );
    else
        aDel1 = aDel;

    aDel = ", ";
    if( aRecordManual )
        ( aOut += aDel1 ) += aRecordManual.GetSvIdlString( SvHash_RecordManual() );
    else if( aNoRecord )
        ( aOut += aDel1 ) += aNoRecord.GetSvIdlString( SvHash_NoRecord() );
    else if( !aRecordPerSet )
        // wegen Default == TRUE, nur wenn kein anderer gesetzt
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
    if( aPlugComm )
    {
        ( aOut += aDel ) += aPlugComm.GetSvIdlString( SvHash_PlugComm() );
        aDel = ", ";
    }

    if( aOut.Len() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << aOut.GetBuffer() << endl;
    }
}


/*************************************************************************
|*    SvMetaSlot::Test()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaSlot::Test( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    BOOL bOk = SvMetaAttribute::Test( rBase, rInStm );
    if( bOk )
    {
        SvMetaType * pType = GetType();
        if( pType->GetType() == TYPE_METHOD )
            pType = pType->GetReturnType();
        SvMetaType * pBaseType = pType->GetBaseType();
        if( !pType->IsItem() )
        {
            rBase.SetError( "this attribute is not a slot", rInStm.GetToken() );
            rBase.WriteError( rInStm );
            bOk = FALSE;
        }
    }

/*
    if ( bOk )
    {
        SvMetaSlot* pRef = (SvMetaSlot*) GetRef();
        if ( pRef )
            CompareSlotAttributes( pRef );
    }
*/
    return bOk;
}

/*************************************************************************
|*    SvMetaSlot::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaSlot::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    UINT32  nTokPos     = rInStm.Tell();
    BOOL bOk = TRUE;

    SvMetaAttribute * pAttr = rBase.ReadKnownAttr( rInStm, GetType() );
    if( pAttr )
    {
        // F"ur Testzwecke: Referenz bei Kurz-Syntax
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
            bOk = FALSE;
        }
    }
    else
    {
        bOk = SvMetaAttribute::ReadSvIdl( rBase, rInStm );

        SvMetaAttribute *pAttr = rBase.SearchKnownAttr( GetSlotId() );
        if( pAttr )
        {
            // F"ur Testzwecke: Referenz bei kompletter Definition
            SvMetaSlot * pKnownSlot = PTR_CAST( SvMetaSlot, pAttr );
            if( pKnownSlot )
            {
                SetRef( pKnownSlot );

                // Namen d"urfen abweichen, da mit angegeben
//              SetName( pKnownSlot->GetName(), &rBase );
            }
            else
            {
                ByteString aStr( "attribute " );
                aStr += pAttr->GetName();
                aStr += " is method or variable but not a slot";
                rBase.SetError( aStr, rInStm.GetToken() );
                rBase.WriteError( rInStm );
                bOk = FALSE;
            }
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );

    return bOk;
}

/*************************************************************************
|*    SvMetaSlot::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaSlot::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab )
{
    SvMetaAttribute::WriteSvIdl( rBase, rOutStm, nTab );
}

/*************************************************************************
|*    SvMetaSlot::Write()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaSlot::Write( SvIdlDataBase & rBase,
                        SvStream & rOutStm, USHORT nTab,
                        WriteType nT, WriteAttribute nA )
{
    if ( nT == WRITE_DOCU )
    {
        if ( GetHidden() )
            return;
    }
    else
    {
        // keine Attribut fuer Automation
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
    // Einf"ugeposition durch bin"are Suche in der SlotListe ermitteln
    USHORT nId = (USHORT) GetSlotId().GetValue();
    USHORT nListCount = (USHORT) rList.Count();
    USHORT nPos;
    if ( !nListCount )
        nPos = 0;
    else if ( nListCount == 1 )
        nPos = rList.GetObject(0)->xSlot->GetSlotId().GetValue() >= nId ? 0 : 1;
    else
    {
        USHORT nMid, nLow = 0;
        USHORT nHigh = nListCount - 1;
        BOOL bFound = FALSE;
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
                bFound = TRUE;
        }

        DBG_ASSERT(!bFound, "SlotId ist doppelt !");
        nPos = bFound ? nMid : nLow;
    }

    DBG_ASSERT( nPos <= nListCount,
        "nPos zu groá" );
    DBG_ASSERT( nPos == nListCount || nId <=
        (USHORT) rList.GetObject(nPos)->xSlot->GetSlotId().GetValue(),
        "Nachfolger hat kleinere SlotId" );
    DBG_ASSERT( nPos == 0 || nId >
        (USHORT) rList.GetObject(nPos-1)->xSlot->GetSlotId().GetValue(),
        "Vorg„nger hat gr”áere SlotId" );
    DBG_ASSERT( nPos+1 >= nListCount || nId <
        (USHORT) rList.GetObject(nPos+1)->xSlot->GetSlotId().GetValue(),
        "Nachfolger hat kleinere SlotId" );

    rList.Insert( new SvSlotElement( this, rPrefix ), nPos );

    // EnumSlots plattklopfen
    SvMetaTypeEnum * pEnum = NULL;
    SvMetaType * pBType = GetType()->GetBaseType();
    pEnum = PTR_CAST( SvMetaTypeEnum, pBType );
    if( GetPseudoSlots() && pEnum && pEnum->Count() )
    {
        // Den MasterSlot clonen
        SvMetaSlotRef xEnumSlot = Clone();
        SvMetaSlot *pFirstEnumSlot = xEnumSlot;
        for( ULONG n = 0; n < pEnum->Count(); n++ )
        {
            // Die Slaves sind kein Master !
            xEnumSlot->aPseudoSlots = FALSE;

            // Die SlotId erzeugen
            SvMetaEnumValue *pEnumValue = pEnum->GetObject(n);
            xEnumSlot->SetEnumValue(pEnumValue);
            ByteString aValName = pEnumValue->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            USHORT nLen = pEnum->GetPrefix().Len();
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            ULONG nValue;
            if ( rBase.FindId(aSId , &nValue) )
            {
                SvNumberIdentifier aId;
                *((SvIdentifier*)&aId) = aSId;
                aId.SetValue(nValue);
                xEnumSlot->SetSlotId(aId);
            }

            if ( xEnumSlot->GetSlotId().GetValue() <
                 pFirstEnumSlot->GetSlotId().GetValue() )
            {
                pFirstEnumSlot = xEnumSlot;
            }

            // Den erzeugten Slave ebenfalls einf"ugen
            xEnumSlot->Insert( rList, rPrefix, rBase);

            // Die EnumSlots mit dem Master verketten
            xEnumSlot->pLinkedSlot = this;

            // N"achster EnumSlot
            if ( n != pEnum->Count() - 1 )
                xEnumSlot = Clone();
        }

        // Master zeigt auf den ersten Slave
        pLinkedSlot = pFirstEnumSlot;

        // Slaves untereinander verketten
        rList.Seek(0UL);
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


/*************************************************************************
|*    SvMetaSlot::WriteSlotMap()
|*
|*    Beschreibung
*************************************************************************/
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
/*
    ByteString aName = GetName();
    SvMetaAttribute * pAttr = rAttrList.First();
    while( pAttr )
    {
        if( pAttr->GetName() == aName )
            break;
        pAttr = rAttrList.Next();
    }
    if( pAttr )
        return;
*/
    if ( !GetExport() && !GetHidden() )
        return;

    ByteString aMethodName( GetExecMethod() );
    if ( aMethodName.Len() && aMethodName != "NoExec" )
    {
        BOOL bIn = FALSE;
        for( USHORT n = 0; n < rList.Count(); n++ )
        {
            if( *(rList.GetObject(n)) == aMethodName )
            {
                bIn=TRUE;
                break;
            }
        }

        if ( !bIn )
        {
            rList.Insert( new ByteString(aMethodName), LIST_APPEND );
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
        BOOL bIn = FALSE;
        for ( USHORT n=0; n < rList.Count(); n++ )
        {
            if ( *(rList.GetObject(n)) == aMethodName )
            {
                bIn=TRUE;
                break;
            }
        }

        if ( !bIn )
        {
            rList.Insert( new ByteString(aMethodName), LIST_APPEND );
            rOutStm << "SFX_STATE_STUB("
                    << rShellName.GetBuffer()
                    << ','
                    << aMethodName.GetBuffer()
                    << ')' << endl;
        }
    }
}

void SvMetaSlot::WriteSlot( const ByteString & rShellName, USHORT nCount,
                            const ByteString & rSlotId,
                            const ByteString & rValueName,
                            SvSlotElementList& rSlotList,
                            const ByteString & rPrefix,
                            SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if ( !GetExport() && !GetHidden() )
        return;

//  BOOL bIsEnumSlot = 0 != rValueName.Len();
    BOOL bIsEnumSlot = 0 != pEnumValue;

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
        // Den n"achsten Slot suchen, der die gleiche StateMethod hat wie ich
        // Die SlotList ist auf den aktuellen Slot geseekt
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
            // Es gibt nach mir keinen Slot mehr, der die gleiche ExecMethode
            // hat. Also suche ich den ersten Slot, der diese hatte (das
            // k"onnte auch ich selbst sein)
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

        // ExecMethod schreiben, wenn nicht angegeben, standard Namen
        if( GetExecMethod().Len() && GetExecMethod() != "NoExec")
            rOutStm << "SFX_STUB_PTR(" << rShellName.GetBuffer() << ','
                    << GetExecMethod().GetBuffer() << ')';
        else
            rOutStm << "SFX_STUB_PTR_EXEC_NONE";
        rOutStm << ',';

        // StateMethod schreiben, wenn nicht angegeben, standard Namen
        if( GetStateMethod().Len() && GetStateMethod() != "NoState")
            rOutStm << "SFX_STUB_PTR(" << rShellName.GetBuffer() << ','
                    << GetStateMethod().GetBuffer() << ')';
        else
            rOutStm << "SFX_STUB_PTR_STATE_NONE";
    }
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 4 );

    // Flags schreiben
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
    if( GetPlugComm() )
        rOutStm << MakeSlotName( SvHash_PlugComm() ).GetBuffer() << '|';

    rOutStm << '0';

    rOutStm << ',' << endl;
       WriteTab( rOutStm, 4 );
    if ( !GetDisableFlags().Len() )
        rOutStm << "0";
    else
        rOutStm << GetDisableFlags().GetBuffer();

    // Attribut Typ schreiben
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
            ULONG nSCount = pType->GetAttrCount();
            rOutStm << ByteString::CreateFromInt32( nSCount ).GetBuffer() << "/*Count*/";
        }
        else
            rOutStm << '0';

        // Name f"urs Recording
        if ( GetExport() )
        {
            rOutStm << ",\"";
            if( rPrefix.Len() )
                rOutStm << rPrefix.GetBuffer();
            rOutStm << '.';
            if ( !IsVariable() || !GetType() ||
                 GetType()->GetBaseType()->GetType() != TYPE_STRUCT )
                rOutStm << GetMangleName( FALSE ).GetBuffer();
            rOutStm << "\",";
        }
        else
            rOutStm << ", 0, ";

        // Method/Property Flags
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

    if ( GetUnoName().Len() )
    {
        rOutStm << ",\"";
        rOutStm << GetUnoName().GetBuffer();
        rOutStm << "\"";
    }
    else
        rOutStm << ", 0";

    rOutStm << " )," << endl;
}

USHORT SvMetaSlot::WriteSlotParamArray( SvIdlDataBase & rBase, SvStream & rOutStm )
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
        for( ULONG n = 0; n < rList.Count(); n++ )
        {
            SvMetaAttribute * pPar  = rList.GetObject( n );
            SvMetaType * pPType     = pPar->GetType();
            WriteTab( rOutStm, 1 );
            rOutStm << "SFX_ARGUMENT("
                << pPar->GetSlotId().GetBuffer() << ',' // SlodId
                // Parameter Name
                << "\"" << pPar->GetName().GetBuffer() << "\","
                // Item Name
                << pPType->GetName().GetBuffer() << ")," << endl;
            if( !rBase.FindType( pPType, rBase.aUsedTypes ) )
                rBase.aUsedTypes.Append( pPType );
        }
        return (USHORT)rList.Count();
    }
    return 0;
}

USHORT SvMetaSlot::WriteSlotMap( const ByteString & rShellName, USHORT nCount,
                                SvSlotElementList& rSlotList,
                                const ByteString & rPrefix,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    // SlotId, wenn nicht angegeben, aus Namen generieren
    ByteString aSlotId = GetSlotId();

    USHORT nSCount = 0;
    if( IsMethod() )
    {
        SvMetaType * pType;
        SvMetaAttribute * pMethod = GetMethod();
        if( pMethod )
            pType = pMethod->GetType();
        else
            pType = GetType();

        nSCount = (USHORT)pType->GetAttrCount();
    }

    WriteSlot( rShellName, nCount, aSlotId, ByteString(),
                rSlotList, rPrefix, rBase, rOutStm );
/*
    SvMetaTypeEnum * pEnum = NULL;
    SvMetaType * pBType = GetType()->GetBaseType();
    pEnum = PTR_CAST( SvMetaTypeEnum, pBType );
    if( aPseudoSlots && pEnum )
    {
        rBase.nMasterPos = rBase.nSlotPos-1;
        rBase.nEnumPos = rBase.nMasterPos + pEnum->Count() + 1;

        for( ULONG n = 0; n < pEnum->Count(); n++ )
        {
            ByteString aValName = pEnum->GetObject( n )->GetName();
            ByteString aSId( aSlotId );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            USHORT nLen = pEnum->GetPrefix().Len();
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );
            WriteSlot( rShellName, nCount + nSCount, aSId, aValName, rSlotList, rBase, rOutStm );
        }
    }
*/
    return nSCount;
}

/*************************************************************************
|*    SvMetaSlot::WriteSrc()
*************************************************************************/
void SvMetaSlot::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    if (!GetToolBoxConfig() && !GetAccelConfig() && !GetMenuConfig() && !GetStatusBarConfig() )
        return;

    ULONG nSId = GetSlotId().GetValue();
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
        for( ULONG n = 0; n < pEnum->Count(); n++ )
        {
            ByteString aValName = pEnum->GetObject( n )->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            USHORT nLen = pEnum->GetPrefix().Len();
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            ULONG nSId;
            BOOL bIdOk = FALSE;
            if( rBase.FindId( aSId, &nSId ) )
            {
                aSId = ByteString::CreateFromInt32( nSId );
                bIdOk = TRUE;
            }

            // wenn Id nicht gefunden, immer schreiben
            if( !bIdOk || !pTable->IsKeyValid( nSId ) )
            {
                pTable->Insert( nSId, this );
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
    ULONG nSId = GetSlotId().GetValue();
    if( !pTable->IsKeyValid( nSId ) )
    {
        pTable->Insert( nSId, this );
        rOutStm << "#define " << GetSlotId().GetBuffer() << '\t' << ByteString::CreateFromInt32( nSId ).GetBuffer() << endl;
    }

    SvMetaTypeEnum * pEnum = PTR_CAST( SvMetaTypeEnum, GetType() );
    if( GetPseudoSlots() && pEnum )
    {
        for( ULONG n = 0; n < pEnum->Count(); n++ )
        {
            ByteString aValName = pEnum->GetObject( n )->GetName();
            ByteString aSId( GetSlotId() );
            if( GetPseudoPrefix().Len() )
                aSId = GetPseudoPrefix();
            aSId += '_';
            USHORT nLen = pEnum->GetPrefix().Len();
            aSId += aValName.Copy( pEnum->GetPrefix().Len() );

            ULONG nSId;
            BOOL bIdOk = FALSE;
            if( rBase.FindId( aSId, &nSId ) )
            {
                aSId = ByteString::CreateFromInt32( nSId );
                bIdOk = TRUE;
            }

            // wenn Id nicht gefunden, immer schreiben
            if( !bIdOk || !pTable->IsKeyValid( nSId ) )
            {
                pTable->Insert( nSId, this );

                rOutStm << "#define " << aSId.GetBuffer() << '\t'
                        << ByteString::CreateFromInt32( nSId ).GetBuffer() << endl;
            }
        }
    }
}


void SvMetaSlot::CompareSlotAttributes( SvMetaSlot* pSlot )
{
    ByteString aStr = Compare( pSlot );

    if ( GetMenuConfig() || GetToolBoxConfig() || GetAccelConfig() || GetStatusBarConfig() )
    {
        if ( GetHelpText().Len() )
        {
            ByteString aRefText( pSlot->GetHelpText() );
            ByteString aText( GetHelpText() );
            if ( !aText.Equals( aRefText ) )
            {
                aStr += "    HelpText\n";
                aStr += aText;
                aStr += '\n';
                aStr += aRefText;
                aStr += '\n';
            }
        }

        if ( GetConfigName().Len() )
        {
            ByteString aRefText( pSlot->GetConfigName() );
            ByteString aText( GetConfigName() );
            if ( !aText.Equals( aRefText ) )
            {
                aStr += "    ConfigName\n";
                aStr += aText;
                aStr += '\n';
                aStr += aRefText;
                aStr += '\n';
            }
        }
    }

    if ( GetExport() && !GetName().Equals( pSlot->GetName() ) )
    {
        aStr += "    Name\n";
        aStr += GetName();
        aStr += '\n';
        aStr += pSlot->GetName();
        aStr += '\n';
    }

    if ( (SvMetaType *)GetSlotType() != (SvMetaType *)pSlot->GetSlotType() )
        aStr += "    SlotType\n";

    if ( GetMethod() && !pSlot->GetMethod() ||
         !GetMethod() && pSlot->GetMethod() )
        aStr += "    Method\n";

    if ( GetMethod() && pSlot->GetMethod() )
        GetMethod()->Compare( pSlot->GetMethod() );

    if ( !GetGroupId().Equals( pSlot->GetGroupId() ) )
        aStr += "    GroupId\n";

    if ( GetHasCoreId() != (BOOL) pSlot->GetHasCoreId() )
        aStr += "    HasCoreId\n";

    if ( !GetConfigId().Equals( pSlot->GetConfigId() ) )
        aStr += "    ConfigId\n";

    if ( !GetDefault().Equals( pSlot->GetDefault() ) )
        aStr += "    Default\n";

    if ( GetPseudoSlots() != pSlot->GetPseudoSlots() )
        aStr += "    PseudoSlots\n";

    if ( GetCachable() != pSlot->GetCachable() )
        aStr += "    Cachable\n";

    if ( GetVolatile() != pSlot->GetVolatile() )
        aStr += "    Volatile";

    if ( GetToggle() != pSlot->GetToggle() )
        aStr += "    Toggle\n";

    if ( GetAutoUpdate() != pSlot->GetAutoUpdate() )
        aStr += "    AutoUpdate\n";

    if ( GetSynchron() != pSlot->GetSynchron() )
        aStr += "    Synchron\n";

    if ( GetAsynchron() != pSlot->GetAsynchron() )
        aStr += "    Asynchron\n";

    if ( GetRecordPerItem() != pSlot->GetRecordPerItem() )
        aStr += "    RecordPerItem\n";

    if ( GetRecordPerSet() != pSlot->GetRecordPerSet() )
        aStr += "    RecordPerSet\n";

    if ( GetRecordManual() != pSlot->GetRecordManual() )
        aStr += "    RecordManual\n";

    if ( GetNoRecord() != pSlot->GetNoRecord() )
        aStr += "    NoRecord\n";

    if ( GetRecordAbsolute() != pSlot->GetRecordAbsolute() )
        aStr += "    RecordAbsolute\n";

    if ( GetHasDialog() != pSlot->GetHasDialog() )
        aStr += "    HasDialog\n";

    if ( GetMenuConfig() != pSlot->GetMenuConfig() )
        aStr += "    MenuConfig\n";

    if ( GetToolBoxConfig() != pSlot->GetToolBoxConfig() )
        aStr += "    ToolBoxConfig\n";

    if ( GetStatusBarConfig() != pSlot->GetStatusBarConfig() )
        aStr += "    StatusBarConfig\n";

    if ( GetAccelConfig() != pSlot->GetAccelConfig() )
        aStr += "    AccelConfig\n";

    if ( pSlot->GetFastCall() && !GetFastCall() )
        aStr += "    FastCall\n";

    if ( GetContainer() != pSlot->GetContainer() )
        aStr += "    Container\n";

    if ( GetPlugComm() != pSlot->GetPlugComm() )
        aStr += "    PlugComm\n";

    if ( !GetPseudoPrefix().Equals( pSlot->GetPseudoPrefix() ) )
        aStr += "    PseudoPrefix\n";

    if ( IsVariable() != pSlot->IsVariable() )
        aStr += "    Variable\n";

    if ( aStr.Len() )
    {
        DBG_ERROR( GetSlotId().GetBuffer() );
        DBG_ERROR( aStr.GetBuffer() );
    }
}

void WriteBool( BOOL bSet, SvStream& rStream )
{
    if ( bSet )
        rStream << "TRUE" << ',';
    else
        rStream << "FALSE" << ',';
}

void SvMetaSlot::WriteCSV( SvIdlDataBase& rBase, SvStream& rStrm )
{
    rStrm << "PROJECT" << ",,";
    if ( GetSlotType() )
        rStrm << GetSlotType()->GetName().GetBuffer() << ',';
    else
        rStrm << ',';

    rStrm << GetType()->GetSvName().GetBuffer() << ',';
    rStrm << GetName().GetBuffer() << ',';
    rStrm << GetUnoName().GetBuffer() << ',';
    rStrm << GetSlotId().GetBuffer() << ',';
    USHORT nId = (USHORT) GetSlotId().GetValue();
    rStrm << ByteString::CreateFromInt32( GetSlotId().GetValue() ).GetBuffer() << ',';
    rStrm << "\"" << GetConfigName().GetBuffer() << "\"" << ',';
    rStrm << "\"" << GetHelpText().GetBuffer()   << "\"" << ',';

    rStrm << ",,,";

    WriteBool( GetAccelConfig(), rStrm );
    WriteBool( GetAutomation(), rStrm );
    WriteBool( GetAutoUpdate(), rStrm );

    if ( GetCachable() )
        rStrm << "Cachable" << ',';
    else
        rStrm << "Volatile" << ',';

    WriteBool( GetContainer(), rStrm );
    WriteBool( GetExport(), rStrm );
    WriteBool( GetFastCall(), rStrm );
    WriteBool( GetHidden(), rStrm );

    rStrm << GetGroupId().GetBuffer() << ',';

    rStrm << ',';
    rStrm << GetDisableFlags().GetBuffer() << ',';

    WriteBool( GetHasCoreId(), rStrm );
    WriteBool( GetHasDialog(), rStrm );
    WriteBool( GetIsCollection(), rStrm );
    WriteBool( GetMenuConfig(), rStrm );
    WriteBool( GetPlugComm(), rStrm );
    WriteBool( GetReadonly(), rStrm );
    WriteBool( GetReadOnlyDoc(), rStrm );

    if( GetRecordPerSet() )
        rStrm << "RecordPerSet" << ',';
    else
        rStrm << "RecordPerItem" << ',';

    WriteBool( GetRecordAbsolute(), rStrm );

    rStrm << ',';

    WriteBool( GetStatusBarConfig(), rStrm );

    if( GetSynchron() )
        rStrm << "Synchron" << ',';
    else
        rStrm << "Asynchron" << ',';

    WriteBool( GetToggle(), rStrm );
    WriteBool( GetToolBoxConfig(), rStrm );

    if ( GetPseudoPrefix().Len() )
        rStrm << GetPseudoPrefix().GetBuffer() << ',';
    else
        rStrm << ',';

    rStrm << ',';

    if ( GetType()->GetType() != TYPE_METHOD && GetMethod() )
    {
        rStrm << GetMethod()->GetType()->GetReturnType()->GetName().GetBuffer() << ',';
        rStrm << GetMethod()->GetName().GetBuffer() << ',';
    }
    else
    {
        rStrm << ",,";
    }

    rStrm << "TRUE,";

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


