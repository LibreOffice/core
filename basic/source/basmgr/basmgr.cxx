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
#include <svl/hint.hxx>
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
#include <o3tl/make_unique.hxx>

#include <basic/sbuno.hxx>
#include <basic/basmgr.hxx>
#include "global.hxx"
#include <sbunoobj.hxx>
#include "basrid.hxx"
#include "sbintern.hxx"
#include <sb.hrc>

#include <memory>
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
#include <com/sun/star/ucb/ContentCreationException.hpp>

#include <cppuhelper/implbase.hxx>

using com::sun::star::uno::Reference;
using namespace com::sun::star;
using namespace com::sun::star::script;
using namespace cppu;

typedef WeakImplHelper< container::XNameContainer > NameContainerHelper;
typedef WeakImplHelper< script::XStarBasicModuleInfo > ModuleInfoHelper;
typedef WeakImplHelper< script::XStarBasicAccess > StarBasicAccessHelper;

// Version 1
//    sal_uInt32    nEndPos
//    sal_uInt16    nId
//    sal_uInt16    nVer
//    bool      bDoLoad
//    String    LibName
//    String    AbsStorageName
//    String    RelStorageName
// Version 2
//  + bool      bReference

static const char szStdLibName[] = "Standard";
static const char szBasicStorage[] = "StarBASIC";
static const char szOldManagerStream[] = "BasicManager";
static const char szManagerStream[] = "BasicManager2";
static const char szImbedded[] = "LIBIMBEDDED";
static const char szCryptingKey[] = "CryptedBasic";


const StreamMode eStreamReadMode = StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYALL;
const StreamMode eStorageReadMode = StreamMode::READ | StreamMode::SHARE_DENYWRITE;


// BasicManager impl data
struct BasicManagerImpl
{
    LibraryContainerInfo    maContainerInfo;

    std::vector<std::unique_ptr<BasicLibInfo>> aLibs;
    OUString         aBasicLibPath;

    BasicManagerImpl()
    {}
};

// BasMgrContainerListenerImpl


typedef ::cppu::WeakImplHelper< container::XContainerListener > ContainerListenerHelper;

class BasMgrContainerListenerImpl: public ContainerListenerHelper
{
    BasicManager* mpMgr;
    OUString maLibName;      // empty -> no lib, but lib container

public:
    BasMgrContainerListenerImpl( BasicManager* pMgr, const OUString& aLibName )
        : mpMgr( pMgr )
        , maLibName( aLibName ) {}

    static void insertLibraryImpl( const uno::Reference< script::XLibraryContainer >& xScriptCont, BasicManager* pMgr,
                                   const uno::Any& aLibAny, const OUString& aLibName );
    static void addLibraryModulesImpl( BasicManager* pMgr, const uno::Reference< container::XNameAccess >& xLibNameAccess,
                                       const OUString& aLibName );


    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& Event ) override;
};


// BasMgrContainerListenerImpl


void BasMgrContainerListenerImpl::insertLibraryImpl( const uno::Reference< script::XLibraryContainer >& xScriptCont,
    BasicManager* pMgr, const uno::Any& aLibAny, const OUString& aLibName )
{
    Reference< container::XNameAccess > xLibNameAccess;
    aLibAny >>= xLibNameAccess;

    if( !pMgr->GetLib( aLibName ) )
    {
        StarBASIC* pLib =
            pMgr->CreateLibForLibContainer( aLibName, xScriptCont );
        DBG_ASSERT( pLib, "XML Import: Basic library could not be created");
    }

    uno::Reference< container::XContainer> xLibContainer( xLibNameAccess, uno::UNO_QUERY );
    if( xLibContainer.is() )
    {
        // Register listener for library
        Reference< container::XContainerListener > xLibraryListener
            = new BasMgrContainerListenerImpl( pMgr, aLibName );
        xLibContainer->addContainerListener( xLibraryListener );
    }

    if( xScriptCont->isLibraryLoaded( aLibName ) )
    {
        addLibraryModulesImpl( pMgr, xLibNameAccess, aLibName );
    }
}


void BasMgrContainerListenerImpl::addLibraryModulesImpl( BasicManager* pMgr,
    const uno::Reference< container::XNameAccess >& xLibNameAccess, const OUString& aLibName )
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
                ModuleInfo aInfo = xVBAModuleInfo->getModuleInfo( aModuleName );
                pLib->MakeModule( aModuleName, aInfo, aMod );
            }
            else
        pLib->MakeModule( aModuleName, aMod );
        }

        pLib->SetModified( false );
    }
}


// XEventListener


void SAL_CALL BasMgrContainerListenerImpl::disposing( const lang::EventObject& Source )
{
    (void)Source;
}

// XContainerListener


void SAL_CALL BasMgrContainerListenerImpl::elementInserted( const container::ContainerEvent& Event )
{
    bool bLibContainer = maLibName.isEmpty();
    OUString aName;
    Event.Accessor >>= aName;

    if( bLibContainer )
    {
        uno::Reference< script::XLibraryContainer > xScriptCont( Event.Source, uno::UNO_QUERY );
        if (xScriptCont.is())
            insertLibraryImpl(xScriptCont, mpMgr, Event.Element, aName);
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
                    ModuleInfo aInfo = xVBAModuleInfo->getModuleInfo( aName );
                    pLib->MakeModule( aName, aInfo, aMod );
                }
                else
                    pLib->MakeModule( aName, aMod );
                pLib->SetModified( false );
            }
        }
    }
}


void SAL_CALL BasMgrContainerListenerImpl::elementReplaced( const container::ContainerEvent& Event )
{
    OUString aName;
    Event.Accessor >>= aName;

    // Replace not possible for library container
    DBG_ASSERT( !maLibName.isEmpty(), "library container fired elementReplaced()");

    StarBASIC* pLib = mpMgr->GetLib( maLibName );
    if( pLib )
    {
        SbModule* pMod = pLib->FindModule( aName );
        OUString aMod;
        Event.Element >>= aMod;

        if( pMod )
                pMod->SetSource32( aMod );
        else
                pLib->MakeModule( aName, aMod );

        pLib->SetModified( false );
    }
}


void SAL_CALL BasMgrContainerListenerImpl::elementRemoved( const container::ContainerEvent& Event )
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
            mpMgr->RemoveLib( nLibId, false );
        }
    }
    else
    {
        StarBASIC* pLib = mpMgr->GetLib( maLibName );
        SbModule* pMod = pLib ? pLib->FindModule( aName ) : nullptr;
        if( pMod )
        {
            pLib->Remove( pMod );
            pLib->SetModified( false );
        }
    }
}

BasicError::BasicError( sal_uInt64 nId, BasicErrorReason nR )
{
    nErrorId    = nId;
    nReason     = nR;
}

BasicError::BasicError( const BasicError& rErr )
{
    nErrorId    = rErr.nErrorId;
    nReason     = rErr.nReason;
}


class BasicLibInfo
{
private:
    StarBASICRef    xLib;
    OUString          aLibName;
    OUString          aStorageName;   // string is sufficient, unique at runtime
    OUString          aRelStorageName;
    OUString          aPassword;

    bool              bDoLoad;
    bool              bReference;
    bool              bPasswordVerified;

    // Lib represents library in new UNO library container
    uno::Reference< script::XLibraryContainer > mxScriptCont;

public:
    BasicLibInfo();

    bool&             IsReference()           { return bReference; }

    bool              IsExtern() const        { return aStorageName != szImbedded; }

    void              SetStorageName( const OUString& rName )   { aStorageName = rName; }
    const OUString&   GetStorageName() const                  { return aStorageName; }

    void              SetRelStorageName( const OUString& rN )   { aRelStorageName = rN; }
    const OUString&   GetRelStorageName() const               { return aRelStorageName; }

    StarBASICRef      GetLib() const
    {
        if( mxScriptCont.is() && mxScriptCont->hasByName( aLibName ) &&
            !mxScriptCont->isLibraryLoaded( aLibName ) )
                return StarBASICRef();
        return xLib;
    }
    StarBASICRef&     GetLibRef()                         { return xLib; }
    void              SetLib( StarBASIC* pBasic )         { xLib = pBasic; }

    const OUString&   GetLibName() const                  { return aLibName; }
    void              SetLibName( const OUString& rName )   { aLibName = rName; }

    // Only temporary for Load/Save
    bool              DoLoad()                            { return bDoLoad; }

    bool              HasPassword() const                 { return !aPassword.isEmpty(); }
    const OUString&   GetPassword() const                 { return aPassword; }
    void              SetPassword( const OUString& rNewPassword )
                                                        { aPassword = rNewPassword; }
    void              SetPasswordVerified()               { bPasswordVerified = true; }

    static BasicLibInfo*    Create( SotStorageStream& rSStream );

    const uno::Reference< script::XLibraryContainer >& GetLibraryContainer()
        { return mxScriptCont; }
    void SetLibraryContainer( const uno::Reference< script::XLibraryContainer >& xScriptCont )
        { mxScriptCont = xScriptCont; }
};


BasicLibInfo::BasicLibInfo()
{
    bReference          = false;
    bPasswordVerified   = false;
    bDoLoad             = false;
    mxScriptCont        = nullptr;
    aStorageName        = szImbedded;
    aRelStorageName     = szImbedded;
}

BasicLibInfo* BasicLibInfo::Create( SotStorageStream& rSStream )
{
    BasicLibInfo* pInfo = new BasicLibInfo;

    sal_uInt32 nEndPos;
    sal_uInt16 nId;
    sal_uInt16 nVer;

    rSStream.ReadUInt32( nEndPos );
    rSStream.ReadUInt16( nId );
    rSStream.ReadUInt16( nVer );

    DBG_ASSERT( nId == LIBINFO_ID, "No BasicLibInfo?!" );
    if( nId == LIBINFO_ID )
    {
        // Reload?
        bool bDoLoad;
        rSStream.ReadCharAsBool( bDoLoad );
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
            bool bReferenz;
            rSStream.ReadCharAsBool( bReferenz );
            pInfo->IsReference() = bReferenz;
        }

        rSStream.Seek( nEndPos );
    }
    return pInfo;
}

BasicManager::BasicManager( SotStorage& rStorage, const OUString& rBaseURL, StarBASIC* pParentFromStdLib, OUString* pLibPath, bool bDocMgr ) : mbDocMgr( bDocMgr )
{
    Init();

    if( pLibPath )
    {
        mpImpl->aBasicLibPath = *pLibPath;
    }
    OUString aStorName( rStorage.GetName() );
    maStorageName = INetURLObject(aStorName, INetProtocol::File).GetMainURL( INetURLObject::DecodeMechanism::NONE );


    // If there is no Manager Stream, no further actions are necessary
    if ( rStorage.IsStream( szManagerStream ) )
    {
        LoadBasicManager( rStorage, rBaseURL );
        // StdLib contains Parent:
        StarBASIC* pStdLib = GetStdLib();
        DBG_ASSERT( pStdLib, "Standard-Lib not loaded?" );
        if ( !pStdLib )
        {
            // Should never happen, but if it happens we won't crash...
            pStdLib = new StarBASIC( nullptr, mbDocMgr );

            if (mpImpl->aLibs.empty())
                CreateLibInfo();

            BasicLibInfo& rStdLibInfo = *mpImpl->aLibs.front();

            rStdLibInfo.SetLib( pStdLib );
            StarBASICRef xStdLib = rStdLibInfo.GetLib();
            xStdLib->SetName( szStdLibName );
            rStdLibInfo.SetLibName( szStdLibName );
            xStdLib->SetFlag( SbxFlagBits::DontStore | SbxFlagBits::ExtSearch );
            xStdLib->SetModified( false );
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
                    pBasic->SetFlag( SbxFlagBits::ExtSearch );
                }
            }
            // Modified through insert
            pStdLib->SetModified( false );
        }
    }
    else
    {
        ImpCreateStdLib( pParentFromStdLib );
        if ( rStorage.IsStream( szOldManagerStream ) )
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

    for ( const auto& pModule: pBasic->GetModules() )
    {
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
            = new BasMgrContainerListenerImpl( this, aEmptyLibName );

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
            for (auto const& rpBasLibInfo : mpImpl->aLibs)
            {
                StarBASIC* pLib = rpBasLibInfo->GetLib().get();
                if( !pLib )
                {
                    bool bLoaded = ImpLoadLibrary( rpBasLibInfo.get(), nullptr );
                    if( bLoaded )
                        pLib = rpBasLibInfo->GetLib().get();
                }
                if( pLib )
                {
                    copyToLibraryContainer( pLib, mpImpl->maContainerInfo );
                    if (rpBasLibInfo->HasPassword())
                    {
                        OldBasicPassword* pOldBasicPassword =
                            mpImpl->maContainerInfo.mpOldBasicPassword;
                        if( pOldBasicPassword )
                        {
                            pOldBasicPassword->setLibraryPassword(
                                pLib->GetName(), rpBasLibInfo->GetPassword() );
                            rpBasLibInfo->SetPasswordVerified();
                        }
                    }
                }
            }
        }
    }

    SetGlobalUNOConstant( "BasicLibraries", uno::Any( mpImpl->maContainerInfo.mxScriptCont ) );
    SetGlobalUNOConstant( "DialogLibraries", uno::Any( mpImpl->maContainerInfo.mxDialogCont ) );
}

BasicManager::BasicManager( StarBASIC* pSLib, OUString* pLibPath, bool bDocMgr ) : mbDocMgr( bDocMgr )
{
    Init();
    DBG_ASSERT( pSLib, "BasicManager cannot be created with a NULL-Pointer!" );

    if( pLibPath )
    {
        mpImpl->aBasicLibPath = *pLibPath;
    }
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( pSLib );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName(szStdLibName);
    pStdLibInfo->SetLibName(szStdLibName );
    pSLib->SetFlag( SbxFlagBits::DontStore | SbxFlagBits::ExtSearch );

    // Save is only necessary if basic has changed
    xStdLib->SetModified( false );
}

void BasicManager::ImpMgrNotLoaded( const OUString& rStorageName )
{
    // pErrInf is only destroyed if the error is processed by an
    // ErrorHandler
    StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, rStorageName, ErrorHandlerFlags::ButtonsOk );
    aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::OPENMGRSTREAM));

    // Create a stdlib otherwise we crash!
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( new StarBASIC( nullptr, mbDocMgr ) );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( szStdLibName );
    pStdLibInfo->SetLibName( szStdLibName );
    xStdLib->SetFlag( SbxFlagBits::DontStore | SbxFlagBits::ExtSearch );
    xStdLib->SetModified( false );
}


void BasicManager::ImpCreateStdLib( StarBASIC* pParentFromStdLib )
{
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    StarBASIC* pStdLib = new StarBASIC( pParentFromStdLib, mbDocMgr );
    pStdLibInfo->SetLib( pStdLib );
    pStdLib->SetName( szStdLibName );
    pStdLibInfo->SetLibName( szStdLibName );
    pStdLib->SetFlag( SbxFlagBits::DontStore | SbxFlagBits::ExtSearch );
}

void BasicManager::LoadBasicManager( SotStorage& rStorage, const OUString& rBaseURL )
{
    tools::SvRef<SotStorageStream> xManagerStream = rStorage.OpenSotStream( szManagerStream, eStreamReadMode );

    OUString aStorName( rStorage.GetName() );
    // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    maStorageName = INetURLObject(aStorName, INetProtocol::File).GetMainURL( INetURLObject::DecodeMechanism::NONE );
    // #i13114 removed, DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    OUString aRealStorageName = maStorageName;  // for relative paths, can be modified through BaseURL

    if ( !rBaseURL.isEmpty() )
    {
        INetURLObject aObj( rBaseURL );
        if ( aObj.GetProtocol() == INetProtocol::File )
        {
            aRealStorageName = aObj.PathToFileName();
        }
    }

    xManagerStream->SetBufferSize( 1024 );
    xManagerStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_uInt32 nEndPos;
    xManagerStream->ReadUInt32( nEndPos );

    sal_uInt16 nLibs;
    xManagerStream->ReadUInt16( nLibs );
    // Plausibility!
    if( nLibs & 0xF000 )
    {
        SAL_WARN( "basic", "BasicManager-Stream defect!" );
        return;
    }
    const size_t nMinBasicLibSize(8);
    const size_t nMaxPossibleLibs = xManagerStream->remainingSize() / nMinBasicLibSize;
    if (nLibs > nMaxPossibleLibs)
    {
        SAL_WARN("basic", "Parsing error: " << nMaxPossibleLibs <<
                 " max possible entries, but " << nLibs << " claimed, truncating");
        nLibs = nMaxPossibleLibs;
    }
    for (sal_uInt16 nL = 0; nL < nLibs; ++nL)
    {
        BasicLibInfo* pInfo = BasicLibInfo::Create( *xManagerStream );

        // Correct absolute pathname if relative is existing.
        // Always try relative first if there are two stands on disk
        if ( !pInfo->GetRelStorageName().isEmpty() && pInfo->GetRelStorageName() != szImbedded )
        {
            INetURLObject aObj( aRealStorageName, INetProtocol::File );
            aObj.removeSegment();
            bool bWasAbsolute = false;
            aObj = aObj.smartRel2Abs( pInfo->GetRelStorageName(), bWasAbsolute );

            //*** TODO: Replace if still necessary
            //*** TODO-End
            if ( ! mpImpl->aBasicLibPath.isEmpty() )
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

        mpImpl->aLibs.push_back(std::unique_ptr<BasicLibInfo>(pInfo));
        // Libs from external files should be loaded only when necessary.
        // But references are loaded at once, otherwise some big customers get into trouble
        if ( pInfo->DoLoad() &&
            ( !pInfo->IsExtern() ||  pInfo->IsReference()))
        {
            ImpLoadLibrary( pInfo, &rStorage );
        }
    }

    xManagerStream->Seek( nEndPos );
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.clear();
}

void BasicManager::LoadOldBasicManager( SotStorage& rStorage )
{
    tools::SvRef<SotStorageStream> xManagerStream = rStorage.OpenSotStream( szOldManagerStream, eStreamReadMode );

    OUString aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.getLength(), "No Storage Name!" );

    if ( !xManagerStream.is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    xManagerStream->SetBufferSize( 1024 );
    xManagerStream->Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt32 nBasicStartOff, nBasicEndOff;
    xManagerStream->ReadUInt32( nBasicStartOff );
    xManagerStream->ReadUInt32( nBasicEndOff );

    DBG_ASSERT( !xManagerStream->GetError(), "Invalid Manager-Stream!" );

    xManagerStream->Seek( nBasicStartOff );
    if (!ImplLoadBasic( *xManagerStream, mpImpl->aLibs.front()->GetLibRef() ))
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, aStorName, ErrorHandlerFlags::ButtonsOk );
        aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::OPENMGRSTREAM));
        // and it proceeds ...
    }
    xManagerStream->Seek( nBasicEndOff+1 ); // +1: 0x00 as separator
    OUString aLibs = xManagerStream->ReadUniOrByteString(xManagerStream->GetStreamCharSet());
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.clear(); // Close stream

    if ( !aLibs.isEmpty() )
    {
        INetURLObject aCurStorage( aStorName, INetProtocol::File );
        sal_Int32 nLibs = comphelper::string::getTokenCount(aLibs, LIB_SEP);
        for ( sal_Int32 nLib = 0; nLib < nLibs; nLib++ )
        {
            OUString aLibInfo(aLibs.getToken(nLib, LIB_SEP));
            // TODO: Remove == 2
            DBG_ASSERT( ( comphelper::string::getTokenCount(aLibInfo, LIBINFO_SEP) == 2 ) || ( comphelper::string::getTokenCount(aLibInfo, LIBINFO_SEP) == 3 ), "Invalid Lib-Info!" );
            OUString aLibName( aLibInfo.getToken( 0, LIBINFO_SEP ) );
            OUString aLibAbsStorageName( aLibInfo.getToken( 1, LIBINFO_SEP ) );
            OUString aLibRelStorageName( aLibInfo.getToken( 2, LIBINFO_SEP ) );
            INetURLObject aLibAbsStorage( aLibAbsStorageName, INetProtocol::File );

            INetURLObject aLibRelStorage( aStorName );
            aLibRelStorage.removeSegment();
            bool bWasAbsolute = false;
            aLibRelStorage = aLibRelStorage.smartRel2Abs( aLibRelStorageName, bWasAbsolute);
            DBG_ASSERT(!bWasAbsolute, "RelStorageName was absolute!" );

            tools::SvRef<SotStorage> xStorageRef;
            if ( aLibAbsStorage == aCurStorage || aLibRelStorageName == szImbedded )
            {
                xStorageRef = &rStorage;
            }
            else
            {
                xStorageRef = new SotStorage( false, aLibAbsStorage.GetMainURL
                    ( INetURLObject::DecodeMechanism::NONE ), eStorageReadMode );
                if ( xStorageRef->GetError() != ERRCODE_NONE )
                    xStorageRef = new SotStorage( false, aLibRelStorage.
                    GetMainURL( INetURLObject::DecodeMechanism::NONE ), eStorageReadMode );
            }
            if ( xStorageRef.is() )
            {
                AddLib( *xStorageRef, aLibName, false );
            }
            else
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, aStorName, ErrorHandlerFlags::ButtonsOk );
                aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::STORAGENOTFOUND));
            }
        }
    }
}

BasicManager::~BasicManager()
{
    // Notify listener if something needs to be saved
    Broadcast( SfxHint( SfxHintId::Dying) );
}

void BasicManager::LegacyDeleteBasicManager( BasicManager*& _rpManager )
{
    delete _rpManager;
    _rpManager = nullptr;
}


bool BasicManager::HasExeCode( const OUString& sLib )
{
    StarBASIC* pLib = GetLib(sLib);
    if ( pLib )
    {
        for (const auto& pModule: pLib->GetModules())
        {
            if (pModule->HasExeCode())
                return true;
        }
    }
    return false;
}

void BasicManager::Init()
{
    mpImpl.reset( new BasicManagerImpl );
}

BasicLibInfo* BasicManager::CreateLibInfo()
{
    BasicLibInfo* pInf(new BasicLibInfo);
    mpImpl->aLibs.push_back(std::unique_ptr<BasicLibInfo>(pInf));
    return pInf;
}

bool BasicManager::ImpLoadLibrary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage )
{
    try {
    DBG_ASSERT( pLibInfo, "LibInfo!?" );

    OUString aStorageName( pLibInfo->GetStorageName() );
    if ( aStorageName.isEmpty() || aStorageName == szImbedded )
    {
        aStorageName = GetStorageName();
    }
    tools::SvRef<SotStorage> xStorage;
    // The current must not be opened again...
    if ( pCurStorage )
    {
        OUString aStorName( pCurStorage->GetName() );
        // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

        INetURLObject aCurStorageEntry(aStorName, INetProtocol::File);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::DecodeMechanism::NONE ).Len() != 0, "Bad storage name");

        INetURLObject aStorageEntry(aStorageName, INetProtocol::File);
        // #i13114 removed, DBG_ASSERT(aCurStorageEntry.GetMainURL( INetURLObject::DecodeMechanism::NONE ).Len() != 0, "Bad storage name");

        if ( aCurStorageEntry == aStorageEntry )
        {
            xStorage = pCurStorage;
        }
    }

    if ( !xStorage.is() )
    {
        xStorage = new SotStorage( false, aStorageName, eStorageReadMode );
    }
    tools::SvRef<SotStorage> xBasicStorage = xStorage->OpenSotStorage( szBasicStorage, eStorageReadMode, false );

    if ( !xBasicStorage.is() || xBasicStorage->GetError() )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, xStorage->GetName(), ErrorHandlerFlags::ButtonsOk );
        aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::OPENLIBSTORAGE));
    }
    else
    {
        // In the Basic-Storage every lib is in a Stream...
        tools::SvRef<SotStorageStream> xBasicStream = xBasicStorage->OpenSotStream( pLibInfo->GetLibName(), eStreamReadMode );
        if ( !xBasicStream.is() || xBasicStream->GetError() )
        {
            StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD , pLibInfo->GetLibName(), ErrorHandlerFlags::ButtonsOk );
            aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::OPENLIBSTREAM));
        }
        else
        {
            bool bLoaded = false;
            if ( xBasicStream->Seek( STREAM_SEEK_TO_END ) != 0 )
            {
                if ( !pLibInfo->GetLib().is() )
                {
                    pLibInfo->SetLib( new StarBASIC( GetStdLib(), mbDocMgr ) );
                }
                xBasicStream->SetBufferSize( 1024 );
                xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                bLoaded = ImplLoadBasic( *xBasicStream, pLibInfo->GetLibRef() );
                xBasicStream->SetBufferSize( 0 );
                StarBASICRef xStdLib = pLibInfo->GetLib();
                xStdLib->SetName( pLibInfo->GetLibName() );
                xStdLib->SetModified( false );
                xStdLib->SetFlag( SbxFlagBits::DontStore );
            }
            if ( !bLoaded )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, pLibInfo->GetLibName(), ErrorHandlerFlags::ButtonsOk );
                aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::BASICLOADERROR));
            }
            else
            {
                // Perhaps there are additional information in the stream...
                xBasicStream->SetCryptMaskKey(szCryptingKey);
                xBasicStream->RefreshBuffer();
                sal_uInt32 nPasswordMarker = 0;
                xBasicStream->ReadUInt32( nPasswordMarker );
                if ( ( nPasswordMarker == PASSWORD_MARKER ) && !xBasicStream->IsEof() )
                {
                    OUString aPassword = xBasicStream->ReadUniOrByteString(
                        xBasicStream->GetStreamCharSet());
                    pLibInfo->SetPassword( aPassword );
                }
                xBasicStream->SetCryptMaskKey(OString());
                CheckModules( pLibInfo->GetLib().get(), pLibInfo->IsReference() );
            }
            return bLoaded;
        }
    }
    }
    catch (const css::ucb::ContentCreationException&)
    {
    }
    return false;
}

bool BasicManager::ImplEncryptStream( SvStream& rStrm )
{
    sal_uInt64 const nPos = rStrm.Tell();
    sal_uInt32 nCreator;
    rStrm.ReadUInt32( nCreator );
    rStrm.Seek( nPos );
    bool bProtected = false;
    if ( nCreator != SBXCR_SBX )
    {
        // Should only be the case for encrypted Streams
        bProtected = true;
        rStrm.SetCryptMaskKey(szCryptingKey);
        rStrm.RefreshBuffer();
    }
    return bProtected;
}

// This code is necessary to load the BASIC of Beta 1
// TODO: Which Beta 1?
bool BasicManager::ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const
{
    bool bProtected = ImplEncryptStream( rStrm );
    SbxBaseRef xNew = SbxBase::Load( rStrm );
    bool bLoaded = false;
    if( xNew.is() )
    {
        if( auto pNew = dynamic_cast<StarBASIC*>( xNew.get() ) )
        {
            // Use the Parent of the old BASICs
            if( rOldBasic.is() )
            {
                pNew->SetParent( rOldBasic->GetParent() );
                if( pNew->GetParent() )
                {
                    pNew->GetParent()->Insert( pNew );
                }
                pNew->SetFlag( SbxFlagBits::ExtSearch );
            }
            rOldBasic = pNew;

            // Fill new library container (5.2 -> 6.0)
            copyToLibraryContainer( pNew, mpImpl->maContainerInfo );

            pNew->SetModified( false );
            bLoaded = true;
        }
    }
    if ( bProtected )
    {
        rStrm.SetCryptMaskKey(OString());
    }
    return bLoaded;
}

void BasicManager::CheckModules( StarBASIC* pLib, bool bReference )
{
    if ( !pLib )
    {
        return;
    }
    bool bModified = pLib->IsModified();

    for ( const auto& pModule: pLib->GetModules() )
    {
        DBG_ASSERT( pModule.get(), "Module not received!" );
        if ( !pModule->IsCompiled() && !StarBASIC::GetErrorCode() )
        {
            pModule->Compile();
        }
    }

    // #67477, AB 8.12.99 On demand compile in referenced libs should not
    // cause modified
    if( !bModified && bReference )
    {
        OSL_FAIL( "Referenced basic library is not compiled!" );
        pLib->SetModified( false );
    }
}

StarBASIC* BasicManager::AddLib( SotStorage& rStorage, const OUString& rLibName, bool bReference )
{
    OUString aStorName( rStorage.GetName() );
    DBG_ASSERT( !aStorName.isEmpty(), "No Storage Name!" );

    OUString aStorageName = INetURLObject(aStorName, INetProtocol::File).GetMainURL( INetURLObject::DecodeMechanism::NONE );
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
    sal_uInt16 nLibId = (sal_uInt16) mpImpl->aLibs.size() - 1;

    // Set StorageName before load because it is compared with pCurStorage
    pLibInfo->SetStorageName( aStorageName );
    bool bLoaded = ImpLoadLibrary( pLibInfo, &rStorage );

    if ( bLoaded )
    {
        if ( aNewLibName != rLibName )
        {
            pLibInfo->SetLibName(aNewLibName);
        }
        if ( bReference )
        {
            pLibInfo->GetLib()->SetModified( false );   // Don't save in this case
            pLibInfo->SetRelStorageName( OUString() );
            pLibInfo->IsReference() = true;
        }
        else
        {
            pLibInfo->GetLib()->SetModified( true ); // Must be saved after Add!
            pLibInfo->SetStorageName( szImbedded ); // Save in BasicManager-Storage
        }
    }
    else
    {
        RemoveLib( nLibId, false );
        pLibInfo = nullptr;
    }

    return pLibInfo ? &*pLibInfo->GetLib() : nullptr;

}

bool BasicManager::IsReference( sal_uInt16 nLib )
{
    DBG_ASSERT( nLib < mpImpl->aLibs.size(), "Lib does not exist!" );
    if ( nLib < mpImpl->aLibs.size() )
    {
        return mpImpl->aLibs[nLib]->IsReference();
    }
    return false;
}

void BasicManager::RemoveLib( sal_uInt16 nLib )
{
    // Only physical deletion if no reference
    RemoveLib( nLib, !IsReference( nLib ) );
}

bool BasicManager::RemoveLib( sal_uInt16 nLib, bool bDelBasicFromStorage )
{
    DBG_ASSERT( nLib, "Standard-Lib cannot be removed!" );

    DBG_ASSERT( !nLib || nLib  < mpImpl->aLibs.size(), "Lib not found!" );

    if( !nLib || nLib  < mpImpl->aLibs.size() )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, OUString(), ErrorHandlerFlags::ButtonsOk );
        aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::STDLIB));
        return false;
    }

    auto const itLibInfo = mpImpl->aLibs.begin() + nLib;

    // If one of the streams cannot be opened, this is not an error,
    // because BASIC was never written before...
    if (bDelBasicFromStorage && !(*itLibInfo)->IsReference() &&
           (!(*itLibInfo)->IsExtern() || SotStorage::IsStorageFile((*itLibInfo)->GetStorageName())))
    {
        tools::SvRef<SotStorage> xStorage;
        try
        {
            if (!(*itLibInfo)->IsExtern())
            {
                xStorage = new SotStorage(false, GetStorageName());
            }
            else
            {
                xStorage = new SotStorage(false, (*itLibInfo)->GetStorageName());
            }
        }
        catch (const css::ucb::ContentCreationException& e)
        {
            SAL_WARN("basic", "BasicManager::RemoveLib: Caught exception: " << e.Message);
        }

        if (xStorage.is() && xStorage->IsStorage(szBasicStorage))
        {
            tools::SvRef<SotStorage> xBasicStorage = xStorage->OpenSotStorage
                            ( szBasicStorage, StreamMode::STD_READWRITE, false );

            if ( !xBasicStorage.is() || xBasicStorage->GetError() )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, OUString(), ErrorHandlerFlags::ButtonsOk );
                aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::OPENLIBSTORAGE));
            }
            else if (xBasicStorage->IsStream((*itLibInfo)->GetLibName()))
            {
                xBasicStorage->Remove((*itLibInfo)->GetLibName());
                xBasicStorage->Commit();

                // If no further stream available,
                // delete the SubStorage.
                SvStorageInfoList aInfoList;
                xBasicStorage->FillInfoList( &aInfoList );
                if ( aInfoList.empty() )
                {
                    xBasicStorage.clear();
                    xStorage->Remove( szBasicStorage );
                    xStorage->Commit();
                    // If no further Streams or SubStorages available,
                    // delete the Storage, too.
                    aInfoList.clear();
                    xStorage->FillInfoList( &aInfoList );
                    if ( aInfoList.empty() )
                    {
                        //OUString aName_( xStorage->GetName() );
                        xStorage.clear();
                        //*** TODO: Replace if still necessary
                        //SfxContentHelper::Kill( aName );
                        //*** TODO-End
                    }
                }
            }
        }
    }
    if ((*itLibInfo)->GetLib().is())
    {
        GetStdLib()->Remove( (*itLibInfo)->GetLib().get() );
    }
    mpImpl->aLibs.erase(itLibInfo);
    return true;    // Remove was successful, del unimportant
}

sal_uInt16 BasicManager::GetLibCount() const
{
    return (sal_uInt16)mpImpl->aLibs.size();
}

StarBASIC* BasicManager::GetLib( sal_uInt16 nLib ) const
{
    DBG_ASSERT( nLib < mpImpl->aLibs.size(), "Lib does not exist!" );
    if ( nLib < mpImpl->aLibs.size() )
    {
        return mpImpl->aLibs[nLib]->GetLib().get();
    }
    return nullptr;
}

StarBASIC* BasicManager::GetStdLib() const
{
    StarBASIC* pLib = GetLib( 0 );
    return pLib;
}

StarBASIC* BasicManager::GetLib( const OUString& rName ) const
{
    for (auto const& rpLib : mpImpl->aLibs)
    {
        if (rpLib->GetLibName().equalsIgnoreAsciiCase(rName)) // Check if available...
        {
            return rpLib->GetLib().get();
        }
    }
    return nullptr;
}

sal_uInt16 BasicManager::GetLibId( const OUString& rName ) const
{
    for (size_t i = 0; i < mpImpl->aLibs.size(); i++)
    {
        if (mpImpl->aLibs[i]->GetLibName().equalsIgnoreAsciiCase( rName ))
        {
            return (sal_uInt16)i;
        }
    }
    return LIB_NOTFOUND;
}

bool BasicManager::HasLib( const OUString& rName ) const
{
    for (const auto& rpLib : mpImpl->aLibs)
    {
        if (rpLib->GetLibName().equalsIgnoreAsciiCase(rName)) // Check if available...
        {
            return true;
        }
    }
    return false;
}

OUString BasicManager::GetLibName( sal_uInt16 nLib )
{
    DBG_ASSERT(  nLib < mpImpl->aLibs.size(), "Lib?!" );
    if ( nLib < mpImpl->aLibs.size() )
    {
        return mpImpl->aLibs[nLib]->GetLibName();
    }
    return OUString();
}

bool BasicManager::LoadLib( sal_uInt16 nLib )
{
    bool bDone = false;
    DBG_ASSERT( nLib < mpImpl->aLibs.size() , "Lib?!" );
    if ( nLib < mpImpl->aLibs.size() )
    {
        BasicLibInfo& rLibInfo = *mpImpl->aLibs[nLib];
        uno::Reference< script::XLibraryContainer > xLibContainer = rLibInfo.GetLibraryContainer();
        if( xLibContainer.is() )
        {
            OUString aLibName = rLibInfo.GetLibName();
            xLibContainer->loadLibrary( aLibName );
            bDone = xLibContainer->isLibraryLoaded( aLibName );
        }
        else
        {
            bDone = ImpLoadLibrary( &rLibInfo, nullptr );
            StarBASIC* pLib = GetLib( nLib );
            if ( pLib )
            {
                GetStdLib()->Insert( pLib );
                pLib->SetFlag( SbxFlagBits::ExtSearch );
            }
        }
    }
    else
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, OUString(), ErrorHandlerFlags::ButtonsOk );
        aErrors.push_back(BasicError(pErrInf->GetErrorCode(), BasicErrorReason::LIBNOTFOUND));
    }
    return bDone;
}

StarBASIC* BasicManager::CreateLib( const OUString& rLibName )
{
    if ( GetLib( rLibName ) )
    {
        return nullptr;
    }
    BasicLibInfo* pLibInfo = CreateLibInfo();
    StarBASIC* pNew = new StarBASIC( GetStdLib(), mbDocMgr );
    GetStdLib()->Insert( pNew );
    pNew->SetFlag( SbxFlagBits::ExtSearch | SbxFlagBits::DontStore );
    pLibInfo->SetLib( pNew );
    pLibInfo->SetLibName( rLibName );
    pLibInfo->GetLib()->SetName( rLibName );
    return pLibInfo->GetLib().get();
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
            try
            {
                tools::SvRef<SotStorage> xStorage = new SotStorage(false, LinkTargetURL, StreamMode::READ | StreamMode::SHARE_DENYWRITE);
                if (!xStorage->GetError())
                {
                    pLib = AddLib(*xStorage, rLibName, true);
                }
            }
            catch (const css::ucb::ContentCreationException& e)
            {
                SAL_WARN("basic", "BasicManager::RemoveLib: Caught exception: " << e.Message);
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
    if ( GetLib( rLibName ) )
    {
        return nullptr;
    }
    BasicLibInfo* pLibInfo = CreateLibInfo();
    StarBASIC* pNew = new StarBASIC( GetStdLib(), mbDocMgr );
    GetStdLib()->Insert( pNew );
    pNew->SetFlag( SbxFlagBits::ExtSearch | SbxFlagBits::DontStore );
    pLibInfo->SetLib( pNew );
    pLibInfo->SetLibName( rLibName );
    pLibInfo->GetLib()->SetName( rLibName );
    pLibInfo->SetLibraryContainer( xScriptCont );
    return pNew;
}


BasicLibInfo* BasicManager::FindLibInfo( StarBASIC* pBasic )
{
    for (auto const& rpLib : mpImpl->aLibs)
    {
        if (rpLib->GetLib().get() == pBasic)
        {
            return rpLib.get();
        }
    }
    return nullptr;
}


bool BasicManager::IsBasicModified() const
{
    for (auto const& rpLib : mpImpl->aLibs)
    {
        if (rpLib->GetLib().is() && rpLib->GetLib()->IsModified())
        {
            return true;
        }
    }
    return false;
}


bool BasicManager::GetGlobalUNOConstant( const OUString& rName, uno::Any& aOut )
{
    bool bRes = false;
    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::GetGlobalUNOConstant: no lib to read from!" );
    if ( pStandardLib )
        bRes = pStandardLib->GetUNOConstant( rName, aOut );
    return bRes;
}

uno::Any BasicManager::SetGlobalUNOConstant( const OUString& rName, const uno::Any& _rValue )
{
    uno::Any aOldValue;

    StarBASIC* pStandardLib = GetStdLib();
    OSL_PRECOND( pStandardLib, "BasicManager::SetGlobalUNOConstant: no lib to insert into!" );
    if ( !pStandardLib )
        return aOldValue;

    // obtain the old value
    SbxVariable* pVariable = pStandardLib->Find( rName, SbxClassType::Object );
    if ( pVariable )
        aOldValue = sbxToUnoValue( pVariable );

    SbxObjectRef xUnoObj = GetSbUnoObject( rName, _rValue );
    xUnoObj->SetFlag( SbxFlagBits::DontStore );
    pStandardLib->Insert( xUnoObj.get() );

    return aOldValue;
}

bool BasicManager::LegacyPsswdBinaryLimitExceeded( std::vector< OUString >& _out_rModuleNames )
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

            std::vector< OUString > aBigModules( nLen );
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
                 _out_rModuleNames.swap(aBigModules);
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
        OUString sLibName = i_fullyQualifiedName.getToken( (sal_Int32)0, (sal_Unicode)'.', nLast );
        OUString sModule = i_fullyQualifiedName.getToken( (sal_Int32)0, (sal_Unicode)'.', nLast );
        OUString sMacro;
        if(nLast >= 0)
        {
            sMacro = i_fullyQualifiedName.copy(nLast);
        }
        else
        {
            sMacro = i_fullyQualifiedName;
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
                    bool const bLoaded = i_manager->LoadLib( nLib );
                    if (bLoaded)
                    {
                        pLib = i_manager->GetLib( nLib );
                    }
                }

                if( pLib )
                {
                    for ( const auto& pMod: pLib->GetModules() )
                    {
                        if ( rTransliteration.isEqual( pMod->GetName(), sModule ) )
                        {
                            SbMethod* pMethod = static_cast<SbMethod*>(pMod->Find( sMacro, SbxClassType::Method ));
                            if( pMethod )
                            {
                                return pMethod;
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }
}

bool BasicManager::HasMacro( OUString const& i_fullyQualifiedName ) const
{
    return ( nullptr != lcl_queryMacro( const_cast< BasicManager* >( this ), i_fullyQualifiedName ) );
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

        OUStringBuffer aBuff;
        OUString sArgs2 = sArgs.makeStringAndClear();
        sal_Int32 nCount = comphelper::string::getTokenCount(sArgs2, ',');

        aBuff.append("(");
        for (sal_Int32 n = 0; n < nCount; ++n)
        {
            aBuff.append( "\"" );
            aBuff.append( sArgs2.getToken(n, ',') );
            aBuff.append( "\"" );

            if ( n < nCount - 1 )
            {
                aBuff.append( "," );
            }
        }
        aBuff.append( ")" );

        sQuotedArgs = aBuff.makeStringAndClear();
    }

    // add quoted arguments and do the call
    OUString sCall = "["
                   + pMethod->GetName()
                   + sQuotedArgs
                   + "]";

    SbxVariable* pRet = pMethod->GetParent()->Execute( sCall );
    if ( pRet && ( pRet != pMethod ) )
    {
        *i_retValue = *pRet;
    }
    return SbxBase::GetError();
}


class ModuleInfo_Impl : public ModuleInfoHelper
{
    OUString maName;
    OUString maLanguage;
    OUString maSource;

public:
    ModuleInfo_Impl( const OUString& aName, const OUString& aLanguage, const OUString& aSource )
        : maName( aName ), maLanguage( aLanguage), maSource( aSource ) {}

    // Methods XStarBasicModuleInfo
    virtual OUString SAL_CALL getName() override
        { return maName; }
    virtual OUString SAL_CALL getLanguage() override
        { return maLanguage; }
    virtual OUString SAL_CALL getSource() override
        { return maSource; }
};


class DialogInfo_Impl : public WeakImplHelper< script::XStarBasicDialogInfo >
{
    OUString maName;
    uno::Sequence< sal_Int8 > mData;

public:
    DialogInfo_Impl( const OUString& aName, const uno::Sequence< sal_Int8 >& Data )
        : maName( aName ), mData( Data ) {}

    // Methods XStarBasicDialogInfo
    virtual OUString SAL_CALL getName() override
        { return maName; }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getData() override
        { return mData; }
};


class LibraryInfo_Impl : public WeakImplHelper< script::XStarBasicLibraryInfo >
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
        uno::Reference< container::XNameContainer > const & xModuleContainer,
        uno::Reference< container::XNameContainer > const & xDialogContainer,
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
    virtual OUString SAL_CALL getName() override
        { return maName; }
    virtual uno::Reference< container::XNameContainer > SAL_CALL getModuleContainer() override
        { return mxModuleContainer; }
    virtual uno::Reference< container::XNameContainer > SAL_CALL getDialogContainer() override
        { return mxDialogContainer; }
    virtual OUString SAL_CALL getPassword() override
        { return maPassword; }
    virtual OUString SAL_CALL getExternalSourceURL() override
        { return maExternaleSourceURL; }
    virtual OUString SAL_CALL getLinkTargetURL() override
        { return maLinkTargetURL; }
};


class ModuleContainer_Impl : public NameContainerHelper
{
    StarBASIC* mpLib;

public:
    explicit ModuleContainer_Impl( StarBASIC* pLib )
        :mpLib( pLib ) {}

    // Methods XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;
};

// Methods XElementAccess
uno::Type ModuleContainer_Impl::getElementType()
{
    uno::Type aModuleType = cppu::UnoType<script::XStarBasicModuleInfo>::get();
    return aModuleType;
}

sal_Bool ModuleContainer_Impl::hasElements()
{
    return mpLib && !mpLib->GetModules().empty();
}

// Methods XNameAccess
uno::Any ModuleContainer_Impl::getByName( const OUString& aName )
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : nullptr;
    if( !pMod )
        throw container::NoSuchElementException();
    uno::Reference< script::XStarBasicModuleInfo > xMod = new ModuleInfo_Impl( aName, "StarBasic", pMod->GetSource32() );
    uno::Any aRetAny;
    aRetAny <<= xMod;
    return aRetAny;
}

uno::Sequence< OUString > ModuleContainer_Impl::getElementNames()
{
    sal_uInt16 nMods = mpLib ? mpLib->GetModules().size() : 0;
    uno::Sequence< OUString > aRetSeq( nMods );
    OUString* pRetSeq = aRetSeq.getArray();
    for( sal_uInt16 i = 0 ; i < nMods ; i++ )
    {
        pRetSeq[i] = mpLib->GetModules()[i]->GetName();
    }
    return aRetSeq;
}

sal_Bool ModuleContainer_Impl::hasByName( const OUString& aName )
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : nullptr;
    bool bRet = (pMod != nullptr);
    return bRet;
}


// Methods XNameReplace
void ModuleContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void ModuleContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
{
    uno::Type aModuleType = cppu::UnoType<script::XStarBasicModuleInfo>::get();
    const uno::Type& aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
    {
        throw lang::IllegalArgumentException();
    }
    uno::Reference< script::XStarBasicModuleInfo > xMod;
    aElement >>= xMod;
    mpLib->MakeModule( aName, xMod->getSource() );
}

void ModuleContainer_Impl::removeByName( const OUString& Name )
{
    SbModule* pMod = mpLib ? mpLib->FindModule( Name ) : nullptr;
    if( !pMod )
    {
        throw container::NoSuchElementException();
    }
    mpLib->Remove( pMod );
}


uno::Sequence< sal_Int8 > implGetDialogData( SbxObject* pDialog )
{
    SvMemoryStream aMemStream;
    pDialog->Store( aMemStream );
    sal_Int32 nLen = aMemStream.Tell();
    if (nLen < 0) { abort(); }
    uno::Sequence< sal_Int8 > aData( nLen );
    sal_Int8* pDestData = aData.getArray();
    const sal_Int8* pSrcData = static_cast<const sal_Int8*>(aMemStream.GetData());
    memcpy( pDestData, pSrcData, nLen );
    return aData;
}

SbxObject* implCreateDialog( const uno::Sequence< sal_Int8 >& aData )
{
    sal_Int8* pData = const_cast< uno::Sequence< sal_Int8 >& >(aData).getArray();
    SvMemoryStream aMemStream( pData, aData.getLength(), StreamMode::READ );
    SbxBase* pBase = SbxBase::Load( aMemStream );
    return dynamic_cast<SbxObject*>(pBase);
}

// HACK! Because this value is defined in basctl/inc/vcsbxdef.hxx
// which we can't include here, we have to use the value directly
#define SBXID_DIALOG        101


class DialogContainer_Impl : public NameContainerHelper
{
    StarBASIC* mpLib;

public:
    explicit DialogContainer_Impl( StarBASIC* pLib )
        :mpLib( pLib ) {}

    // Methods XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;
};

// Methods XElementAccess
uno::Type DialogContainer_Impl::getElementType()
{
    uno::Type aModuleType = cppu::UnoType<script::XStarBasicDialogInfo>::get();
    return aModuleType;
}

sal_Bool DialogContainer_Impl::hasElements()
{
    bool bRet = false;

    sal_Int16 nCount = mpLib->GetObjects()->Count();
    for( sal_Int16 nObj = 0; nObj < nCount ; nObj++ )
    {
        SbxVariable* pVar = mpLib->GetObjects()->Get( nObj );
        SbxObject* pObj = dynamic_cast<SbxObject*>(pVar);
        if ( pObj && (pObj->GetSbxId() == SBXID_DIALOG ) )
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

// Methods XNameAccess
uno::Any DialogContainer_Impl::getByName( const OUString& aName )
{
    SbxVariable* pVar = mpLib->GetObjects()->Find( aName, SbxClassType::DontCare );
    SbxObject* pObj = dynamic_cast<SbxObject*>(pVar);
    if( !( pObj && pObj->GetSbxId() == SBXID_DIALOG ) )
    {
        throw container::NoSuchElementException();
    }

    uno::Reference< script::XStarBasicDialogInfo > xDialog =
        new DialogInfo_Impl(aName, implGetDialogData(pObj));

    uno::Any aRetAny;
    aRetAny <<= xDialog;
    return aRetAny;
}

uno::Sequence< OUString > DialogContainer_Impl::getElementNames()
{
    sal_Int16 nCount = mpLib->GetObjects()->Count();
    uno::Sequence< OUString > aRetSeq( nCount );
    OUString* pRetSeq = aRetSeq.getArray();
    sal_Int32 nDialogCounter = 0;

    for( sal_Int16 nObj = 0; nObj < nCount ; nObj++ )
    {
        SbxVariable* pVar = mpLib->GetObjects()->Get( nObj );
        SbxObject* pObj = dynamic_cast<SbxObject*> (pVar);
        if ( pObj && ( pObj->GetSbxId() == SBXID_DIALOG ) )
        {
            pRetSeq[ nDialogCounter ] = pVar->GetName();
            nDialogCounter++;
        }
    }
    aRetSeq.realloc( nDialogCounter );
    return aRetSeq;
}

sal_Bool DialogContainer_Impl::hasByName( const OUString& aName )
{
    bool bRet = false;
    SbxVariable* pVar = mpLib->GetObjects()->Find( aName, SbxClassType::DontCare );
    SbxObject* pObj = dynamic_cast<SbxObject*>(pVar);
    if( pObj &&  ( pObj->GetSbxId() == SBXID_DIALOG ) )
    {
        bRet = true;
    }
    return bRet;
}


// Methods XNameReplace
void DialogContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void DialogContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
{
    (void)aName;
    uno::Type aModuleType = cppu::UnoType<script::XStarBasicDialogInfo>::get();
    const uno::Type& aAnyType = aElement.getValueType();
    if( aModuleType != aAnyType )
    {
        throw lang::IllegalArgumentException();
    }
    uno::Reference< script::XStarBasicDialogInfo > xMod;
    aElement >>= xMod;
    SbxObjectRef xDialog = implCreateDialog( xMod->getData() );
    mpLib->Insert( xDialog.get() );
}

void DialogContainer_Impl::removeByName( const OUString& Name )
{
    (void)Name;
    SbxVariable* pVar = mpLib->GetObjects()->Find( Name, SbxClassType::DontCare );
    SbxObject* pObj = dynamic_cast<SbxObject*>(pVar);
    if( !( pObj && ( pObj->GetSbxId() == SBXID_DIALOG ) ) )
    {
        throw container::NoSuchElementException();
    }
    mpLib->Remove( pVar );
}


class LibraryContainer_Impl : public NameContainerHelper
{
    BasicManager* mpMgr;

public:
    explicit LibraryContainer_Impl( BasicManager* pMgr )
        :mpMgr( pMgr ) {}

    // Methods XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // Methods XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;
};


// Methods XElementAccess
uno::Type LibraryContainer_Impl::getElementType()
{
    uno::Type aType = cppu::UnoType<script::XStarBasicLibraryInfo>::get();
    return aType;
}

sal_Bool LibraryContainer_Impl::hasElements()
{
    sal_Int32 nLibs = mpMgr->GetLibCount();
    bool bRet = (nLibs > 0);
    return bRet;
}

// Methods XNameAccess
uno::Any LibraryContainer_Impl::getByName( const OUString& aName )
{
    uno::Any aRetAny;
    if( !mpMgr->HasLib( aName ) )
        throw container::NoSuchElementException();
    StarBASIC* pLib = mpMgr->GetLib( aName );

    uno::Reference< container::XNameContainer > xModuleContainer =
        new ModuleContainer_Impl( pLib );

    uno::Reference< container::XNameContainer > xDialogContainer =
        new DialogContainer_Impl( pLib );

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
{
    sal_uInt16 nLibs = mpMgr->GetLibCount();
    uno::Sequence< OUString > aRetSeq( nLibs );
    OUString* pRetSeq = aRetSeq.getArray();
    for( sal_uInt16 i = 0 ; i < nLibs ; i++ )
    {
        pRetSeq[i] = mpMgr->GetLibName( i );
    }
    return aRetSeq;
}

sal_Bool LibraryContainer_Impl::hasByName( const OUString& aName )
{
    bool bRet = mpMgr->HasLib( aName );
    return bRet;
}

// Methods XNameReplace
void LibraryContainer_Impl::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// Methods XNameContainer
void LibraryContainer_Impl::insertByName( const OUString& aName, const uno::Any& aElement )
{
    (void)aName;
    (void)aElement;
    // TODO: Insert a complete Library?!
}

void LibraryContainer_Impl::removeByName( const OUString& Name )
{
    StarBASIC* pLib = mpMgr->GetLib( Name );
    if( !pLib )
    {
        throw container::NoSuchElementException();
    }
    sal_uInt16 nLibId = mpMgr->GetLibId( Name );
    mpMgr->RemoveLib( nLibId );
}


typedef WeakImplHelper< script::XStarBasicAccess > StarBasicAccessHelper;


class StarBasicAccess_Impl : public StarBasicAccessHelper
{
    BasicManager* mpMgr;
    uno::Reference< container::XNameContainer > mxLibContainer;

public:
    explicit StarBasicAccess_Impl( BasicManager* pMgr )
        :mpMgr( pMgr ) {}

public:
    // Methods
    virtual uno::Reference< container::XNameContainer > SAL_CALL getLibraryContainer() override;
    virtual void SAL_CALL createLibrary( const OUString& LibName, const OUString& Password,
        const OUString& ExternalSourceURL, const OUString& LinkTargetURL ) override;
    virtual void SAL_CALL addModule( const OUString& LibraryName, const OUString& ModuleName,
        const OUString& Language, const OUString& Source ) override;
    virtual void SAL_CALL addDialog( const OUString& LibraryName, const OUString& DialogName,
        const uno::Sequence< sal_Int8 >& Data ) override;
};

uno::Reference< container::XNameContainer > SAL_CALL StarBasicAccess_Impl::getLibraryContainer()
{
    if( !mxLibContainer.is() )
        mxLibContainer = new LibraryContainer_Impl( mpMgr );
    return mxLibContainer;
}

void SAL_CALL StarBasicAccess_Impl::createLibrary
(
    const OUString& LibName,
    const OUString& Password,
    const OUString& ExternalSourceURL,
    const OUString& LinkTargetURL
)
{
    (void)ExternalSourceURL;
    StarBASIC* pLib = mpMgr->CreateLib( LibName, Password, LinkTargetURL );
    DBG_ASSERT( pLib, "XML Import: Basic library could not be created");
}

void SAL_CALL StarBasicAccess_Impl::addModule
(
    const OUString& LibraryName,
    const OUString& ModuleName,
    const OUString& Language,
    const OUString& Source
)
{
    (void)Language;
    StarBASIC* pLib = mpMgr->GetLib( LibraryName );
    DBG_ASSERT( pLib, "XML Import: Lib for module unknown");
    if( pLib )
    {
        pLib->MakeModule( ModuleName, Source );
    }
}

void SAL_CALL StarBasicAccess_Impl::addDialog
(
    const OUString& LibraryName,
    const OUString& DialogName,
    const uno::Sequence< sal_Int8 >& Data
)
{
    (void)LibraryName;
    (void)DialogName;
    (void)Data;
}

// Basic XML Import/Export
uno::Reference< script::XStarBasicAccess > getStarBasicAccess( BasicManager* pMgr )
{
    uno::Reference< script::XStarBasicAccess > xRet =
        new StarBasicAccess_Impl( pMgr );
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
