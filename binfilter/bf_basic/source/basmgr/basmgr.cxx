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
#include <bf_svtools/smplhint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <sbx.hxx>
#include <sot/storinfo.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <tools/debug.hxx>

#include <sbmod.hxx>

#include "basmgr.hxx"
#include "sbintern.hxx"


#define LIB_SEP			0x01
#define LIBINFO_SEP		0x02
#define LIBINFO_ID		0x1491
#define PASSWORD_MARKER	0x31452134


// Library API, implemented for XML import/export

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include <com/sun/star/script/XStarBasicModuleInfo.hpp>
#include <com/sun/star/script/XStarBasicDialogInfo.hpp>
#include <com/sun/star/script/XStarBasicLibraryInfo.hpp>

#include <cppuhelper/implbase1.hxx>

namespace binfilter {

using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace cppu;
using namespace rtl;

typedef WeakImplHelper1< XNameContainer > NameContainerHelper;
typedef WeakImplHelper1< XStarBasicModuleInfo > ModuleInfoHelper;
typedef WeakImplHelper1< XStarBasicDialogInfo > DialogInfoHelper;
typedef WeakImplHelper1< XStarBasicLibraryInfo > LibraryInfoHelper;
typedef WeakImplHelper1< XStarBasicAccess > StarBasicAccessHelper;


#define CURR_VER		2

// Version 1
//	  ULONG 	nEndPos
//	  USHORT 	nId
//	  USHORT	nVer
//	  BOOL		bDoLoad
//	  String	LibName
//	  String	AbsStorageName
//	  String	RelStorageName
// Version 2
//	+ BOOL		bReference

static const char szStdLibName[] = "Standard";
static const char szBasicStorage[] = "StarBASIC";
static const char szOldManagerStream[] = "BasicManager";
static const char szManagerStream[] = "BasicManager2";
static const char szImbedded[] = "LIBIMBEDDED";
static const char szCryptingKey[] = "CryptedBasic";
static const char szScriptLanguage[] = "StarBasic";

static const String BasicStreamName( RTL_CONSTASCII_USTRINGPARAM(szBasicStorage) );
static const String ManagerStreamName( RTL_CONSTASCII_USTRINGPARAM(szManagerStream) );

#define	DEFINE_CONST_UNICODE(CONSTASCII)    UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))


TYPEINIT1( BasicManager, SfxBroadcaster );
DBG_NAME( BasicManager );

StreamMode eStreamReadMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL;
StreamMode eStorageReadMode = STREAM_READ | STREAM_SHARE_DENYWRITE;

DECLARE_LIST( BasErrorLst, BasicError* )


//----------------------------------------------------------------------------
// BasicManager impl data
struct BasicManagerImpl
{
    LibraryContainerInfo* mpInfo;

    // Save stream data
    SvMemoryStream*  mpManagerStream;
    SvMemoryStream** mppLibStreams;
    sal_Int32        mnLibStreamCount;
    sal_Bool         mbModifiedByLibraryContainer;
    sal_Bool         mbError;

    BasicManagerImpl( void )
        : mpInfo( NULL )
        , mpManagerStream( NULL )
        , mppLibStreams( NULL )
        , mnLibStreamCount( 0 )
        , mbModifiedByLibraryContainer( sal_False )
        , mbError( sal_False )
    {}
    ~BasicManagerImpl();
};

BasicManagerImpl::~BasicManagerImpl()
{
    delete mpInfo;
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
    OUString maLibName;		// empty -> no lib, but lib container

public:
    BasMgrContainerListenerImpl( BasicManager* pMgr, OUString aLibName )
        : mpMgr( pMgr )
        , maLibName( aLibName ) {}

    static void insertLibraryImpl( const Reference< XLibraryContainer >& xScriptCont, BasicManager* pMgr,
        Any aLibAny, OUString aLibName );
    static void addLibraryModulesImpl( BasicManager* pMgr, Reference< XNameAccess > xLibNameAccess,
        OUString aLibName );


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
    BasicManager* pMgr, Any aLibAny, OUString aLibName )
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
    Reference< XNameAccess > xLibNameAccess, OUString aLibName )
{
    OUString aScriptLanguage = DEFINE_CONST_UNICODE( szScriptLanguage );
    Sequence< OUString > aModuleNames = xLibNameAccess->getElementNames();
    sal_Int32 nModuleCount = aModuleNames.getLength();

    StarBASIC* pLib = pMgr->GetLib( aLibName );
    DBG_ASSERT( pLib, "BasMgrContainerListenerImpl::addLibraryModulesImpl: Unknown lib!");
    if( pLib )
    {
        const OUString* pNames = aModuleNames.getConstArray();
        for( sal_Int32 j = 0 ; j < nModuleCount ; j++ )
        {
            OUString aModuleName = pNames[ j ];
            Any aElement = xLibNameAccess->getByName( aModuleName );
            OUString aMod;
            aElement >>= aMod;
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
    OUString aName;
    Event.Accessor >>= aName;

    mpMgr->mpImpl->mbModifiedByLibraryContainer = sal_True;

    if( bLibContainer )
    {
        Reference< XLibraryContainer > xScriptCont( Event.Source, UNO_QUERY );
        insertLibraryImpl( xScriptCont, mpMgr, Event.Element, aName );
    }
    else
    {
        OUString aScriptLanguage = DEFINE_CONST_UNICODE( szScriptLanguage );
        OUString aMod;
        Event.Element >>= aMod;

        StarBASIC* pLib = mpMgr->GetLib( maLibName );
        DBG_ASSERT( pLib, "BasMgrContainerListenerImpl::elementInserted: Unknown lib!");
        if( pLib )
        {
            SbModule* pMod = pLib->FindModule( aName );
            if( !pMod )
            {
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
    OUString aName;
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
        OUString aMod;
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
    OUString aName;
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
    BasErrorLst	aErrorList;

public:
                ~BasicErrorManager();

    void		Reset();
    void		InsertError( const BasicError& rError );

    BOOL		HasErrors()			{ return (BOOL)aErrorList.Count(); }
    BasicError*	GetFirstError()		{ return aErrorList.First(); }
    BasicError*	GetNextError()		{ return aErrorList.Next(); }
};


BasicErrorManager::~BasicErrorManager()
{
    Reset();
}

void BasicErrorManager::Reset()
{
    BasicError* pError = (BasicError*)aErrorList.First();
    while ( pError )
    {
        delete pError;
        pError = (BasicError*)aErrorList.Next();
    }
    aErrorList.Clear();
}

void BasicErrorManager::InsertError( const BasicError& rError )
{
    aErrorList.Insert( new BasicError( rError ), LIST_APPEND );
}

BasicError::BasicError( ULONG nId, USHORT nR, const String& rErrStr ) :
    aErrStr( rErrStr )
{
    nErrorId 	= nId;
    nReason 	= nR;
}

BasicError::BasicError( const BasicError& rErr ) :
    aErrStr( rErr.aErrStr )
{
    nErrorId 	= rErr.nErrorId;
    nReason		= rErr.nReason;
}


class BasicLibInfo
{
private:
    StarBASICRef	xLib;
    String			aLibName;
    String			aStorageName;	// String reicht, da zur Laufzeit eindeutig.
    String			aRelStorageName;
    String			aPassword;

    BOOL			bDoLoad;
    BOOL			bReference;
    BOOL			bPasswordVerified;
    BOOL			bFoundInPath;	// Darf dann nicht neu relativiert werden!

    // Lib represents library in new UNO library container
    Reference< XLibraryContainer > mxScriptCont;

public:
    BasicLibInfo();

    BOOL			IsReference() const		{ return bReference; }
    BOOL&			IsReference()			{ return bReference; }

    BOOL			IsExtern() const 		{ return ! aStorageName.EqualsAscii(szImbedded); }

    void			SetStorageName( const String& rName )	{ aStorageName = rName; }
    const String&	GetStorageName() const					{ return aStorageName; }

    void			SetRelStorageName( const String& rN )	{ aRelStorageName = rN; }
    const String&	GetRelStorageName()	const				{ return aRelStorageName; }
    void			CalcRelStorageName( const String& rMgrStorageName );

    StarBASICRef	GetLib() const
    {
        if( mxScriptCont.is() && mxScriptCont->hasByName( aLibName ) &&
            !mxScriptCont->isLibraryLoaded( aLibName ) )
                return StarBASICRef();
        return xLib;
    }
    StarBASICRef&	GetLibRef()							{ return xLib; }
    void			SetLib( StarBASIC* pBasic )			{ xLib = pBasic; }

    const String&	GetLibName() const					{ return aLibName; }
    void			SetLibName( const String& rName )	{ aLibName = rName; }

    // Nur temporaer fuer Laden/Speichern....
    BOOL			DoLoad()							{ return bDoLoad; }

    BOOL			HasPassword() const 				{ return aPassword.Len() != 0; }
    const String&	GetPassword() const					{ return aPassword; }
    void			SetPassword( const String& rNewPassword )
                                                        { aPassword = rNewPassword;	}
    BOOL			IsPasswordVerified() const			{ return bPasswordVerified; }
    void			SetPasswordVerified()				{ bPasswordVerified = TRUE; }

    BOOL			IsFoundInPath() const				{ return bFoundInPath; }
    void			SetFoundInPath( BOOL bInPath )		{ bFoundInPath = bInPath; }

    void 					Store( SotStorageStream& rSStream, const String& rBasMgrStorageName, BOOL bUseOldReloadInfo );
    static BasicLibInfo*	Create( SotStorageStream& rSStream );

    Reference< XLibraryContainer > GetLibraryContainer( void )
        { return mxScriptCont; }
    void SetLibraryContainer( const Reference< XLibraryContainer >& xScriptCont )
        { mxScriptCont = xScriptCont; }
};

DECLARE_LIST( BasicLibsBase, BasicLibInfo* )

class BasicLibs : public BasicLibsBase
{
public:
    String	aBasicLibPath;		// soll eigentlich Member vom Manager werden, aber jetzt nicht inkompatibel!
};

BasicLibInfo::BasicLibInfo()
{
    bReference 			= FALSE;
    bPasswordVerified 	= FALSE;
    bDoLoad 			= FALSE;
    bFoundInPath		= FALSE;
    mxScriptCont    	= NULL;
    aStorageName 		= String::CreateFromAscii(szImbedded);
    aRelStorageName 	= String::CreateFromAscii(szImbedded);
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

    // Falls nicht gesetzt, StorageName initialisieren
    if ( aStorageName.Len() == 0 )
        aStorageName = aCurStorageName;

    // Wieder laden?
    BOOL bDoLoad_ = xLib.Is();
    if ( bUseOldReloadInfo )
        bDoLoad_ = DoLoad();
    rSStream << bDoLoad_;

    // Den Namen der Lib...
    rSStream.WriteByteString(GetLibName());

    // Absoluter Pfad....
    if ( ! GetStorageName().EqualsAscii(szImbedded) )
    {
        String aSName = INetURLObject( GetStorageName(), INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
        DBG_ASSERT(aSName.Len() != 0, "Bad storage name");
        rSStream.WriteByteString( aSName );
    }
    else
        rSStream.WriteByteString( szImbedded );

    // Relativer Pfad...
    if ( ( aStorageName == aCurStorageName ) || ( aStorageName.EqualsAscii(szImbedded) ) )
        rSStream.WriteByteString( szImbedded );
    else
    {
        // Nicht den relativen Pfad ermitteln, wenn die Datei nur im Pfad
        // gefunden wurde: Dann andert sich der relative Pfad und nach einem
        // verschieben der Libs in einen anderen Pfad werden sie nicht gefunden.
        if ( !IsFoundInPath() )
            CalcRelStorageName( aCurStorageName );
        rSStream.WriteByteString(aRelStorageName);
    }

    // ------------------------------
    // Version 2
    // ------------------------------

    // Referenz...
    rSStream << bReference;

    // ------------------------------
    // Schluss
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
        // Wieder laden?
        BOOL bDoLoad;
        rSStream >> bDoLoad;
        pInfo->bDoLoad = bDoLoad;

        // Den Namen der Lib...
        String aName;
        rSStream.ReadByteString(aName);
        pInfo->SetLibName( aName );

        // Absoluter Pfad....
        String aStorageName;
        rSStream.ReadByteString(aStorageName);
        pInfo->SetStorageName( aStorageName );

        // Relativer Pfad...
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

BasicManager::BasicManager( SotStorage& rStorage, const String& rBaseURL, StarBASIC* pParentFromStdLib, String* pLibPath )
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

    // Wenn es den Manager-Stream nicht gibt, sind keine weiteren
    // Aktionen noetig.
    if ( rStorage.IsStream( ManagerStreamName ) )
    {
        LoadBasicManager( rStorage, rBaseURL );
        // StdLib erhaelt gewuenschten Parent:
        StarBASIC* pStdLib = GetStdLib();
        DBG_ASSERT( pStdLib, "Standard-Lib nicht geladen?" );
        if ( !pStdLib )
        {
            // Sollte eigentlich nie passieren, aber dann wenigstens nicht abstuerzen...
            pStdLib = new StarBASIC;
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
            // Die anderen erhalten die StdLib als Parent:
            for ( USHORT nBasic = 1; nBasic < GetLibCount(); nBasic++ )
            {
                StarBASIC* pBasic = GetLib( nBasic );
                if ( pBasic )
                {
//					pBasic->SetParent( pStdLib );
                    pStdLib->Insert( pBasic );
                    pBasic->SetFlag( SBX_EXTSEARCH );
                }
            }
            // Durch das Insert modified:
            pStdLib->SetModified( FALSE );
        }

        // #91626 Save all stream data to save it unmodified if basic isn't modified
        // in an 6.0+ office. So also the old basic dialogs can be saved.
        SotStorageStreamRef xManagerStream = rStorage.OpenSotStream
            ( ManagerStreamName, eStreamReadMode );
        mpImpl->mpManagerStream = new SvMemoryStream();
        *static_cast<SvStream*>(&xManagerStream) >> *mpImpl->mpManagerStream;

        SotStorageRef xBasicStorage = rStorage.OpenSotStorage
                                ( BasicStreamName, eStorageReadMode, FALSE );
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


void copyToLibraryContainer( StarBASIC* pBasic, LibraryContainerInfo* pInfo )
{
    Reference< XLibraryContainer > xScriptCont;
    String aLibName = pBasic->GetName();
    if( pInfo && (xScriptCont = pInfo->mxScriptCont).is() )
    {
        if( !xScriptCont->hasByName( aLibName ) )
            xScriptCont->createLibrary( aLibName );

        Any aLibAny = xScriptCont->getByName( aLibName );
        Reference< XNameContainer > xLib;
        aLibAny >>= xLib;
        if( xLib.is() )
        {
            USHORT nModCount = pBasic->GetModules()->Count();
            for ( USHORT nMod = 0 ; nMod < nModCount ; nMod++ )
            {
                SbModule* pModule = (SbModule*)pBasic->GetModules()->Get( nMod );
                DBG_ASSERT( pModule, "Modul nicht erhalten!" );

                String aModName = pModule->GetName();
                if( !xLib->hasByName( aModName ) )
                {
                    OUString aSource = pModule->GetSource32();
                    Any aSourceAny;
                    aSourceAny <<= aSource;
                    xLib->insertByName( aModName, aSourceAny );
                }
            }
        }
    }
}

void BasicManager::SetLibraryContainerInfo( LibraryContainerInfo* pInfo )
{
    if( !pInfo )
        return;
    mpImpl->mpInfo = pInfo;

    Reference< XLibraryContainer > xScriptCont;
    StarBASIC* pStdLib = GetStdLib();
    String aLibName = pStdLib->GetName();
    if( mpImpl->mpInfo && (xScriptCont = mpImpl->mpInfo->mxScriptCont).is() )
    {
        OUString aScriptLanguage = DEFINE_CONST_UNICODE( "StarBasic" );

        // Register listener for lib container
        OUString aEmptyLibName;
        Reference< XContainerListener > xLibContainerListener
            = static_cast< XContainerListener* >
                ( new BasMgrContainerListenerImpl( this, aEmptyLibName ) );

        Reference< XContainer> xLibContainer( xScriptCont, UNO_QUERY );
        xLibContainer->addContainerListener( xLibContainerListener );

        Sequence< OUString > aNames = xScriptCont->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 i, nNameCount = aNames.getLength();

        if( nNameCount )
        {
            for( i = 0 ; i < nNameCount ; i++ )
            {
                OUString aLibName2 = pNames[ i ];
                Any aLibAny = xScriptCont->getByName( aLibName2 );

                if( String( aLibName2 ).EqualsAscii( "Standard" ) )
                    xScriptCont->loadLibrary( aLibName2 );

                BasMgrContainerListenerImpl::insertLibraryImpl
                    ( xScriptCont, this, aLibAny, aLibName2 );
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
                    copyToLibraryContainer( pLib, mpImpl->mpInfo );
                    if( pBasLibInfo->HasPassword() )
                    {
                        OldBasicPassword* pOldBasicPassword =
                            mpImpl->mpInfo->mpOldBasicPassword;
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
}

BasicManager::BasicManager( StarBASIC* pSLib, String* pLibPath )
{
    DBG_CTOR( BasicManager, 0 );
    Init();
    DBG_ASSERT( pSLib, "BasicManager kann nicht mit einem NULL-Pointer erzeugt werden!" );

    if( pLibPath )
        pLibs->aBasicLibPath = *pLibPath;

    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( pSLib );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( String::CreateFromAscii(szStdLibName));
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    pSLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );

    // Speichern lohnt sich nur, wenn sich das Basic aendert.
    xStdLib->SetModified( FALSE );
    bBasMgrModified = FALSE;
}

BasicManager::BasicManager()
{
    DBG_CTOR( BasicManager, 0 );
    // Diese CTOR darf nur verwendet werden um bei 'Speichern unter'
    // die relativen Pfade anzupassen, das gibt kein AppBasic und somit
    // duerfen auch keine Libs geladen werden...
    Init();
}

BOOL BasicManager::HasBasicWithModules( const SotStorage& rStorage, const String& rBaseURL )
{
    if( !rStorage.IsStream( ManagerStreamName ) )
        return FALSE;

    StarBASIC* pDummyParentBasic = new StarBASIC();
    BasicManager* pBasMgr = new BasicManager( (SotStorage&)rStorage, rBaseURL, pDummyParentBasic );
    BOOL bRet = FALSE;

    USHORT nLibs = pBasMgr->GetLibCount();
    for( USHORT nL = 0; nL < nLibs; nL++ )
    {
        BasicLibInfo* pInfo = pBasMgr->pLibs->GetObject( nL );
        StarBASIC* pLib = pInfo->GetLib();
        if( !pLib )
        {
            BOOL bLoaded = pBasMgr->ImpLoadLibary( pInfo, NULL, FALSE );
            if( bLoaded )
                pLib = pInfo->GetLib();
        }
        if( pLib )
        {
            SbxArray* pModules = pLib->GetModules();
            if( pModules->Count() )
            {
                bRet = TRUE;
                break;
            }
        }
    }

    delete pBasMgr;
    return bRet;
}

void BasicManager::ImpMgrNotLoaded( const String& rStorageName )
{
    // pErrInf wird nur zerstoert, wenn der Fehler von einem ErrorHandler
    // gehandelt wird!
//	String aErrorText( BasicResId( IDS_SBERR_MGROPEN ) );
//	aErrorText.SearchAndReplace( "XX", rStorageName );
    StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN,	rStorageName, ERRCODE_BUTTON_OK );
    pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENMGRSTREAM, rStorageName ) );

    // Eine STD-Lib erzeugen, sonst macht es Peng!
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    pStdLibInfo->SetLib( new StarBASIC );
    StarBASICRef xStdLib = pStdLibInfo->GetLib();
    xStdLib->SetName( String::CreateFromAscii(szStdLibName) );
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    xStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
    xStdLib->SetModified( FALSE );
}


void BasicManager::ImpCreateStdLib( StarBASIC* pParentFromStdLib )
{
    BasicLibInfo* pStdLibInfo = CreateLibInfo();
    StarBASIC* pStdLib = new StarBASIC( pParentFromStdLib );
    pStdLibInfo->SetLib( pStdLib );
    pStdLib->SetName( String::CreateFromAscii(szStdLibName) );
    pStdLibInfo->SetLibName( String::CreateFromAscii(szStdLibName) );
    pStdLib->SetFlag( SBX_DONTSTORE | SBX_EXTSEARCH );
}


void BasicManager::LoadBasicManager( SotStorage& rStorage, const String& rBaseURL, BOOL bLoadLibs )
{
    DBG_CHKTHIS( BasicManager, 0 );

//	StreamMode eStreamMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE;

    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream
        ( ManagerStreamName, eStreamReadMode );

    String aStorName( rStorage.GetName() );
    // #i13114 removed, DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.Is() || xManagerStream->GetError() || ( xManagerStream->Seek( STREAM_SEEK_TO_END ) == 0 ) )
    {
        ImpMgrNotLoaded( aStorName );
        return;
    }

    maStorageName = INetURLObject(aStorName, INET_PROT_FILE).GetMainURL( INetURLObject::NO_DECODE );
    // #i13114 removed, DBG_ASSERT(aStorageName.Len() != 0, "Bad storage name");

    String aRealStorageName = maStorageName;  // fuer relative Pfade, kann durch BaseURL umgebogen werden.

    // Wenn aus Vorlagen geladen wird, gilt nur die BaseURL:
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
    // Plausi!
    if( nLibs & 0xF000 )
    {
        DBG_ASSERT( !this, "BasicManager-Stream defekt!" );
        return;
    }
    for ( USHORT nL = 0; nL < nLibs; nL++ )
    {
        BasicLibInfo* pInfo = BasicLibInfo::Create( *xManagerStream );

        // ggf. absoluten Pfad-Namen korrigieren, wenn rel. existiert
        // Immer erst den relativen versuchen, falls zwei Staende auf der Platte
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
                // Lib im Pfad suchen...
                String aSearchFile = pInfo->GetRelStorageName();
                SvtPathOptions aPathCFG;
                if( aPathCFG.SearchFile( aSearchFile, SvtPathOptions::PATH_BASIC ) )
                {
                    pInfo->SetStorageName( aSearchFile );
                    pInfo->SetFoundInPath( TRUE );
                }
            }
        }

        pLibs->Insert( pInfo, LIST_APPEND );
        // Libs aus externen Dateien sollen erst bei Bedarf geladen werden.
        // Aber Referenzen werden gleich geladen, sonst bekommen die Grosskunden
        // vielleicht Probleme...
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

//	StreamMode eStreamMode = STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE;

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
//		String aErrorText( BasicResId( IDS_SBERR_MGROPEN ) );
//      aErrorText.SearchAndReplace( "XX", aStorName );
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN, aStorName, ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENMGRSTREAM, aStorName ) );
        // und es geht weiter...
    }
    xManagerStream->Seek( nBasicEndOff+1 );	// +1: 0x00 als Trenner
    String aLibs;
    xManagerStream->ReadByteString(aLibs);
    xManagerStream->SetBufferSize( 0 );
    xManagerStream.Clear();	// Sream schliessen

    if ( aLibs.Len() )
    {
        String aCurStorageName( aStorName );
        INetURLObject aCurStorage( aCurStorageName, INET_PROT_FILE );
        USHORT nLibs = aLibs.GetTokenCount( LIB_SEP );
        for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
        {
            String aLibInfo( aLibs.GetToken( nLib, LIB_SEP ) );
            // == 2 soll irgendwann weg!
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
//				String aErrorText( BasicResId( IDS_SBERR_LIBLOAD ) );
//				aErrorText.SearchAndReplace( "XX", aLibName );
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD, aStorName, ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_STORAGENOTFOUND, aStorName ) );
            }
        }
    }
}

BasicManager::~BasicManager()
{
    DBG_DTOR( BasicManager, 0 );

    // Listener benachrichtigen, falls noch etwas zu Speichern...
    Broadcast( SfxSimpleHint( SFX_HINT_DYING) );

    // Basic-Infos zerstoeren...
    // In umgekehrter Reihenfolge, weil die StdLib Referenzen haelt, die
    // anderen nur die StdLib als Parent haben.
    BasicLibInfo* pInf = pLibs->Last();
    while ( pInf )
    {
        delete pInf;
        pInf = pLibs->Prev();
    }
    pLibs->Clear();
    delete pLibs;
    delete pErrorMgr;
    delete mpImpl;
}

void BasicManager::LegacyDeleteBasicManager( BasicManager*& _rpManager )
{
    delete _rpManager;
    _rpManager = NULL;
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
    pLibs->Insert( pInf, LIST_APPEND );
    return pInf;
}

BOOL BasicManager::CopyBasicData( SotStorage* pStorFrom, const String& rSourceURL, const String& rBaseURL, SotStorage* pStorTo )
{
    /*-----------------------------------------------------------------
     Diese Methode wird vom SXF gerufen bei 'Datei speichern unter',
     damit die Basic-Storages kopiert werden.
     Neu: ggf. muessen relative Pfade angepasst werden!
    ------------------------------------------------------------------*/
    BOOL bOk = TRUE;

    // bei remote Dokumenten identische Storage
    if ( pStorFrom != pStorTo )
    {
        if( pStorFrom->IsStorage( BasicStreamName ) )
            bOk = pStorFrom->CopyTo( BasicStreamName, pStorTo, BasicStreamName );
        if( bOk && pStorFrom->IsStream( ManagerStreamName ) )
        {
            // bOk = pStorFrom->CopyTo( szManagerStream, pStorTo, szManagerStream );
            BasicManager aBasMgr;
            // Die aktuelle Base-URL ist die vom speichern...
            String aStorName( pStorFrom->GetName() );
            DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

            aBasMgr.LoadBasicManager( *pStorFrom, rSourceURL, FALSE );
            aBasMgr.Store( *pStorTo, rBaseURL, FALSE );
        }
    }

    return bOk;
}

void BasicManager::Store( SotStorage& rStorage, const String& rBaseURL )
{
    Store( rStorage, rBaseURL, TRUE );
}

void BasicManager::Store( SotStorage& rStorage, const String& rBaseURL, BOOL bStoreLibs )
{
    // #91626
    sal_Bool bModified = mpImpl->mbModifiedByLibraryContainer || mpImpl->mbError;

    // #92172 Password handling
    OldBasicPassword* pOldBasicPassword = NULL;
    USHORT nLibs = GetLibCount();
    if( mpImpl->mpInfo )
        pOldBasicPassword = mpImpl->mpInfo->mpOldBasicPassword;
    if( pOldBasicPassword )
    {
        Reference< XLibraryContainer > xScriptCont = mpImpl->mpInfo->mxScriptCont;
        for( USHORT nL = 0; nL < nLibs; nL++ )
        {
            BasicLibInfo* pInfo = pLibs->GetObject( nL );
            DBG_ASSERT( pInfo, "pInfo?!" );

            String aLibName = pInfo->GetLibName();
            sal_Bool bPassword = pOldBasicPassword->hasLibraryPassword( aLibName );
            String aPassword = pOldBasicPassword->getLibraryPassword( aLibName );
            if( pInfo->GetPassword() != aPassword )
                bModified = sal_True;

            if( xScriptCont.is() && xScriptCont->hasByName( aLibName ) )
                xScriptCont->loadLibrary( aLibName );

            if( bPassword && aPassword.Len() == 0 )
            {
                String aDummySource( String::CreateFromAscii(
                    "REM This module belonged to a password protected library that was exported without\n"
                    "REM verifying the password. So the original module source could not be exported!\n"
                    "sub main\n"
                    "end sub\n" ) );
                Any aDummySourceAny;
                aDummySourceAny <<= OUString( aDummySource );

                if( xScriptCont.is() && xScriptCont->hasByName( aLibName ) )
                {
                    // Now the library isn't password protected any more
                    // but the modules don't contain any source
                    pOldBasicPassword->clearLibraryPassword( aLibName );

                    Any aLibAny = xScriptCont->getByName( aLibName );
                    Reference< XNameContainer > xLib;
                    aLibAny >>= xLib;

                    Sequence< OUString > aNames = xLib->getElementNames();
                    sal_Int32 nNameCount = aNames.getLength();
                    const OUString* pNames = aNames.getConstArray();
                    for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
                    {
                        OUString aElementName = pNames[ i ];
                        xLib->replaceByName( aElementName, aDummySourceAny );
                    }
                }

                StarBASIC* pBasic = GetLib( aLibName );
                if( pBasic )
                {
                    SbxArray* pModules = pBasic->GetModules();
                    USHORT nModCount = pModules->Count();
                    for( USHORT j = 0; j < nModCount ; j++ )
                    {
                        SbModule* pMod = (SbModule*)pModules->Get( j );
                        pMod->SetSource32( aDummySource );
                        pMod->Compile();
                    }
                }

                bModified = sal_True;
            }
            else
            {
                if( pInfo->GetPassword().Len() != 0 )
                    bModified = sal_True;
                pInfo->SetPassword( aPassword );
            }
        }
    }

    // #91626
    if( !bModified && bStoreLibs && mpImpl->mpManagerStream )
    {
        // Store saved streams
        SotStorageStreamRef xManagerStream = rStorage.OpenSotStream(
            ManagerStreamName, STREAM_STD_READWRITE | STREAM_TRUNC );
            //ManagerStreamName, STREAM_STD_READWRITE /* | STREAM_TRUNC */ );
        // STREAM_TRUNC buggy??!!! Then
        // xManagerStream->Seek( 0 );
        mpImpl->mpManagerStream->Seek( 0 );
        *static_cast<SvStream*>(&xManagerStream) << *mpImpl->mpManagerStream;

        SotStorageRef xBasicStorage = rStorage.OpenSotStorage
                                ( BasicStreamName, STREAM_STD_READWRITE, FALSE );

        if ( xBasicStorage.Is() && !xBasicStorage->GetError() )
        {
            for( USHORT nL = 0; nL < nLibs; nL++ )
            {
                BasicLibInfo* pInfo = pLibs->GetObject( nL );
                DBG_ASSERT( pInfo, "pInfo?!" );
                SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pInfo->GetLibName(), STREAM_STD_READWRITE );
                mpImpl->mppLibStreams[nL]->Seek( 0 );
                *static_cast<SvStream*>(&xBasicStream) << *( mpImpl->mppLibStreams[nL] );
            }

            xBasicStorage->Commit();
        }
        return;
    }


    DBG_CHKTHIS( BasicManager, 0 );
    // Neue Bibliotheken sind nicht unbedingt modified, muessen aber trotzdem
    // gespeichert werden.
    BOOL bStoreAll = FALSE;
    if ( bStoreLibs && !rStorage.IsStorage( BasicStreamName ) )
        bStoreAll = TRUE;

    SotStorageStreamRef xManagerStream = rStorage.OpenSotStream( ManagerStreamName, STREAM_STD_READWRITE /* | STREAM_TRUNC */ );
    // Assertion, weil es eigentlich nie vorkommen darf.
    DBG_ASSERT( xManagerStream.Is(), "Kein ManagerStream!");

    ClearErrors();

    String aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.Len(), "No Storage Name!" );

    if ( !xManagerStream.Is() || xManagerStream->GetError() )
    {
//		String aErrorText( BasicResId( IDS_SBERR_MGRSAVE ) );
//      aErrorText.SearchAndReplace( "XX", aStorName );
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGRSAVE, aStorName, ERRCODE_BUTTON_OK );
        ((BasicManager*)this)->pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENMGRSTREAM, aStorName ) );
    }
    else
    {
        maStorageName = aStorName;

        // Funktioniert nicht, auber auch OPEN_TRUNC nicht (UAE in  OLE32)
        // => Erst mal den alten Schrott stehen lassen!
//		xManagerStream->SetSize( 0 );
        xManagerStream->SetBufferSize( 1024 );
        xManagerStream->Seek( 0 );

        ULONG nStartPos = 0;
        sal_uInt32 nEndPos = 0;
        *xManagerStream << nEndPos;

        // Fehlerabfrage, falls schreiben nicht moeglich ?
        DBG_ASSERT( !xManagerStream->GetError(), "Unerwarteter Fehler beim Schreiben des Manager-Streams" );


        // Die Namen der Libs und der Storages...
//        DirEntry aCurStorage( aStorName() );
//        aCurStorage.ToAbs();
        String aLibInfo;
        *xManagerStream << nLibs;
        for ( USHORT nL = 0; nL < nLibs; nL++ )
        {
            BasicLibInfo* pInfo = pLibs->GetObject( nL );
            DBG_ASSERT( pInfo, "pInfo?!" );

            String aStrgName = GetStorageName();

            // Falls ins Temp-Verzeichniss gesichert wird, sollte der richtige
            // Storage-Name an der globalen URL haengen...
            //String aBaseURL = INetURLObject::GetBaseURL();
            if ( rBaseURL.Len() )
            {
                INetURLObject aObj( rBaseURL );
                if ( aObj.GetProtocol() == INET_PROT_FILE )
                    aStrgName = aObj.PathToFileName();
            }

            pInfo->Store( *xManagerStream, aStrgName, !bStoreLibs );
        }

        nEndPos = xManagerStream->Tell();
        xManagerStream->Seek( nStartPos );
        *xManagerStream << nEndPos;
        xManagerStream->Seek( nEndPos );
        xManagerStream->SetBufferSize( 0 );
        xManagerStream.Clear();
    }

    // Und dann die Libs in den entsprechenden Storages...
    if ( bStoreLibs )
    {
        String aCurStorage( aStorName );
        for ( USHORT nL = 0; nL < nLibs; nL++ )
        {
            BasicLibInfo* pInfo = pLibs->GetObject( nL );
            DBG_ASSERT( pInfo, "Info?!" );
            StarBASIC* pLib = pInfo->GetLib();
            if ( pLib && ( pLib->IsModified() || bStoreAll ) )	// 0, wenn nicht geladen...
            {
                if ( pInfo->IsReference() )
                {
                    // Vielleicht nicht modified, aber bStoreAll.
                    // Fehlermeldung nicht, wenn Macro lauft
                    // (also speichern durch Macro)
                    if ( pLib->IsModified() && !StarBASIC::IsRunning() )
                    {
                        String sTemp( String::CreateFromAscii( "Reference will not be saved: " ) );
                        sTemp += pLib->GetName();
                        WarningBox( NULL, WB_OK, sTemp ).Execute();
                        // Einmal meckern reicht.
                        pLib->SetModified( FALSE );
                    }
                }
                else
                {
                    SotStorageRef xStorage;
                    if ( ( pInfo->GetStorageName() != aCurStorage ) && ( !pInfo->GetStorageName().EqualsAscii(szImbedded) ) )
                        xStorage = new SotStorage( FALSE, pInfo->GetStorageName() );
                    else
                        xStorage = &rStorage;
                    ImpStoreLibary( pLib, *xStorage );
                }
            }
        }
    }

    if ( !HasErrors() )
        bBasMgrModified = FALSE;
}


BOOL BasicManager::ImpStoreLibary( StarBASIC* pLib, SotStorage& rStorage ) const
{
    DBG_CHKTHIS( BasicManager, 0 );
    DBG_ASSERT( pLib, "pLib = 0 (ImpStorageLibary)" );

    SotStorageRef xBasicStorage = rStorage.OpenSotStorage
                            ( BasicStreamName, STREAM_STD_READWRITE, FALSE );

    String aStorName( rStorage.GetName() );
    DBG_ASSERT( aStorName.Len(), "No Storage Name!" );
    if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
    {
//		String aErrorText( BasicResId( IDS_SBERR_MGRSAVE ) );
//      aErrorText.SearchAndReplace( "XX", aStorName );
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGRSAVE, aStorName, ERRCODE_BUTTON_OK );
        ((BasicManager*)this)->pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLib->GetName() ) );
    }
    else
    {
        // In dem Basic-Storage liegt jede Lib in einem Stream...
        SotStorageStreamRef xBasicStream = xBasicStorage->OpenSotStream( pLib->GetName(), STREAM_STD_READWRITE );
        if ( !xBasicStream.Is() || xBasicStream->GetError() )
        {
//			String aErrorText( BasicResId( IDS_SBERR_LIBSAVE ) );
//			aErrorText.SearchAndReplace( "XX", pLib->GetName() );
            StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBSAVE,	pLib->GetName(), ERRCODE_BUTTON_OK );
            ((BasicManager*)this)->pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTREAM, pLib->GetName() ) );
        }
        else
        {
            BasicLibInfo* pLibInfo = FindLibInfo( pLib );
            DBG_ASSERT( pLibInfo, "ImpStoreLibary: LibInfo?!" );

            xBasicStream->SetSize( 0 );
            xBasicStream->SetBufferSize( 1024 );

            // SBX_DONTSTORE, damit Child-Basics nicht gespeichert werden
            SetFlagToAllLibs( SBX_DONTSTORE, TRUE );
            // Aber das hier will ich jetzt speichern:
            pLib->ResetFlag( SBX_DONTSTORE );
            if ( pLibInfo->HasPassword() )
                xBasicStream->SetKey( szCryptingKey );
            BOOL bDone = pLib->Store( *xBasicStream );
            xBasicStream->SetBufferSize( 0 );
            if ( bDone )
            {
                // Diese Informationen immer verschluesseln...
                xBasicStream->SetBufferSize( 1024 );
                xBasicStream->SetKey( szCryptingKey );
                *xBasicStream << static_cast<sal_uInt32>(PASSWORD_MARKER);
                String aTmpPassword = pLibInfo->GetPassword();
                xBasicStream->WriteByteString( aTmpPassword, RTL_TEXTENCODING_MS_1252 );
                xBasicStream->SetBufferSize( 0 );
            }
            // Vorsichtshalber alle Dont't Store lassen...
            pLib->SetFlag( SBX_DONTSTORE );
//			SetFlagToAllLibs( SBX_DONTSTORE, FALSE );
            pLib->SetModified( FALSE );
            if( !xBasicStorage->Commit() )
                bDone = FALSE;
            xBasicStream->SetKey( ByteString() );
            DBG_ASSERT( bDone, "Warum geht Basic::Store() nicht ?" );
            return bDone;
        }
    }
    return FALSE;
}

BOOL BasicManager::ImpLoadLibary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, BOOL bInfosOnly ) const
{
    DBG_CHKTHIS( BasicManager, 0 );

    DBG_ASSERT( pLibInfo, "LibInfo!?" );

    String aStorageName( pLibInfo->GetStorageName() );
    if ( !aStorageName.Len() || ( aStorageName.EqualsAscii(szImbedded) ) )
        aStorageName = GetStorageName();

    SotStorageRef xStorage;
    // Der aktuelle darf nicht nochmal geoffnet werden...
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
                            ( BasicStreamName, eStorageReadMode, FALSE );

    if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
    {
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_MGROPEN,	xStorage->GetName(), ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName() ) );
    }
    else
    {
        // In dem Basic-Storage liegt jede Lib in einem Stream...
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
                        pLibInfo->SetLib( new StarBASIC( GetStdLib() ) );
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
                    // Das Basic skippen...
                    xBasicStream->Seek( STREAM_SEEK_TO_BEGIN );
                    ImplEncryptStream( *xBasicStream );
                    SbxBase::Skip( *xBasicStream );
                    bLoaded = TRUE;
                }
            }
            if ( !bLoaded )
            {
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_LIBLOAD,	pLibInfo->GetLibName(), ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_BASICLOADERROR, pLibInfo->GetLibName() ) );
            }
            else
            {
                // Ggf. stehen weitere Informationen im Stream...
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
//			bBasMgrModified = TRUE;	// Warum?
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
        // sollte nur bei verschluesselten Streams nicht stimmen.
        bProtected = TRUE;
        rStrm.SetKey( szCryptingKey );
        rStrm.RefreshBuffer();
    }
    return bProtected;
}


// Dieser Code ist notwendig, um das BASIC der Beta 1 laden zu koennen
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
            // Den Parent des alten BASICs uebernehmen
            if( rOldBasic.Is() )
            {
                pNew->SetParent( rOldBasic->GetParent() );
                if( pNew->GetParent() )
                    pNew->GetParent()->Insert( pNew );
                pNew->SetFlag( SBX_EXTSEARCH );
            }
            rOldBasic = pNew;

            // Fill new libray container (5.2 -> 6.0)
            copyToLibraryContainer( pNew, mpImpl->mpInfo );

/*
            if( rOldBasic->GetParent() )
            {
                rOldBasic->GetParent()->Insert( rOldBasic );
                rOldBasic->SetFlag( SBX_EXTSEARCH );
            }
*/
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

/*?*/ // 	for ( USHORT nMod = 0; nMod < pLib->GetModules()->Count(); nMod++ )
/*?*/ // 	{
/*?*/ // 		SbModule* pModule = (SbModule*)pLib->GetModules()->Get( nMod );
/*?*/ // 		DBG_ASSERT( pModule, "Modul nicht erhalten!" );
/*?*/ // 		if ( !pModule->IsCompiled() && !StarBASIC::GetErrorCode() )
/*?*/ // 			pLib->Compile( pModule );
/*?*/ // 	}

    // #67477, AB 8.12.99 On demand Compilieren bei referenzierten
    // Libraries sollte nicht zu modified fuehren
    if( !bModified && bReference )
    {
        DBG_ERROR( "Per Reference eingebundene Basic-Library ist nicht compiliert!" );
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
    // Erstmal mit dem Original-Namen, da sonst ImpLoadLibary nicht geht...
    pLibInfo->SetLibName( rLibName );
    // Funktioniert so aber nicht, wenn Name doppelt
//	USHORT nLibId = GetLibId( rLibName );
    USHORT nLibId = (USHORT) pLibs->GetPos( pLibInfo );

    // Vorm Laden StorageNamen setzen, da er mit pCurStorage verglichen wird.
    pLibInfo->SetStorageName( aStorageName );
    BOOL bLoaded = ImpLoadLibary( pLibInfo, &rStorage );

    if ( bLoaded )
    {
        if ( aNewLibName != rLibName )
            SetLibName( nLibId, aNewLibName );

        if ( bReference )
        {
            pLibInfo->GetLib()->SetModified( FALSE );	// Dann nicht speichern
            pLibInfo->SetRelStorageName( String() );
//			pLibInfo->CalcRelStorageName( GetStorageName() );
            pLibInfo->IsReference() = TRUE;
        }
        else
        {
            pLibInfo->GetLib()->SetModified( TRUE );		// Muss nach Add gespeichert werden!
            pLibInfo->SetStorageName( String::CreateFromAscii(szImbedded) );			// Im BasicManager-Storage speichern
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
    // Nur physikalisch loeschen, wenn keine Referenz.
    return RemoveLib( nLib, !IsReference( nLib ) );
}

BOOL BasicManager::RemoveLib( USHORT nLib, BOOL bDelBasicFromStorage )
{
    DBG_CHKTHIS( BasicManager, 0 );
    DBG_ASSERT( nLib, "Standard-Lib kann nicht entfernt werden!" );

    BasicLibInfo* pLibInfo = pLibs->GetObject( nLib );
    DBG_ASSERT( pLibInfo, "Lib nicht gefunden!" );

    if ( !pLibInfo || !nLib )
    {
//		String aErrorText( BasicResId( IDS_SBERR_REMOVELIB ) );
        StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, String(), ERRCODE_BUTTON_OK );
        pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_STDLIB, pLibInfo->GetLibName() ) );
        return FALSE;
    }

    // Wenn einer der Streams nicht geoeffnet werden kann, ist es kein
    // Fehler, es gibt halt noch nichts zum loeschen, weil das Basic noch
    // nie geschrieben wurde...
    if ( bDelBasicFromStorage && !pLibInfo->IsReference() &&
            ( !pLibInfo->IsExtern() || SotStorage::IsStorageFile( pLibInfo->GetStorageName() ) ) )
    {
        SotStorageRef xStorage;
        if ( !pLibInfo->IsExtern() )
            xStorage = new SotStorage( FALSE, GetStorageName() );
        else
            xStorage = new SotStorage( FALSE, pLibInfo->GetStorageName() );

        if ( xStorage->IsStorage( BasicStreamName ) )
        {
            SotStorageRef xBasicStorage = xStorage->OpenSotStorage
                            ( BasicStreamName, STREAM_STD_READWRITE, FALSE );

            if ( !xBasicStorage.Is() || xBasicStorage->GetError() )
            {
//				String aErrorText( BasicResId( IDS_SBERR_REMOVELIB ) );
                StringErrorInfo* pErrInf = new StringErrorInfo( ERRCODE_BASMGR_REMOVELIB, String(), ERRCODE_BUTTON_OK );
                pErrorMgr->InsertError( BasicError( *pErrInf, BASERR_REASON_OPENLIBSTORAGE, pLibInfo->GetLibName() ) );
            }
            else if ( xBasicStorage->IsStream( pLibInfo->GetLibName() ) )
            {
                xBasicStorage->Remove( pLibInfo->GetLibName() );
                xBasicStorage->Commit();

                // Wenn kein weiterer Stream vorhanden,
                // dann auch den SubStorage loeschen.
                SvStorageInfoList aInfoList( 0, 4 );
                xBasicStorage->FillInfoList( &aInfoList );
                if ( !aInfoList.Count() )
                {
                    xBasicStorage.Clear();
                    xStorage->Remove( BasicStreamName );
                    xStorage->Commit();
                    // Wenn kein weiterer Streams oder SubStorages vorhanden,
                    // dann auch den Storage loeschen.
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
    return TRUE;	// Remove hat geklappt, Del unwichtig.
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
        if ( pInf->GetLibName().CompareIgnoreCaseToAscii( rName ) == COMPARE_EQUAL )		// prueffen, ob vorhanden...
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

StarBASIC* BasicManager::CreateLib( const String& rLibName )
{
    DBG_CHKTHIS( BasicManager, 0 );
    if ( GetLib( rLibName ) )
        return 0;

    BasicLibInfo* pLibInfo = CreateLibInfo();
    StarBASIC* pNew = new StarBASIC( GetStdLib() );
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

                //if( !pLibInfo )
                    //pLibInfo = FindLibInfo( pLib );
                //pLibInfo->SetStorageName( LinkTargetURL );
                //pLibInfo->GetLib()->SetModified( FALSE );	// Dann nicht speichern
                //pLibInfo->SetRelStorageName( String() );
                //pLibInfo->IsReference() = TRUE;
            }
            //else
                //Message?

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
    StarBASIC* pNew = new StarBASIC( GetStdLib() );
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
    virtual OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual OUString SAL_CALL getLanguage() throw(RuntimeException)
        { return maLanguage; }
    virtual OUString SAL_CALL getSource() throw(RuntimeException)
        { return maSource; }
};


//=====================================================================

class DialogInfo_Impl : public DialogInfoHelper
{
    OUString maName;
    Sequence< sal_Int8 > mData;

public:
    DialogInfo_Impl( const OUString& aName, Sequence< sal_Int8 > Data )
        : maName( aName ), mData( Data ) {}

    // Methods XStarBasicDialogInfo
    virtual OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual Sequence< sal_Int8 > SAL_CALL getData() throw(RuntimeException)
        { return mData; }
};


//=====================================================================

class LibraryInfo_Impl : public LibraryInfoHelper
{
    OUString maName;
    Reference< XNameContainer > mxModuleContainer;
    Reference< XNameContainer > mxDialogContainer;
    OUString maPassword;
    OUString maExternaleSourceURL;
    OUString maLinkTargetURL;

public:
    LibraryInfo_Impl
    (
        const OUString& aName,
        Reference< XNameContainer > xModuleContainer,
        Reference< XNameContainer > xDialogContainer,
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
    virtual OUString SAL_CALL getName() throw(RuntimeException)
        { return maName; }
    virtual Reference< XNameContainer > SAL_CALL getModuleContainer() throw(RuntimeException)
        { return mxModuleContainer; }
    virtual Reference< XNameContainer > SAL_CALL getDialogContainer() throw(RuntimeException)
        { return mxDialogContainer; }
    virtual OUString SAL_CALL getPassword() throw(RuntimeException)
        { return maPassword; }
    virtual OUString SAL_CALL getExternalSourceURL() throw(RuntimeException)
        { return maExternaleSourceURL; }
    virtual OUString SAL_CALL getLinkTargetURL() throw(RuntimeException)
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
    virtual Any SAL_CALL getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
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
Any ModuleContainer_Impl::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    if( !pMod )
        throw NoSuchElementException();
    Reference< XStarBasicModuleInfo > xMod = (XStarBasicModuleInfo*)new ModuleInfo_Impl
        ( aName, OUString::createFromAscii( "StarBasic" ), pMod->GetSource32() );
    Any aRetAny;
    aRetAny <<= xMod;
    return aRetAny;
}

Sequence< OUString > ModuleContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    SbxArray* pMods = mpLib ? mpLib->GetModules() : NULL;
    USHORT nMods = pMods ? pMods->Count() : 0;
    Sequence< OUString > aRetSeq( nMods );
    OUString* pRetSeq = aRetSeq.getArray();
    for( USHORT i = 0 ; i < nMods ; i++ )
    {
        SbxVariable* pMod = pMods->Get( i );
        pRetSeq[i] = OUString( pMod->GetName() );
    }
    return aRetSeq;
}

sal_Bool ModuleContainer_Impl::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    SbModule* pMod = mpLib ? mpLib->FindModule( aName ) : NULL;
    sal_Bool bRet = (pMod != NULL);
    return bRet;
}


// Methods XNameReplace
void ModuleContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void ModuleContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
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

void ModuleContainer_Impl::removeByName( const OUString& Name )
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
#define SBXID_DIALOG		101


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
    virtual Any SAL_CALL getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
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
Any DialogContainer_Impl::getByName( const OUString& aName )
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

Sequence< OUString > DialogContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    mpLib->GetAll( SbxCLASS_OBJECT );
    sal_Int16 nCount = mpLib->GetObjects()->Count();
    Sequence< OUString > aRetSeq( nCount );
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
void DialogContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}


// Methods XNameContainer
void DialogContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
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

void DialogContainer_Impl::removeByName( const OUString& Name )
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
    virtual Any SAL_CALL getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames()
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(RuntimeException);

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name )
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
Any LibraryContainer_Impl::getByName( const OUString& aName )
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

    OUString aPassword = pLibInfo->GetPassword();

    // TODO Nur extern-Info liefern!
    OUString aExternaleSourceURL;
    OUString aLinkTargetURL;
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

Sequence< OUString > LibraryContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    USHORT nLibs = mpMgr->GetLibCount();
    Sequence< OUString > aRetSeq( nLibs );
    OUString* pRetSeq = aRetSeq.getArray();
    for( USHORT i = 0 ; i < nLibs ; i++ )
    {
        pRetSeq[i] = OUString( mpMgr->GetLibName( i ) );
    }
    return aRetSeq;
}

sal_Bool LibraryContainer_Impl::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    sal_Bool bRet = mpMgr->HasLib( aName );
    return bRet;
}

// Methods XNameReplace
void LibraryContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    removeByName( aName );
    insertByName( aName, aElement );
}

// Methods XNameContainer
void LibraryContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    (void)aName;
    (void)aElement;
    // TODO: Insert a complete Library?!
}

void LibraryContainer_Impl::removeByName( const OUString& Name )
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
    virtual void SAL_CALL createLibrary( const OUString& LibName, const OUString& Password,
        const OUString& ExternalSourceURL, const OUString& LinkTargetURL )
            throw(ElementExistException, RuntimeException);
    virtual void SAL_CALL addModule( const OUString& LibraryName, const OUString& ModuleName,
        const OUString& Language, const OUString& Source )
            throw(NoSuchElementException, RuntimeException);
    virtual void SAL_CALL addDialog( const OUString& LibraryName, const OUString& DialogName,
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
    const OUString& LibName,
    const OUString& Password,
    const OUString& ExternalSourceURL,
    const OUString& LinkTargetURL
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
    const OUString& LibraryName,
    const OUString& ModuleName,
    const OUString& Language,
    const OUString& Source
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
    const OUString& LibraryName,
    const OUString& DialogName,
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
