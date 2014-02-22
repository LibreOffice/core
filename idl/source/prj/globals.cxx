/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/shl.hxx>

#include <globals.hxx>
#include <database.hxx>

IdlDll * GetIdlApp()
{
    if( !(*(IdlDll**)GetAppData(SHL_IDL)) )
    {
        (*(IdlDll**)GetAppData(SHL_IDL)) = new IdlDll();
    }
    return (*(IdlDll**)GetAppData(SHL_IDL));
}

IdlDll::IdlDll()
    : pHashTable( NULL )
    , pClassMgr( NULL )
    , pGlobalNames( NULL )

{
    pClassMgr   = new SvClassManager();
    
    pClassMgr->SV_CLASS_REGISTER( SvMetaAttribute );
    pClassMgr->SV_CLASS_REGISTER( SvMetaClass );
    
    pClassMgr->SV_CLASS_REGISTER( SvMetaSlot );
    
    pClassMgr->SV_CLASS_REGISTER( SvMetaModule );
    
    pClassMgr->SV_CLASS_REGISTER( SvMetaObject );
    pClassMgr->SV_CLASS_REGISTER( SvMetaName );
    pClassMgr->SV_CLASS_REGISTER( SvMetaExtern );
    pClassMgr->SV_CLASS_REGISTER( SvMetaReference );
    
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

inline SvStringHashEntry * INS( const OString& rName )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
