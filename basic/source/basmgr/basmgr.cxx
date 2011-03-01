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
#include "precompiled_basic.hxx"
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
#include <basic/sbobjmod.hxx>

#include <basic/sbuno.hxx>
#include <basic/basmgr.hxx>
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
typedef vector< BasicError* > BasErrorLst;


#define CURR_VER        2

// Version 1
//    ULONG     nEndPos
//    USHORT    nId
//    USHORT    nVer
//    BOOL      bDoLoad
//    String    LibName
//    String    AbsStorageName
//    String    RelStorageName
// Version 2
//  + BOOL      bReference

static const char* szStdLibName = "Standard";
static const char szBasicStorage[] = "StarBASIC";
static const char* szOldManagerStream = "BasicManager";
static const char szManagerStream[] = "BasicManager2";
static const char* szImbedded = "LIBIMBEDDED";
static const char* szCryptingKey = "CryptedBasic";
static const char* szScriptLanguage = "StarBasic";

TYPEINIT1( BasicManager, SfxBroadcaster );
DBG_NAME( BasicManager );

StreamMode eStreamReadMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL;
StreamMode eStorageReadMode = STREAM_READ | STREAM_SHARE_DENYWRITE;

//----------------------------------------------------------------------------
// BasicManager impl data
struct BasicManagerImpl
{
    LibraryContainerInfo    maContainerInfo;

    // Save stream data
    SvMemoryStream*  mpManagerStream;
    SvMemoryStream** mppLibStreams;
    sal_Int32        mnLibStreamCount;
    sal_Bool         mbModifiedByLibraryContainer;
    sal_Bool         mbError;

    BasicManagerImpl( void )
        : mpManagerStream( NULL )
        , mppLibStreams( NULL )
        , mnLibStreamCount( 0 )
        , mbModifiedByLibraryContainer( sal_False )
        , mbError( sal_False )
    {}
    ~BasicManagerImpl();
};

BasicManagerImpl::~BasicManagerImpl()
{
    delete mpManagerStream;
    if( mppLibStreams )
    {
        for( sal_Int32 i = 0 ; i < mnLibStreamCount ; i++ )
            delete mppLibStreams[i];
        delete[] mppLibStreams;
    }
}

//============================================================================
// BasMgrContainerListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > ContainerListenerHelper;

class BasMgrContainerListenerImpl: public ContainerListenerHelper
{
    BasicManager* mpMgr;
    ::rtl::OUString maLibName;      // empty -> no lib, but lib container

public:
    BasMgrContainerListenerImpl( BasicManager* pMgr, ::rtl::OUString aLibName )
        : mpMgr( pMgr )
        , maLibName( aLibName ) {}

    static void insertLibraryImpl( const Reference< XLibraryContainer >& xScriptCont, BasicManager* pMgr,
        Any aLibAny, ::rtl::OUString aLibName );
    static void addLibraryModulesImpl( BasicManager* pMgr, Reference< XNameAccess > xLibNameAccess,
        ::rtl::OUString aLibName );


    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source )
        throw(::com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event )
        throw(::com::sun::star::uno::RuntimeException);
};


//============================================================================
// BasMgrContainerListenerImpl
//============================================================================

void BasMgrContainerListenerImpl::insertLibraryImpl( const Reference< XLibraryContainer >& xScriptCont,
    BasicManager* pMgr, Any aLibAny, ::rtl::OUString aLibName )
{
    Reference< XNameAccess > xLibNameAccess;
    aLibAny >>= xLibNameAccess;

    if( !pMgr->GetLib( aLibName ) )
    {
        BasicManager* pBasMgr = static_cast< BasicManager* >( pMgr );
#ifdef DBG_UTIL
        StarBASIC* pLib =
#endif
        pBasMgr->CreateLibForLibContainer( aLibName, xScriptCont );
        DBG_ASSERT( pLib, "XML Import: Basic library could not be created");
    }

    Reference< XContainer> xLibContainer( xLibNameAccess, UNO_QUERY );
    if( xLibContainer.is() )
    {
        // Register listener for library
        Reference< XContainerListener > xLibraryListener
            = static_cast< XContainerListener* >
                ( new BasMgrContainerListenerImpl( pMgr, aLibName ) );
        xLibContainer->addContainerListener( xLibraryListener );
    }

    if( xScriptCont->isLibraryLoaded( aLibName ) )
    {
        addLibraryModulesImpl( pMgr, xLibNameAccess, aLibName );
    }
}


void BasMgrContainerListenerImpl::addLibraryModulesImpl( BasicManager* pMgr,
    Reference< XNameAccess > xLibNameAccess, ::rtl::OUString aLibName )
{
    Sequence< ::rtl::OUString > aModuleNames = xLibNameAccess->getElementNames();
    sal_Int32 nModuleCount = aModuleNames.getLength();

    StarBASIC* pLib = pMgr->GetLib( aLibName );
    DBG_ASSERT( pLib, "BasMgrContainerListenerImpl::addLibraryModulesImpl: Unknown lib!");
    if( pLib )
    {
        const ::rtl::OUString* pNames = aModuleNames.getConstArray();
        for( sal_Int32 j = 0 ; j < nModuleCount ; j++ )
        {
            ::rtl::OUString aModuleName = pNames[ j ];
            Any aElement = xLibNameAccess->getByName( aModuleName );
            ::rtl::OUString aMod;
            aElement >>= aMod;
            Reference< vba::XVBAModuleInfo > xVBAModuleInfo( xLibNameAccess, UNO_QUERY );
            if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( aModuleName ) )
            {
                ModuleInfo mInfo = xVBAModuleInfo->getModuleInfo( aModuleName );
                OSL_TRACE("#addLibraryModulesImpl - aMod");
                pLib->MakeModule32( aModuleName, mInfo, aMod );
            }
            else
        pLib->MakeModule32( aModuleName, aMod );
        }
    }

    pLib->SetModified( FALSE );
}



// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::disposing( const  EventObject& Source )
    throw( RuntimeException )
{
    (void)Source;
}

// XContainerListener
//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementInserted( const ContainerEvent& Event )
    throw( RuntimeException )
{
    sal_Bool bLibContainer = ( maLibName.getLength() == 0 );
    ::rtl::OUString aName;
    Event.Accessor >>= aName;

    mpMgr->mpImpl->mbModifiedByLibraryContainer = sal_True;

    if( bLibContainer )
    {
        Reference< XLibraryContainer > xScriptCont( Event.Source, UNO_QUERY );
        insertLibraryImpl( xScriptCont, mpMgr, Event.Element, aName );
        StarBASIC* pLib = mpMgr->GetLib( aName );
        if ( pLib )
        {
            Reference< vba::XVBACompatibility > xVBACompat( xScriptCont, UNO_QUERY );
            if ( xVBACompat.is() )
                pLib->SetVBAEnabled( xVBACompat->getVBACompatibilityMode() );
        }
    }
    else
    {

        StarBASIC* pLib = mpMgr->GetLib( maLibName );
        DBG_ASSERT( pLib, "BasMgrContainerListenerImpl::elementInserted: Unknown lib!");
        if( pLib )
        {
            SbModule* pMod = pLib->FindModule( aName );
            if( !pMod )
            {
            ::rtl::OUString aMod;
            Event.Element >>= aMod;
                Reference< vba::XVBAModuleInfo > xVBAModuleInfo( Event.Source, UNO_QUERY );
                if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( aName ) )
                {
                    ModuleInfo mInfo = xVBAModuleInfo->getModuleInfo( aName );
                    pLib->MakeModule32( aName, mInfo, aMod );
                }
                else
                    pLib->MakeModule32( aName, aMod );
                pLib->SetModified( FALSE );
            }
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementReplaced( const ContainerEvent& Event )
    throw( RuntimeException )
{
    ::rtl::OUString aName;
    Event.Accessor >>= aName;

    mpMgr->mpImpl->mbModifiedByLibraryContainer = sal_True;

    // Replace not possible for library container
#ifdef DBG_UTIL
    sal_Bool bLibContainer = ( maLibName.getLength() == 0 );
#endif
    DBG_ASSERT( !bLibContainer, "library container fired elementReplaced()");

    StarBASIC* pLib = mpMgr->GetLib( maLibName );
    if( pLib )
    {
        SbModule* pMod = pLib->FindModule( aName );
        ::rtl::OUString aMod;
        Event.Element >>= aMod;

        if( pMod )
                pMod->SetSource32( aMod );
        else
                pLib->MakeModule32( aName, aMod );

        pLib->SetModified( FALSE );
    }
}

//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementRemoved( const ContainerEvent& Event )
    throw( RuntimeException )
{
    ::rtl::OUString aName;
    Event.Accessor >>= aName;

    mpMgr->mpImpl->mbModifiedByLibraryContainer = sal_True;

    sal_Bool bLibContainer = ( maLibName.getLength() == 0 );
    if( bLibContainer )
    {
        StarBASIC* pLib = mpMgr->GetLib( aName );
        if( pLib )
        {
            USHORT nLibId = mpMgr->GetLibId( aName );
            mpMgr->RemoveLib( nLibId, FALSE );
        }
    }
    else
    {
        StarBASIC* pLib = mpMgr->GetLib( maLibName );
        SbModule* pMod = pLib ? pLib->FindModule( aName ) : NULL;
        if( pMod )
        {
            pLib->Remove( pMod );
            pLib->SetModified( FALSE );
        }
    }
}


//=====================================================================

class BasicErrorManager
{
private:
    BasErrorLst aErrorList;
    size_t CurrentError;

public:
                BasicErrorManager();
                ~BasicErrorManager();

    void        Reset();
    void        InsertError( const BasicError& rError );

    bool        HasErrors()         { return !aErrorList.empty(); }
    BasicError* GetFirstError();
    BasicError* GetNextError();
};

BasicErrorManager::BasicErrorManager()
    : CurrentError( 0 )
{
}

BasicErrorManager::~BasicErrorManager()
{
    Reset();
}

void BasicErrorManager::Reset()
{
    for ( size_t i = 0, n = aErrorList.size(); i < n; ++i )
        delete aErrorList[ i ];
    aErrorList.clear();
}

void BasicErrorManager::InsertError( const BasicError& rError )
{
    aErrorList.push_back( new BasicError( rError ) );
}

BasicError* BasicErrorManager::GetFirstError()
{
    CurrentError = 0;
    return aErrorList.empty() ? NULL : aErrorList[ CurrentError ];
}

BasicError* BasicErrorManager::GetNextError()
{
    if (  !aErrorList.empty()
       && CurrentError < ( aErrorList.size() - 1)
       )
    {
        ++CurrentError;
        return aErrorList[ CurrentError ];
    }
    return NULL;
}


//=====================================================================

BasicError::BasicError()
{
    nErrorId    = 0;
    nReason     = 0;
}

BasicError::BasicError( ULONG nId, USHORT nR, const String& rErrStr ) :
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
private:
    StarBASICRef    xLib;
    String          aLibName;
    String          aStorageName;   // String is sufficient, unique at runtime
    String          aRelStorageName;
    String          aPassword;

    BOOL            bDoLoad;
    BOOL            bReference;
    BOOL            bPasswordVerified;
    BOOL            bFoundInPath;   // Must not relativated again!

    // Lib represents library in new UNO library container
    Reference< XLibraryContainer > mxScriptCont;

public:
    BasicLibInfo();
    BasicLibInfo( const String& rStorageName );

    BOOL            IsReference() const     { return bReference; }
    BOOL&           IsReference()           { return bReference; }

    BOOL            IsExtern() const        { return ! aStorageName.EqualsAscii(szImbedded); }

    void            SetStorageName( const String& rName )   { aStorageName = rName; }
    const String&   GetStorageName() const                  { return aStorageName; }

    void            SetRelStorageName( const String& rN )   { aRelStorageName = rN; }
    const String&   GetRelStorageName() const               { return aRelStorageName; }
    void            CalcRelStorageName( const String& rMgrStorageName );

    StarBASICRef    GetLib() const
    {
        if( mxScriptCont.is() && mxScriptCont->hasByName( aLibName ) &&
            !mxScriptCont->isLibraryLoaded( aLibName ) )
                return StarBASICRef();
        return xLib;
    }
    StarBASICRef&   GetLibRef()                         { return xLib; }
    void            SetLib( StarBASIC* pBasic )         { xLib = pBasic; }

    const String&   GetLibName() const                  { return aLibName; }
    void            SetLibName( const String& rName )   { aLibName = rName; }

    // Only temporary for Load/Save
    BOOL            DoLoad()                            { return bDoLoad; }

    BOOL            HasPassword() const                 { return aPassword.Len() != 0; }
    const String&   GetPassword() const                 { return aPassword; }
    void            SetPassword( const String& rNewPassword )
                                                        { aPassword = rNewPassword; }
    BOOL            IsPasswordVerified() const          { return bPasswordVerified; }
    void            SetPasswordVerified()               { bPasswordVerified = TRUE; }

    BOOL            IsFoundInPath() const               { return bFoundInPath; }
    void            SetFoundInPath( BOOL bInPath )      { bFoundInPath = bInPath; }

    void                    Store( SotStorageStream& rSStream, const String& rBasMgrStorageName, BOOL bUseOldReloadInfo );
    static BasicLibInfo*    Create( SotStorageStream& rSStream );

    Reference< XLibraryContainer > GetLibraryContainer( void )
        { return mxScriptCont; }
    void SetLibraryContainer( const Reference< XLibraryContainer >& xScriptCont )
        { mxScriptCont = xScriptCont; }
};


//=====================================================================

class BasicLibs
{
private:
    vector< BasicLibInfo* > aList;
    size_t CurrentLib;

public:
    ~BasicLibs();
    String          aBasicLibPath; // TODO: Should be member of manager, but currently not incompatible
    BasicLibInfo*   GetObject( size_t i );
    BasicLibInfo*   First();
    BasicLibInfo*   Last();
    BasicLibInfo*   Prev();
    BasicLibInfo*   Next();
    size_t          GetPos( BasicLibInfo* LibInfo );
    size_t          Count() const { return aList.size(); };
    size_t          GetCurPos() const { return CurrentLib; };
    void            Insert( BasicLibInfo* LibInfo );
    BasicLibInfo*   Remove( BasicLibInfo* LibInfo );
};

BasicLibs::~BasicLibs() {
    for ( size_t i = 0, n = aList.size(); i < n; ++i )
        delete aList[ i ];
    aList.clear();
}

BasicLibInfo* BasicLibs::GetObject( size_t i )
{
    if (  aList.empty()
       || aList.size()  <= i
       )
        return NULL;
    CurrentLib = i;
    return aList[ CurrentLib ];
}

BasicLibInfo* BasicLibs::First()
{
    if ( aList.empty() )
        return NULL;
    CurrentLib = 0;
    return aList[ CurrentLib ];
}

BasicLibInfo* BasicLibs::Last()
{
    if ( aList.empty() )
        return NULL;
    CurrentLib = aList.size() - 1;
    return aList[ CurrentLib ];
}

BasicLibInfo* BasicLibs::Prev()
{
    if (  aList.empty()
       || CurrentLib == 0
       )
        return NULL;
    --CurrentLib;
    return aList[ CurrentLib ];
}

BasicLibInfo* BasicLibs::Next()
{
    if (  aList.empty()
       || CurrentLib >= ( aList.size() - 1 )
       )
        return NULL;
    ++CurrentLib;
    return aList[ CurrentLib ];
}

size_t BasicLibs::GetPos( BasicLibInfo* LibInfo )
{
    for ( size_t i = 0, n = aList.size(); i < n; ++i )
        if ( aList[ i ] == LibInfo )
            return i;
    return size_t( -1 );
}

void BasicLibs::Insert( BasicLibInfo* LibInfo )
{
    aList.push_back( LibInfo );
    CurrentLib = aList.size() - 1;
}

BasicLibInfo* BasicLibs::Remove( BasicLibInfo* LibInfo )
{
    size_t i = GetPos( LibInfo );
    if ( i < aList.size() )
    {
        vector< BasicLibInfo* >::iterator it = aList.begin();
        advance( it , i );
        it = aList.erase( it );
    }
    return LibInfo;
}


//=====================================================================

BasicLibInfo::BasicLibInfo()
{
    bReference          = FALSE;
    bPasswordVerified   = FALSE;
    bDoLoad             = FALSE;
    bFoundInPath        = FALSE;
    mxScriptCont        = NULL;
    aStorageName        = String::CreateFromAscii(szImbedded);
    aRelStorageName     = String::CreateFromAscii(szImbedded);
}

BasicLibInfo::BasicLibInfo( const String& rStorageName )
{
    bReference          = TRUE;
    bPasswordVerified   = FALSE;
    bDoLoad             = FALSE;
    mxScriptCont        = NULL;
    aStorageName        = rStorageName;
}

void BasicLibInfo::Store( SotStorageStream& rSStream, const String& rBasMgrStorageName, BOOL bUseOldReloadInfo )
{
    ULONG nStartPos = rSStream.Tell();
    sal_uInt32 nEndPos = 0;

    USHORT nId = LIBINFO_ID;
    USHORT nVer = CURR_VER;

    rSStream << nEndPos;
    rSStream << nId;
    rSStream << nVer;

    String aCurStorageName = INetURLObject(rBasMgrStorageName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    DBG_ASSERT(aCurStorageName.Len() != 0, "Bad storage name");

    // If not set initialize StorageName
    if ( aStorageName.Len() == 0 )
        aStorageName = aCurStorageName;

    // Load again?
    BOOL bDoLoad_ = xLib.Is();
    if ( bUseOldReloadInfo )
        bDoLoad_ = DoLoad();
    rSStream << bDoLoad_;

    // The name of the lib...
    rSStream.WriteByteString(GetLibName());

    // Absolute path...
    if ( ! GetStorageName().EqualsAscii(szImbedded) )
    {
        String aSName = INetURLObject( GetStorageName(), INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
        DBG_ASSERT(aSName.Len() != 0, "Bad storage name");
        rSStream.WriteByteString( aSName );
    }
    else
        rSStream.WriteByteString( szImbedded );

    // Relative path...
    if ( ( aStorageName == aCurStorageName ) || ( aStorageName.EqualsAscii(szImbedded) ) )
        rSStream.WriteByteString( szImbedded );
    else
    {
        // Do not determine the relative path if the file was only found in path:
        // because the relative path would change and after moving the lib the
        // the file cannot be found.
        if ( !IsFoundInPath() )
            CalcRelStorageName( aCurStorageName );
        rSStream.WriteByteString(aRelStorageName);
    }

    // ------------------------------
    // Version 2
    // ------------------------------

    // reference...
    rSStream << bReference;

    // ------------------------------
    // End
    // ------------------------------

    nEndPos = rSStream.Tell();
    rSStream.Seek( nStartPos );
    rSStream << nEndPos;
    rSStream.Seek( nEndPos );
}

BasicLibInfo* BasicLibInfo::Create( SotStorageStream& rSStream )
{
    BasicLibInfo* pInfo = new BasicLibInfo;

    sal_uInt32 nEndPos;
    USHORT nId;
    USHORT nVer;

    rSStream >> nEndPos;
    rSStream >> nId;
    rSStream >> nVer;

    DBG_ASSERT( nId == LIBINFO_ID, "Keine BasicLibInfo !?" );
    if( nId == LIBINFO_ID )
    {
        // Reload?
        BOOL bDoLoad;
        rSStream >> bDoLoad;
        pInfo->bDoLoad = bDoLoad;

        // The name of the lib...
        String aName;
        rSStream.ReadByteString(aName);
        pInfo->SetLibName( aName );

        // Absolute path...
        String aStorageName;
        rSStream.ReadByteString(aStorageName);
        pInfo->SetStorageName( aStorageName );

        // Relative path...
        String aRelStorageName;
        rSStream.ReadByteString(aRelStorageName);
        pInfo->SetRelStorageName( aRelStorageName );

        if ( nVer >= 2 )
        {
            BOOL bReferenz;
            rSStream >> bReferenz;
            pInfo->IsReference() = bReferenz;
        }

        rSStream.Seek( nEndPos );
    }
    return pInfo;
}

void BasicLibInfo::CalcRelStorageName( const String& rMgrStorageName )
{
    if ( rMgrStorageName.Len() )
    {
        INetURLObject aAbsURLObj( rMgrStorageName );
        aAbsURLObj.removeSegment();
        String aPath = aAbsURLObj.GetMainURL( INetURLObject::NO_DECODE );
        UniString aRelURL = INetURLObject::GetRelURL( aPath, GetStorageName() );
        SetRelStorageName( aRelURL );
    }
    else
        SetRelStorageName( String() );
}
BasicManager::BasicManager( SotStorage& rStorage, const String& rBaseURL, StarBASIC* pParentFromStdLib, String* pLibPath, BOOL bDocMgr ) : mbDocMgr( bDocMgr )
{
    DBG_CTOR( BasicManager, 0 );

    Init();

    if( pLibPath )
        pLibs->aBasicLibPath = *pLibPath;

    String aStorName( rStorage.GetName() );
    maStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );

    // #91251: Storage name not longer available for documents < 5.0
    // Should be no real problem, because only relative storage names
    // (links) can be affected.
    // DBG_ASSERT( aStorName.Len(), "No Storage Name!" );
    // DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    // If there is no Manager Stream, no further actions are necessary
    if ( rStorage.IsStream( String(RTL_CONSTASCII_USTRINGPARAM(szManagerStream)) ) )
    {
        LoadBasicManager( rStorage, rBaseURL );
        // StdLib contains Parent:
        StarBASIC* pStdLib = GetStdLib();
        DBG_ASSERT( pStdLib, "Standard-Lib not loaded?" );
        if ( !pStdLib )
        {
            // Should never happen, but if it happens we wont crash...
            pStdLib = new StarBASIC( NULL, mbDocMgr );
            BasicLibInfo* pStdLibInfo = pLibs->GetObject( 0 );
            if ( !pStdLibInfo )
                pStdLibInfo = CreateLibInfo();
            pStdLibInfo->SetLib( pStdLib );
            StarBASICRef xStdLib = pStdLibInfo->GetLib();
            xStdLib->SetName( String::CreateFromAscii(szStdLibName) );
            pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
            xStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
            xStdLib->SetModified( FALSE );
        }
        else
        {
            pStdLib->SetParent( pParentFromStdLib );
            // The other get StdLib as parent:
            for ( USHORT nBasic = 1; nBasic < GetLibCount(); nBasic++ )
            {
                StarBASIC* pBasic = GetLib( nBasic );
                if ( pBasic )
                {
//                  pBasic->SetParent( pStdLib );
                    pStdLib->Insert( pBasic );
                    pBasic->SetFlag( SBX_EXTSEARCH );
                }
            }
            // Modified through insert
            pStdLib->SetModified( FALSE );
        }

        // #91626 Save all stream data to save it unmodified if basic isn't modified
        // in an 6.0+ office. So also the old basic dialogs can be saved.
        SotStorageStreamRef xManagerStream = rStorage.OpenSotStream
            ( String(RTL_CONSTASCII_USTRINGPARAM(szManagerStream)), eStreamReadMode );
        mpImpl->mpManagerStream = new SvMemoryStream();
        *static_cast<SvStream*>(&xManagerStream) >> *mpImpl->mpManagerStream;

        SotStorageRef xBasicStorage = rStorage.OpenSotStorage
                                ( String(RTL_CONSTASCII_USTRINGPARAM(szBasicStorage)), eStorageReadMode, FALSE );
        if( xBasicStorage.Is() && !xBasicStorage->GetError() )
        {
            USHORT nLibs = GetLibCount();
            mpImpl->mppLibStreams = new SvMemoryStream*[ nLibs ];
            for( USHORT nL = 0; nL < nLibs; nL++ )
            {
                BasicLibInfo* pInfo = pLibs->GetObject( nL );
                DBG_ASSERT( pInfo, "pInfo?!" );
                SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pInfo->GetLibName(), eStreamReadMode );
                mpImpl->mppLibStreams[nL] = new SvMemoryStream();
                *static_cast<SvStream*>(&xBasicStream) >> *( mpImpl->mppLibStreams[nL] );
            }
        }
        else
            mpImpl->mbError = sal_True;
    }
    else
    {
        ImpCreateStdLib( pParentFromStdLib );
        if ( rStorage.IsStream( String::CreateFromAscii(szOldManagerStream) ) )
            LoadOldBasicManager( rStorage );
    }

    bBasMgrModified = FALSE;
}

void copyToLibraryContainer( StarBASIC* pBasic, const LibraryContainerInfo& rInfo )
{
    Reference< XLibraryContainer > xScriptCont( rInfo.mxScriptCont.get() );
    if ( !xScriptCont.is() )
        return;

    String aLibName = pBasic->GetName();
    if( !xScriptCont->hasByName( aLibName ) )
        xScriptCont->createLibrary( aLibName );

    Any aLibAny = xScriptCont->getByName( aLibName );
    Reference< XNameContainer > xLib;
    aLibAny >>= xLib;
    if ( !xLib.is() )
        return;

    USHORT nModCount = pBasic->GetModules()->Count();
    for ( USHORT nMod = 0 ; nMod < nModCount ; nMod++ )
    {
        SbModule* pModule = (SbModule*)pBasic->GetModules()->Get( nMod );
        DBG_ASSERT( pModule, "Modul nicht erhalten!" );

        String aModName = pModule->GetName();
        if( !xLib->hasByName( aModName ) )
        {
            ::rtl::OUString aSource = pModule->GetSource32();
            Any aSourceAny;
            aSourceAny <<= aSource;
            xLib->insertByName( aModName, aSourceAny );
        }
    }
}

const Reference< XPersistentLibraryContainer >& BasicManager::GetDialogLibraryContainer()  const
{
    return mpImpl->maContainerInfo.mxDialogCont;
}

const Reference< XPersistentLibraryContainer >& BasicManager::GetScriptLibraryContainer()  const
{
    return mpImpl->maContainerInfo.mxScriptCont;
}

void BasicManager::SetLibraryContainerInfo( const LibraryContainerInfo& rInfo )
{
    mpImpl->maContainerInfo = rInfo;

    Reference< XLibraryContainer > xScriptCont( mpImpl->maContainerInfo.mxScriptCont.get() );
    StarBASIC* pStdLib = GetStdLib();
    String aLibName = pStdLib->GetName();
    if( xScriptCont.is() )
    {
        // Register listener for lib container
        ::rtl::OUString aEmptyLibName;
        Reference< XContainerListener > xLibContainerListener
            = static_cast< XContainerListener* >
                ( new BasMgrContainerListenerImpl( this, aEmptyLibName ) );

        Reference< XContainer> xLibContainer( xScriptCont, UNO_QUERY );
        xLibContainer->addContainerListener( xLibContainerListener );

        Sequence< ::rtl::OUString > aScriptLibNames = xScriptCont->getElementNames();
        const ::rtl::OUString* pScriptLibName = aScriptLibNames.getConstArray();
        sal_Int32 i, nNameCount = aScriptLibNames.getLength();

        if( nNameCount )
        {
            for( i = 0 ; i < nNameCount ; ++i, ++pScriptLibName )
            {
                Any aLibAny = xScriptCont->getByName( *pScriptLibName );

                if ( pScriptLibName->equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Standard")) )
                    xScriptCont->loadLibrary( *pScriptLibName );

                BasMgrContainerListenerImpl::insertLibraryImpl
                    ( xScriptCont, this, aLibAny, *pScriptLibName );
            }
        }
        else
        {
            // No libs? Maybe an 5.2 document already loaded
            USHORT nLibs = GetLibCount();
            for( USHORT nL = 0; nL < nLibs; nL++ )
            {
                BasicLibInfo* pBasLibInfo = pLibs->GetObject( nL );
                StarBASIC* pLib = pBasLibInfo->GetLib();
                if( !pLib )
                {
                    BOOL bLoaded = ImpLoadLibary( pBasLibInfo, NULL, FALSE );
                    if( bLoaded )
                        pLib = pBasLibInfo->GetLib();
                }
                if( pLib )
                {
                    copyToLibraryContainer( pLib, mpImpl->maContainerInfo );
                    if( pBasLibInfo->HasPassword() )
                    {
                        OldBasicPassword* pOldBasicPassword =
                            mpImpl->maContainerInfo.mpOldBasicPassword;
                        if( pOldBasicPassword )
                        {
                            pOldBasicPassword->setLibraryPassword
                                ( pLib->GetName(), pBasLibInfo->GetPassword() );
                            pBasLibInfo->SetPasswordVerified();
                        }
                    }
                }
            }

            mpImpl->mbModifiedByLibraryContainer = sal_False;
        }
    }

    SetGlobalUNOConstant( "BasicLibraries", makeAny( mpImpl->maContainerInfo.mxScriptCont ) );
    SetGlobalUNOConstant( "DialogLibraries", makeAny( mpImpl->maContainerInfo.mxDialogCont ) );
}

BasicManager::BasicManager( StarBASIC* pSLib, String* pLibPath, BOOL bDocMgr ) : mbDocMgr( bDocMgr )
{
    DBG_CTOR( BasicManager, 0 );
    Init();
    DBG_ASSERT( pSLib, "BasicManager cannot be created with a NULL-Pointer!" );

    if( pLibPath )
        pLibs->aBasicLibPath = *pLibPath;

    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( pSLib );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( String::CreateFromAscii(szStdLibName));
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    pSLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );

    // Save is only necessary if basic has changed
    xStdLib->SetModified( FALSE );
    bBasMgrModified = FALSE;
}

BasicManager::BasicManager()
{
    DBG_CTOR( BasicManager, 0 );
    // This ctor may only be used to adapt relative paths for 'Save As'.
    // There is no AppBasic so libs must not be loaded...
    Init();
}

void BasicManager::ImpMgrNotLoaded( const String& rStorageName )
{
    // pErrInf is only destroyed if the error os processed by an
    // ErrorHandler
    StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, rStorageName, ERRCODE_BUTTON_OK );
    pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENMGRSTREAM, rStorageName ) );

    // Create a stdlib otherwise we crash!
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( new StarBASIC( NULL, mbDocMgr ) );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( String::CreateFromAscii(szStdLibName) );
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    xStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
    xStdLib->SetModified( FALSE );
}


void BasicManager::ImpCreateStdLib( StarBASIC* pParentFromStdLib )
{
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    StarBASIC* pStdLib = new StarBASIC( pParentFromStdLib, mbDocMgr );
    pStdLibInfo->SetLib( pStdLib );
    pStdLib->SetName( String::CreateFromAscii(szStdLibName) );
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    pStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
}


void BasicManager::LoadBasicManager( SotStorage& rStorage, const String& rBaseURL, BOOL bLoadLibs )
{
    DBG_CHKTHIS( BasicManager, 0 );

//  StreamMode eStreamMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE;

    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream
        ( String(RTL_CONSTASCII_USTRINGPARAM(szManagerStream)), eStreamReadMode );

    String aStorName( rStorage.GetName() );
    // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.Is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    maStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    // #i13114 removed, DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    String aRealStorageName = maStorageName;  // for relative paths, can be modified through BaseURL

    // If loaded from template, only BaseURL is used:
    //String aBaseURL = INetURLObject::GetBaseURL();
    if ( rBaseURL.Len() )
    {
        INetURLObject aObj( rBaseURL );
        if ( aObj.GetProtocol() == INET_PROT_FILE )
            aRealStorageName = aObj.PathToFileName();
    }

    xManagerStream->SetBufferSize( 1024 );
    xManagerStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_uInt32 nEndPos;
    *xManagerStream >> nEndPos;

    USHORT nLibs;
    *xManagerStream >> nLibs;
    // Plausibility!
    if( nLibs & 0xF000 )
    {
        DBG_ASSERT( !this, "BasicManager-Stream defect!" );
        return;
    }
    for ( USHORT nL = 0; nL < nLibs; nL++ )
    {
        BasicLibInfo* pInfo = BasicLibInfo::Create( *xManagerStream );

        // Correct absolute pathname if relative is existing.
        // Always try relative first if there are two stands on disk
        if ( pInfo->GetRelStorageName().Len() && ( ! pInfo->GetRelStorageName().EqualsAscii(szImbedded) ) )
        {
            INetURLObject aObj( aRealStorageName, INET_PROT_FILE );
            aObj.removeSegment();
            bool bWasAbsolute = FALSE;
            aObj = aObj.smartRel2Abs( pInfo->GetRelStorageName(), bWasAbsolute );

            //*** TODO: Replace if still necessary
            /* if ( SfxContentHelper::Exists( aObj.GetMainURL() ) )
                pInfo->SetStorageName( aObj.GetMainURL() );
            else */
            //*** TODO-End
            if ( pLibs->aBasicLibPath.Len() )
            {
                // Search lib in path
                String aSearchFile = pInfo->GetRelStorageName();
                SvtPathOptions aPathCFG;
                if( aPathCFG.SearchFile( aSearchFile, SvtPathOptions::PATH_BASIC ) )
                {
                    pInfo->SetStorageName( aSearchFile );
                    pInfo->SetFoundInPath( TRUE );
                }
            }
        }

        pLibs->Insert( pInfo );
        // Libs from external files should be loaded only when necessary.
        // But references are loaded at once, otherwise some big customers get into trouble
        if ( bLoadLibs && pInfo->DoLoad() &&
            ( ( !pInfo->IsExtern() ) || ( pInfo->IsReference() ) ) )
        {
            ImpLoadLibary( pInfo, &rStorage );
        }
    }

    xManagerStream->Seek( nEndPos );
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.Clear();
}

void BasicManager::LoadOldBasicManager( SotStorage& rStorage )
{
    DBG_CHKTHIS( BasicManager, 0 );

//  StreamMode eStreamMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE;

    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream
        ( String::CreateFromAscii(szOldManagerStream), eStreamReadMode );

    String aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.Is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    xManagerStream->SetBufferSize( 1024 );
    xManagerStream->Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt32 nBasicStartOff, nBasicEndOff;
    *xManagerStream >> nBasicStartOff;
    *xManagerStream >> nBasicEndOff;

    DBG_ASSERT( !xManagerStream->GetError(), "Ungueltiger Manager-Stream!" );

    xManagerStream->Seek( nBasicStartOff );
    if( !ImplLoadBasic( *xManagerStream, pLibs->GetObject(0)->GetLibRef() ) )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, aStorName, ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENMGRSTREAM, aStorName ) );
        // and it proceeds ...
    }
    xManagerStream->Seek( nBasicEndOff+1 ); // +1: 0x00 as separator
    String aLibs;
    xManagerStream->ReadByteString(aLibs);
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.Clear(); // Close stream

    if ( aLibs.Len() )
    {
        String aCurStorageName( aStorName );
        INetURLObject aCurStorage( aCurStorageName, INET_PROT_FILE );
        USHORT nLibs = aLibs.GetTokenCount( LIB_SEP );
        for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
        {
            String aLibInfo( aLibs.GetToken( nLib, LIB_SEP ) );
            // TODO: Remove == 2
            DBG_ASSERT( ( aLibInfo.GetTokenCount( LIBINFO_SEP ) == 2 ) || ( aLibInfo.GetTokenCount( LIBINFO_SEP ) == 3 ), "Ungueltige Lib-Info!" );
            String aLibName( aLibInfo.GetToken( 0, LIBINFO_SEP ) );
            String aLibAbsStorageName( aLibInfo.GetToken( 1, LIBINFO_SEP ) );
            String aLibRelStorageName( aLibInfo.GetToken( 2, LIBINFO_SEP ) );
            INetURLObject aLibAbsStorage( aLibAbsStorageName, INET_PROT_FILE );

            INetURLObject aLibRelStorage( aStorName );
            aLibRelStorage.removeSegment();
            bool bWasAbsolute = FALSE;
            aLibRelStorage = aLibRelStorage.smartRel2Abs( aLibRelStorageName, bWasAbsolute);
            DBG_ASSERT(!bWasAbsolute, "RelStorageName was absolute!" );

            SotStorageRef xStorageRef;
            if ( ( aLibAbsStorage == aCurStorage ) || ( aLibRelStorageName.EqualsAscii(szImbedded) ) )
                xStorageRef = &rStorage;
            else
            {
                xStorageRef = new SotStorage( FALSE, aLibAbsStorage.GetMainURL
                    ( INetURLObject::NO_DECODE ), eStorageReadMode, TRUE );
                if ( xStorageRef->GetError() != ERRCODE_NONE )
                    xStorageRef = new SotStorage( FALSE, aLibRelStorage.
                    GetMainURL( INetURLObject::NO_DECODE ), eStorageReadMode, TRUE );
            }
            if ( xStorageRef.Is() )
                AddLib( *xStorageRef, aLibName, FALSE );
            else
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, aStorName, ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_STORAGENOTFOUND, aStorName ) );
            }
        }
    }
}

BasicManager::~BasicManager()
{
    DBG_DTOR( BasicManager, 0 );

    // Notify listener if something needs to be saved
    Broadcast( SfxSimpleHint( SFX_HINT_DYING) );

    // Destroy Basic-Infos...
    // In reverse order
    delete pLibs;
    delete pErrorMgr;
    delete mpImpl;
}

void BasicManager::LegacyDeleteBasicManager( BasicManager*& _rpManager )
{
    delete _rpManager;
    _rpManager = NULL;
}


bool BasicManager::HasExeCode( const String& sLib )
{
    StarBASIC* pLib = GetLib(sLib);
    if ( pLib )
    {
        SbxArray* pMods = pLib->GetModules();
        USHORT nMods = pMods ? pMods->Count() : 0;
        for( USHORT i = 0; i < nMods; i++ )
        {
            SbModule* p = (SbModule*) pMods->Get( i );
            if ( p )
                if ( p->HasExeCode() )
                    return true;
        }
    }
    return false;
}

void BasicManager::Init()
{
    DBG_CHKTHIS( BasicManager, 0 );

    bBasMgrModified = FALSE;
    pErrorMgr = new BasicErrorManager;
    pLibs = new BasicLibs;
    mpImpl = new BasicManagerImpl();
}

BasicLibInfo* BasicManager::CreateLibInfo()
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = new BasicLibInfo;
    pLibs->Insert( pInf );
    return pInf;
}

BOOL BasicManager::ImpLoadLibary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, BOOL bInfosOnly ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    DBG_ASSERT( pLibInfo, "LibInfo!?" );

    String aStorageName( pLibInfo->GetStorageName() );
    if ( !aStorageName.Len() || ( aStorageName.EqualsAscii(szImbedded) ) )
        aStorageName = GetStorageName();

    SotStorageRef xStorage;
    // The current must not be opened again...
    if ( pCurStorage )
    {
        String aStorName( pCurStorage->GetName() );
        // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

        INetURLObject aCurStorageEntry(aStorName, INET_PROT_FILE);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::NO_DECODE ).Len() != 0, "Bad storage name");

        INetURLObject aStorageEntry(aStorageName, INET_PROT_FILE);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::NO_DECODE ).Len() != 0, "Bad storage name");

        if ( aCurStorageEntry == aStorageEntry )
            xStorage = pCurStorage;
    }

    if ( !xStorage.Is() )
        xStorage = new SotStorage( FALSE, aStorageName, eStorageReadMode );

    SotStorageRef xBasicStorage = xStorage->OpenSotStorage
                            ( String(RTL_CONSTASCII_USTRINGPARAM(szBasicStorage)), eStorageReadMode, FALSE );

    if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, xStorage->GetName(), ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName() ) );
    }
    else
    {
        // In the Basic-Storage every lib is in a Stream...
        SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pLibInfo->GetLibName(), eStreamReadMode );
        if ( !xBasicStream.Is() || xBasicStream->GetError() )
        {
            StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD , pLibInfo->GetLibName(), ERRCODE_BUTTON_OK );
            pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTREAM, pLibInfo->GetLibName() ) );
        }
        else
        {
            BOOL bLoaded = FALSE;
            if ( xBasicStream->Seek( STREAM_SEEK_TO_END ) != 0 )
            {
                if ( !bInfosOnly )
                {
                    if ( !pLibInfo->GetLib().Is() )
                        pLibInfo->SetLib( new StarBASIC( GetStdLib(), mbDocMgr ) );
                    xBasicStream->SetBufferSize( 1024 );
                    xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                    bLoaded = ImplLoadBasic( *xBasicStream, pLibInfo->GetLibRef() );
                    xBasicStream->SetBufferSize( 0 );
                    StarBASICRef xStdLib = pLibInfo->GetLib();
                    xStdLib->SetName( pLibInfo->GetLibName() );
                    xStdLib->SetModified( FALSE );
                    xStdLib->SetFlag( SBX_DONTSTORE );
                }
                else
                {
                    // Skip Basic...
                    xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                    ImplEncryptStream( *xBasicStream );
                    SbxBase::Skip( *xBasicStream );
                    bLoaded = TRUE;
                }
            }
            if ( !bLoaded )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, pLibInfo->GetLibName(), ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_BASICLOADERROR, pLibInfo->GetLibName() ) );
            }
            else
            {
                // Perhaps there are additional information in the stream...
                xBasicStream->SetKey( szCryptingKey );
                xBasicStream->RefreshBuffer();
                sal_uInt32 nPasswordMarker = 0;
                *xBasicStream >> nPasswordMarker;
                if ( ( nPasswordMarker == PASSWORD_MARKER ) && !xBasicStream->IsEof() )
                {
                    String aPassword;
                    xBasicStream->ReadByteString(aPassword);
                    pLibInfo->SetPassword( aPassword );
                }
                xBasicStream->SetKey( ByteString() );
                CheckModules( pLibInfo->GetLib(), pLibInfo->IsReference() );
            }
            return bLoaded;
        }
    }
    return FALSE;
}

BOOL BasicManager::ImplEncryptStream( SvStream& rStrm ) const
{
    ULONG nPos = rStrm.Tell();
    UINT32 nCreator;
    rStrm >> nCreator;
    rStrm.Seek( nPos );
    BOOL bProtected = FALSE;
    if ( nCreator != SBXCR_SBX )
    {
        // Should only be the case for encrypted Streams
        bProtected = TRUE;
        rStrm.SetKey( szCryptingKey );
        rStrm.RefreshBuffer();
    }
    return bProtected;
}

// This code is necessary to load the BASIC of Beta 1
// TODO: Which Beta 1?
BOOL BasicManager::ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const
{
    BOOL bProtected = ImplEncryptStream( rStrm );
    SbxBaseRef xNew = SbxBase::Load( rStrm );
    BOOL bLoaded = FALSE;
    if( xNew.Is() )
    {
        if( xNew->IsA( TYPE(StarBASIC) ) )
        {
            StarBASIC* pNew = (StarBASIC*)(SbxBase*) xNew;
            // Use the Parent of the old BASICs
            if( rOldBasic.Is() )
            {
                pNew->SetParent( rOldBasic->GetParent() );
                if( pNew->GetParent() )
                    pNew->GetParent()->Insert( pNew );
                pNew->SetFlag( SBX_EXTSEARCH );
            }
            rOldBasic = pNew;

            // Fill new libray container (5.2 -> 6.0)
            copyToLibraryContainer( pNew, mpImpl->maContainerInfo );

            pNew->SetModified( FALSE );
            bLoaded = TRUE;
        }
    }
    if ( bProtected )
        rStrm.SetKey( ByteString() );
    return bLoaded;
}

void BasicManager::CheckModules( StarBASIC* pLib, BOOL bReference ) const
{
    if ( !pLib )
        return;

    BOOL bModified = pLib->IsModified();

    for ( USHORT nMod = 0; nMod < pLib->GetModules()->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pLib->GetModules()->Get( nMod );
        DBG_ASSERT( pModule, "Modul nicht erhalten!" );
        if ( !pModule->IsCompiled() && !StarBASIC::GetErrorCode() )
            pLib->Compile( pModule );
    }

    // #67477, AB 8.12.99 On demand compile in referenced libs should not
    // cause modified
    if( !bModified && bReference )
    {
        OSL_FAIL( "Per Reference eingebundene Basic-Library ist nicht compiliert!" );
        pLib->SetModified( FALSE );
    }
}

StarBASIC* BasicManager::AddLib( SotStorage& rStorage, const String& rLibName, BOOL bReference )
{
    DBG_CHKTHIS( BasicManager, 0 );

    String aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    String aStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    String aNewLibName( rLibName );
    while ( HasLib( aNewLibName ) )
        aNewLibName += '_';

    BasicLibInfo* pLibInfo = CreateLibInfo();
    // Use original name otherwise ImpLoadLibary failes...
    pLibInfo->SetLibName( rLibName );
    // Funktioniert so aber nicht, wenn Name doppelt
    USHORT nLibId = (USHORT) pLibs->GetPos( pLibInfo );

    // Set StorageName before load because it is compared with pCurStorage
    pLibInfo->SetStorageName( aStorageName );
    BOOL bLoaded = ImpLoadLibary( pLibInfo, &rStorage );

    if ( bLoaded )
    {
        if ( aNewLibName != rLibName )
            SetLibName( nLibId, aNewLibName );

        if ( bReference )
        {
            pLibInfo->GetLib()->SetModified( FALSE );   // Don't save in this case
            pLibInfo->SetRelStorageName( String() );
            pLibInfo->IsReference() = TRUE;
        }
        else
        {
            pLibInfo->GetLib()->SetModified( TRUE ); // Must be saved after Add!
            pLibInfo->SetStorageName( String::CreateFromAscii(szImbedded) ); // Save in BasicManager-Storage
        }
        bBasMgrModified = TRUE;
    }
    else
    {
        RemoveLib( nLibId, FALSE );
        pLibInfo = 0;
    }

    if( pLibInfo )
        return &*pLibInfo->GetLib() ;
    else
        return 0;
}

BOOL BasicManager::IsReference( USHORT nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
        return pLibInfo->IsReference();

    return FALSE;
}

BOOL BasicManager::RemoveLib( USHORT nLib )
{
    // Only pyhsical deletion if no reference
    return RemoveLib( nLib, !IsReference( nLib ) );
}

BOOL BasicManager::RemoveLib( USHORT nLib, BOOL bDelBasicFromStorage )
{
    DBG_CHKTHIS( BasicManager, 0 );
    DBG_ASSERT( nLib, "Standard-Lib cannot be removed!" );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib not found!" );

    if ( !pLibInfo || !nLib )
    {
//      String aErrorText( BasicResId( IDS_SBERR_REMOVELIB ) );
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, String(), ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_STDLIB, pLibInfo->GetLibName() ) );
        return FALSE;
    }

    // If one of the streams cannot be opened, this is not an error,
    // because BASIC was never written before...
    if ( bDelBasicFromStorage && !pLibInfo->IsReference() &&
            ( !pLibInfo->IsExtern() || SotStorage::IsStorageFile( pLibInfo->GetStorageName() ) ) )
    {
        SotStorageRef xStorage;
        if ( !pLibInfo->IsExtern() )
            xStorage = new SotStorage( FALSE, GetStorageName() );
        else
            xStorage = new SotStorage( FALSE, pLibInfo->GetStorageName() );

        if ( xStorage->IsStorage( String(RTL_CONSTASCII_USTRINGPARAM(szBasicStorage)) ) )
        {
            SotStorageRef xBasicStorage = xStorage->OpenSotStorage
                            ( String(RTL_CONSTASCII_USTRINGPARAM(szBasicStorage)), STREAM_STD_READWRITE, FALSE );

            if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, String(), ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName() ) );
            }
            else if ( xBasicStorage->IsStream( pLibInfo->GetLibName() ) )
            {
                xBasicStorage->Remove( pLibInfo->GetLibName() );
                xBasicStorage->Commit();

                // If no further stream available,
                // delete the SubStorage.
                SvStorageInfoList aInfoList( 0, 4 );
                xBasicStorage->FillInfoList( &aInfoList );
                if ( !aInfoList.Count() )
                {
                    xBasicStorage.Clear();
                    xStorage->Remove( String(RTL_CONSTASCII_USTRINGPARAM(szBasicStorage)) );
                    xStorage->Commit();
                    // If no further Streams or SubStorages available,
                    // delete the Storage, too.
                    aInfoList.Clear();
                    xStorage->FillInfoList( &aInfoList );
                    if ( !aInfoList.Count() )
                    {
                        String aName_( xStorage->GetName() );
                        xStorage.Clear();
                        //*** TODO: Replace if still necessary
                        //SfxContentHelper::Kill( aName );
                        //*** TODO-End
                    }
                }
            }
        }
    }
    bBasMgrModified = TRUE;
    if ( pLibInfo->GetLib().Is() )
        GetStdLib()->Remove( pLibInfo->GetLib() );
    delete pLibs->Remove( pLibInfo );
    return TRUE;    // Remove was successful, del unimportant
}

USHORT BasicManager::GetLibCount() const
{
    DBG_CHKTHIS( BasicManager, 0 );
    return (USHORT)pLibs->Count();
}

StarBASIC* BasicManager::GetLib( USHORT nLib ) const
{
    DBG_CHKTHIS( BasicManager, 0 );
    BasicLibInfo* pInf = pLibs->GetObject( nLib );
    DBG_ASSERT( pInf, "Lib existiert nicht!" );
    if ( pInf )
        return pInf->GetLib();
    return 0;
}

StarBASIC* BasicManager::GetStdLib() const
{
    DBG_CHKTHIS( BasicManager, 0 );
    StarBASIC* pLib = GetLib( 0 );
    return pLib;
}

StarBASIC* BasicManager::GetLib( const String& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().CompareIgnoreCaseToAscii( rName ) == COMPARE_EQUAL )// Check if available...
            return pInf->GetLib();

        pInf = pLibs->Next();
    }
    return 0;
}

USHORT BasicManager::GetLibId( const String& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().CompareIgnoreCaseToAscii( rName ) == COMPARE_EQUAL )
            return (USHORT)pLibs->GetCurPos();

        pInf = pLibs->Next();
    }
    return LIB_NOTFOUND;
}

BOOL BasicManager::HasLib( const String& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().CompareIgnoreCaseToAscii( rName ) == COMPARE_EQUAL )
            return TRUE;

        pInf = pLibs->Next();
    }
    return FALSE;
}

BOOL BasicManager::SetLibName( USHORT nLib, const String& rName )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
    {
        pLibInfo->SetLibName( rName );
        if ( pLibInfo->GetLib().Is() )
        {
            StarBASICRef xStdLib = pLibInfo->GetLib();
            xStdLib->SetName( rName );
            xStdLib->SetModified( TRUE );
        }
        bBasMgrModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

String BasicManager::GetLibName( USHORT nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
        return pLibInfo->GetLibName();
    return String();
}

BOOL BasicManager::LoadLib( USHORT nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BOOL bDone = FALSE;
    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
    {
        Reference< XLibraryContainer > xLibContainer = pLibInfo->GetLibraryContainer();
        if( xLibContainer.is() )
        {
            String aLibName = pLibInfo->GetLibName();
            xLibContainer->loadLibrary( aLibName );
            bDone = xLibContainer->isLibraryLoaded( aLibName );;
        }
        else
        {
            bDone = ImpLoadLibary( pLibInfo, NULL, FALSE );
            StarBASIC* pLib = GetLib( nLib );
            if ( pLib )
            {
                GetStdLib()->Insert( pLib );
                pLib->SetFlag( SBX_EXTSEARCH );
            }
        }
    }
    else
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, String(), ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_LIBNOTFOUND, String::CreateFromInt32(nLib) ) );
    }
    return bDone;
}

StarBASIC* BasicManager::CreateLib( const String& rLibName )
{
    DBG_CHKTHIS( BasicManager, 0 );
    if ( GetLib( rLibName ) )
        return 0;

    BasicLibInfo* pLibInfo = CreateLibInfo();
    StarBASIC* pNew = new StarBASIC( GetStdLib(), mbDocMgr );
    GetStdLib()->Insert( pNew );
    pNew->SetFlag( SBX_EXTSEARCH | SBX_DONTSTORE );
    pLibInfo->SetLib( pNew );
    pLibInfo->SetLibName( rLibName );
    pLibInfo->GetLib()->SetName( rLibName );
    return pLibInfo->GetLib();
}

// For XML import/export:
StarBASIC* BasicManager::CreateLib
    ( const String& rLibName, const String& Password, const String& LinkTargetURL )
{
    // Ask if lib exists because standard lib is always there
    StarBASIC* pLib = GetLib( rLibName );
    if( !pLib )
    {
        if( LinkTargetURL.Len() != 0 )
        {
            SotStorageRef xStorage = new SotStorage( FALSE, LinkTargetURL, STREAM_READ | STREAM_SHARE_DENYWRITE );
            if( !xStorage->GetError() )
            {
                pLib = AddLib( *xStorage, rLibName, TRUE );
            }

            DBG_ASSERT( pLib, "XML Import: Linked basic library could not be loaded");
        }
        else
        {
            pLib = CreateLib( rLibName );
            if( Password.Len() != 0 )
            {
                BasicLibInfo* pLibInfo = FindLibInfo( pLib );
                pLibInfo ->SetPassword( Password );
            }
        }
        //ExternalSourceURL ?
    }
    return pLib;
}

StarBASIC* BasicManager::CreateLibForLibContainer( const String& rLibName,
    const Reference< XLibraryContainer >& xScriptCont )
{
    DBG_CHKTHIS( BasicManager, 0 );
    if ( GetLib( rLibName ) )
        return 0;

    BasicLibInfo* pLibInfo = CreateLibInfo();
    StarBASIC* pNew = new StarBASIC( GetStdLib(), mbDocMgr );
    GetStdLib()->Insert( pNew );
    pNew->SetFlag( SBX_EXTSEARCH | SBX_DONTSTORE );
    pLibInfo->SetLib( pNew );
    pLibInfo->SetLibName( rLibName );
    pLibInfo->GetLib()->SetName( rLibName );
    pLibInfo->SetLibraryContainer( xScriptCont );
    return pNew;
}


BasicLibInfo* BasicManager::FindLibInfo( StarBASIC* pBasic ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = ((BasicManager*)this)->pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLib() == pBasic )
            return pInf;

        pInf = ((BasicManager*)this)->pLibs->Next();
    }
    return 0;
}


BOOL BasicManager::IsModified() const
{
    DBG_CHKTHIS( BasicManager, 0 );

    if ( bBasMgrModified )
        return TRUE;
    return IsBasicModified();
}

BOOL BasicManager::IsBasicModified() const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLib().Is() && pInf->GetLib()->IsModified() )
            return TRUE;

        pInf = pLibs->Next();
    }
    return FALSE;
}

void BasicManager::SetFlagToAllLibs( short nFlag, BOOL bSet ) const
{
    USHORT nLibs = GetLibCount();
    for ( USHORT nL = 0; nL < nLibs; nL++ )
    {
        BasicLibInfo* pInfo = pLibs->GetObject( nL );
        DBG_ASSERT( pInfo, "Info?!" );
        StarBASIC* pLib = pInfo->GetLib();
        if ( pLib )
        {
            if ( bSet )
                pLib->SetFlag( nFlag );
            else
                pLib->ResetFlag( nFlag );
        }
    }
}

BOOL BasicManager::HasErrors()
{
    DBG_CHKTHIS( BasicManager, 0 );
    return pErrorMgr->HasErrors();
}

void BasicManager::ClearErrors()
{
    DBG_CHKTHIS( BasicManager, 0 );
    pErrorMgr->Reset();
}

BasicError* BasicManager::GetFirstError()
{
    DBG_CHKTHIS( BasicManager, 0 );
    return pErrorMgr->GetFirstError();
}

BasicError* BasicManager::GetNextError()
{
    DBG_CHKTHIS( BasicManager, 0 );
    return pErrorMgr->GetNextError();
}
bool BasicManager::GetGlobalUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut )
{
    bool bRes = false;
    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::GetGlobalUNOConstant: no lib to read from!" );
    if ( pStandardLib )
        bRes = pStandardLib->GetUNOConstant( _pAsciiName, aOut );
    return bRes;
}

Any BasicManager::SetGlobalUNOConstant( const sal_Char* _pAsciiName, const Any& _rValue )
{
    Any aOldValue;

    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::SetGlobalUNOConstant: no lib to insert into!" );
    if ( !pStandardLib )
        return aOldValue;

    ::rtl::OUString sVarName( ::rtl::OUString::createFromAscii( _pAsciiName ) );

    // obtain the old value
    SbxVariable* pVariable = pStandardLib->Find( sVarName, SbxCLASS_OBJECT );
    if ( pVariable )
        aOldValue = sbxToUnoValue( pVariable );

    SbxObjectRef xUnoObj = GetSbUnoObject( sVarName, _rValue );
    xUnoObj->SetFlag( SBX_DONTSTORE );
    pStandardLib->Insert( xUnoObj );

    return aOldValue;
}

bool BasicManager::LegacyPsswdBinaryLimitExceeded( ::com::sun::star::uno::Sequence< rtl::OUString >& _out_rModuleNames )
{
    try
    {
        Reference< XNameAccess > xScripts( GetScriptLibraryContainer(), UNO_QUERY_THROW );
        Reference< XLibraryContainerPassword > xPassword( GetScriptLibraryContainer(), UNO_QUERY_THROW );

        Sequence< ::rtl::OUString > aNames( xScripts->getElementNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pNamesEnd = aNames.getConstArray() + aNames.getLength();
        for ( ; pNames != pNamesEnd; ++pNames )
        {
            if( !xPassword->isLibraryPasswordProtected( *pNames ) )
                continue;

            StarBASIC* pBasicLib = GetLib( *pNames );
            if ( !pBasicLib )
                continue;

            Reference< XNameAccess > xScriptLibrary( xScripts->getByName( *pNames ), UNO_QUERY_THROW );
            Sequence< ::rtl::OUString > aElementNames( xScriptLibrary->getElementNames() );
            sal_Int32 nLen = aElementNames.getLength();

            Sequence< ::rtl::OUString > aBigModules( nLen );
            sal_Int32 nBigModules = 0;

            const ::rtl::OUString* pElementNames = aElementNames.getConstArray();
            const ::rtl::OUString* pElementNamesEnd = aElementNames.getConstArray() + aElementNames.getLength();
            for ( ; pElementNames != pElementNamesEnd; ++pElementNames )
            {
                SbModule* pMod = pBasicLib->FindModule( *pElementNames );
                if ( pMod && pMod->ExceedsLegacyModuleSize() )
                    aBigModules[ nBigModules++ ] = *pElementNames;
            }

            if ( nBigModules )
            {
                aBigModules.realloc( nBigModules );
                _out_rModuleNames = aBigModules;
                return true;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

//=====================================================================

class ModuleInfo_Impl : public ModuleInfoHelper
{
    ::rtl::OUString maName;
    ::rtl::OUString maLanguage;
    ::rtl::OUString maSource;

public:
    ModuleInfo_Impl( const ::rtl::OUString& aName, const ::rtl::OUString& aLanguage, const ::rtl::OUString& aSource )
        : maName( aName ), maLanguage( aLanguage), maSource( aSource ) {}

    // Methods XStarBasicModuleInfo
    virtual ::rtl::OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual ::rtl::OUString SAL_CALL getLanguage() throw(RuntimeException)
        { return maLanguage; }
    virtual ::rtl::OUString SAL_CALL getSource() throw(RuntimeException)
        { return maSource; }
};


//=====================================================================

class DialogInfo_Impl : public DialogInfoHelper
{
    ::rtl::OUString maName;
    Sequence< sal_Int8 > mData;

public:
    DialogInfo_Impl( const ::rtl::OUString& aName, Sequence< sal_Int8 > Data )
        : maName( aName ), mData( Data ) {}

    // Methods XStarBasicDialogInfo
    virtual ::rtl::OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual Sequence< sal_Int8 > SAL_CALL getData() throw(RuntimeException)
        { return mData; }
};


//=====================================================================

class LibraryInfo_Impl : public LibraryInfoHelper
{
    ::rtl::OUString maName;
    Reference< XNameContainer > mxModuleContainer;
    Reference< XNameContainer > mxDialogContainer;
    ::rtl::OUString maPassword;
    ::rtl::OUString maExternaleSourceURL;
    ::rtl::OUString maLinkTargetURL;

public:
    LibraryInfo_Impl
    (
        const ::rtl::OUString& aName,
        Reference< XNameContainer > xModuleContainer,
        Reference< XNameContainer > xDialogContainer,
        const ::rtl::OUString& aPassword,
        const ::rtl::OUString& aExternaleSourceURL,
        const ::rtl::OUString& aLinkTargetURL
    )
        : maName( aName )
        , mxModuleContainer( xModuleContainer )
        , mxDialogContainer( xDialogContainer )
        , maPassword( aPassword )
        , maExternaleSourceURL( aExternaleSourceURL )
        , maLinkTargetURL( aLinkTargetURL )
    {}

    // Methods XStarBasicLibraryInfo
    virtual ::rtl::OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual Reference< XNameContainer > SAL_CALL getModuleContainer() throw(RuntimeException)
        { return mxModuleContainer; }
    virtual Reference< XNameContainer > SAL_CALL getDialogContainer() throw(RuntimeException)
        { return mxDialogContainer; }
    virtual ::rtl::OUString SAL_CALL getPassword() throw(RuntimeException)
        { return maPassword; }
    virtual ::rtl::OUString SAL_CALL getExternalSourceURL() throw(RuntimeException)
        { return maExternaleSourceURL; }
    virtual ::rtl::OUString SAL_CALL getLinkTargetURL() throw(RuntimeException)
        { return maLinkTargetURL; }
};

//=====================================================================

class ModuleContainer_Impl : public NameContainerHelper
{
    StarBASIC* mpLib;

public:
    ModuleContainer_Impl( StarBASIC* pLib )
        :mpLib( pLib ) {}

    // Methods XElementAccess
    virtual Type SAL_CALL getElementType()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(RuntimeException);

    // Methods XNameAccess
    virtual Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
};

// Methods XElementAccess
Type ModuleContainer_Impl::getElementType()
    throw(RuntimeException)
{
    Type aModuleType = ::getCppuType( (const Reference< XStarBasicModuleInfo > *)0 );
    return aModuleType;
}

sal_Bool ModuleContainer_Impl::hasElements()
    throw(RuntimeException)
{
    SbxArray* pMods = mpLib ? mpLib->GetModules() : NULL;
    return pMods && pMods->Count() > 0;
}

// Methods XNameAccess
Any ModuleContainer_Impl::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    if( !pMod )
        throw NoSuchElementException();
    Reference< XStarBasicModuleInfo > xMod = (XStarBasicModuleInfo*)new ModuleInfo_Impl
        ( aName, ::rtl::OUString::createFromAscii( szScriptLanguage ), pMod->GetSource32() );
    Any aRetAny;
    aRetAny <<= xMod;
    return aRetAny;
}

Sequence< ::rtl::OUString > ModuleContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    SbxArray* pMods = mpLib ? mpLib->GetModules() : NULL;
    USHORT nMods = pMods ? pMods->Count() : 0;
    Sequence< ::rtl::OUString > aRetSeq( nMods );
    ::rtl::OUString* pRetSeq = aRetSeq.getArray();
    for( USHORT i = 0 ; i < nMods ; i++ )
    {
        SbxVariable* pMod = pMods->Get( i );
        pRetSeq[i] = ::rtl::OUString( pMod->GetName() );
    }
    return aRetSeq;
}

sal_Bool ModuleContainer_Impl::hasByName( const ::rtl::OUString& aName )
    throw(RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    sal_Bool bRet = (pMod != NULL);
    return bRet;
}


// Methods XNameReplace
void ModuleContainer_Impl::replaceByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void ModuleContainer_Impl::insertByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    Type aModuleType = ::getCppuType( (const Reference< XStarBasicModuleInfo > *)0 );
    Type aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
        throw IllegalArgumentException();
    Reference< XStarBasicModuleInfo > xMod;
    aElement >>= xMod;
    mpLib->MakeModule32( aName, xMod->getSource() );
}

void ModuleContainer_Impl::removeByName( const ::rtl::OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( Name ) : NULL;
    if( !pMod )
        throw NoSuchElementException();
    mpLib->Remove( pMod );
}


//=====================================================================

Sequence< sal_Int8 > implGetDialogData( SbxObject* pDialog )
{
    SvMemoryStream aMemStream;
    pDialog->Store( aMemStream );
    sal_Int32 nLen = aMemStream.Tell();
    Sequence< sal_Int8 > aData( nLen );
    sal_Int8* pDestData = aData.getArray();
    const sal_Int8* pSrcData = (const sal_Int8*)aMemStream.GetData();
    rtl_copyMemory( pDestData, pSrcData, nLen );
    return aData;
}

SbxObject* implCreateDialog( Sequence< sal_Int8 > aData )
{
    sal_Int8* pData = aData.getArray();
    SvMemoryStream aMemStream( pData, aData.getLength(), STREAM_READ );
    SbxObject* pDialog = (SbxObject*)SbxBase::Load( aMemStream );
    return pDialog;
}

// HACK! Because this value is defined in basctl/inc/vcsbxdef.hxx
// which we can't include here, we have to use the value directly
#define SBXID_DIALOG        101


class DialogContainer_Impl : public NameContainerHelper
{
    StarBASIC* mpLib;

public:
    DialogContainer_Impl( StarBASIC* pLib )
        :mpLib( pLib ) {}

    // Methods XElementAccess
    virtual Type SAL_CALL getElementType()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(RuntimeException);

    // Methods XNameAccess
    virtual Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
};

// Methods XElementAccess
Type DialogContainer_Impl::getElementType()
    throw(RuntimeException)
{
    Type aModuleType = ::getCppuType( (const Reference< XStarBasicDialogInfo > *)0 );
    return aModuleType;
}

sal_Bool DialogContainer_Impl::hasElements()
    throw(RuntimeException)
{
    sal_Bool bRet = sal_False;

    mpLib->GetAll( SbxCLASS_OBJECT );
    sal_Int16 nCount = mpLib->GetObjects()->Count();
    for( sal_Int16 nObj = 0; nObj < nCount ; nObj++ )
    {
        SbxVariable* pVar = mpLib->GetObjects()->Get( nObj );
        if ( pVar->ISA( SbxObject ) && ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) )
        {
            bRet = sal_True;
            break;
        }
    }
    return bRet;
}

// Methods XNameAccess
Any DialogContainer_Impl::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SbxVariable* pVar = mpLib->GetObjects()->Find( aName, SbxCLASS_DONTCARE );
    if( !( pVar && pVar->ISA( SbxObject ) &&
           ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) ) )
    {
        throw NoSuchElementException();
    }

    Reference< XStarBasicDialogInfo > xDialog =
        (XStarBasicDialogInfo*)new DialogInfo_Impl
            ( aName, implGetDialogData( (SbxObject*)pVar ) );

    Any aRetAny;
    aRetAny <<= xDialog;
    return aRetAny;
}

Sequence< ::rtl::OUString > DialogContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    mpLib->GetAll( SbxCLASS_OBJECT );
    sal_Int16 nCount = mpLib->GetObjects()->Count();
    Sequence< ::rtl::OUString > aRetSeq( nCount );
    ::rtl::OUString* pRetSeq = aRetSeq.getArray();
    sal_Int32 nDialogCounter = 0;

    for( sal_Int16 nObj = 0; nObj < nCount ; nObj++ )
    {
        SbxVariable* pVar = mpLib->GetObjects()->Get( nObj );
        if ( pVar->ISA( SbxObject ) && ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) )
        {
            pRetSeq[ nDialogCounter ] = ::rtl::OUString( pVar->GetName() );
            nDialogCounter++;
        }
    }
    aRetSeq.realloc( nDialogCounter );
    return aRetSeq;
}

sal_Bool DialogContainer_Impl::hasByName( const ::rtl::OUString& aName )
    throw(RuntimeException)
{
    sal_Bool bRet = sal_False;
    SbxVariable* pVar = mpLib->GetObjects()->Find( aName, SbxCLASS_DONTCARE );
    if( pVar && pVar->ISA( SbxObject ) &&
           ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) )
    {
        bRet = sal_True;
    }
    return bRet;
}


// Methods XNameReplace
void DialogContainer_Impl::replaceByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void DialogContainer_Impl::insertByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    (void)aName;
    Type aModuleType = ::getCppuType( (const Reference< XStarBasicDialogInfo > *)0 );
    Type aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
        throw IllegalArgumentException();
    Reference< XStarBasicDialogInfo > xMod;
    aElement >>= xMod;
    SbxObjectRef xDialog = implCreateDialog( xMod->getData() );
    mpLib->Insert( xDialog );
}

void DialogContainer_Impl::removeByName( const ::rtl::OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    (void)Name;
    SbxVariable* pVar = mpLib->GetObjects()->Find( Name, SbxCLASS_DONTCARE );
    if( !( pVar && pVar->ISA( SbxObject ) &&
           ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) ) )
    {
        throw NoSuchElementException();
    }
    mpLib->Remove( pVar );
}


//=====================================================================


class LibraryContainer_Impl : public NameContainerHelper
{
    BasicManager* mpMgr;

public:
    LibraryContainer_Impl( BasicManager* pMgr )
        :mpMgr( pMgr ) {}

    // Methods XElementAccess
    virtual Type SAL_CALL getElementType()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(RuntimeException);

    // Methods XNameAccess
    virtual Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
};


// Methods XElementAccess
Type LibraryContainer_Impl::getElementType()
    throw(RuntimeException)
{
    Type aType = ::getCppuType( (const Reference< XStarBasicLibraryInfo > *)0 );
    return aType;
}

sal_Bool LibraryContainer_Impl::hasElements()
    throw(RuntimeException)
{
    sal_Int32 nLibs = mpMgr->GetLibCount();
    sal_Bool bRet = (nLibs > 0);
    return bRet;
}

// Methods XNameAccess
Any LibraryContainer_Impl::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Any aRetAny;
    if( !mpMgr->HasLib( aName ) )
        throw NoSuchElementException();
    StarBASIC* pLib = mpMgr->GetLib( aName );

    Reference< XNameContainer > xModuleContainer =
        (XNameContainer*)new ModuleContainer_Impl( pLib );

    Reference< XNameContainer > xDialogContainer;
        (XNameContainer*)new DialogContainer_Impl( pLib );

    BasicLibInfo* pLibInfo = mpMgr->FindLibInfo( pLib );

    ::rtl::OUString aPassword = pLibInfo->GetPassword();

    // TODO Only provide extern info!
    ::rtl::OUString aExternaleSourceURL;
    ::rtl::OUString aLinkTargetURL;
    if( pLibInfo->IsReference() )
        aLinkTargetURL = pLibInfo->GetStorageName();
    else if( pLibInfo->IsExtern() )
        aExternaleSourceURL = pLibInfo->GetStorageName();

    Reference< XStarBasicLibraryInfo > xLibInfo = new LibraryInfo_Impl
    (
        aName,
        xModuleContainer,
        xDialogContainer,
        aPassword,
        aExternaleSourceURL,
        aLinkTargetURL
    );

    aRetAny <<= xLibInfo;
    return aRetAny;
}

Sequence< ::rtl::OUString > LibraryContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    USHORT nLibs = mpMgr->GetLibCount();
    Sequence< ::rtl::OUString > aRetSeq( nLibs );
    ::rtl::OUString* pRetSeq = aRetSeq.getArray();
    for( USHORT i = 0 ; i < nLibs ; i++ )
    {
        pRetSeq[i] = ::rtl::OUString( mpMgr->GetLibName( i ) );
    }
    return aRetSeq;
}

sal_Bool LibraryContainer_Impl::hasByName( const ::rtl::OUString& aName )
    throw(RuntimeException)
{
    sal_Bool bRet = mpMgr->HasLib( aName );
    return bRet;
}

// Methods XNameReplace
void LibraryContainer_Impl::replaceByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// Methods XNameContainer
void LibraryContainer_Impl::insertByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    (void)aName;
    (void)aElement;
    // TODO: Insert a complete Library?!
}

void LibraryContainer_Impl::removeByName( const ::rtl::OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    StarBASIC* pLib = mpMgr->GetLib( Name );
    if( !pLib )
        throw NoSuchElementException();
    USHORT nLibId = mpMgr->GetLibId( Name );
    mpMgr->RemoveLib( nLibId );
}

//=====================================================================

typedef WeakImplHelper1< XStarBasicAccess > StarBasicAccessHelper;


class StarBasicAccess_Impl : public StarBasicAccessHelper
{
    BasicManager* mpMgr;
    Reference< XNameContainer > mxLibContainer;

public:
    StarBasicAccess_Impl( BasicManager* pMgr )
        :mpMgr( pMgr ) {}

public:

    // Methods
    virtual Reference< XNameContainer > SAL_CALL getLibraryContainer()
        throw(RuntimeException);
    virtual void SAL_CALL createLibrary( const ::rtl::OUString& LibName, const ::rtl::OUString& Password,
        const ::rtl::OUString& ExternalSourceURL, const ::rtl::OUString& LinkTargetURL )
            throw(ElementExistException, RuntimeException);
    virtual void SAL_CALL addModule( const ::rtl::OUString& LibraryName, const ::rtl::OUString& ModuleName,
        const ::rtl::OUString& Language, const ::rtl::OUString& Source )
            throw(NoSuchElementException, RuntimeException);
    virtual void SAL_CALL addDialog( const ::rtl::OUString& LibraryName, const ::rtl::OUString& DialogName,
        const Sequence< sal_Int8 >& Data )
            throw(NoSuchElementException, RuntimeException);

};

Reference< XNameContainer > SAL_CALL StarBasicAccess_Impl::getLibraryContainer()
    throw(RuntimeException)
{
    if( !mxLibContainer.is() )
        mxLibContainer = (XNameContainer*)new LibraryContainer_Impl( mpMgr );
    return mxLibContainer;
}

void SAL_CALL StarBasicAccess_Impl::createLibrary
(
    const ::rtl::OUString& LibName,
    const ::rtl::OUString& Password,
    const ::rtl::OUString& ExternalSourceURL,
    const ::rtl::OUString& LinkTargetURL
)
    throw(ElementExistException, RuntimeException)
{
    (void)ExternalSourceURL;
#ifdef DBG_UTIL
    StarBASIC* pLib =
#endif
    mpMgr->CreateLib( LibName, Password, LinkTargetURL );
    DBG_ASSERT( pLib, "XML Import: Basic library could not be created");
}

void SAL_CALL StarBasicAccess_Impl::addModule
(
    const ::rtl::OUString& LibraryName,
    const ::rtl::OUString& ModuleName,
    const ::rtl::OUString& Language,
    const ::rtl::OUString& Source
)
    throw(NoSuchElementException, RuntimeException)
{
    (void)Language;
    StarBASIC* pLib = mpMgr->GetLib( LibraryName );
    DBG_ASSERT( pLib, "XML Import: Lib for module unknown");
    if( pLib )
        pLib->MakeModule32( ModuleName, Source );
}

void SAL_CALL StarBasicAccess_Impl::addDialog
(
    const ::rtl::OUString& LibraryName,
    const ::rtl::OUString& DialogName,
    const Sequence< sal_Int8 >& Data
)
    throw(NoSuchElementException, RuntimeException)
{
    (void)LibraryName;
    (void)DialogName;
    (void)Data;
}

// Basic XML Import/Export
Reference< XStarBasicAccess > getStarBasicAccess( BasicManager* pMgr )
{
    Reference< XStarBasicAccess > xRet =
        new StarBasicAccess_Impl( (BasicManager*)pMgr );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
