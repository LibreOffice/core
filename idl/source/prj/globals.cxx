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

/****************** I N C L U D E S **************************************/
#include <tools/shl.hxx>

#include <globals.hxx>
#include <database.hxx>

/****************** G L O B A L S ****************************************/
IdlDll * GetIdlApp()
{
    if( !(*(IdlDll**)GetAppData(SHL_IDL)) )
    {
        (*(IdlDll**)GetAppData(SHL_IDL)) = new IdlDll();
        // Temporaer, sonst nur wenn Compiler
        //GetIdlApp()->pHashTable      = new SvStringHashTable( 2801 );
        //GetIdlApp()->pGlobalNames    = new SvGlobalHashNames();
    }
    return (*(IdlDll**)GetAppData(SHL_IDL));
}

IdlDll::IdlDll()
    : pHashTable( NULL )
    , pClassMgr( NULL )
    , pGlobalNames( NULL )

{
    pClassMgr   = new SvClassManager();
    // File OBJECT.HXX:
    pClassMgr->SV_CLASS_REGISTER( SvMetaAttribute );
    pClassMgr->SV_CLASS_REGISTER( SvMetaClass );
    //File SLOT.HXX:
    pClassMgr->SV_CLASS_REGISTER( SvMetaSlot );
    //File MODULE.HXX:
    pClassMgr->SV_CLASS_REGISTER( SvMetaModule );
    //File BASOBJ.HXX:
    pClassMgr->SV_CLASS_REGISTER( SvMetaObject );
    pClassMgr->SV_CLASS_REGISTER( SvMetaName );
    pClassMgr->SV_CLASS_REGISTER( SvMetaExtern );
    pClassMgr->SV_CLASS_REGISTER( SvMetaReference );
    //File TYPES.HXX:
    pClassMgr->SV_CLASS_REGISTER( SvMetaType );
    pClassMgr->SV_CLASS_REGISTER( SvMetaTypeString );
    pClassMgr->SV_CLASS_REGISTER( SvMetaEnumValue );
    pClassMgr->SV_CLASS_REGISTER( SvMetaTypeEnum );
    pClassMgr->SV_CLASS_REGISTER( SvMetaTypevoid );
    pClassMgr->SV_CLASS_REGISTER( SvClassElement );
}

IdlDll::~IdlDll()
{
    delete pGlobalNames;
    delete pClassMgr;
    delete pHashTable;
}

/*************************************************************************
|*
|*    SvGlobalHashNames::SvGlobalHashNames()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.08.91
|*    Letzte Aenderung  MM 08.08.91
|*
*************************************************************************/
inline SvStringHashEntry * INS( const ByteString & rName )
{
    sal_uInt32  nIdx;
    IDLAPP->pHashTable->Insert( rName, &nIdx );
    return (SvStringHashEntry * )IDLAPP->pHashTable->Get( nIdx );
}
#define A_ENTRY( Name ) , MM_##Name( INS( #Name ) )

SvGlobalHashNames::SvGlobalHashNames()
    : MM_Name( INS( "Name" ) )
    , MM_module( INS( "module" ) )
    , MM_interface( INS( "interface" ) )
    , MM_in( INS( "in" ) )
    , MM_out( INS( "out" ) )
    , MM_inout( INS( "inout" ) )
    , MM_String( INS( "String" ) )
    , MM_UCHAR( INS( "UCHAR" ) )
    , MM_USHORT( INS( "USHORT" ) )
    , MM_uuid( INS( "uuid" ) )
    , MM_HelpContext( INS( "HelpContext" ) )
    , MM_HelpText( INS( "HelpText" ) )
    , MM_void( INS( "void" ) )
    , MM_shell( INS( "shell" ) )
    , MM_Get( INS( "Get" ) )
    , MM_Set( INS( "Set" ) )
    , MM_SlotId( INS( "SlotId" ) )
    , MM_HasCoreId( INS( "HasCoreId" ) )
    , MM_Cachable( INS( "Cachable" ) )
//    , MM_Volatile( INS( "Volatile" ) )
    , MM_Toggle( INS( "Toggle" ) )
    , MM_AutoUpdate( INS( "AutoUpdate" ) )
    , MM_Synchron( INS( "Synchron" ) )
    , MM_Asynchron( INS( "Asynchron" ) )
    A_ENTRY(RecordPerSet)
    A_ENTRY(RecordPerItem)
    A_ENTRY(RecordManual)
    A_ENTRY(NoRecord)
    A_ENTRY(RecordAbsolute)
    A_ENTRY(enum)
    A_ENTRY(UINT16)
    A_ENTRY(INT16)
    A_ENTRY(UINT32)
    A_ENTRY(INT32)
    A_ENTRY(int)
    A_ENTRY(BOOL)
    A_ENTRY(char)
    A_ENTRY(BYTE)
    A_ENTRY(float)
    A_ENTRY(double)
    A_ENTRY(item)
    A_ENTRY(PseudoSlots)
    A_ENTRY(map)
    A_ENTRY(Default)
    A_ENTRY(HelpFile)
    A_ENTRY(Version)
    A_ENTRY(import)
    A_ENTRY(SlotIdFile)
    A_ENTRY(SvName)
    A_ENTRY(SbxName)
    A_ENTRY(ItemName)
    A_ENTRY(OdlName)
    A_ENTRY(include)
    A_ENTRY(ExecMethod)
    A_ENTRY(StateMethod)
    A_ENTRY(GroupId)
    A_ENTRY(HasDialog)
    A_ENTRY(TypeLibFile)
    A_ENTRY(Export)
    A_ENTRY(Automation)
    A_ENTRY(PseudoPrefix)
    A_ENTRY(define)
    A_ENTRY(MenuConfig)
    A_ENTRY(ToolBoxConfig)
    A_ENTRY(StatusBarConfig)
    A_ENTRY(AccelConfig)
    A_ENTRY(AllConfig)
    A_ENTRY(FastCall)
    A_ENTRY(SbxObject)
    A_ENTRY(Container)
    A_ENTRY(ImageRotation)
    A_ENTRY(ImageReflection)
    A_ENTRY(IsCollection)
    A_ENTRY(ReadOnlyDoc)
    A_ENTRY(ConfigName)
    A_ENTRY(union)
    A_ENTRY(struct)
    A_ENTRY(typedef)
    A_ENTRY(Readonly)
    A_ENTRY(SlotType)
    A_ENTRY(ModulePrefix)
    A_ENTRY(DisableFlags)
    A_ENTRY(Hidden)
    A_ENTRY(Description)
    A_ENTRY(UnoName)
{}

