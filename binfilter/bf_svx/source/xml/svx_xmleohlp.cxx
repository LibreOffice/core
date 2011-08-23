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

#include <stdio.h>

#include <tools/debug.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>

#include <comphelper/classids.hxx>
#include <bf_so3/embobj.hxx>
#include <cppuhelper/implbase1.hxx>	// helper for implementations

#include <map>

#include "xmleohlp.hxx"
namespace binfilter {

// -----------
// - Defines -
// -----------

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::utl;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

#define XML_CONTAINERSTORAGE_NAME		"Objects"
#define XML_EMBEDDEDOBJECT_URL_BASE		"vnd.sun.star.EmbeddedObject:"

// -----------------------------------------------------------------------------

class InputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<stario::XInputStream>
{
    ::osl::Mutex	maMutex;
    SvStorageRef xStor;
    Reference < XInputStream > xIn;
    TempFile aTempFile;

public:
    InputStorageWrapper_Impl( SvPersist *pPersist );
    virtual ~InputStorageWrapper_Impl();

    virtual sal_Int32	SAL_CALL	readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32	SAL_CALL	readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void		SAL_CALL	skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32	SAL_CALL	available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void		SAL_CALL	closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
};


/*N*/ InputStorageWrapper_Impl::InputStorageWrapper_Impl(
/*N*/ 		SvPersist *pPersist )
/*N*/ {
/*N*/ 	SvStream *pStream = 0;
/*N*/ 	aTempFile.EnableKillingFile();
/*N*/ 	pStream = aTempFile.GetStream( STREAM_READWRITE );
/*N*/ 	SvStorageRef aTempStor = new SvStorage( sal_False, *pStream );
/*N*/	// the object should not be stored in 5.2 storage since the alien objects are stored in a wrapped way in this case
/*N*/	aTempStor->SetVersion( SOFFICE_FILEFORMAT_31 );
/*N*/ 	if( pPersist->DoSaveAs( aTempStor ) )
/*N*/ 	{
/*N*/ 		aTempStor->Commit();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		aTempStor.Clear();
/*?*/ 		pStream = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pStream )
/*N*/ 	{
/*N*/ 		pStream->Seek( 0 );
/*N*/ 		xIn = new OInputStreamWrapper( *pStream );
/*N*/ 	}
/*N*/ }

/*N*/ InputStorageWrapper_Impl::~InputStorageWrapper_Impl()
/*N*/ {
/*N*/ 	xIn = 0;
/*N*/ 	xStor = 0;
/*N*/ }

/*N*/ sal_Int32 SAL_CALL InputStorageWrapper_Impl::readBytes(
/*N*/ 		Sequence< sal_Int8 >& aData,
/*N*/ 		sal_Int32 nBytesToRead)
/*N*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*N*/ {
/*N*/ 	MutexGuard			aGuard( maMutex );
/*N*/ 	return xIn->readBytes( aData, nBytesToRead );
/*N*/ }

/*?*/ sal_Int32 SAL_CALL InputStorageWrapper_Impl::readSomeBytes(
/*?*/ 		Sequence< sal_Int8 >& aData,
/*?*/ 		sal_Int32 nMaxBytesToRead)
/*?*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*?*/ void SAL_CALL InputStorageWrapper_Impl::skipBytes( sal_Int32 nBytesToSkip )
/*?*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*?*/ sal_Int32 SAL_CALL InputStorageWrapper_Impl::available()
/*?*/ 	throw(NotConnectedException, RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*?*/ void SAL_CALL InputStorageWrapper_Impl::closeInput()
/*?*/ 	throw(NotConnectedException, RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

// -----------------------------------------------------------------------------

class OutputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<XOutputStream>
{
    ::osl::Mutex	maMutex;
    SvStorageRef xStor;
    Reference < XOutputStream > xOut;
    TempFile aTempFile;
    sal_Bool bStreamClosed : 1;
    sal_Bool bCreateStorageFailed : 1;

public:
    OutputStorageWrapper_Impl();
    virtual ~OutputStorageWrapper_Impl();

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL flush() throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL closeOutput() throw(NotConnectedException, BufferSizeExceededException, RuntimeException);

    SvStorage *GetStorage();
};

/*N*/ OutputStorageWrapper_Impl::OutputStorageWrapper_Impl() :
/*N*/ 	bStreamClosed( sal_False ),
/*N*/ 	bCreateStorageFailed( sal_False )
/*N*/ {
/*N*/ 	aTempFile.EnableKillingFile();
/*N*/ 	SvStream *pStream = aTempFile.GetStream( STREAM_READWRITE );
/*N*/ 	xOut = new OOutputStreamWrapper( *pStream );
/*N*/ }

/*N*/ OutputStorageWrapper_Impl::~OutputStorageWrapper_Impl()
/*N*/ {
/*N*/ 	xStor = 0;
/*N*/ 	xOut = 0;
/*N*/ }

/*N*/ SvStorage *OutputStorageWrapper_Impl::GetStorage()
/*N*/ {
/*N*/ 	if( !xStor )
/*N*/ 	{
/*N*/ 		if( bStreamClosed && !bCreateStorageFailed )
/*N*/ 		{
/*N*/ 			xStor = new SvStorage( *aTempFile.GetStream( STREAM_READWRITE ) );
/*N*/ 			if( xStor->GetError() != 0 )
/*N*/ 			{
/*?*/ 				xStor = 0;
/*?*/ 				bCreateStorageFailed = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return xStor;
/*N*/ }

/*N*/ void SAL_CALL OutputStorageWrapper_Impl::writeBytes(
/*N*/ 		const Sequence< sal_Int8 >& aData)
/*N*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*N*/ {
/*N*/ 	MutexGuard			aGuard( maMutex );
/*N*/ 	xOut->writeBytes( aData );
/*N*/ }

/*?*/ void SAL_CALL OutputStorageWrapper_Impl::flush()
/*?*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*N*/ void SAL_CALL OutputStorageWrapper_Impl::closeOutput()
/*N*/ 	throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
/*N*/ {
/*N*/ 	MutexGuard			aGuard( maMutex );
/*N*/ 	xOut->closeOutput();
/*N*/ 	bStreamClosed = sal_True;
/*N*/ }

// -----------------------------------------------------------------------------

struct OUStringLess
{
    bool operator() ( const ::rtl::OUString& r1, const ::rtl::OUString& r2 ) const
    {
        return (r1 < r2) != sal_False;
    }
};

// -----------------------------------------------------------------------------

// -----------------------------
// - SvXMLEmbeddedObjectHelper -
// -----------------------------

/*N*/ SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
/*N*/ 	WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
/*N*/ 	maDefaultContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
/*N*/ 	mpRootStorage( 0 ),
/*N*/ 	mpDocPersist( 0 ),
/*N*/ 	mpStreamMap( 0 ),
/*N*/ 	meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
/*N*/ {
/*N*/ }

/*N*/ SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( SvPersist& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
/*N*/ 	WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
/*N*/ 	maDefaultContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
/*N*/ 	mpRootStorage( 0 ),
/*N*/ 	mpDocPersist( 0 ),
/*N*/ 	mpStreamMap( 0 ),
/*N*/ 	meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
/*N*/ {
/*N*/ 	Init( 0, rDocPersist, eCreateMode );
/*N*/ }


// -----------------------------------------------------------------------------

/*N*/ SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
/*N*/ {
/*N*/ 	if( mpStreamMap )
/*N*/ 	{
/*?*/ 		SvXMLEmbeddedObjectHelper_Impl::iterator aIter = mpStreamMap->begin();
/*?*/ 		SvXMLEmbeddedObjectHelper_Impl::iterator aEnd = mpStreamMap->end();
/*?*/ 		for( aIter; aIter != aEnd; aIter++ )
/*?*/ 		{
/*?*/ 			if( aIter->second )
/*?*/ 			{
/*?*/ 				aIter->second->release();
/*?*/ 				aIter->second = 0;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SAL_CALL SvXMLEmbeddedObjectHelper::disposing()
/*N*/ {
/*N*/ 	Flush();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ sal_Bool SvXMLEmbeddedObjectHelper::ImplGetStorageNames(
/*N*/ 		const OUString& rURLStr, 
/*N*/ 		OUString& rContainerStorageName, 
/*N*/ 		OUString& rObjectStorageName,
/*N*/ 		sal_Bool bInternalToExternal ) const
/*N*/ {
/*N*/ 	// internal URL: vnd.sun.star.EmbeddedObject:<object-name>
/*N*/ 	// 			 or: vnd.sun.star.EmbeddedObject:<path>/<object-name>
/*N*/ 	// external URL: #./<path>/<object-name>
/*N*/ 	// 			 or: #<path>/<object-name>
/*N*/ 	// 			 or: #<object-name>
/*N*/ 	// currently, path may only consist of a single directory name
/*N*/ 	sal_Bool	bRet = sal_False;
/*N*/ 
/*N*/ 	if( !rURLStr.getLength() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	if( bInternalToExternal )
/*N*/ 	{
/*N*/ 		sal_Int32 nPos = rURLStr.indexOf( ':' );
/*N*/ 		if( -1 == nPos ||
/*N*/ 			0 != rURLStr.compareToAscii( XML_EMBEDDEDOBJECT_URL_BASE,
/*N*/ 								 sizeof( XML_EMBEDDEDOBJECT_URL_BASE ) -1 ) )
/*N*/ 			return sal_False;
/*N*/ 
/*N*/ 		sal_Int32 nPathStart = nPos + 1;
/*N*/ 		nPos = rURLStr.lastIndexOf( '/' );
/*N*/ 		if( -1 == nPos )
/*N*/ 		{
/*N*/ 			rContainerStorageName = OUString();
/*N*/ 			rObjectStorageName = rURLStr.copy( nPathStart );
/*N*/ 		}
/*N*/ 		else if( nPos > nPathStart )
/*N*/ 		{
/*?*/ 			rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
/*?*/ 			rObjectStorageName = rURLStr.copy( nPos+1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			return sal_False;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( '#' != rURLStr[0 ] )
/*N*/ 			return sal_False;
/*N*/ 
/*N*/ 		sal_Int32 nPos = rURLStr.lastIndexOf( '/' );
/*N*/ 		if( -1 == nPos )
/*N*/ 		{
/*N*/ 			rContainerStorageName = OUString();
/*N*/ 			rObjectStorageName = rURLStr.copy( 1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			sal_Int32 nPathStart = 1;
/*?*/ 			if( 0 == rURLStr.compareToAscii( "#./", 3 ) )
/*?*/ 				nPathStart = 3;
/*?*/ 			if( nPos >= nPathStart )
/*?*/ 				rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
/*?*/ 			rObjectStorageName = rURLStr.copy( nPos+1 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( -1 != rContainerStorageName.indexOf( '/' ) )
/*N*/ 	{
/*?*/ 		DBG_ERROR( "SvXMLEmbeddedObjectHelper: invalid path name" );
/*?*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }


// -----------------------------------------------------------------------------

/*N*/ SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
/*N*/ 		const OUString& rStorageName )
/*N*/ {
/*N*/ 	DBG_ASSERT( -1 == rStorageName.indexOf( '/' ) &&
/*N*/ 				-1 == rStorageName.indexOf( '\\' ),
/*N*/ 				"nested embedded storages aren't supported" );
/*N*/ 	if( !mxContainerStorage.Is() ||
/*N*/ 		( rStorageName != maCurContainerStorageName ) )
/*N*/ 	{
/*N*/ 		if( mxContainerStorage.Is() &&
/*N*/ 			maCurContainerStorageName.getLength() > 0 &&
/*N*/ 			EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode )
/*?*/ 			mxContainerStorage->Commit();
/*N*/ 
/*N*/ 		if( rStorageName.getLength() > 0 && mpRootStorage )
/*N*/ 		{
/*?*/ 			StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
/*?*/ 									? STREAM_STD_READWRITE			
/*?*/ 									: STREAM_STD_READ;
/*?*/ 			mxContainerStorage = mpRootStorage->OpenStorage( rStorageName,
/*N*/ 															 eMode );	
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			mxContainerStorage = mpRootStorage;
/*N*/ 		}
/*N*/ 		maCurContainerStorageName = rStorageName;
/*N*/ 	}
/*N*/ 
/*N*/ 	return mxContainerStorage;
/*N*/ }

// -----------------------------------------------------------------------------

/*?*/ SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetObjectStorage(
/*?*/ 		const OUString& rContainerStorageName,
/*?*/ 		const OUString& rObjectStorageName,
/*?*/ 		sal_Bool bUCBStorage )
/*?*/ {
/*?*/ 	SvStorageRef xObjStor;
/*?*/ 	SvStorageRef xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) ); 
/*?*/ 	if( xCntnrStor.Is() )
/*?*/ 	{
/*?*/ 		StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
/*?*/ 								? STREAM_STD_READWRITE			
/*?*/ 								: STREAM_STD_READ;
/*?*/ 		xObjStor = xCntnrStor->OpenStorage( rObjectStorageName, eMode );
/*?*/ 	}
/*?*/ 
/*?*/ 	return xObjStor;
/*?*/ }

// -----------------------------------------------------------------------------

/*N*/ String SvXMLEmbeddedObjectHelper::ImplGetUniqueName( SvStorage* pStg,
/*N*/ 											  const sal_Char* p ) const
/*N*/ {
/*N*/ 	String aP = String( ByteString( p ), RTL_TEXTENCODING_UTF8 );
/*N*/ 	String aName;
/*N*/ 	static ULONG nId = (ULONG)&aP;
/*N*/ 	nId++;
/*N*/ 	for( ;; )
/*N*/ 	{
/*N*/ 		aName = aP;
/*N*/ 		sal_Int32 nBitsLeft = 32;
/*N*/ 		while( nBitsLeft )
/*N*/ 		{
/*N*/ 			sal_uInt32 nNumb = ( nId << ( 32 - nBitsLeft ) ) >> 28;
/*N*/ 			if ( nNumb > 9 )
/*N*/ 				nNumb += 'A' - 10;
/*N*/ 			else
/*N*/ 				nNumb += '0';
/*N*/ 			aName+=(sal_Unicode)((sal_Char)nNumb);
/*N*/ 			nBitsLeft -= 4;
/*N*/ 		}
/*N*/ 		if( !pStg->IsContained( aName ) )
/*N*/ 			break;
/*N*/ 		nId++;
/*N*/ 	}
/*N*/ 	return aName;
/*N*/ }

/*N*/ sal_Bool SvXMLEmbeddedObjectHelper::ImplReadObject(
/*N*/ 		const OUString& rContainerStorageName, 
/*N*/ 		OUString& rObjName,
/*N*/ 	    const SvGlobalName *pClassId,
/*N*/ 	    SvStorage *pTempStor )
/*N*/ {
/*N*/ 	SvStorageRef xDocStor( mpDocPersist->GetStorage() );
/*N*/ 	SvStorageRef xCntnrStor( ImplGetContainerStorage(
/*N*/ 										rContainerStorageName ) );
/*N*/ 
/*N*/ 	if( !xCntnrStor.Is() && !pTempStor )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	String aSrcObjName( rObjName );
/*N*/ 	if( xDocStor != xCntnrStor || pTempStor )
/*N*/ 	{
/*N*/ 
/*N*/ 		// Is the object name unique?
/*N*/ 		if( mpDocPersist->GetObjectList() )
/*N*/ 		{
/*N*/ 			sal_uInt32 nCount = mpDocPersist->GetObjectList()->Count();
/*N*/ 			for( sal_uInt32 i = 0; i < nCount; i++ )
/*N*/ 			{
/*N*/ 				SvInfoObject* pTst = mpDocPersist->GetObjectList()->GetObject(i);
/*N*/ 				// TODO: unicode: is this correct?
/*N*/ 				if( rObjName.equalsIgnoreAsciiCase( pTst->GetObjName() ) ||
/*N*/ 					rObjName.equalsIgnoreAsciiCase( pTst->GetStorageName() ) )
/*N*/ 				{
/*N*/ 					rObjName = ImplGetUniqueName( xDocStor, "Obj" );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pTempStor )
/*N*/ 		{
/*N*/ 			SvStorageRef xDstStor = xDocStor->OpenOLEStorage( rObjName );
/*N*/ 			if( !pTempStor->CopyTo( xDstStor ) )
/*N*/ 				return sal_False;
/*N*/ 			xDstStor->Commit();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if( !xCntnrStor->CopyTo( aSrcObjName, xDocStor, rObjName ) )
/*?*/ 				return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SvGlobalName aClassId;
/*N*/ 	if( pClassId )
/*N*/ 	{
/*N*/ 		// If a class id is specifies, use it.
/*N*/ 		aClassId = *pClassId;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Otherwise try to get one from the storage. For packages, the
/*N*/ 		// class id is derived from the package's mime type. The mime type
/*N*/ 		// is stored in the packages manifest and the manifest is read when
/*N*/ 		// the stoage is opened. Therfor, the class id is available without
/*N*/ 		// realy accessing the storage.
/*N*/ 		SvStorageRef xObjStor;
/*N*/ 		if( xDocStor == xCntnrStor )
/*N*/ 			xObjStor = ImplGetObjectStorage( rContainerStorageName,
/*N*/ 											 rObjName, sal_False );
/*N*/ 		else
/*N*/ 			xObjStor = xDocStor->OpenStorage( rObjName, STREAM_STD_READ );
/*N*/ 		DBG_ASSERT( xObjStor.Is(), "Couldn't open object storage" );
/*N*/ 		if( xObjStor.Is() )
/*N*/ 			aClassId = xObjStor->GetClassName();
/*N*/ 	}
/*N*/ 
/*N*/ 	// For all unkown class id, the OLE object has to be wrapped by an
/*N*/ 	// outplace object.
/*N*/ 	SvGlobalName aOutClassId( SO3_OUT_CLASSID ); //STRIP003 
/*N*/ 	if( SvGlobalName() == aClassId ||
/*N*/ 		( aOutClassId != aClassId &&
/*N*/ 		  !SvFactory::IsIntern( aClassId, 0 ) ) )
/*?*/ 		aClassId = SvGlobalName( aOutClassId );
/*N*/ 
/*N*/ 	SvInfoObjectRef xInfo = new SvEmbeddedInfoObject( rObjName, aClassId );
/*N*/ 	mpDocPersist->Insert( xInfo );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
/*N*/ 		const OUString& rURLStr )
/*N*/ {
/*N*/ 	OUString sRetURL;
/*N*/ 
/*N*/ 	OUString	aContainerStorageName, aObjectStorageName;
/*N*/ 	if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
/*N*/ 							  aObjectStorageName,
/*N*/ 							  EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode ) )
/*N*/ 		return sRetURL;
/*N*/ 
/*N*/ 	if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
/*N*/ 	{
/*N*/ 		OutputStorageWrapper_Impl *pOut = 0;
/*N*/ 		SvXMLEmbeddedObjectHelper_Impl::iterator aIter;
/*N*/ 
/*N*/ 		if( mpStreamMap )
/*N*/ 		{
/*N*/ 			aIter = mpStreamMap->find( rURLStr );
/*N*/ 			if( aIter != mpStreamMap->end() && aIter->second )
/*N*/ 				pOut = aIter->second;
/*N*/ 		}
/*N*/ 
/*N*/ 		SvGlobalName aClassId, *pClassId = 0;
/*N*/ 		sal_Int32 nPos = aObjectStorageName.lastIndexOf( '!' );
/*N*/ 		if( -1 != nPos && aClassId.MakeId( aObjectStorageName.copy( nPos+1 ) ) )
/*N*/ 		{
/*?*/ 			aObjectStorageName = aObjectStorageName.copy( 0, nPos );
/*?*/ 			pClassId = &aClassId;
/*N*/ 		}
/*N*/ 		ImplReadObject( aContainerStorageName, aObjectStorageName, pClassId,
/*N*/ 					 	pOut ? pOut->GetStorage() : 0 );
/*N*/ 		sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_EMBEDDEDOBJECT_URL_BASE		) );
/*N*/ 		sRetURL += aObjectStorageName;
/*N*/ 
/*N*/ 		if( pOut )
/*N*/ 		{
/*N*/ 			mpStreamMap->erase( aIter );
/*N*/ 			pOut->release();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Objects are written using SfxObjectShell::SaveAs
/*?*/ 		sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM("#./") );
/*?*/ 		if( aContainerStorageName.getLength() )
/*?*/ 		{
/*?*/ 			sRetURL += aContainerStorageName;
/*?*/ 			sRetURL += OUString( '/' );
/*?*/ 		}
/*?*/ 		sRetURL += aObjectStorageName;
/*N*/ 	}
/*N*/ 
/*N*/ 	return sRetURL;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLEmbeddedObjectHelper::Init(
/*N*/ 		SvStorage *pRootStorage, 
/*N*/ 		SvPersist& rPersist, 
/*N*/ 		SvXMLEmbeddedObjectHelperMode eCreateMode )
/*N*/ {
/*N*/ 	mpRootStorage = pRootStorage;
/*N*/ 	mpDocPersist = &rPersist;
/*N*/ 	meCreateMode = eCreateMode;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
/*N*/ 		SvStorage& rRootStorage, 
/*N*/ 		SvPersist& rDocPersist, 
/*N*/ 		SvXMLEmbeddedObjectHelperMode eCreateMode,
/*N*/ 		sal_Bool bDirect )
/*N*/ {
/*N*/ 	SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;
/*N*/ 
/*N*/ 	pThis->acquire();
/*N*/ 	pThis->Init( &rRootStorage, rDocPersist, eCreateMode );
/*N*/ 
/*N*/ 	return pThis;
/*N*/ }

/*N*/ SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
/*N*/ 		SvPersist& rDocPersist, 
/*N*/ 		SvXMLEmbeddedObjectHelperMode eCreateMode )
/*N*/ {
/*N*/ 	SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;
/*N*/ 
/*N*/ 	pThis->acquire();
/*N*/ 	pThis->Init( 0, rDocPersist, eCreateMode );
/*N*/ 
/*N*/ 	return pThis;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLEmbeddedObjectHelper::Destroy(
/*N*/ 		SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper )
/*N*/ {
/*N*/ 	if( pSvXMLEmbeddedObjectHelper )
/*N*/ 	{
/*N*/ 		pSvXMLEmbeddedObjectHelper->dispose();
/*N*/ 		pSvXMLEmbeddedObjectHelper->release();
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLEmbeddedObjectHelper::Flush()
/*N*/ {
/*N*/ }

// XGraphicObjectResolver
/*N*/ OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL( const OUString& aURL )
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	MutexGuard			aGuard( maMutex );
/*N*/ 
/*N*/ 	return ImplInsertEmbeddedObjectURL( aURL );
/*N*/ }

// XNameAccess
/*N*/ Any SAL_CALL SvXMLEmbeddedObjectHelper::getByName(
/*N*/ 		const OUString& rURLStr )
/*N*/ 	throw (NoSuchElementException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	MutexGuard			aGuard( maMutex );
/*N*/ 	Any aRet;
/*N*/ 	if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
/*N*/ 	{
/*N*/ 		Reference < XOutputStream > xStrm;
/*N*/ 		if( mpStreamMap )
/*N*/ 		{
/*N*/ 			SvXMLEmbeddedObjectHelper_Impl::iterator aIter =
/*N*/ 				mpStreamMap->find( rURLStr );
/*N*/ 			if( aIter != mpStreamMap->end() && aIter->second )
/*?*/ 				xStrm = aIter->second;
/*N*/ 		}
/*N*/ 		if( !xStrm.is() )
/*N*/ 		{
/*N*/ 			OutputStorageWrapper_Impl *pOut = new OutputStorageWrapper_Impl;
/*N*/ 			pOut->acquire();
/*N*/ 			if( !mpStreamMap )
/*N*/ 				mpStreamMap = new SvXMLEmbeddedObjectHelper_Impl;
/*N*/ 			(*mpStreamMap)[rURLStr] = pOut;
/*N*/ 			xStrm = pOut;
/*N*/ 		}
/*N*/ 
/*N*/ 		aRet <<= xStrm;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Reference < XInputStream > xStrm;
/*N*/ 		OUString aContainerStorageName, aObjectStorageName;
/*N*/ 		if( ImplGetStorageNames( rURLStr, aContainerStorageName,
/*N*/ 								 aObjectStorageName,
/*N*/ 								 sal_True ) )
/*N*/ 		{
/*N*/ 			SvPersistRef xObj = mpDocPersist->GetObject( aObjectStorageName );
/*N*/ 			if( xObj.Is() )
/*N*/ 				xStrm = new InputStorageWrapper_Impl( xObj );
/*N*/ 		}
/*N*/ 		aRet <<= xStrm;
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }

/*?*/ Sequence< OUString > SAL_CALL SvXMLEmbeddedObjectHelper::getElementNames()
/*?*/ 	throw (RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); Sequence< OUString > aSeq; return aSeq;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*?*/ sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasByName( const OUString& rURLStr )
/*?*/ 	throw (RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

// XNameAccess
/*?*/ Type SAL_CALL SvXMLEmbeddedObjectHelper::getElementType()
/*?*/ 	throw (RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); Type aType; return aType;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }

/*?*/ sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasElements()
/*?*/ 	throw (RuntimeException)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 	MutexGuard			aGuard( maMutex );
/*?*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
