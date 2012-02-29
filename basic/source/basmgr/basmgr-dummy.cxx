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

#include <tools/stream.hxx>
#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbx.hxx>
#include <sot/storinfo.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <basic/sbmod.hxx>
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <basic/sbuno.hxx>
#include <basic/basmgr.hxx>
#include <basic/global.hxx>
#include <sbunoobj.hxx>
#include "basrid.hxx"
#include "sbintern.hxx"
#include <sb.hrc>

#include <vector>

#define LIB_SEP         0x01
#define LIBINFO_SEP     0x02
#define LIBINFO_ID      0x1491
#define PASSWORD_MARKER 0x31452134


// Library API, implemented for XML import/export

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include <com/sun/star/script/XStarBasicModuleInfo.hpp>
#include <com/sun/star/script/XStarBasicDialogInfo.hpp>
#include <com/sun/star/script/XStarBasicLibraryInfo.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>

#include <cppuhelper/implbase1.hxx>

using com::sun::star::uno::Reference;
using ::std::vector;
using ::std::advance;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace cppu;

typedef WeakImplHelper1< XNameContainer > NameContainerHelper;
typedef WeakImplHelper1< XStarBasicModuleInfo > ModuleInfoHelper;
typedef WeakImplHelper1< XStarBasicDialogInfo > DialogInfoHelper;
typedef WeakImplHelper1< XStarBasicLibraryInfo > LibraryInfoHelper;
typedef WeakImplHelper1< XStarBasicAccess > StarBasicAccessHelper;

// Version 1
//    sal_uIntPtr   nEndPos
//    sal_uInt16    nId
//    sal_uInt16    nVer
//    sal_Bool      bDoLoad
//    String    LibName
//    String    AbsStorageName
//    String    RelStorageName
// Version 2
//  + sal_Bool      bReference

TYPEINIT1( BasicManager, SfxBroadcaster );
DBG_NAME( BasicManager );

StreamMode eStreamReadMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL;
StreamMode eStorageReadMode = STREAM_READ | STREAM_SHARE_DENYWRITE;

//----------------------------------------------------------------------------
// BasicManager impl data
struct BasicManagerImpl
{
};

BasicError::BasicError( sal_uIntPtr nId, sal_uInt16 nR, const String& rErrStr ) :
    aErrStr( rErrStr )
{
    nErrorId    = nId;
    nReason     = nR;
}

BasicError::BasicError( const BasicError& rErr ) :
    aErrStr( rErr.aErrStr )
{
    nErrorId    = rErr.nErrorId;
    nReason     = rErr.nReason;
}


//=====================================================================

class BasicLibInfo
{
};


//=====================================================================

class BasicLibs
{
};

BasicManager::BasicManager( SotStorage& /* rStorage */, const String& /* rBaseURL */, StarBASIC* /* pParentFromStdLib */, String* /* pLibPath */, sal_Bool /* bDocMgr */ )
{
}

#if 0
const Reference< XPersistentLibraryContainer >& BasicManager::GetDialogLibraryContainer()  const
{
    static Reference< XPersistentLibraryContainer > dummy;

    return dummy;
}

const Reference< XPersistentLibraryContainer >& BasicManager::GetScriptLibraryContainer()  const
{
    static Reference< XPersistentLibraryContainer > dummy;

    return dummy;
}
#endif
void BasicManager::SetLibraryContainerInfo( const LibraryContainerInfo& /* rInfo */ )
{
}

BasicManager::BasicManager( StarBASIC* /* pSLib */, String* /* pLibPath */, sal_Bool /* bDocMgr */ )
{
}

void BasicManager::ImpMgrNotLoaded( const String& /* rStorageName */ )
{
}


void BasicManager::ImpCreateStdLib( StarBASIC* /* pParentFromStdLib */ )
{
}

void BasicManager::LoadBasicManager( SotStorage& /* rStorage */, const String& /* rBaseURL */, sal_Bool /* bLoadLibs */ )
{
}

void BasicManager::LoadOldBasicManager( SotStorage& /* rStorage */ )
{
}

BasicManager::~BasicManager()
{
    DBG_DTOR( BasicManager, 0 );

    // Notify listener if something needs to be saved
    Broadcast( SfxSimpleHint( SFX_HINT_DYING) );

    // Destroy Basic-Infos...
    // In reverse order
    delete pLibs;
    delete mpImpl;
}

void BasicManager::LegacyDeleteBasicManager( BasicManager*& _rpManager )
{
    delete _rpManager;
    _rpManager = NULL;
}


bool BasicManager::HasExeCode( const String& /* sLib */ )
{
    return false;
}

void BasicManager::Init()
{
}

BasicLibInfo* BasicManager::CreateLibInfo()
{
    return 0;
}

sal_Bool BasicManager::ImpLoadLibary( BasicLibInfo* /* pLibInfo */, SotStorage* /* pCurStorage */, sal_Bool /* bInfosOnly */ )
{
    return sal_False;
}

sal_Bool BasicManager::ImplEncryptStream( SvStream& /* rStrm */ ) const
{
    return sal_False;
}

// This code is necessary to load the BASIC of Beta 1
// TODO: Which Beta 1?
sal_Bool BasicManager::ImplLoadBasic( SvStream& /* rStrm */, StarBASICRef& /* rOldBasic */ ) const
{
    return sal_False;
}

void BasicManager::CheckModules( StarBASIC* /* pLib */, sal_Bool /* bReference */ ) const
{
}

StarBASIC* BasicManager::AddLib( SotStorage& /* rStorage */, const String& /* rLibName */, sal_Bool /* bReference */ )
{
    return 0;
}

sal_Bool BasicManager::IsReference( sal_uInt16 /* nLib */ )
{
    return sal_False;
}

sal_Bool BasicManager::RemoveLib( sal_uInt16 /* nLib */ )
{
    return sal_False;
}

sal_Bool BasicManager::RemoveLib( sal_uInt16 /* nLib */, sal_Bool /* bDelBasicFromStorage */ )
{
    return sal_False;
}

sal_uInt16 BasicManager::GetLibCount() const
{
    return 0;
}

StarBASIC* BasicManager::GetLib( sal_uInt16 /* nLib */ ) const
{
    return 0;
}

StarBASIC* BasicManager::GetStdLib() const
{
    return 0;
}

StarBASIC* BasicManager::GetLib( const String& /* rName */ ) const
{
    return 0;
}

sal_uInt16 BasicManager::GetLibId( const String& /* rName */ ) const
{
    return LIB_NOTFOUND;
}

sal_Bool BasicManager::HasLib( const String& /* rName */ ) const
{
    return sal_False;
}

sal_Bool BasicManager::SetLibName( sal_uInt16 /* nLib */, const String& /* rName */ )
{
    return sal_False;
}

String BasicManager::GetLibName( sal_uInt16 /* nLib */ )
{
    return String();
}

sal_Bool BasicManager::LoadLib( sal_uInt16 /* nLib */ )
{
    return sal_False;
}

StarBASIC* BasicManager::CreateLib( const String& /* rLibName */ )
{
    return 0;
}

// For XML import/export:
StarBASIC* BasicManager::CreateLib
( const String& /* rLibName */, const String& /* Password */, const String& /* LinkTargetURL */ )
{
    return 0;
}

StarBASIC* BasicManager::CreateLibForLibContainer( const String& /* rLibName */,
                                                   const Reference< XLibraryContainer >& /* xScriptCont */ )
{
    return 0;
}


BasicLibInfo* BasicManager::FindLibInfo( StarBASIC* /* pBasic */ ) const
{
    return 0;
}


sal_Bool BasicManager::IsModified() const
{
    return sal_False;
}

sal_Bool BasicManager::IsBasicModified() const
{
    return sal_False;
}

std::vector<BasicError>& BasicManager::GetErrors()
{
    static std::vector<BasicError> dummy;

    return dummy;
}

bool BasicManager::GetGlobalUNOConstant( const sal_Char* /* _pAsciiName */, ::com::sun::star::uno::Any& /* aOut */ )
{
    return false;
}

Any BasicManager::SetGlobalUNOConstant( const sal_Char* /* _pAsciiName */, const Any& /* _rValue */ )
{
    return Any();
}

bool BasicManager::LegacyPsswdBinaryLimitExceeded( ::com::sun::star::uno::Sequence< rtl::OUString >& /* _out_rModuleNames */ )
{
    return false;
}


bool BasicManager::HasMacro( String const& /* i_fullyQualifiedName */ ) const
{
    return false;
}

ErrCode BasicManager::ExecuteMacro( String const& /* i_fullyQualifiedName */, SbxArray* /* i_arguments */, SbxValue* /* retValue */ )
{
    return ERRCODE_BASIC_PROC_UNDEFINED;
}

ErrCode BasicManager::ExecuteMacro( String const& /* i_fullyQualifiedName */, String const& /* i_commaSeparatedArgs */, SbxValue* /* i_retValue */ )
{
    return ERRCODE_BASIC_PROC_UNDEFINED;
}

// Basic XML Import/Export
Reference< XStarBasicAccess > getStarBasicAccess( BasicManager* /* pMgr */ )
{
    static Reference< XStarBasicAccess > dummy;

    return dummy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
