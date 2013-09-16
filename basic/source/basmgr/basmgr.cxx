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
using namespace com::sun::star;
using namespace com::sun::star::script;
using namespace cppu;

typedef WeakImplHelper1< container::XNameContainer > NameContainerHelper;
typedef WeakImplHelper1< script::XStarBasicModuleInfo > ModuleInfoHelper;
typedef WeakImplHelper1< script::XStarBasicDialogInfo > DialogInfoHelper;
typedef WeakImplHelper1< script::XStarBasicLibraryInfo > LibraryInfoHelper;
typedef WeakImplHelper1< script::XStarBasicAccess > StarBasicAccessHelper;

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

static const char szStdLibName[] = "Standard";
static const char szBasicStorage[] = "StarBASIC";
static const char szOldManagerStream[] = "BasicManager";
static const char szManagerStream[] = "BasicManager2";
static const char szImbedded[] = "LIBIMBEDDED";
static const char szCryptingKey[] = "CryptedBasic";
static const char szScriptLanguage[] = "StarBasic";

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

    BasicManagerImpl( void )
        : mpManagerStream( NULL )
        , mppLibStreams( NULL )
        , mnLibStreamCount( 0 )
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

typedef ::cppu::WeakImplHelper1< container::XContainerListener > ContainerListenerHelper;

class BasMgrContainerListenerImpl: public ContainerListenerHelper
{
    BasicManager* mpMgr;
    OUString maLibName;      // empty -> no lib, but lib container

public:
    BasMgrContainerListenerImpl( BasicManager* pMgr, OUString aLibName )
        : mpMgr( pMgr )
        , maLibName( aLibName ) {}

    static void insertLibraryImpl( const uno::Reference< script::XLibraryContainer >& xScriptCont, BasicManager* pMgr,
                                   uno::Any aLibAny, OUString aLibName );
    static void addLibraryModulesImpl( BasicManager* pMgr, uno::Reference< container::XNameAccess > xLibNameAccess,
                                       OUString aLibName );


    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source )
        throw(uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& Event )
        throw(uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& Event )
        throw(uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& Event )
        throw(uno::RuntimeException);
};


//============================================================================
// BasMgrContainerListenerImpl
//============================================================================

void BasMgrContainerListenerImpl::insertLibraryImpl( const uno::Reference< script::XLibraryContainer >& xScriptCont,
    BasicManager* pMgr, uno::Any aLibAny, OUString aLibName )
{
    Reference< container::XNameAccess > xLibNameAccess;
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

    uno::Reference< container::XContainer> xLibContainer( xLibNameAccess, uno::UNO_QUERY );
    if( xLibContainer.is() )
    {
        // Register listener for library
        Reference< container::XContainerListener > xLibraryListener
            = static_cast< container::XContainerListener* >
                ( new BasMgrContainerListenerImpl( pMgr, aLibName ) );
        xLibContainer->addContainerListener( xLibraryListener );
    }

    if( xScriptCont->isLibraryLoaded( aLibName ) )
    {
        addLibraryModulesImpl( pMgr, xLibNameAccess, aLibName );
    }
}


void BasMgrContainerListenerImpl::addLibraryModulesImpl( BasicManager* pMgr,
    uno::Reference< container::XNameAccess > xLibNameAccess, OUString aLibName )
{
    uno::Sequence< OUString > aModuleNames = xLibNameAccess->getElementNames();
    sal_Int32 nModuleCount = aModuleNames.getLength();

    StarBASIC* pLib = pMgr->GetLib( aLibName );
    DBG_ASSERT( pLib, "BasMgrContainerListenerImpl::addLibraryModulesImpl: Unknown lib!");
    if( pLib )
    {
        const OUString* pNames = aModuleNames.getConstArray();
        for( sal_Int32 j = 0 ; j < nModuleCount ; j++ )
        {
            OUString aModuleName = pNames[ j ];
            uno::Any aElement = xLibNameAccess->getByName( aModuleName );
            OUString aMod;
            aElement >>= aMod;
            uno::Reference< vba::XVBAModuleInfo > xVBAModuleInfo( xLibNameAccess, uno::UNO_QUERY );
            if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( aModuleName ) )
            {
                ModuleInfo mInfo = xVBAModuleInfo->getModuleInfo( aModuleName );
                OSL_TRACE("#addLibraryModulesImpl - aMod");
                pLib->MakeModule32( aModuleName, mInfo, aMod );
            }
            else
        pLib->MakeModule32( aModuleName, aMod );
        }

        pLib->SetModified( sal_False );
    }
}



// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::disposing( const lang::EventObject& Source )
    throw( uno::RuntimeException )
{
    (void)Source;
}

// XContainerListener
//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementInserted( const container::ContainerEvent& Event )
    throw( uno::RuntimeException )
{
    bool bLibContainer = maLibName.isEmpty();
    OUString aName;
    Event.Accessor >>= aName;

    if( bLibContainer )
    {
        uno::Reference< script::XLibraryContainer > xScriptCont( Event.Source, uno::UNO_QUERY );
        insertLibraryImpl( xScriptCont, mpMgr, Event.Element, aName );
        StarBASIC* pLib = mpMgr->GetLib( aName );
        if ( pLib )
        {
            uno::Reference< vba::XVBACompatibility > xVBACompat( xScriptCont, uno::UNO_QUERY );
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
            OUString aMod;
            Event.Element >>= aMod;
                uno::Reference< vba::XVBAModuleInfo > xVBAModuleInfo( Event.Source, uno::UNO_QUERY );
                if ( xVBAModuleInfo.is() && xVBAModuleInfo->hasModuleInfo( aName ) )
                {
                    ModuleInfo mInfo = xVBAModuleInfo->getModuleInfo( aName );
                    pLib->MakeModule32( aName, mInfo, aMod );
                }
                else
                    pLib->MakeModule32( aName, aMod );
                pLib->SetModified( sal_False );
            }
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementReplaced( const container::ContainerEvent& Event )
    throw( uno::RuntimeException )
{
    OUString aName;
    Event.Accessor >>= aName;

    // Replace not possible for library container
#ifdef DBG_UTIL
    bool bLibContainer = maLibName.isEmpty();
#endif
    DBG_ASSERT( !bLibContainer, "library container fired elementReplaced()");

    StarBASIC* pLib = mpMgr->GetLib( maLibName );
    if( pLib )
    {
        SbModule* pMod = pLib->FindModule( aName );
        OUString aMod;
        Event.Element >>= aMod;

        if( pMod )
                pMod->SetSource32( aMod );
        else
                pLib->MakeModule32( aName, aMod );

        pLib->SetModified( sal_False );
    }
}

//----------------------------------------------------------------------------

void SAL_CALL BasMgrContainerListenerImpl::elementRemoved( const container::ContainerEvent& Event )
    throw( uno::RuntimeException )
{
    OUString aName;
    Event.Accessor >>= aName;

    bool bLibContainer = maLibName.isEmpty();
    if( bLibContainer )
    {
        StarBASIC* pLib = mpMgr->GetLib( aName );
        if( pLib )
        {
            sal_uInt16 nLibId = mpMgr->GetLibId( aName );
            mpMgr->RemoveLib( nLibId, sal_False );
        }
    }
    else
    {
        StarBASIC* pLib = mpMgr->GetLib( maLibName );
        SbModule* pMod = pLib ? pLib->FindModule( aName ) : NULL;
        if( pMod )
        {
            pLib->Remove( pMod );
            pLib->SetModified( sal_False );
        }
    }
}

BasicError::BasicError( sal_uIntPtr nId, sal_uInt16 nR, const OUString& rErrStr ) :
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
    OUString          aLibName;
    OUString          aStorageName;   // string is sufficient, unique at runtime
    OUString          aRelStorageName;
    OUString          aPassword;

    sal_Bool            bDoLoad;
    sal_Bool            bReference;
    sal_Bool            bPasswordVerified;

    // Lib represents library in new UNO library container
    uno::Reference< script::XLibraryContainer > mxScriptCont;

public:
    BasicLibInfo();

    sal_Bool            IsReference() const     { return bReference; }
    sal_Bool&           IsReference()           { return bReference; }

    sal_Bool            IsExtern() const        { return ! aStorageName.equalsAscii(szImbedded); }

    void            SetStorageName( const OUString& rName )   { aStorageName = rName; }
    const OUString&   GetStorageName() const                  { return aStorageName; }

    void            SetRelStorageName( const OUString& rN )   { aRelStorageName = rN; }
    const OUString&   GetRelStorageName() const               { return aRelStorageName; }

    StarBASICRef    GetLib() const
    {
        if( mxScriptCont.is() && mxScriptCont->hasByName( aLibName ) &&
            !mxScriptCont->isLibraryLoaded( aLibName ) )
                return StarBASICRef();
        return xLib;
    }
    StarBASICRef&   GetLibRef()                         { return xLib; }
    void            SetLib( StarBASIC* pBasic )         { xLib = pBasic; }

    const OUString&   GetLibName() const                  { return aLibName; }
    void            SetLibName( const OUString& rName )   { aLibName = rName; }

    // Only temporary for Load/Save
    sal_Bool            DoLoad()                            { return bDoLoad; }

    sal_Bool            HasPassword() const                 { return !aPassword.isEmpty(); }
    const OUString&   GetPassword() const                 { return aPassword; }
    void            SetPassword( const OUString& rNewPassword )
                                                        { aPassword = rNewPassword; }
    sal_Bool            IsPasswordVerified() const          { return bPasswordVerified; }
    void            SetPasswordVerified()               { bPasswordVerified = sal_True; }

    static BasicLibInfo*    Create( SotStorageStream& rSStream );

    uno::Reference< script::XLibraryContainer > GetLibraryContainer( void )
        { return mxScriptCont; }
    void SetLibraryContainer( const uno::Reference< script::XLibraryContainer >& xScriptCont )
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
    OUString          aBasicLibPath; // TODO: Should be member of manager, but currently not incompatible
    BasicLibInfo*   GetObject( size_t i );
    BasicLibInfo*   First();
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
    vector< BasicLibInfo* >::iterator it, eit = aList.end();
    for (it = aList.begin(); it != eit; ++it)
    {
        if (*it == LibInfo)
        {
            aList.erase(it);
            break;
        }
    }
    return LibInfo;
}


//=====================================================================

BasicLibInfo::BasicLibInfo()
{
    bReference          = sal_False;
    bPasswordVerified   = sal_False;
    bDoLoad             = sal_False;
    mxScriptCont        = NULL;
    aStorageName        = szImbedded;
    aRelStorageName     = szImbedded;
}

BasicLibInfo* BasicLibInfo::Create( SotStorageStream& rSStream )
{
    BasicLibInfo* pInfo = new BasicLibInfo;

    sal_uInt32 nEndPos;
    sal_uInt16 nId;
    sal_uInt16 nVer;

    rSStream >> nEndPos;
    rSStream >> nId;
    rSStream >> nVer;

    DBG_ASSERT( nId == LIBINFO_ID, "No BasicLibInfo?!" );
    if( nId == LIBINFO_ID )
    {
        // Reload?
        sal_Bool bDoLoad;
        rSStream >> bDoLoad;
        pInfo->bDoLoad = bDoLoad;

        // The name of the lib...
        OUString aName = rSStream.ReadUniOrByteString(rSStream.GetStreamCharSet());
        pInfo->SetLibName( aName );

        // Absolute path...
        OUString aStorageName = rSStream.ReadUniOrByteString(rSStream.GetStreamCharSet());
        pInfo->SetStorageName( aStorageName );

        // Relative path...
        OUString aRelStorageName = rSStream.ReadUniOrByteString(rSStream.GetStreamCharSet());
        pInfo->SetRelStorageName( aRelStorageName );

        if ( nVer >= 2 )
        {
            sal_Bool bReferenz;
            rSStream >> bReferenz;
            pInfo->IsReference() = bReferenz;
        }

        rSStream.Seek( nEndPos );
    }
    return pInfo;
}

BasicManager::BasicManager( SotStorage& rStorage, const OUString& rBaseURL, StarBASIC* pParentFromStdLib, OUString* pLibPath, bool bDocMgr ) : mbDocMgr( bDocMgr )
{
    DBG_CTOR( BasicManager, 0 );

    Init();

    if( pLibPath )
    {
        pLibs->aBasicLibPath = *pLibPath;
    }
    OUString aStorName( rStorage.GetName() );
    maStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );


    // If there is no Manager Stream, no further actions are necessary
    if ( rStorage.IsStream( OUString(szManagerStream) ) )
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
            xStdLib->SetName( OUString(szStdLibName) );
            pStdLibInfo->SetLibName( OUString(szStdLibName) );
            xStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
            xStdLib->SetModified( sal_False );
        }
        else
        {
            pStdLib->SetParent( pParentFromStdLib );
            // The other get StdLib as parent:
            for ( sal_uInt16 nBasic = 1; nBasic < GetLibCount(); nBasic++ )
            {
                StarBASIC* pBasic = GetLib( nBasic );
                if ( pBasic )
                {
                    pStdLib->Insert( pBasic );
                    pBasic->SetFlag( SBX_EXTSEARCH );
                }
            }
            // Modified through insert
            pStdLib->SetModified( sal_False );
        }

        // #91626 Save all stream data to save it unmodified if basic isn't modified
        // in an 6.0+ office. So also the old basic dialogs can be saved.
        SotStorageStreamRef xManagerStream = rStorage.OpenSotStream( OUString(szManagerStream), eStreamReadMode );
        mpImpl->mpManagerStream = new SvMemoryStream();
        *static_cast<SvStream*>(&xManagerStream) >> *mpImpl->mpManagerStream;

        SotStorageRef xBasicStorage = rStorage.OpenSotStorage( OUString(szBasicStorage), eStorageReadMode, sal_False );
        if( xBasicStorage.Is() && !xBasicStorage->GetError() )
        {
            sal_uInt16 nLibs = GetLibCount();
            mpImpl->mppLibStreams = new SvMemoryStream*[ nLibs ];
            for( sal_uInt16 nL = 0; nL < nLibs; nL++ )
            {
                BasicLibInfo* pInfo = pLibs->GetObject( nL );
                DBG_ASSERT( pInfo, "pInfo?!" );
                SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pInfo->GetLibName(), eStreamReadMode );
                mpImpl->mppLibStreams[nL] = new SvMemoryStream();
                *static_cast<SvStream*>(&xBasicStream) >> *( mpImpl->mppLibStreams[nL] );
            }
        }
    }
    else
    {
        ImpCreateStdLib( pParentFromStdLib );
        if ( rStorage.IsStream( OUString(szOldManagerStream) ) )
            LoadOldBasicManager( rStorage );
    }
}

void copyToLibraryContainer( StarBASIC* pBasic, const LibraryContainerInfo& rInfo )
{
    uno::Reference< script::XLibraryContainer > xScriptCont( rInfo.mxScriptCont.get() );
    if ( !xScriptCont.is() )
        return;

    OUString aLibName = pBasic->GetName();
    if( !xScriptCont->hasByName( aLibName ) )
        xScriptCont->createLibrary( aLibName );

    uno::Any aLibAny = xScriptCont->getByName( aLibName );
    uno::Reference< container::XNameContainer > xLib;
    aLibAny >>= xLib;
    if ( !xLib.is() )
        return;

    sal_uInt16 nModCount = pBasic->GetModules()->Count();
    for ( sal_uInt16 nMod = 0 ; nMod < nModCount ; nMod++ )
    {
        SbModule* pModule = (SbModule*)pBasic->GetModules()->Get( nMod );
        DBG_ASSERT( pModule, "Module not received!" );

        OUString aModName = pModule->GetName();
        if( !xLib->hasByName( aModName ) )
        {
            OUString aSource = pModule->GetSource32();
            uno::Any aSourceAny;
            aSourceAny <<= aSource;
            xLib->insertByName( aModName, aSourceAny );
        }
    }
}

const uno::Reference< script::XPersistentLibraryContainer >& BasicManager::GetDialogLibraryContainer()  const
{
    return mpImpl->maContainerInfo.mxDialogCont;
}

const uno::Reference< script::XPersistentLibraryContainer >& BasicManager::GetScriptLibraryContainer()  const
{
    return mpImpl->maContainerInfo.mxScriptCont;
}

void BasicManager::SetLibraryContainerInfo( const LibraryContainerInfo& rInfo )
{
    mpImpl->maContainerInfo = rInfo;

    uno::Reference< script::XLibraryContainer > xScriptCont( mpImpl->maContainerInfo.mxScriptCont.get() );
    if( xScriptCont.is() )
    {
        // Register listener for lib container
        OUString aEmptyLibName;
        uno::Reference< container::XContainerListener > xLibContainerListener
            = static_cast< container::XContainerListener* >
                ( new BasMgrContainerListenerImpl( this, aEmptyLibName ) );

        uno::Reference< container::XContainer> xLibContainer( xScriptCont, uno::UNO_QUERY );
        xLibContainer->addContainerListener( xLibContainerListener );

        uno::Sequence< OUString > aScriptLibNames = xScriptCont->getElementNames();
        const OUString* pScriptLibName = aScriptLibNames.getConstArray();
        sal_Int32 i, nNameCount = aScriptLibNames.getLength();

        if( nNameCount )
        {
            for( i = 0 ; i < nNameCount ; ++i, ++pScriptLibName )
            {
                uno::Any aLibAny = xScriptCont->getByName( *pScriptLibName );

                if ( *pScriptLibName == "Standard" )
                    xScriptCont->loadLibrary( *pScriptLibName );

                BasMgrContainerListenerImpl::insertLibraryImpl
                    ( xScriptCont, this, aLibAny, *pScriptLibName );
            }
        }
        else
        {
            // No libs? Maybe an 5.2 document already loaded
            sal_uInt16 nLibs = GetLibCount();
            for( sal_uInt16 nL = 0; nL < nLibs; nL++ )
            {
                BasicLibInfo* pBasLibInfo = pLibs->GetObject( nL );
                StarBASIC* pLib = pBasLibInfo->GetLib();
                if( !pLib )
                {
                    sal_Bool bLoaded = ImpLoadLibrary( pBasLibInfo, NULL, sal_False );
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
        }
    }

    SetGlobalUNOConstant( "BasicLibraries", makeAny( mpImpl->maContainerInfo.mxScriptCont ) );
    SetGlobalUNOConstant( "DialogLibraries", makeAny( mpImpl->maContainerInfo.mxDialogCont ) );
}

BasicManager::BasicManager( StarBASIC* pSLib, OUString* pLibPath, bool bDocMgr ) : mbDocMgr( bDocMgr )
{
    DBG_CTOR( BasicManager, 0 );
    Init();
    DBG_ASSERT( pSLib, "BasicManager cannot be created with a NULL-Pointer!" );

    if( pLibPath )
    {
        pLibs->aBasicLibPath = *pLibPath;
    }
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( pSLib );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName(OUString(szStdLibName));
    pStdLibInfo->SetLibName(OUString(szStdLibName) );
    pSLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );

    // Save is only necessary if basic has changed
    xStdLib->SetModified( sal_False );
}

void BasicManager::ImpMgrNotLoaded( const OUString& rStorageName )
{
    // pErrInf is only destroyed if the error os processed by an
    // ErrorHandler
    StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, rStorageName, ERRCODE_BUTTON_OK );
    aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_OPENMGRSTREAM, rStorageName));

    // Create a stdlib otherwise we crash!
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( new StarBASIC( NULL, mbDocMgr ) );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( OUString(szStdLibName) );
    pStdLibInfo->SetLibName( OUString(szStdLibName) );
    xStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
    xStdLib->SetModified( sal_False );
}


void BasicManager::ImpCreateStdLib( StarBASIC* pParentFromStdLib )
{
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    StarBASIC* pStdLib = new StarBASIC( pParentFromStdLib, mbDocMgr );
    pStdLibInfo->SetLib( pStdLib );
    pStdLib->SetName( OUString(szStdLibName) );
    pStdLibInfo->SetLibName( OUString(szStdLibName) );
    pStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
}

void BasicManager::LoadBasicManager( SotStorage& rStorage, const OUString& rBaseURL, sal_Bool bLoadLibs )
{
    DBG_CHKTHIS( BasicManager, 0 );


    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream( OUString(szManagerStream), eStreamReadMode );

    OUString aStorName( rStorage.GetName() );
    // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.Is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    maStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    // #i13114 removed, DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    OUString aRealStorageName = maStorageName;  // for relative paths, can be modified through BaseURL

    if ( !rBaseURL.isEmpty() )
    {
        INetURLObject aObj( rBaseURL );
        if ( aObj.GetProtocol() == INET_PROT_FILE )
        {
            aRealStorageName = aObj.PathToFileName();
        }
    }

    xManagerStream->SetBufferSize( 1024 );
    xManagerStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_uInt32 nEndPos;
    *xManagerStream >> nEndPos;

    sal_uInt16 nLibs;
    *xManagerStream >> nLibs;
    // Plausibility!
    if( nLibs & 0xF000 )
    {
        DBG_ASSERT( !this, "BasicManager-Stream defect!" );
        return;
    }
    for ( sal_uInt16 nL = 0; nL < nLibs; nL++ )
    {
        BasicLibInfo* pInfo = BasicLibInfo::Create( *xManagerStream );

        // Correct absolute pathname if relative is existing.
        // Always try relative first if there are two stands on disk
        if ( !pInfo->GetRelStorageName().isEmpty() && ( ! pInfo->GetRelStorageName().equalsAscii(szImbedded) ) )
        {
            INetURLObject aObj( aRealStorageName, INET_PROT_FILE );
            aObj.removeSegment();
            bool bWasAbsolute = sal_False;
            aObj = aObj.smartRel2Abs( pInfo->GetRelStorageName(), bWasAbsolute );

            //*** TODO: Replace if still necessary
            //*** TODO-End
            if ( ! pLibs->aBasicLibPath.isEmpty() )
            {
                // Search lib in path
                OUString aSearchFile = pInfo->GetRelStorageName();
                OUString aSearchFileOldFormat(aSearchFile);
                SvtPathOptions aPathCFG;
                if( aPathCFG.SearchFile( aSearchFileOldFormat, SvtPathOptions::PATH_BASIC ) )
                {
                    pInfo->SetStorageName( aSearchFile );
                }
            }
        }

        pLibs->Insert( pInfo );
        // Libs from external files should be loaded only when necessary.
        // But references are loaded at once, otherwise some big customers get into trouble
        if ( bLoadLibs && pInfo->DoLoad() &&
            ( !pInfo->IsExtern() ||  pInfo->IsReference()))
        {
            ImpLoadLibrary( pInfo, &rStorage );
        }
    }

    xManagerStream->Seek( nEndPos );
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.Clear();
}

void BasicManager::LoadOldBasicManager( SotStorage& rStorage )
{
    DBG_CHKTHIS( BasicManager, 0 );


    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream( OUString(szOldManagerStream), eStreamReadMode );

    OUString aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.getLength(), "No Storage Name!" );

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

    DBG_ASSERT( !xManagerStream->GetError(), "Invalid Manager-Stream!" );

    xManagerStream->Seek( nBasicStartOff );
    if( !ImplLoadBasic( *xManagerStream, pLibs->GetObject(0)->GetLibRef() ) )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, aStorName, ERRCODE_BUTTON_OK );
        aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_OPENMGRSTREAM, aStorName));
        // and it proceeds ...
    }
    xManagerStream->Seek( nBasicEndOff+1 ); // +1: 0x00 as separator
    OUString aLibs = xManagerStream->ReadUniOrByteString(xManagerStream->GetStreamCharSet());
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.Clear(); // Close stream

    if ( !aLibs.isEmpty() )
    {
        OUString aCurStorageName( aStorName );
        INetURLObject aCurStorage( aCurStorageName, INET_PROT_FILE );
        sal_Int32 nLibs = comphelper::string::getTokenCount(aLibs, LIB_SEP);
        for ( sal_Int32 nLib = 0; nLib < nLibs; nLib++ )
        {
            OUString aLibInfo(comphelper::string::getToken(aLibs, nLib, LIB_SEP));
            // TODO: Remove == 2
            DBG_ASSERT( ( comphelper::string::getTokenCount(aLibInfo, LIBINFO_SEP) == 2 ) || ( comphelper::string::getTokenCount(aLibInfo, LIBINFO_SEP) == 3 ), "Invalid Lib-Info!" );
            OUString aLibName( aLibInfo.getToken( 0, LIBINFO_SEP ) );
            OUString aLibAbsStorageName( aLibInfo.getToken( 1, LIBINFO_SEP ) );
            OUString aLibRelStorageName( aLibInfo.getToken( 2, LIBINFO_SEP ) );
            INetURLObject aLibAbsStorage( aLibAbsStorageName, INET_PROT_FILE );

            INetURLObject aLibRelStorage( aStorName );
            aLibRelStorage.removeSegment();
            bool bWasAbsolute = sal_False;
            aLibRelStorage = aLibRelStorage.smartRel2Abs( aLibRelStorageName, bWasAbsolute);
            DBG_ASSERT(!bWasAbsolute, "RelStorageName was absolute!" );

            SotStorageRef xStorageRef;
            if ( ( aLibAbsStorage == aCurStorage ) || ( aLibRelStorageName.equalsAscii(szImbedded) ) )
            {
                xStorageRef = &rStorage;
            }
            else
            {
                xStorageRef = new SotStorage( sal_False, aLibAbsStorage.GetMainURL
                    ( INetURLObject::NO_DECODE ), eStorageReadMode, sal_True );
                if ( xStorageRef->GetError() != ERRCODE_NONE )
                    xStorageRef = new SotStorage( sal_False, aLibRelStorage.
                    GetMainURL( INetURLObject::NO_DECODE ), eStorageReadMode, sal_True );
            }
            if ( xStorageRef.Is() )
            {
                AddLib( *xStorageRef, aLibName, sal_False );
            }
            else
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, aStorName, ERRCODE_BUTTON_OK );
                aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_STORAGENOTFOUND, aStorName));
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
    delete mpImpl;
}

void BasicManager::LegacyDeleteBasicManager( BasicManager*& _rpManager )
{
    delete _rpManager;
    _rpManager = NULL;
}


bool BasicManager::HasExeCode( const OUString& sLib )
{
    StarBASIC* pLib = GetLib(sLib);
    if ( pLib )
    {
        SbxArray* pMods = pLib->GetModules();
        sal_uInt16 nMods = pMods ? pMods->Count() : 0;
        for( sal_uInt16 i = 0; i < nMods; i++ )
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

sal_Bool BasicManager::ImpLoadLibrary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, sal_Bool bInfosOnly )
{
    DBG_CHKTHIS( BasicManager, 0 );

    DBG_ASSERT( pLibInfo, "LibInfo!?" );

    OUString aStorageName( pLibInfo->GetStorageName() );
    if ( aStorageName.isEmpty() || ( aStorageName.equalsAscii(szImbedded) ) )
    {
        aStorageName = GetStorageName();
    }
    SotStorageRef xStorage;
    // The current must not be opened again...
    if ( pCurStorage )
    {
        OUString aStorName( pCurStorage->GetName() );
        // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

        INetURLObject aCurStorageEntry(aStorName, INET_PROT_FILE);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::NO_DECODE ).Len() != 0, "Bad storage name");

        INetURLObject aStorageEntry(aStorageName, INET_PROT_FILE);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::NO_DECODE ).Len() != 0, "Bad storage name");

        if ( aCurStorageEntry == aStorageEntry )
        {
            xStorage = pCurStorage;
        }
    }

    if ( !xStorage.Is() )
    {
        xStorage = new SotStorage( sal_False, aStorageName, eStorageReadMode );
    }
    SotStorageRef xBasicStorage = xStorage->OpenSotStorage( OUString(szBasicStorage), eStorageReadMode, sal_False );

    if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, xStorage->GetName(), ERRCODE_BUTTON_OK );
        aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName()));
    }
    else
    {
        // In the Basic-Storage every lib is in a Stream...
        SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pLibInfo->GetLibName(), eStreamReadMode );
        if ( !xBasicStream.Is() || xBasicStream->GetError() )
        {
            StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD , pLibInfo->GetLibName(), ERRCODE_BUTTON_OK );
            aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_OPENLIBSTREAM, pLibInfo->GetLibName()));
        }
        else
        {
            sal_Bool bLoaded = sal_False;
            if ( xBasicStream->Seek( STREAM_SEEK_TO_END ) != 0 )
            {
                if ( !bInfosOnly )
                {
                    if ( !pLibInfo->GetLib().Is() )
                    {
                        pLibInfo->SetLib( new StarBASIC( GetStdLib(), mbDocMgr ) );
                    }
                    xBasicStream->SetBufferSize( 1024 );
                    xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                    bLoaded = ImplLoadBasic( *xBasicStream, pLibInfo->GetLibRef() );
                    xBasicStream->SetBufferSize( 0 );
                    StarBASICRef xStdLib = pLibInfo->GetLib();
                    xStdLib->SetName( pLibInfo->GetLibName() );
                    xStdLib->SetModified( sal_False );
                    xStdLib->SetFlag( SBX_DONTSTORE );
                }
                else
                {
                    // Skip Basic...
                    xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                    ImplEncryptStream( *xBasicStream );
                    SbxBase::Skip( *xBasicStream );
                    bLoaded = sal_True;
                }
            }
            if ( !bLoaded )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, pLibInfo->GetLibName(), ERRCODE_BUTTON_OK );
                aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_BASICLOADERROR, pLibInfo->GetLibName()));
            }
            else
            {
                // Perhaps there are additional information in the stream...
                xBasicStream->SetCryptMaskKey(szCryptingKey);
                xBasicStream->RefreshBuffer();
                sal_uInt32 nPasswordMarker = 0;
                *xBasicStream >> nPasswordMarker;
                if ( ( nPasswordMarker == PASSWORD_MARKER ) && !xBasicStream->IsEof() )
                {
                    OUString aPassword = xBasicStream->ReadUniOrByteString(
                        xBasicStream->GetStreamCharSet());
                    pLibInfo->SetPassword( aPassword );
                }
                xBasicStream->SetCryptMaskKey(OString());
                CheckModules( pLibInfo->GetLib(), pLibInfo->IsReference() );
            }
            return bLoaded;
        }
    }
    return sal_False;
}

sal_Bool BasicManager::ImplEncryptStream( SvStream& rStrm ) const
{
    sal_uIntPtr nPos = rStrm.Tell();
    sal_uInt32 nCreator;
    rStrm >> nCreator;
    rStrm.Seek( nPos );
    sal_Bool bProtected = sal_False;
    if ( nCreator != SBXCR_SBX )
    {
        // Should only be the case for encrypted Streams
        bProtected = sal_True;
        rStrm.SetCryptMaskKey(szCryptingKey);
        rStrm.RefreshBuffer();
    }
    return bProtected;
}

// This code is necessary to load the BASIC of Beta 1
// TODO: Which Beta 1?
sal_Bool BasicManager::ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const
{
    sal_Bool bProtected = ImplEncryptStream( rStrm );
    SbxBaseRef xNew = SbxBase::Load( rStrm );
    sal_Bool bLoaded = sal_False;
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
                {
                    pNew->GetParent()->Insert( pNew );
                }
                pNew->SetFlag( SBX_EXTSEARCH );
            }
            rOldBasic = pNew;

            // Fill new libray container (5.2 -> 6.0)
            copyToLibraryContainer( pNew, mpImpl->maContainerInfo );

            pNew->SetModified( sal_False );
            bLoaded = sal_True;
        }
    }
    if ( bProtected )
    {
        rStrm.SetCryptMaskKey(OString());
    }
    return bLoaded;
}

void BasicManager::CheckModules( StarBASIC* pLib, sal_Bool bReference ) const
{
    if ( !pLib )
    {
        return;
    }
    sal_Bool bModified = pLib->IsModified();

    for ( sal_uInt16 nMod = 0; nMod < pLib->GetModules()->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pLib->GetModules()->Get( nMod );
        DBG_ASSERT( pModule, "Module not received!" );
        if ( !pModule->IsCompiled() && !StarBASIC::GetErrorCode() )
        {
            pLib->Compile( pModule );
        }
    }

    // #67477, AB 8.12.99 On demand compile in referenced libs should not
    // cause modified
    if( !bModified && bReference )
    {
        OSL_FAIL( "Referenced basic library is not compiled!" );
        pLib->SetModified( sal_False );
    }
}

StarBASIC* BasicManager::AddLib( SotStorage& rStorage, const OUString& rLibName, sal_Bool bReference )
{
    DBG_CHKTHIS( BasicManager, 0 );

    OUString aStorName( rStorage.GetName() );
    DBG_ASSERT( !aStorName.isEmpty(), "No Storage Name!" );

    OUString aStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    DBG_ASSERT(!aStorageName.isEmpty(), "Bad storage name");

    OUString aNewLibName( rLibName );
    while ( HasLib( aNewLibName ) )
    {
        aNewLibName += "_";
    }
    BasicLibInfo* pLibInfo = CreateLibInfo();
    // Use original name otherwise ImpLoadLibrary failes...
    pLibInfo->SetLibName( rLibName );
    // but doesn't work this way if name exists twice
    sal_uInt16 nLibId = (sal_uInt16) pLibs->GetPos( pLibInfo );

    // Set StorageName before load because it is compared with pCurStorage
    pLibInfo->SetStorageName( aStorageName );
    sal_Bool bLoaded = ImpLoadLibrary( pLibInfo, &rStorage );

    if ( bLoaded )
    {
        if ( aNewLibName != rLibName )
        {
            SetLibName( nLibId, aNewLibName );
        }
        if ( bReference )
        {
            pLibInfo->GetLib()->SetModified( sal_False );   // Don't save in this case
            pLibInfo->SetRelStorageName( OUString() );
            pLibInfo->IsReference() = sal_True;
        }
        else
        {
            pLibInfo->GetLib()->SetModified( sal_True ); // Must be saved after Add!
            pLibInfo->SetStorageName( OUString(szImbedded) ); // Save in BasicManager-Storage
        }
    }
    else
    {
        RemoveLib( nLibId, sal_False );
        pLibInfo = 0;
    }

    return pLibInfo ? &*pLibInfo->GetLib() : 0;

}

sal_Bool BasicManager::IsReference( sal_uInt16 nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
    {
        return pLibInfo->IsReference();
    }
    return sal_False;
}

sal_Bool BasicManager::RemoveLib( sal_uInt16 nLib )
{
    // Only pyhsical deletion if no reference
    return RemoveLib( nLib, !IsReference( nLib ) );
}

sal_Bool BasicManager::RemoveLib( sal_uInt16 nLib, sal_Bool bDelBasicFromStorage )
{
    DBG_CHKTHIS( BasicManager, 0 );
    DBG_ASSERT( nLib, "Standard-Lib cannot be removed!" );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib not found!" );

    if ( !pLibInfo || !nLib )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, OUString(), ERRCODE_BUTTON_OK );
        aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_STDLIB, pLibInfo->GetLibName()));
        return sal_False;
    }

    // If one of the streams cannot be opened, this is not an error,
    // because BASIC was never written before...
    if ( bDelBasicFromStorage && !pLibInfo->IsReference() &&
            ( !pLibInfo->IsExtern() || SotStorage::IsStorageFile( pLibInfo->GetStorageName() ) ) )
    {
        SotStorageRef xStorage;
        if ( !pLibInfo->IsExtern() )
        {
            xStorage = new SotStorage( sal_False, GetStorageName() );
        }
        else
        {
            xStorage = new SotStorage( sal_False, pLibInfo->GetStorageName() );
        }

        if ( xStorage->IsStorage( OUString(szBasicStorage) ) )
        {
            SotStorageRef xBasicStorage = xStorage->OpenSotStorage
                            ( OUString(szBasicStorage), STREAM_STD_READWRITE, sal_False );

            if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, OUString(), ERRCODE_BUTTON_OK );
                aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName()));
            }
            else if ( xBasicStorage->IsStream( pLibInfo->GetLibName() ) )
            {
                xBasicStorage->Remove( pLibInfo->GetLibName() );
                xBasicStorage->Commit();

                // If no further stream available,
                // delete the SubStorage.
                SvStorageInfoList aInfoList;
                xBasicStorage->FillInfoList( &aInfoList );
                if ( aInfoList.empty() )
                {
                    xBasicStorage.Clear();
                    xStorage->Remove( OUString(szBasicStorage) );
                    xStorage->Commit();
                    // If no further Streams or SubStorages available,
                    // delete the Storage, too.
                    aInfoList.clear();
                    xStorage->FillInfoList( &aInfoList );
                    if ( aInfoList.empty() )
                    {
                        //OUString aName_( xStorage->GetName() );
                        xStorage.Clear();
                        //*** TODO: Replace if still necessary
                        //SfxContentHelper::Kill( aName );
                        //*** TODO-End
                    }
                }
            }
        }
    }
    if ( pLibInfo->GetLib().Is() )
    {
        GetStdLib()->Remove( pLibInfo->GetLib() );
    }
    delete pLibs->Remove( pLibInfo );
    return sal_True;    // Remove was successful, del unimportant
}

sal_uInt16 BasicManager::GetLibCount() const
{
    DBG_CHKTHIS( BasicManager, 0 );
    return (sal_uInt16)pLibs->Count();
}

StarBASIC* BasicManager::GetLib( sal_uInt16 nLib ) const
{
    DBG_CHKTHIS( BasicManager, 0 );
    BasicLibInfo* pInf = pLibs->GetObject( nLib );
    DBG_ASSERT( pInf, "Lib does not exist!" );
    if ( pInf )
    {
        return pInf->GetLib();
    }
    return 0;
}

StarBASIC* BasicManager::GetStdLib() const
{
    DBG_CHKTHIS( BasicManager, 0 );
    StarBASIC* pLib = GetLib( 0 );
    return pLib;
}

StarBASIC* BasicManager::GetLib( const OUString& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().equalsIgnoreAsciiCase( rName ))// Check if available...
        {
            return pInf->GetLib();
        }
        pInf = pLibs->Next();
    }
    return 0;
}

sal_uInt16 BasicManager::GetLibId( const OUString& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().equalsIgnoreAsciiCase( rName ))
        {
            return (sal_uInt16)pLibs->GetCurPos();
        }
        pInf = pLibs->Next();
    }
    return LIB_NOTFOUND;
}

sal_Bool BasicManager::HasLib( const OUString& rName ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLibName().equalsIgnoreAsciiCase(rName))
        {
            return sal_True;
        }
        pInf = pLibs->Next();
    }
    return sal_False;
}

sal_Bool BasicManager::SetLibName( sal_uInt16 nLib, const OUString& rName )
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
            xStdLib->SetModified( sal_True );
        }
        return sal_True;
    }
    return sal_False;
}

OUString BasicManager::GetLibName( sal_uInt16 nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
    {
        return pLibInfo->GetLibName();
    }
    return OUString();
}

sal_Bool BasicManager::LoadLib( sal_uInt16 nLib )
{
    DBG_CHKTHIS( BasicManager, 0 );

    sal_Bool bDone = sal_False;
    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib?!" );
    if ( pLibInfo )
    {
        uno::Reference< script::XLibraryContainer > xLibContainer = pLibInfo->GetLibraryContainer();
        if( xLibContainer.is() )
        {
            OUString aLibName = pLibInfo->GetLibName();
            xLibContainer->loadLibrary( aLibName );
            bDone = xLibContainer->isLibraryLoaded( aLibName );;
        }
        else
        {
            bDone = ImpLoadLibrary( pLibInfo, NULL, sal_False );
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
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, OUString(), ERRCODE_BUTTON_OK );
        aErrors.push_back(BasicError(*pErrInf, BASERR_REASON_LIBNOTFOUND, OUString::number(nLib)));
    }
    return bDone;
}

StarBASIC* BasicManager::CreateLib( const OUString& rLibName )
{
    DBG_CHKTHIS( BasicManager, 0 );
    if ( GetLib( rLibName ) )
    {
        return 0;
    }
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
StarBASIC* BasicManager::CreateLib( const OUString& rLibName, const OUString& Password,
                                    const OUString& LinkTargetURL )
{
    // Ask if lib exists because standard lib is always there
    StarBASIC* pLib = GetLib( rLibName );
    if( !pLib )
    {
        if( !LinkTargetURL.isEmpty())
        {
            SotStorageRef xStorage = new SotStorage( sal_False, LinkTargetURL, STREAM_READ | STREAM_SHARE_DENYWRITE );
            if( !xStorage->GetError() )
            {
                pLib = AddLib( *xStorage, rLibName, sal_True );
            }

            DBG_ASSERT( pLib, "XML Import: Linked basic library could not be loaded");
        }
        else
        {
            pLib = CreateLib( rLibName );
            if( Password.isEmpty())
            {
                BasicLibInfo* pLibInfo = FindLibInfo( pLib );
                pLibInfo ->SetPassword( Password );
            }
        }
        //ExternalSourceURL ?
    }
    return pLib;
}

StarBASIC* BasicManager::CreateLibForLibContainer( const OUString& rLibName,
    const uno::Reference< script::XLibraryContainer >& xScriptCont )
{
    DBG_CHKTHIS( BasicManager, 0 );
    if ( GetLib( rLibName ) )
    {
        return 0;
    }
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
        {
            return pInf;
        }
        pInf = ((BasicManager*)this)->pLibs->Next();
    }
    return 0;
}


sal_Bool BasicManager::IsBasicModified() const
{
    DBG_CHKTHIS( BasicManager, 0 );

    BasicLibInfo* pInf = pLibs->First();
    while ( pInf )
    {
        if ( pInf->GetLib().Is() && pInf->GetLib()->IsModified() )
        {
            return sal_True;
        }
        pInf = pLibs->Next();
    }
    return sal_False;
}

std::vector<BasicError>& BasicManager::GetErrors()
{
    return aErrors;
}

bool BasicManager::GetGlobalUNOConstant( const sal_Char* _pAsciiName, uno::Any& aOut )
{
    bool bRes = false;
    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::GetGlobalUNOConstant: no lib to read from!" );
    if ( pStandardLib )
        bRes = pStandardLib->GetUNOConstant( _pAsciiName, aOut );
    return bRes;
}

uno::Any BasicManager::SetGlobalUNOConstant( const sal_Char* _pAsciiName, const uno::Any& _rValue )
{
    uno::Any aOldValue;

    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::SetGlobalUNOConstant: no lib to insert into!" );
    if ( !pStandardLib )
        return aOldValue;

    OUString sVarName( OUString::createFromAscii( _pAsciiName ) );

    // obtain the old value
    SbxVariable* pVariable = pStandardLib->Find( sVarName, SbxCLASS_OBJECT );
    if ( pVariable )
        aOldValue = sbxToUnoValue( pVariable );

    SbxObjectRef xUnoObj = GetSbUnoObject( sVarName, _rValue );
    xUnoObj->SetFlag( SBX_DONTSTORE );
    pStandardLib->Insert( xUnoObj );

    return aOldValue;
}

bool BasicManager::LegacyPsswdBinaryLimitExceeded( uno::Sequence< OUString >& _out_rModuleNames )
{
    try
    {
        uno::Reference< container::XNameAccess > xScripts( GetScriptLibraryContainer(), uno::UNO_QUERY_THROW );
        uno::Reference< script::XLibraryContainerPassword > xPassword( GetScriptLibraryContainer(), uno::UNO_QUERY_THROW );

        uno::Sequence< OUString > aNames( xScripts->getElementNames() );
        const OUString* pNames = aNames.getConstArray();
        const OUString* pNamesEnd = aNames.getConstArray() + aNames.getLength();
        for ( ; pNames != pNamesEnd; ++pNames )
        {
            if( !xPassword->isLibraryPasswordProtected( *pNames ) )
                continue;

            StarBASIC* pBasicLib = GetLib( *pNames );
            if ( !pBasicLib )
                continue;

            uno::Reference< container::XNameAccess > xScriptLibrary( xScripts->getByName( *pNames ), uno::UNO_QUERY_THROW );
            uno::Sequence< OUString > aElementNames( xScriptLibrary->getElementNames() );
            sal_Int32 nLen = aElementNames.getLength();

            uno::Sequence< OUString > aBigModules( nLen );
            sal_Int32 nBigModules = 0;

            const OUString* pElementNames = aElementNames.getConstArray();
            const OUString* pElementNamesEnd = aElementNames.getConstArray() + aElementNames.getLength();
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
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}


namespace
{
    SbMethod* lcl_queryMacro( BasicManager* i_manager, OUString const& i_fullyQualifiedName )
    {
        sal_Int32 nLast = 0;
        const OUString sParse = i_fullyQualifiedName;
        OUString sLibName = sParse.getToken( (sal_Int32)0, (sal_Unicode)'.', nLast );
        OUString sModule = sParse.getToken( (sal_Int32)0, (sal_Unicode)'.', nLast );
        OUString sMacro;
        if(nLast >= 0)
        {
            sMacro = OUString(sParse.getStr() + nLast, sParse.getLength() - nLast );
        }
        else
        {
            sMacro = sParse;
        }

        utl::TransliterationWrapper& rTransliteration = SbGlobal::GetTransliteration();
        sal_uInt16 nLibCount = i_manager->GetLibCount();
        for ( sal_uInt16 nLib = 0; nLib < nLibCount; ++nLib )
        {
            if ( rTransliteration.isEqual( i_manager->GetLibName( nLib ), sLibName ) )
            {
                StarBASIC* pLib = i_manager->GetLib( nLib );
                if( !pLib )
                {
                    i_manager->LoadLib( nLib );
                    pLib = i_manager->GetLib( nLib );
                }

                if( pLib )
                {
                    sal_uInt16 nModCount = pLib->GetModules()->Count();
                    for( sal_uInt16 nMod = 0; nMod < nModCount; ++nMod )
                    {
                        SbModule* pMod = (SbModule*)pLib->GetModules()->Get( nMod );
                        if ( pMod && rTransliteration.isEqual( pMod->GetName(), sModule ) )
                        {
                            SbMethod* pMethod = (SbMethod*)pMod->Find( sMacro, SbxCLASS_METHOD );
                            if( pMethod )
                            {
                                return pMethod;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
}

bool BasicManager::HasMacro( OUString const& i_fullyQualifiedName ) const
{
    return ( NULL != lcl_queryMacro( const_cast< BasicManager* >( this ), i_fullyQualifiedName ) );
}

ErrCode BasicManager::ExecuteMacro( OUString const& i_fullyQualifiedName, SbxArray* i_arguments, SbxValue* i_retValue )
{
    SbMethod* pMethod = lcl_queryMacro( this, i_fullyQualifiedName );
    ErrCode nError = 0;
    if ( pMethod )
    {
        if ( i_arguments )
            pMethod->SetParameters( i_arguments );
        nError = pMethod->Call( i_retValue );
    }
    else
        nError = ERRCODE_BASIC_PROC_UNDEFINED;
    return nError;
}

ErrCode BasicManager::ExecuteMacro( OUString const& i_fullyQualifiedName, OUString const& i_commaSeparatedArgs, SbxValue* i_retValue )
{
    SbMethod* pMethod = lcl_queryMacro( this, i_fullyQualifiedName );
    if ( !pMethod )
    {
        return ERRCODE_BASIC_PROC_UNDEFINED;
    }
    // arguments must be quoted
    OUString sQuotedArgs;
    OUStringBuffer sArgs( i_commaSeparatedArgs );
    if ( sArgs.getLength()<2 || sArgs[1] == '\"')
    {
        // no args or already quoted args
        sQuotedArgs = sArgs.makeStringAndClear();
    }
    else
    {
        // quote parameters
        sArgs.remove( 0, 1 );
        sArgs.remove( sArgs.getLength() - 1, 1 );

        sQuotedArgs = "(";
        OUString sArgs2 = sArgs.makeStringAndClear();
        sal_Int32 nCount = comphelper::string::getTokenCount(sArgs2, ',');
        for (sal_Int32 n = 0; n < nCount; ++n)
        {
            sQuotedArgs += "\"";
            sQuotedArgs += comphelper::string::getToken(sArgs2, n, ',');
            sQuotedArgs += "\"";
            if ( n < nCount - 1 )
            {
                sQuotedArgs += ",";
            }
        }

        sQuotedArgs += ")";
    }

    // add quoted arguments and do the call
    OUString sCall;
    sCall += "[";
    sCall += pMethod->GetName();
    sCall += sQuotedArgs;
    sCall += "]";

    SbxVariable* pRet = pMethod->GetParent()->Execute( sCall );
    if ( pRet && ( pRet != pMethod ) )
    {
        *i_retValue = *pRet;
    }
    return SbxBase::GetError();
}

//=====================================================================

class ModuleInfo_Impl : public ModuleInfoHelper
{
    OUString maName;
    OUString maLanguage;
    OUString maSource;

public:
    ModuleInfo_Impl( const OUString& aName, const OUString& aLanguage, const OUString& aSource )
        : maName( aName ), maLanguage( aLanguage), maSource( aSource ) {}

    // Methods XStarBasicModuleInfo
    virtual OUString SAL_CALL getName() throw(uno::RuntimeException)
        { return maName; }
    virtual OUString SAL_CALL getLanguage() throw(uno::RuntimeException)
        { return maLanguage; }
    virtual OUString SAL_CALL getSource() throw(uno::RuntimeException)
        { return maSource; }
};


//=====================================================================

class DialogInfo_Impl : public DialogInfoHelper
{
    OUString maName;
    uno::Sequence< sal_Int8 > mData;

public:
    DialogInfo_Impl( const OUString& aName, uno::Sequence< sal_Int8 > Data )
        : maName( aName ), mData( Data ) {}

    // Methods XStarBasicDialogInfo
    virtual OUString SAL_CALL getName() throw(uno::RuntimeException)
        { return maName; }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getData() throw(uno::RuntimeException)
        { return mData; }
};


//=====================================================================

class LibraryInfo_Impl : public LibraryInfoHelper
{
    OUString maName;
    uno::Reference< container::XNameContainer > mxModuleContainer;
    uno::Reference< container::XNameContainer > mxDialogContainer;
    OUString maPassword;
    OUString maExternaleSourceURL;
    OUString maLinkTargetURL;

public:
    LibraryInfo_Impl
    (
        const OUString& aName,
        uno::Reference< container::XNameContainer > xModuleContainer,
        uno::Reference< container::XNameContainer > xDialogContainer,
        const OUString& aPassword,
        const OUString& aExternaleSourceURL,
        const OUString& aLinkTargetURL
    )
        : maName( aName )
        , mxModuleContainer( xModuleContainer )
        , mxDialogContainer( xDialogContainer )
        , maPassword( aPassword )
        , maExternaleSourceURL( aExternaleSourceURL )
        , maLinkTargetURL( aLinkTargetURL )
    {}

    // Methods XStarBasicLibraryInfo
    virtual OUString SAL_CALL getName() throw(uno::RuntimeException)
        { return maName; }
    virtual uno::Reference< container::XNameContainer > SAL_CALL getModuleContainer() throw(uno::RuntimeException)
        { return mxModuleContainer; }
    virtual uno::Reference< container::XNameContainer > SAL_CALL getDialogContainer() throw(uno::RuntimeException)
        { return mxDialogContainer; }
    virtual OUString SAL_CALL getPassword() throw(uno::RuntimeException)
        { return maPassword; }
    virtual OUString SAL_CALL getExternalSourceURL() throw(uno::RuntimeException)
        { return maExternaleSourceURL; }
    virtual OUString SAL_CALL getLinkTargetURL() throw(uno::RuntimeException)
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
    virtual uno::Type SAL_CALL getElementType()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(uno::RuntimeException);

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::NoSuchElementException,
              lang::WrappedTargetException, uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::ElementExistException,
              lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
};

// Methods XElementAccess
uno::Type ModuleContainer_Impl::getElementType()
    throw(uno::RuntimeException)
{
    uno::Type aModuleType = ::getCppuType( (const uno::Reference< script::XStarBasicModuleInfo > *)0 );
    return aModuleType;
}

sal_Bool ModuleContainer_Impl::hasElements()
    throw(uno::RuntimeException)
{
    SbxArray* pMods = mpLib ? mpLib->GetModules() : NULL;
    return pMods && pMods->Count() > 0;
}

// Methods XNameAccess
uno::Any ModuleContainer_Impl::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    if( !pMod )
        throw container::NoSuchElementException();
    uno::Reference< script::XStarBasicModuleInfo > xMod = (XStarBasicModuleInfo*)new ModuleInfo_Impl
        ( aName, OUString::createFromAscii( szScriptLanguage ), pMod->GetSource32() );
    uno::Any aRetAny;
    aRetAny <<= xMod;
    return aRetAny;
}

uno::Sequence< OUString > ModuleContainer_Impl::getElementNames()
    throw(uno::RuntimeException)
{
    SbxArray* pMods = mpLib ? mpLib->GetModules() : NULL;
    sal_uInt16 nMods = pMods ? pMods->Count() : 0;
    uno::Sequence< OUString > aRetSeq( nMods );
    OUString* pRetSeq = aRetSeq.getArray();
    for( sal_uInt16 i = 0 ; i < nMods ; i++ )
    {
        SbxVariable* pMod = pMods->Get( i );
        pRetSeq[i] = OUString( pMod->GetName() );
    }
    return aRetSeq;
}

sal_Bool ModuleContainer_Impl::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    sal_Bool bRet = (pMod != NULL);
    return bRet;
}


// Methods XNameReplace
void ModuleContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void ModuleContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Type aModuleType = ::getCppuType( (const uno::Reference< script::XStarBasicModuleInfo > *)0 );
    uno::Type aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
    {
        throw lang::IllegalArgumentException();
    }
    uno::Reference< script::XStarBasicModuleInfo > xMod;
    aElement >>= xMod;
    mpLib->MakeModule32( aName, xMod->getSource() );
}

void ModuleContainer_Impl::removeByName( const OUString& Name )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( Name ) : NULL;
    if( !pMod )
    {
        throw container::NoSuchElementException();
    }
    mpLib->Remove( pMod );
}


//=====================================================================

uno::Sequence< sal_Int8 > implGetDialogData( SbxObject* pDialog )
{
    SvMemoryStream aMemStream;
    pDialog->Store( aMemStream );
    sal_Int32 nLen = aMemStream.Tell();
    uno::Sequence< sal_Int8 > aData( nLen );
    sal_Int8* pDestData = aData.getArray();
    const sal_Int8* pSrcData = (const sal_Int8*)aMemStream.GetData();
    memcpy( pDestData, pSrcData, nLen );
    return aData;
}

SbxObject* implCreateDialog( uno::Sequence< sal_Int8 > aData )
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
    virtual uno::Type SAL_CALL getElementType()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(uno::RuntimeException);

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
};

// Methods XElementAccess
uno::Type DialogContainer_Impl::getElementType()
    throw(uno::RuntimeException)
{
    uno::Type aModuleType = ::getCppuType( (const uno::Reference< script::XStarBasicDialogInfo > *)0 );
    return aModuleType;
}

sal_Bool DialogContainer_Impl::hasElements()
    throw(uno::RuntimeException)
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
uno::Any DialogContainer_Impl::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SbxVariable* pVar = mpLib->GetObjects()->Find( aName, SbxCLASS_DONTCARE );
    if( !( pVar && pVar->ISA( SbxObject ) &&
           ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) ) )
    {
        throw container::NoSuchElementException();
    }

    uno::Reference< script::XStarBasicDialogInfo > xDialog =
        (XStarBasicDialogInfo*)new DialogInfo_Impl
            ( aName, implGetDialogData( (SbxObject*)pVar ) );

    uno::Any aRetAny;
    aRetAny <<= xDialog;
    return aRetAny;
}

uno::Sequence< OUString > DialogContainer_Impl::getElementNames()
    throw(uno::RuntimeException)
{
    mpLib->GetAll( SbxCLASS_OBJECT );
    sal_Int16 nCount = mpLib->GetObjects()->Count();
    uno::Sequence< OUString > aRetSeq( nCount );
    OUString* pRetSeq = aRetSeq.getArray();
    sal_Int32 nDialogCounter = 0;

    for( sal_Int16 nObj = 0; nObj < nCount ; nObj++ )
    {
        SbxVariable* pVar = mpLib->GetObjects()->Get( nObj );
        if ( pVar->ISA( SbxObject ) && ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) )
        {
            pRetSeq[ nDialogCounter ] = OUString( pVar->GetName() );
            nDialogCounter++;
        }
    }
    aRetSeq.realloc( nDialogCounter );
    return aRetSeq;
}

sal_Bool DialogContainer_Impl::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
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
void DialogContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void DialogContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    (void)aName;
    uno::Type aModuleType = ::getCppuType( (const uno::Reference< script::XStarBasicDialogInfo > *)0 );
    uno::Type aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
    {
        throw lang::IllegalArgumentException();
    }
    uno::Reference< script::XStarBasicDialogInfo > xMod;
    aElement >>= xMod;
    SbxObjectRef xDialog = implCreateDialog( xMod->getData() );
    mpLib->Insert( xDialog );
}

void DialogContainer_Impl::removeByName( const OUString& Name )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    (void)Name;
    SbxVariable* pVar = mpLib->GetObjects()->Find( Name, SbxCLASS_DONTCARE );
    if( !( pVar && pVar->ISA( SbxObject ) &&
           ( ((SbxObject*)pVar)->GetSbxId() == SBXID_DIALOG ) ) )
    {
        throw container::NoSuchElementException();
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
    virtual uno::Type SAL_CALL getElementType()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw(uno::RuntimeException);

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(uno::RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
};


// Methods XElementAccess
uno::Type LibraryContainer_Impl::getElementType()
    throw(uno::RuntimeException)
{
    uno::Type aType = ::getCppuType( (const uno::Reference< script::XStarBasicLibraryInfo > *)0 );
    return aType;
}

sal_Bool LibraryContainer_Impl::hasElements()
    throw(uno::RuntimeException)
{
    sal_Int32 nLibs = mpMgr->GetLibCount();
    sal_Bool bRet = (nLibs > 0);
    return bRet;
}

// Methods XNameAccess
uno::Any LibraryContainer_Impl::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRetAny;
    if( !mpMgr->HasLib( aName ) )
        throw container::NoSuchElementException();
    StarBASIC* pLib = mpMgr->GetLib( aName );

    uno::Reference< container::XNameContainer > xModuleContainer =
        (container::XNameContainer*)new ModuleContainer_Impl( pLib );

    uno::Reference< container::XNameContainer > xDialogContainer =
        (container::XNameContainer*)new DialogContainer_Impl( pLib );

    BasicLibInfo* pLibInfo = mpMgr->FindLibInfo( pLib );

    OUString aPassword = pLibInfo->GetPassword();

    // TODO Only provide extern info!
    OUString aExternaleSourceURL;
    OUString aLinkTargetURL;
    if( pLibInfo->IsReference() )
    {
        aLinkTargetURL = pLibInfo->GetStorageName();
    }
    else if( pLibInfo->IsExtern() )
    {
        aExternaleSourceURL = pLibInfo->GetStorageName();
    }
    uno::Reference< script::XStarBasicLibraryInfo > xLibInfo = new LibraryInfo_Impl
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

uno::Sequence< OUString > LibraryContainer_Impl::getElementNames()
    throw(uno::RuntimeException)
{
    sal_uInt16 nLibs = mpMgr->GetLibCount();
    uno::Sequence< OUString > aRetSeq( nLibs );
    OUString* pRetSeq = aRetSeq.getArray();
    for( sal_uInt16 i = 0 ; i < nLibs ; i++ )
    {
        pRetSeq[i] = OUString( mpMgr->GetLibName( i ) );
    }
    return aRetSeq;
}

sal_Bool LibraryContainer_Impl::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    sal_Bool bRet = mpMgr->HasLib( aName );
    return bRet;
}

// Methods XNameReplace
void LibraryContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// Methods XNameContainer
void LibraryContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    (void)aName;
    (void)aElement;
    // TODO: Insert a complete Library?!
}

void LibraryContainer_Impl::removeByName( const OUString& Name )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    StarBASIC* pLib = mpMgr->GetLib( Name );
    if( !pLib )
    {
        throw container::NoSuchElementException();
    }
    sal_uInt16 nLibId = mpMgr->GetLibId( Name );
    mpMgr->RemoveLib( nLibId );
}

//=====================================================================

typedef WeakImplHelper1< script::XStarBasicAccess > StarBasicAccessHelper;


class StarBasicAccess_Impl : public StarBasicAccessHelper
{
    BasicManager* mpMgr;
    uno::Reference< container::XNameContainer > mxLibContainer;

public:
    StarBasicAccess_Impl( BasicManager* pMgr )
        :mpMgr( pMgr ) {}

public:
    // Methods
    virtual uno::Reference< container::XNameContainer > SAL_CALL getLibraryContainer()
        throw(uno::RuntimeException);
    virtual void SAL_CALL createLibrary( const OUString& LibName, const OUString& Password,
        const OUString& ExternalSourceURL, const OUString& LinkTargetURL )
            throw(container::ElementExistException, uno::RuntimeException);
    virtual void SAL_CALL addModule( const OUString& LibraryName, const OUString& ModuleName,
        const OUString& Language, const OUString& Source )
            throw(container::NoSuchElementException, uno::RuntimeException);
    virtual void SAL_CALL addDialog( const OUString& LibraryName, const OUString& DialogName,
        const uno::Sequence< sal_Int8 >& Data )
            throw(container::NoSuchElementException, uno::RuntimeException);
};

uno::Reference< container::XNameContainer > SAL_CALL StarBasicAccess_Impl::getLibraryContainer()
    throw(uno::RuntimeException)
{
    if( !mxLibContainer.is() )
        mxLibContainer = (container::XNameContainer*)new LibraryContainer_Impl( mpMgr );
    return mxLibContainer;
}

void SAL_CALL StarBasicAccess_Impl::createLibrary
(
    const OUString& LibName,
    const OUString& Password,
    const OUString& ExternalSourceURL,
    const OUString& LinkTargetURL
)
    throw(container::ElementExistException, uno::RuntimeException)
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
    const OUString& LibraryName,
    const OUString& ModuleName,
    const OUString& Language,
    const OUString& Source
)
    throw(container::NoSuchElementException, uno::RuntimeException)
{
    (void)Language;
    StarBASIC* pLib = mpMgr->GetLib( LibraryName );
    DBG_ASSERT( pLib, "XML Import: Lib for module unknown");
    if( pLib )
    {
        pLib->MakeModule32( ModuleName, Source );
    }
}

void SAL_CALL StarBasicAccess_Impl::addDialog
(
    const OUString& LibraryName,
    const OUString& DialogName,
    const uno::Sequence< sal_Int8 >& Data
)
    throw(container::NoSuchElementException, uno::RuntimeException)
{
    (void)LibraryName;
    (void)DialogName;
    (void)Data;
}

// Basic XML Import/Export
uno::Reference< script::XStarBasicAccess > getStarBasicAccess( BasicManager* pMgr )
{
    uno::Reference< script::XStarBasicAccess > xRet =
        new StarBasicAccess_Impl( (BasicManager*)pMgr );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
