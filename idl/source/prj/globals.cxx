/*************************************************************************
 *
 *  $RCSfile: globals.cxx,v $
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

/****************** I N C L U D E S **************************************/

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#include <globals.hxx>
#include <database.hxx>
#pragma hdrstop

/****************** G L O B A L S ****************************************/
#ifdef DOS
static IdlDll * pApp = NULL;
IdlDll * GetIdlApp()
{
    if( !pApp )
        pApp = new IdlDll();
    return pApp;
}
#else
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
#endif

IdlDll::IdlDll()
    : pHashTable( NULL )
    , pGlobalNames( NULL )
    , pClassMgr( NULL )

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
    UINT32  nIdx;
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
    , MM_Volatile( INS( "Volatile" ) )
    , MM_Toggle( INS( "Toggle" ) )
    , MM_AutoUpdate( INS( "AutoUpdate" ) )
    , MM_Synchron( INS( "Synchron" ) )
    , MM_Asynchron( INS( "Asynchron" ) )
    A_ENTRY(RecordPerItem)
    A_ENTRY(RecordPerSet)
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
    A_ENTRY(MenuConfig)
    A_ENTRY(ToolBoxConfig)
    A_ENTRY(StatusBarConfig)
    A_ENTRY(AccelConfig)
    A_ENTRY(AllConfig)
    A_ENTRY(FastCall)
    A_ENTRY(TypeLibFile)
    A_ENTRY(Export)
    A_ENTRY(Automation)
    A_ENTRY(PseudoPrefix)
    A_ENTRY(define)
    A_ENTRY(SbxObject)
    A_ENTRY(Container)
    A_ENTRY(PlugComm)
    A_ENTRY(IsCollection)
    A_ENTRY(ReadOnlyDoc)
    A_ENTRY(ConfigName)
    A_ENTRY(struct)
    A_ENTRY(union)
    A_ENTRY(typedef)
    A_ENTRY(Readonly)
    A_ENTRY(SlotType)
    A_ENTRY(ModulePrefix)
    A_ENTRY(DisableFlags)
    A_ENTRY(Hidden)
    A_ENTRY(Description)
    A_ENTRY(UnoName)
{}

