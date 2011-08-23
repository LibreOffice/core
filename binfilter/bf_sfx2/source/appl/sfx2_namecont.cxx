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



#include <rtl/uri.hxx>


#include "namecont.hxx"

#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_svtools/ehdl.hxx>
#include "bf_basic/basmgr.hxx"

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star;
using namespace cppu;
using namespace rtl;
using namespace osl;


//============================================================================
// Implementation class NameContainer_Impl

namespace SfxContainer_Impl
{

// Methods XElementAccess
/*?*/ Type NameContainer_Impl::getElementType()
/*?*/ 	throw(RuntimeException)
/*?*/ { 	
        return mType;
/*?*/ }

/*N*/ sal_Bool NameContainer_Impl::hasElements()
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	sal_Bool bRet = (mnElementCount > 0);
/*N*/ 	return bRet;
/*N*/ }

// Methods XNameAccess
/*N*/ Any NameContainer_Impl::getByName( const OUString& aName )
/*N*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
/*N*/ 	if( aIt == mHashMap.end() )
/*N*/ 	{
/*?*/ 		throw NoSuchElementException();
/*N*/ 	}
/*N*/ 	sal_Int32 iHashResult = (*aIt).second;
/*N*/ 	Any aRetAny = mValues.getConstArray()[ iHashResult ];
/*N*/ 	return aRetAny;
/*N*/ }

/*N*/ Sequence< OUString > NameContainer_Impl::getElementNames()
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	return mNames;
/*N*/ }

/*N*/ sal_Bool NameContainer_Impl::hasByName( const OUString& aName )
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
/*N*/ 	sal_Bool bRet = ( aIt != mHashMap.end() );
/*N*/ 	return bRet;
/*N*/ }


// Methods XNameReplace
/*N*/ void NameContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
/*N*/ 	throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	Type aAnyType = aElement.getValueType();
/*N*/ 	if( mType != aAnyType )
/*?*/ 		throw IllegalArgumentException();
/*N*/ 
/*N*/ 	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
/*N*/ 	if( aIt == mHashMap.end() )
/*N*/ 	{
/*?*/ 		throw NoSuchElementException();
/*N*/ 	}
/*N*/ 	sal_Int32 iHashResult = (*aIt).second;
/*N*/ 	Any aOldElement = mValues.getConstArray()[ iHashResult ];
/*N*/ 	mValues.getArray()[ iHashResult ] = aElement;
/*N*/ 
/*N*/ 
/*N*/ 	// Fire event
/*N*/ 	ContainerEvent aEvent;
/*N*/ 	aEvent.Source = mpxEventSource;
/*N*/ 	aEvent.Accessor <<= aName;
/*N*/ 	aEvent.Element = aElement;
/*N*/ 	aEvent.ReplacedElement = aOldElement;
/*N*/ 
/*N*/ 	OInterfaceIteratorHelper aIterator( maListenerContainer );
/*N*/ 	while( aIterator.hasMoreElements() )
/*N*/ 	{
/*?*/ 		Reference< XInterface > xIface = aIterator.next();
/*?*/ 		Reference< XContainerListener > xListener( xIface, UNO_QUERY );
/*?*/         try
/*?*/         {
/*?*/             xListener->elementReplaced( aEvent );
/*?*/         }
/*?*/         catch(RuntimeException&)
/*?*/         {
/*?*/             aIterator.remove();
/*?*/         }
/*N*/ 	}
/*N*/ }


// Methods XNameContainer
/*N*/ void NameContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
/*N*/ 	throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	Type aAnyType = aElement.getValueType();
/*N*/ 	if( mType != aAnyType )
/*?*/ 		throw IllegalArgumentException();
/*N*/ 
/*N*/ 	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
/*N*/ 	if( aIt != mHashMap.end() )
/*N*/ 	{
/*?*/ 		throw ElementExistException();
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_Int32 nCount = mNames.getLength();
/*N*/ 	mNames.realloc( nCount + 1 );
/*N*/ 	mValues.realloc( nCount + 1 );
/*N*/ 	mNames.getArray()[ nCount ] = aName;
/*N*/ 	mValues.getArray()[ nCount ] = aElement;
/*N*/ 
/*N*/ 	mHashMap[ aName ] = nCount;
/*N*/ 	mnElementCount++;
/*N*/ 
/*N*/ 
/*N*/ 	// Fire event
/*N*/ 	ContainerEvent aEvent;
/*N*/ 	aEvent.Source = mpxEventSource;
/*N*/ 	aEvent.Accessor <<= aName;
/*N*/ 	aEvent.Element = aElement;
/*N*/ 
/*N*/ 	OInterfaceIteratorHelper aIterator( maListenerContainer );
/*N*/ 	while( aIterator.hasMoreElements() )
/*N*/ 	{
/*N*/ 		Reference< XInterface > xIface = aIterator.next();
/*N*/ 		Reference< XContainerListener > xListener( xIface, UNO_QUERY );
/*N*/         try
/*N*/         {
/*N*/             xListener->elementInserted( aEvent );
/*N*/         }
/*N*/         catch(RuntimeException&)
/*N*/         {
/*?*/             aIterator.remove();
/*N*/         }
/*N*/ 	}
/*N*/ }

/*?*/ void NameContainer_Impl::removeByName( const OUString& Name )
/*?*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }


// Methods XContainer
/*N*/ void SAL_CALL NameContainer_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
/*N*/ 	throw (RuntimeException)
/*N*/ {
/*N*/ 	if( !xListener.is() )
/*?*/ 		throw RuntimeException();
/*N*/ 	Reference< XInterface > xIface( xListener, UNO_QUERY );
/*N*/ 	maListenerContainer.addInterface( xIface );
/*N*/ }

/*?*/ void SAL_CALL NameContainer_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
/*?*/ 	throw (RuntimeException)
/*?*/ {
        if( !xListener.is() )
        throw RuntimeException();
        Reference< XInterface > xIface( xListener, UNO_QUERY );
        maListenerContainer.removeInterface( xIface );
/*?*/ }

}	// namespace SfxContainer_Impl


//============================================================================

// Implementation class SfxLibraryContainer_Impl

// Ctor
/*N*/ SfxLibraryContainer_Impl::SfxLibraryContainer_Impl( void )
/*N*/ 	: maNameContainer( getCppuType( (Reference< XNameAccess >*) NULL ) )
/*N*/ 	, mbModified( sal_False )
/*N*/     , mbOldInfoFormat( sal_False )
/*N*/     , mpBasMgr( NULL )
/*N*/     , mbOwnBasMgr( sal_False )
/*N*/ {
/*N*/ 	mxMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	if( !mxMSF.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't get ProcessServiceFactory\n" );
/*N*/ 	}
/*N*/ 
/*N*/ 	mxSFI = Reference< XSimpleFileAccess >( mxMSF->createInstance
/*N*/ 		( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
/*N*/ 	if( !mxSFI.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't create SimpleFileAccess component\n" );
/*N*/ 	}
    mxStringSubstitution = Reference< XStringSubstitution >( mxMSF->createInstance
        ( OUString::createFromAscii( "com.sun.star.util.PathSubstitution" ) ), UNO_QUERY );
    OSL_ENSURE( mxStringSubstitution.is(), "### couln't create PathSubstitution component\n" );
/*N*/ }

/*N*/ SfxLibraryContainer_Impl::~SfxLibraryContainer_Impl()
/*N*/ {
/*N*/     if( mbOwnBasMgr )
/*?*/         BasicManager::LegacyDeleteBasicManager( mpBasMgr );
/*N*/ }


/*?*/ static void checkAndCopyFileImpl( const INetURLObject& rSourceFolderInetObj,
/*?*/ 								  const INetURLObject& rTargetFolderInetObj,
/*?*/ 								  const OUString& rCheckFileName,
/*?*/ 								  const OUString& rCheckExtension,
/*?*/ 								  Reference< XSimpleFileAccess > xSFI )
/*?*/ {
        INetURLObject aTargetFolderInetObj( rTargetFolderInetObj );
        aTargetFolderInetObj.insertName( rCheckFileName, sal_True, INetURLObject::LAST_SEGMENT,
                                     sal_True, INetURLObject::ENCODE_ALL );
        aTargetFolderInetObj.setExtension( rCheckExtension );
        OUString aTargetFile = aTargetFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
        if( !xSFI->exists( aTargetFile ) )
        {
            INetURLObject aSourceFolderInetObj( rSourceFolderInetObj );
            aSourceFolderInetObj.insertName( rCheckFileName, sal_True, INetURLObject::LAST_SEGMENT,
                                             sal_True, INetURLObject::ENCODE_ALL );
            aSourceFolderInetObj.setExtension( rCheckExtension );
            OUString aSourceFile = aSourceFolderInetObj.GetMainURL( INetURLObject::NO_DECODE );
            xSFI->copy( aSourceFile, aTargetFile );
        }
/*?*/ }

/*N*/ sal_Bool SfxLibraryContainer_Impl::init(
/*N*/     const OUString& aInitialisationParam,
/*N*/     const OUString& aInfoFileName,
/*N*/     const OUString& aOldInfoFileName,
/*N*/     const OUString& aLibElementFileExtension,
/*N*/     const OUString& aLibrariesDir,
/*N*/     SotStorageRef xStorage )
/*N*/ {
/*N*/     maInitialisationParam = aInitialisationParam;
/*N*/ 	maInfoFileName = aInfoFileName;
/*N*/ 	maOldInfoFileName = aOldInfoFileName;
/*N*/ 	maLibElementFileExtension = aLibElementFileExtension;
/*N*/ 	maLibrariesDir = aLibrariesDir;
/*N*/ 
/*N*/     meInitMode = DEFAULT;
/*N*/     INetURLObject aInitUrlInetObj( maInitialisationParam );
/*N*/     OUString aInitFileName = aInitUrlInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/     if( aInitFileName.getLength() )
/*N*/     {
/*N*/         // We need a BasicManager to avoid problems
/*?*/         StarBASIC* pBas = new StarBASIC();
/*?*/         mpBasMgr = new BasicManager( pBas );
/*?*/         mbOwnBasMgr = sal_True;
/*?*/ 
/*?*/         OUString aExtension = aInitUrlInetObj.getExtension();
/*?*/         if( aExtension.compareToAscii( "xlc" ) == COMPARE_EQUAL )
/*?*/         {
/*?*/             meInitMode = CONTAINER_INIT_FILE;
/*?*/ 	        INetURLObject aLibPathInetObj( aInitUrlInetObj );
/*?*/ 			aLibPathInetObj.removeSegment();
/*?*/ 	        maLibraryPath = aLibPathInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*?*/         }
/*?*/         else if( aExtension.compareToAscii( "xlb" ) == COMPARE_EQUAL )
/*?*/         {
/*?*/             meInitMode = LIBRARY_INIT_FILE;
/*?*/         	SotStorageRef xDummyStor;
/*?*/             ::xmlscript::LibDescriptor aLibDesc;
/*?*/             sal_Bool bReadIndexFile = implLoadLibraryIndexFile( NULL, aLibDesc, xDummyStor, aInitFileName );
/*?*/            	return bReadIndexFile;
/*?*/         }
/*?*/         else
/*?*/         {
/*?*/             // Decide between old and new document
/*?*/             sal_Bool bOldStorage = SotStorage::IsOLEStorage( aInitFileName );
/*?*/             if( bOldStorage )
/*?*/             {
/*?*/                 meInitMode = OLD_BASIC_STORAGE;
/*?*/                 importFromOldStorage( aInitFileName );
/*?*/             	return sal_True;
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 meInitMode = OFFICE_DOCUMENT;
/*?*/                 xStorage = new SotStorage( sal_True, aInitFileName );
/*?*/             }
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // Default pathes
/*N*/         maLibraryPath = SvtPathOptions().GetBasicPath();
/*N*/     }
/*N*/ 
/*N*/ 	Reference< XParser > xParser( mxMSF->createInstance(
/*N*/ 		OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
/*N*/ 	if( !xParser.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't create sax parser component\n" );
/*N*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/ 	Reference< XInputStream > xInput;
/*N*/ 
/*N*/ 	mxStorage = xStorage;
/*N*/ 	sal_Bool bStorage = mxStorage.Is();
/*N*/ 	SotStorageRef xLibrariesStor;
/*N*/ 	SotStorageStreamRef xStream;
/*N*/ 	String aFileName;
/*N*/ 
/*N*/ 	int nPassCount = 1;
/*N*/ 	if( !bStorage && meInitMode == DEFAULT )
/*N*/ 		nPassCount = 2;
/*N*/ 	for( int nPass = 0 ; nPass < nPassCount ; nPass++ )
/*N*/ 	{
/*N*/ 		if( bStorage )
/*N*/ 		{
/*N*/ 			OSL_ENSURE( meInitMode == DEFAULT || meInitMode == OFFICE_DOCUMENT,
/*N*/ 				"### Wrong InitMode for document\n" );
/*N*/ 
/*N*/ 			xLibrariesStor = xStorage->OpenSotStorage( maLibrariesDir, STREAM_READ | STREAM_NOCREATE );
/*N*/ 			if( xLibrariesStor.Is() && xLibrariesStor->GetError() == ERRCODE_NONE )
/*N*/ 			{
/*?*/ 				aFileName = maInfoFileName;
/*?*/ 				aFileName += String( RTL_CONSTASCII_USTRINGPARAM("-lc.xml") );
/*?*/ 
/*?*/ 				xStream = xLibrariesStor->OpenSotStream( aFileName, STREAM_READ | STREAM_NOCREATE );
/*?*/ 				if( xStream->GetError() != ERRCODE_NONE )
/*?*/ 				{
/*?*/ 					mbOldInfoFormat = true;
/*?*/ 
/*?*/ 					// Check old version
/*?*/ 					aFileName = maOldInfoFileName;
/*?*/ 					aFileName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );
/*?*/ 					xStream = xLibrariesStor->OpenSotStream( aFileName, STREAM_READ );
/*?*/ 
/*?*/ 					if( xStream->GetError() != ERRCODE_NONE )
/*?*/ 					{
/*?*/ 						// Check for EA2 document version with wrong extensions
/*?*/ 						aFileName = maOldInfoFileName;
/*?*/ 						aFileName += String( RTL_CONSTASCII_USTRINGPARAM(".xli") );
/*?*/ 						xStream = xLibrariesStor->OpenSotStream( aFileName, STREAM_READ );
/*?*/ 					}
/*?*/ 				}
/*N*/ 
/*N*/ 				if( xStream->GetError() == ERRCODE_NONE )
/*N*/ 				{
/*N*/ 					xInput = new ::utl::OInputStreamWrapper( *xStream );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			INetURLObject* pLibInfoInetObj = NULL;
/*N*/ 			if( meInitMode == CONTAINER_INIT_FILE )
/*N*/ 			{
/*?*/ 				aFileName = aInitFileName;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( nPass == 1 )
/*N*/ 					pLibInfoInetObj = new INetURLObject( String(maLibraryPath).GetToken(0) );
/*N*/ 				else
/*N*/ 					pLibInfoInetObj = new INetURLObject( String(maLibraryPath).GetToken(1) );
/*N*/ 				pLibInfoInetObj->insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 				pLibInfoInetObj->setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlc") ) );
/*N*/ 				aFileName = pLibInfoInetObj->GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 			}
/*N*/ 
/*N*/ 			try
/*N*/ 			{
/*N*/ 				xInput = mxSFI->openFileRead( aFileName );
/*N*/ 			}
/*N*/ 			catch( Exception& )
/*N*/ 			{
/*N*/ 				xInput.clear();
/*N*/                 if( nPass == 0 )
/*N*/                 {
//*N*/ 		            SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFileName );
//*N*/                     ULONG nErrorCode = ERRCODE_IO_GENERAL;
//*N*/                     ErrorHandler::HandleError( nErrorCode );
/*N*/                 }
/*N*/ 			}
/*N*/ 
/*N*/ 			// Old variant?
/*N*/ 			if( !xInput.is() && nPass == 0 )
/*?*/ 			{
/*?*/ 				INetURLObject aLibInfoInetObj( String(maLibraryPath).GetToken(1) );
/*?*/ 				aLibInfoInetObj.insertName( maOldInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*?*/ 				aLibInfoInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xli") ) );
/*?*/ 				aFileName = aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*?*/ 
/*?*/ 				try
/*?*/ 				{
/*?*/ 					xInput = mxSFI->openFileRead( aFileName );
/*?*/ 					mbOldInfoFormat = true;
/*?*/ 				}
/*?*/ 				catch( Exception& )
/*?*/ 				{
/*?*/     				xInput.clear();
//*?*/ 		            SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aFileName );
//*?*/                     ULONG nErrorCode = ERRCODE_IO_GENERAL;
//*?*/                     ErrorHandler::HandleError( nErrorCode );
/*?*/ 				}
/*?*/ 			}
/*N*/ 
/*N*/ 			delete pLibInfoInetObj;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( xInput.is() )
/*N*/         {
/*N*/ 		    InputSource source;
/*N*/ 		    source.aInputStream = xInput;
/*N*/ 		    source.sSystemId 	= aFileName;
/*N*/ 
/*N*/ 		    // start parsing
/*N*/ 		    ::xmlscript::LibDescriptorArray* pLibArray = new ::xmlscript::LibDescriptorArray();
/*N*/ 
/*N*/             try
/*N*/             {
/*N*/                 xParser->setDocumentHandler( ::xmlscript::importLibraryContainer( pLibArray ) );
/*N*/                 xParser->parseStream( source );
/*N*/             }
/*N*/             catch ( xml::sax::SAXException& e )
/*N*/             {
/*N*/                 OSL_ENSURE( 0, OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
/*N*/                 return sal_False;
/*N*/             }
/*N*/             catch ( io::IOException& e )
/*N*/             {
/*N*/                 OSL_ENSURE( 0, OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
/*N*/                 return sal_False;
/*N*/             }
/*N*/ 
/*N*/ 		    sal_Int32 nLibCount = pLibArray->mnLibCount;
/*N*/ 		    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
/*N*/ 		    {
/*N*/ 			    ::xmlscript::LibDescriptor& rLib = pLibArray->mpLibs[i];
/*N*/ 
/*N*/ 			    // Check storage URL
/*N*/ 			    OUString aStorageURL = rLib.aStorageURL;
/*N*/ 			    if( !bStorage && !aStorageURL.getLength() && nPass == 0 )
/*N*/ 			    {
/*N*/ 					String aLibraryPath;
/*N*/ 					if( meInitMode == CONTAINER_INIT_FILE )
/*N*/ 						aLibraryPath = maLibraryPath;
/*N*/ 					else
/*N*/ 						aLibraryPath = String(maLibraryPath).GetToken(1);
/*N*/ 					INetURLObject aInetObj( aLibraryPath );
/*N*/ 
/*N*/ 				    aInetObj.insertName( rLib.aName, sal_True, INetURLObject::LAST_SEGMENT,
/*N*/ 					    sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 				    OUString aLibDirPath = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 				    if( mxSFI->isFolder( aLibDirPath ) )
/*N*/ 				    {
/*N*/ 					    rLib.aStorageURL = aLibDirPath;
/*N*/ 					    mbModified = sal_True;
/*N*/ 				    }
/*N*/ 				    else if( rLib.bLink )
/*N*/ 				    {
/*N*/ 					    // Check "share" path
/*N*/ 					    INetURLObject aShareInetObj( String(maLibraryPath).GetToken(0) );
/*N*/ 					    aShareInetObj.insertName( rLib.aName, sal_True, INetURLObject::LAST_SEGMENT,
/*N*/ 						    sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 					    OUString aShareLibDirPath = aShareInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 					    if( mxSFI->isFolder( aShareLibDirPath ) )
/*N*/ 					    {
/*N*/ 						    rLib.aStorageURL = aShareLibDirPath;
/*N*/ 						    mbModified = sal_True;
/*N*/ 					    }
/*N*/ 				    }
/*N*/ 			    }
/*N*/ 
/*N*/ 			    OUString aLibName = rLib.aName;
/*N*/ 
/*N*/ 			    // If the same library name is used by the shared and the
/*N*/ 			    // user lib container index files the user file wins
/*N*/ 			    if( nPass == 1 && hasByName( aLibName ) )
/*N*/ 				    continue;
/*N*/ 
/*N*/ 			    SfxLibrary_Impl* pImplLib;
/*N*/ 			    if( rLib.bLink )
/*N*/ 			    {
/*N*/ 				    Reference< XNameAccess > xLib =
/*N*/ 					    createLibraryLink( aLibName, rLib.aStorageURL, rLib.bReadOnly );
/*N*/ 				    pImplLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
/*N*/ 			    }
/*N*/ 			    else
/*N*/ 			    {
/*N*/ 				    Reference< XNameContainer > xLib = createLibrary( aLibName );
/*N*/ 				    pImplLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
/*N*/ 				    pImplLib->mbLoaded = sal_False;
/*N*/ 				    pImplLib->mbReadOnly = rLib.bReadOnly;
/*N*/ 				    if( !bStorage )
/*N*/ 					    checkStorageURL( rLib.aStorageURL, pImplLib->maLibInfoFileURL, 
/*N*/                             pImplLib->maStorageURL, pImplLib->maUnexpandedStorageURL );
/*N*/ 			    }
/*N*/ 
/*N*/ 			    // Read library info files
/*N*/ 			    if( !mbOldInfoFormat )
/*N*/ 			    {
/*N*/         		    SotStorageRef xLibraryStor;
/*N*/           		    if( bStorage )
/*N*/ 				    {
/*?*/ 					    xLibraryStor = xLibrariesStor->OpenSotStorage( rLib.aName, STREAM_READ | STREAM_NOCREATE );
/*?*/ 					    if( !xLibraryStor.Is() || xLibraryStor->GetError() != ERRCODE_NONE )
/*?*/ 					    {
/*?*/ 						    OSL_ENSURE( 0, "### couln't open sub storage for library\n" );
/*?*/ 						    xLibraryStor = NULL;
/*N*/ 					    }
/*N*/ 				    }
/*N*/ 
/*N*/ 				    // Link is already initialised in createLibraryLink()
/*N*/ 				    if( !pImplLib->mbInitialised && (!bStorage || xLibraryStor.Is()) )
/*N*/ 				    {
/*N*/ 					    OUString aIndexFileName;
/*N*/ 					    sal_Bool bLoaded = implLoadLibraryIndexFile( pImplLib, rLib, xLibraryStor, aIndexFileName );
/*N*/ 					    if( bLoaded && aLibName != rLib.aName )
/*N*/ 					    {
/*N*/ 						    OSL_ENSURE( 0, "Different library names in library"
/*N*/ 							    " container and library info files!\n" );
/*N*/ 					    }
/*N*/ 				    }
/*N*/ 			    }
/*N*/ 			    else if( !bStorage )
/*N*/ 			    {
/*N*/ 				    // Write new index file immediately because otherwise
/*N*/ 				    // the library elements will be lost when storing into
/*N*/ 				    // the new info format
                        SotStorageRef xStorage;
                        implStoreLibraryIndexFile( pImplLib, rLib, xStorage );
/*N*/ 			    }
/*N*/ 
/*N*/ 			    implImportLibDescriptor( pImplLib, rLib );
/*N*/ 
/*N*/ 			    if( nPass == 1 )
/*N*/ 			    {
/*?*/ 				    pImplLib->mbSharedIndexFile = sal_True;
/*?*/ 				    pImplLib->mbReadOnly = sal_True;
/*N*/ 			    }
/*N*/ 		    }
/*N*/ 
/*N*/ 		    // Keep flag for documents to force writing the new index files
/*N*/ 		    if( !bStorage )
/*N*/ 			    mbOldInfoFormat = sal_False;
/*N*/ 
/*N*/ 		    delete pLibArray;
/*N*/         }
/*N*/ 		// Only in the first pass it's an error when no index file is found
/*N*/ 		else if( nPass == 0 )
/*N*/ 		{
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     // #56666, upgrade installation 6.0 -> 6.1
/*N*/     if( meInitMode == DEFAULT )
/*N*/     {
/*N*/         INetURLObject aUserBasicInetObj( String(maLibraryPath).GetToken(1) );
/*N*/ 		OUString aStandardStr( RTL_CONSTASCII_USTRINGPARAM("Standard") );
/*N*/ 
/*N*/ 		static char str60FolderName_1[] = "__basic_60";
/*N*/ 		static char str60FolderName_2[] = "__basic_60_2";
/*N*/         INetURLObject aUserBasic60InetObj_1( aUserBasicInetObj );
/*N*/         aUserBasic60InetObj_1.removeSegment();
/*N*/ 		INetURLObject aUserBasic60InetObj_2 = aUserBasic60InetObj_1;
/*N*/ 	    aUserBasic60InetObj_1.Append( str60FolderName_1 );
/*N*/ 	    aUserBasic60InetObj_2.Append( str60FolderName_2 );
/*N*/ 
/*N*/ 		INetURLObject aUserBasic60InetObj = aUserBasic60InetObj_1;
/*N*/         String aFolder60 = aUserBasic60InetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 		bool bSecondTime = false;
/*N*/ 	    if( mxSFI->isFolder( aFolder60 ) )
/*N*/ 		{
/*?*/ 			// #110101 Check if Standard folder exists and is complete
/*?*/ 	        INetURLObject aUserBasicStandardInetObj( aUserBasicInetObj );
/*?*/ 			aUserBasicStandardInetObj.insertName( aStandardStr, sal_True, INetURLObject::LAST_SEGMENT, 
/*?*/ 												  sal_True, INetURLObject::ENCODE_ALL );
/*?*/             INetURLObject aUserBasic60StandardInetObj( aUserBasic60InetObj );
/*?*/ 		    aUserBasic60StandardInetObj.insertName( aStandardStr, sal_True, INetURLObject::LAST_SEGMENT, 
/*?*/                                                     sal_True, INetURLObject::ENCODE_ALL );
/*?*/ 		    OUString aStandardFolder60 = aUserBasic60StandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*?*/ 		    if( mxSFI->isFolder( aStandardFolder60 ) )
/*?*/ 			{
/*?*/ 				OUString aXlbExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlb") ) );
/*?*/ 				OUString aCheckFileName;
/*?*/ 
/*?*/ 				// Check if script.xlb exists
/*?*/ 				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("script") );
/*?*/ 				checkAndCopyFileImpl( aUserBasicStandardInetObj,
/*?*/ 									  aUserBasic60StandardInetObj,
/*?*/ 									  aCheckFileName, aXlbExtension, mxSFI );
/*?*/ 
/*?*/ 				// Check if dialog.xlb exists
/*?*/ 				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("dialog") );
/*?*/ 				checkAndCopyFileImpl( aUserBasicStandardInetObj,
/*?*/ 									  aUserBasic60StandardInetObj,
/*?*/ 									  aCheckFileName, aXlbExtension, mxSFI );
/*?*/ 
/*?*/ 				// Check if module1.xba exists
/*?*/ 				OUString aXbaExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xba") ) );
/*?*/ 				aCheckFileName = OUString( RTL_CONSTASCII_USTRINGPARAM("Module1") );
/*?*/ 				checkAndCopyFileImpl( aUserBasicStandardInetObj,
/*?*/ 									  aUserBasic60StandardInetObj,
/*?*/ 									  aCheckFileName, aXbaExtension, mxSFI );
/*?*/ 			}
/*?*/ 			else
/*?*/ 		    {
/*?*/ 	            String aStandardFolder = aUserBasicStandardInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*?*/ 		        mxSFI->copy( aStandardFolder, aStandardFolder60 );
/*?*/ 			}
/*?*/ 
/*?*/ 			String aCopyToFolder60 = aUserBasic60InetObj_2.GetMainURL( INetURLObject::NO_DECODE );
/*?*/ 	        mxSFI->copy( aFolder60, aCopyToFolder60 );
/*?*/ 		}
/*N*/ 		else
/*?*/ 		{
/*?*/ 			bSecondTime = true;
/*?*/ 			aUserBasic60InetObj = aUserBasic60InetObj_2;
/*?*/ 			aFolder60 = aUserBasic60InetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 		}
/*N*/ 	    if( mxSFI->isFolder( aFolder60 ) )
/*?*/         {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*?*/         }
/*N*/     }
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

// Handle maLibInfoFileURL and maStorageURL correctly
/*N*/ void SfxLibraryContainer_Impl::checkStorageURL( const OUString& aSourceURL,
/*N*/     OUString& aLibInfoFileURL, OUString& aStorageURL, OUString& aUnexpandedStorageURL )
/*N*/ {
/*N*/     OUString aExpandedSourceURL = expand_url( aSourceURL );
/*N*/     if( aExpandedSourceURL != aSourceURL )
/*?*/         aUnexpandedStorageURL = aSourceURL;
/*N*/ 
/*N*/ 	INetURLObject aInetObj( aExpandedSourceURL );
/*N*/     OUString aExtension = aInetObj.getExtension();
/*N*/     if( aExtension.compareToAscii( "xlb" ) == COMPARE_EQUAL )
/*N*/     {
/*N*/         // URL to xlb file
/*?*/ 		aLibInfoFileURL = aExpandedSourceURL;
/*?*/         aInetObj.removeSegment();
/*?*/ 		aStorageURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // URL to library folder
/*N*/         aStorageURL = aExpandedSourceURL;
/*N*/ 		aInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 		aInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlb") ) );
/*N*/ 		aLibInfoFileURL = aInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/     }
/*N*/ }

/*N*/ SfxLibrary_Impl* SfxLibraryContainer_Impl::getImplLib( const String& rLibraryName )
/*N*/ {
/*N*/ 	Any aLibAny = maNameContainer.getByName( rLibraryName ) ;
/*N*/ 	Reference< XNameAccess > xNameAccess;
/*N*/ 	aLibAny >>= xNameAccess;
/*N*/ 	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );
/*N*/     return pImplLib;
/*N*/ }


// Storing with password encryption

// Empty implementation, avoids unneccesary implementation in dlgcont.cxx
/*?*/ sal_Bool SfxLibraryContainer_Impl::implStorePasswordLibrary( SfxLibrary_Impl* pLib,
/*?*/     const OUString& aName, SotStorageRef xStorage )
/*?*/ {
/*?*/     return sal_False;
/*?*/ }

/*?*/ sal_Bool SfxLibraryContainer_Impl::implLoadPasswordLibrary
/*?*/     ( SfxLibrary_Impl* pLib, const OUString& Name, sal_Bool bVerifyPasswordOnly )
/*?*/         throw(WrappedTargetException, RuntimeException)
/*?*/ {
/*?*/     return sal_True;
/*?*/ }


#define EXPAND_PROTOCOL "vnd.sun.star.expand"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

/*N*/ OUString SfxLibraryContainer_Impl::createAppLibraryFolder
/*N*/     ( SfxLibrary_Impl* pLib, const OUString& aName )
/*N*/ {
/*N*/ 	OUString aLibDirPath = pLib->maStorageURL;
/*N*/ 	if( !aLibDirPath.getLength() )
/*N*/     {
/*?*/ 		INetURLObject aInetObj( String(maLibraryPath).GetToken(1) );
/*?*/ 		aInetObj.insertName( aName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*?*/         checkStorageURL( aInetObj.GetMainURL( INetURLObject::NO_DECODE ), pLib->maLibInfoFileURL, 
/*?*/             pLib->maStorageURL, pLib->maUnexpandedStorageURL );
/*?*/ 		aLibDirPath = pLib->maStorageURL;
/*N*/     }
/*N*/ 
/*N*/ 	if( !mxSFI->isFolder( aLibDirPath ) )
/*N*/     {
/*N*/ 	    try
/*N*/ 	    {
/*?*/ 		    mxSFI->createFolder( aLibDirPath );
/*N*/         }
/*N*/         catch( Exception& )
/*N*/         {}
/*N*/     }
/*N*/ 
/*N*/     return aLibDirPath;
/*N*/ }

// Storing
void SfxLibraryContainer_Impl::implStoreLibrary( SfxLibrary_Impl* pLib,
    const OUString& aName, SotStorageRef xStorage )
{
    sal_Bool bLink = pLib->mbLink;
    sal_Bool bStorage = xStorage.Is() && !bLink;

    Sequence< OUString > aElementNames = pLib->getElementNames();
    sal_Int32 nNameCount = aElementNames.getLength();
    const OUString* pNames = aElementNames.getConstArray();
    OUString aLibDirPath;

    if( bStorage )
    {
        for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
        {
            OUString aElementName = pNames[ i ];

            OUString aStreamName = aElementName;
            aStreamName += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );

            Any aElement = pLib->getByName( aElementName );
            if( isLibraryElementValid( aElement ) )
            {
                SotStorageStreamRef xElementStream = xStorage->OpenSotStream
                    ( aStreamName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );

                if( xElementStream->GetError() == ERRCODE_NONE )
                {
                    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
                    OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
                    Any aAny;
                    aAny <<= aMime;
                    xElementStream->SetProperty( aPropName, aAny );

                    // #87671 Allow encryption
                    aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Encrypted") );
                    aAny <<= sal_True;
                    xElementStream->SetProperty( aPropName, aAny );

                    Reference< XOutputStream > xOutput =
                        new utl::OOutputStreamWrapper( *xElementStream );
                    writeLibraryElement( aElement, aElementName, xOutput );
                    xOutput->closeOutput();

                    xElementStream->Commit();
                }
            }
        }
    }
    else
    {
        try
        {
            // Get Output stream
            aLibDirPath = createAppLibraryFolder( pLib, aName );

            for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
            {
                OUString aElementName = pNames[ i ];

                INetURLObject aElementInetObj( aLibDirPath );
                aElementInetObj.insertName( aElementName, sal_False,
                    INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
                aElementInetObj.setExtension( maLibElementFileExtension );
                String aElementPath( aElementInetObj.GetMainURL( INetURLObject::NO_DECODE ) );

                Any aElement = pLib->getByName( aElementName );
                if( isLibraryElementValid( aElement ) )
                {
                    // TODO: Check modified
                    try
                    {
                        if( mxSFI->exists( aElementPath ) )
                            mxSFI->kill( aElementPath );
                        Reference< XOutputStream > xOutput = mxSFI->openFileWrite( aElementPath );
                        writeLibraryElement( aElement, aElementName, xOutput );
                        xOutput->closeOutput();
                    }
                    catch( Exception& )
                    {
                        SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aElementPath );
                        ULONG nErrorCode = ERRCODE_IO_GENERAL;
                        ErrorHandler::HandleError( nErrorCode );
                    }
                }
            }
        }
        catch( Exception& )
        //catch( Exception& e )
        {
            // TODO
            //throw e;
        }
    }
}

void SfxLibraryContainer_Impl::implStoreLibraryIndexFile( SfxLibrary_Impl* pLib,
    const ::xmlscript::LibDescriptor& rLib, SotStorageRef xStorage )
{
    // Create sax writer
    Reference< XExtendedDocumentHandler > xHandler(
        mxMSF->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
    if( !xHandler.is() )
    {
        OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
        return;
    }

    sal_Bool bLink = pLib->mbLink;
    sal_Bool bStorage = xStorage.Is() && !bLink;

    // Write info file
    Reference< XOutputStream > xOut;
    SotStorageStreamRef xInfoStream;
    if( bStorage )
    {
        OUString aStreamName( maInfoFileName );
        aStreamName += String( RTL_CONSTASCII_USTRINGPARAM("-lb.xml") );

        xInfoStream = xStorage->OpenSotStream( aStreamName,
            STREAM_WRITE | STREAM_SHARE_DENYWRITE );

        if( xInfoStream->GetError() == ERRCODE_NONE )
        {
            String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
            OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
            Any aAny;
            aAny <<= aMime;
            xInfoStream->SetProperty( aPropName, aAny );

            // #87671 Allow encryption
            aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Encrypted") );
            aAny <<= sal_True;
            xInfoStream->SetProperty( aPropName, aAny );

            xOut = new utl::OOutputStreamWrapper( *xInfoStream );
        }
    }
    else
    {
        // Create Output stream
        createAppLibraryFolder( pLib, rLib.aName );
        String aLibInfoPath = pLib->maLibInfoFileURL;

        try
        {
            if( mxSFI->exists( aLibInfoPath ) )
                mxSFI->kill( aLibInfoPath );
            xOut = mxSFI->openFileWrite( aLibInfoPath );
        }
        catch( Exception& )
        {
            SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
            ULONG nErrorCode = ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError( nErrorCode );
        }

    }
    if( !xOut.is() )
    {
        OSL_ENSURE( 0, "### couln't open output stream\n" );
        return;
    }

    Reference< XActiveDataSource > xSource( xHandler, UNO_QUERY );
    xSource->setOutputStream( xOut );

    xmlscript::exportLibrary( xHandler, rLib );

    if( xInfoStream.Is() )
        xInfoStream->Commit();
}


/*N*/ sal_Bool SfxLibraryContainer_Impl::implLoadLibraryIndexFile(  SfxLibrary_Impl* pLib,
/*N*/     ::xmlscript::LibDescriptor& rLib, SotStorageRef xStorage, const OUString& aIndexFileName )
/*N*/ {
/*N*/ 	Reference< XParser > xParser( mxMSF->createInstance(
/*N*/ 		OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
/*N*/ 	if( !xParser.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't create sax parser component\n" );
/*N*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_Bool bLink = sal_False;
/*N*/ 	sal_Bool bStorage = sal_False;
/*N*/     if( pLib )
/*N*/     {
/*N*/ 	    bLink = pLib->mbLink;
/*N*/ 	    bStorage = xStorage.Is() && !bLink;
/*N*/     }
/*N*/ 
/*N*/ 	// Read info file
/*N*/ 	Reference< XInputStream > xInput;
/*N*/ 	SotStorageStreamRef xInfoStream;
/*N*/ 	String aLibInfoPath;
/*N*/ 	if( bStorage )
/*N*/ 	{
/*?*/ 		aLibInfoPath = maInfoFileName;
/*?*/ 		aLibInfoPath += String( RTL_CONSTASCII_USTRINGPARAM("-lb.xml") );
/*?*/ 
/*?*/ 		xInfoStream = xStorage->OpenSotStream( aLibInfoPath, STREAM_READ );
/*?*/ 		if( xInfoStream->GetError() == ERRCODE_NONE )
/*?*/ 		{
/*?*/ 			xInput = new ::utl::OInputStreamWrapper( *xInfoStream );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Create Input stream
/*N*/         String aLibInfoPath;
/*N*/ 
/*N*/         if( pLib )
/*N*/         {
/*N*/             createAppLibraryFolder( pLib, rLib.aName );
/*N*/             aLibInfoPath = pLib->maLibInfoFileURL;
/*N*/         }
/*N*/         else
/*?*/             aLibInfoPath = aIndexFileName;
/*N*/ 
/*N*/ 		try
/*N*/ 		{
/*N*/ 			xInput = mxSFI->openFileRead( aLibInfoPath );
/*N*/ 		}
/*N*/ 		catch( Exception& )
/*N*/ 		{
/*N*/             xInput.clear();
//*N*/ 		    SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
//*N*/             ULONG nErrorCode = ERRCODE_IO_GENERAL;
//*N*/             ErrorHandler::HandleError( nErrorCode );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !xInput.is() )
/*N*/ 	{
/*N*/ 		// OSL_ENSURE( 0, "### couln't open input stream\n" );
/*N*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	InputSource source;
/*N*/ 	source.aInputStream = xInput;
/*N*/ 	source.sSystemId 	= aLibInfoPath;
/*N*/ 
/*N*/ 	// start parsing
/*N*/ 	try {
/*N*/ 		xParser->setDocumentHandler( ::xmlscript::importLibrary( rLib ) );
/*N*/ 		xParser->parseStream( source );
/*N*/ 	}
/*N*/ 	catch( Exception& e )
/*N*/ 	{
/*N*/ 		// throw WrappedTargetException( OUString::createFromAscii( "parsing error!\n" ),
/*N*/ 		//								Reference< XInterface >(),
/*N*/ 		//								makeAny( e ) );
/*N*/ 		OSL_ENSURE( 0, "Parsing error\n" );
//*N*/ 		SfxErrorContext aEc( ERRCTX_SFX_LOADBASIC, aLibInfoPath );
//*N*/         ULONG nErrorCode = ERRCODE_IO_GENERAL;
//*N*/         ErrorHandler::HandleError( nErrorCode );
/*N*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/     if( !pLib )
/*N*/     {
/*?*/ 		Reference< XNameContainer > xLib = createLibrary( rLib.aName );
/*?*/ 		pLib = static_cast< SfxLibrary_Impl* >( xLib.get() );
/*?*/ 		pLib->mbLoaded = sal_False;
/*?*/         rLib.aStorageURL = aIndexFileName;
/*?*/         checkStorageURL( rLib.aStorageURL, pLib->maLibInfoFileURL, pLib->maStorageURL, 
/*?*/             pLib->maUnexpandedStorageURL );
/*?*/ 
/*?*/         implImportLibDescriptor( pLib, rLib );
/*N*/     }
/*N*/ 
/*N*/     return sal_True;
/*N*/ }

/*N*/ void SfxLibraryContainer_Impl::implImportLibDescriptor
/*N*/     ( SfxLibrary_Impl* pLib, ::xmlscript::LibDescriptor& rLib )
/*N*/ {
/*N*/     if( !pLib->mbInitialised )
/*N*/     {
/*N*/ 	    sal_Int32 nElementCount = rLib.aElementNames.getLength();
/*N*/ 	    const OUString* pElementNames = rLib.aElementNames.getConstArray();
/*N*/ 	    Any aDummyElement = createEmptyLibraryElement();
/*N*/ 	    for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
/*N*/ 	    {
/*N*/ 		    pLib->maNameContainer.insertByName( pElementNames[i], aDummyElement );
/*N*/ 	    }
/*N*/         pLib->mbPasswordProtected = rLib.bPasswordProtected;
/*N*/         pLib->mbReadOnly = rLib.bReadOnly;
/*N*/         pLib->mbModified = sal_False;
/*N*/ 
/*N*/         pLib->mbInitialised = sal_True;
/*N*/     }
/*N*/ }


/*N*/ void SfxLibraryContainer_Impl::storeLibraries( sal_Bool bComplete )
/*N*/ {
/*N*/ 	SotStorageRef xStorage;
/*N*/ 	storeLibraries_Impl( xStorage, bComplete );
/*N*/ }

/*N*/ void SfxLibraryContainer_Impl::storeLibrariesToStorage( SotStorageRef xStorage )
/*N*/ {
/*N*/ 	sal_Bool bComplete = sal_True;
/*N*/ 	storeLibraries_Impl( xStorage, bComplete );
/*N*/ }

// Methods of new XLibraryStorage interface?
/*N*/ void SfxLibraryContainer_Impl::storeLibraries_Impl( SotStorageRef xStorage, sal_Bool bComplete )
/*N*/ {
/*N*/ 	Sequence< OUString > aNames = maNameContainer.getElementNames();
/*N*/ 	const OUString* pNames = aNames.getConstArray();
/*N*/ 	sal_Int32 i, nNameCount = aNames.getLength();
/*N*/ 
/*N*/ 	// Don't count libs from shared index file
/*N*/ 	sal_Int32 nLibsToSave = nNameCount;
/*N*/ 	for( i = 0 ; i < nNameCount ; i++ )
/*N*/ 	{
/*N*/         SfxLibrary_Impl* pImplLib = getImplLib( pNames[ i ] );
/*N*/ 		if( pImplLib->mbSharedIndexFile )
/*N*/ 			nLibsToSave--;
/*N*/ 	}
/*N*/     if( !nLibsToSave )
/*N*/         return;
/*N*/ 
/*N*/ 	::xmlscript::LibDescriptorArray* pLibArray = new ::xmlscript::LibDescriptorArray( nLibsToSave );
/*N*/ 
/*N*/ 	// Write to storage?
/*N*/ 	sal_Bool bStorage = xStorage.Is();
/*N*/ 	SotStorageRef xLibrariesStor;
/*N*/ 	SotStorageRef xSourceLibrariesStor;
/*N*/ 	if( bStorage )
/*N*/ 	{
/*N*/         // Don't write if only empty standard lib exists
/*N*/         if( nNameCount == 1 )
/*N*/         {
/*N*/             // Must be standard lib
/*N*/ 		    Any aLibAny = maNameContainer.getByName( pNames[0] );
/*N*/ 		    Reference< XNameAccess > xNameAccess;
/*N*/ 		    aLibAny >>= xNameAccess;
/*N*/             if( !xNameAccess->hasElements() )
/*N*/                 return;
/*N*/         }
/*N*/ 
/*?*/ 		xLibrariesStor = xStorage->OpenUCBStorage( maLibrariesDir, STREAM_WRITE );
/*?*/ 		if( !xLibrariesStor.Is() || xLibrariesStor->GetError() != ERRCODE_NONE )
/*?*/ 		{
/*?*/ 			OSL_ENSURE( 0, "### couln't create libraries sub storage\n" );
/*?*/ 			return;
/*?*/ 		}
/*?*/ 
/*?*/         xSourceLibrariesStor = mxStorage->OpenSotStorage( maLibrariesDir, STREAM_READ | STREAM_NOCREATE );
/*?*/ 		if( !xSourceLibrariesStor.Is() || xSourceLibrariesStor->GetError() != ERRCODE_NONE )
/*?*/             xSourceLibrariesStor = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	int iArray = 0;
/*N*/ 	for( i = 0 ; i < nNameCount ; i++ )
/*N*/ 	{
/*N*/         SfxLibrary_Impl* pImplLib = getImplLib( pNames[ i ] );
/*N*/ 		if( pImplLib->mbSharedIndexFile )
/*N*/ 			continue;
/*N*/ 		::xmlscript::LibDescriptor& rLib = pLibArray->mpLibs[iArray];
/*N*/ 		rLib.aName = pNames[ i ];
/*N*/ 		iArray++;
/*N*/ 
/*N*/ 		rLib.bLink = pImplLib->mbLink;
/*N*/ 		rLib.aStorageURL = ( pImplLib->maUnexpandedStorageURL.getLength() ) ?
/*N*/             pImplLib->maUnexpandedStorageURL : pImplLib->maLibInfoFileURL;
/*N*/ 		rLib.bReadOnly = pImplLib->mbReadOnly;
/*N*/ 		rLib.bPasswordProtected = pImplLib->mbPasswordProtected;
/*N*/ 		rLib.aElementNames = pImplLib->getElementNames();
/*N*/ 
/*N*/ 		if( pImplLib->mbModified || bComplete )
/*N*/ 		{
/*N*/             // Can we copy the storage?
            if( !mbOldInfoFormat && !pImplLib->mbModified && xSourceLibrariesStor.Is() )
            {
                BOOL bRet = xSourceLibrariesStor->CopyTo
                    ( rLib.aName, xLibrariesStor, rLib.aName );
            }
            else
            {
                SotStorageRef xLibraryStor;
                if( bStorage )
                {
                    xLibraryStor = xLibrariesStor->OpenUCBStorage( rLib.aName, STREAM_WRITE );
                    if( !xLibraryStor.Is() || xLibraryStor->GetError() != ERRCODE_NONE )
                    {
                        OSL_ENSURE( 0, "### couln't create sub storage for library\n" );
                        return;
                    }
                }

                // Maybe lib is not loaded?!
                if( bComplete )
                    loadLibrary( rLib.aName );

                if( pImplLib->mbPasswordProtected )
                    implStorePasswordLibrary( pImplLib, rLib.aName, xLibraryStor );
                    // TODO: Check return value
                else
                    implStoreLibrary( pImplLib, rLib.aName, xLibraryStor );

                implStoreLibraryIndexFile( pImplLib, rLib, xLibraryStor );
                if( bStorage )
                {
                    xLibraryStor->Commit();
                }
            }
/*?*/ 
/*?*/ 			mbModified = sal_True;
/*?*/ 			pImplLib->mbModified = sal_False;
/*N*/ 		}
/*N*/ 
/*N*/         // For container info ReadOnly refers to mbReadOnlyLink
/*N*/ 		rLib.bReadOnly = pImplLib->mbReadOnlyLink;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !mbOldInfoFormat && !mbModified )
/*N*/ 		return;
/*N*/ 	mbModified = sal_False;
/*N*/     mbOldInfoFormat = sal_False;
/*N*/ 
/*N*/ 	// Write library container info
/*N*/ 	// Create sax writer
/*N*/ 	Reference< XExtendedDocumentHandler > xHandler(
/*N*/ 		mxMSF->createInstance(
/*N*/ 			OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
/*N*/ 	if( !xHandler.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Write info file
/*N*/ 	Reference< XOutputStream > xOut;
/*N*/ 	SotStorageStreamRef xInfoStream;
/*N*/ 	if( bStorage )
/*N*/ 	{
/*?*/ 		OUString aStreamName( maInfoFileName );
/*?*/ 		aStreamName += String( RTL_CONSTASCII_USTRINGPARAM("-lc.xml") );
/*?*/ 
/*?*/ 		xInfoStream = xLibrariesStor->OpenSotStream( aStreamName,
/*?*/ 			STREAM_WRITE | STREAM_SHARE_DENYWRITE );
/*?*/ 
/*?*/ 		if( xInfoStream->GetError() == ERRCODE_NONE )
/*?*/ 		{
/*?*/ 			String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
/*?*/ 			OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
/*?*/ 			Any aAny;
/*?*/ 			aAny <<= aMime;
/*?*/ 			xInfoStream->SetProperty( aPropName, aAny );
/*?*/ 
/*?*/             // #87671 Allow encryption
/*?*/ 			aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Encrypted") );
/*?*/ 			aAny <<= sal_True;
/*?*/ 			xInfoStream->SetProperty( aPropName, aAny );
/*?*/ 
/*?*/ 			xOut = new ::utl::OOutputStreamWrapper( *xInfoStream );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Create Output stream
/*N*/ 		INetURLObject aLibInfoInetObj( String(maLibraryPath).GetToken(1) );
/*N*/ 		aLibInfoInetObj.insertName( maInfoFileName, sal_True, INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 		aLibInfoInetObj.setExtension( OUString( RTL_CONSTASCII_USTRINGPARAM("xlc") ) );
/*N*/ 		String aLibInfoPath( aLibInfoInetObj.GetMainURL( INetURLObject::NO_DECODE ) );
/*N*/ 
/*N*/ 		try
/*N*/ 		{
/*N*/ 		    if( mxSFI->exists( aLibInfoPath ) )
/*N*/ 			    mxSFI->kill( aLibInfoPath );
/*N*/ 		    xOut = mxSFI->openFileWrite( aLibInfoPath );
/*N*/         }
/*N*/         catch( Exception& )
/*N*/         {
/*?*/             xOut.clear();
//*?*/ 			SfxErrorContext aEc( ERRCTX_SFX_SAVEDOC, aLibInfoPath );
//*?*/             ULONG nErrorCode = ERRCODE_IO_GENERAL;
//*?*/             ErrorHandler::HandleError( nErrorCode );
/*N*/         }
/*N*/ 
/*N*/ 	}
/*N*/ 	if( !xOut.is() )
/*N*/ 	{
/*N*/ 		OSL_ENSURE( 0, "### couln't open output stream\n" );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	Reference< XActiveDataSource > xSource( xHandler, UNO_QUERY );
/*N*/ 	xSource->setOutputStream( xOut );
/*N*/ 
/*N*/ 	xmlscript::exportLibraryContainer( xHandler, pLibArray );
/*N*/ 	if( xInfoStream.Is() )
/*?*/ 		xInfoStream->Commit();
/*N*/ 	if( xLibrariesStor.Is() )
/*?*/ 		xLibrariesStor->Commit();
/*N*/ 
/*N*/ 	delete pLibArray;
/*N*/ }


// Methods XElementAccess
/*?*/ Type SfxLibraryContainer_Impl::getElementType()
/*?*/ 	throw(RuntimeException)
/*?*/ { 
        return maNameContainer.getElementType();
/*?*/ }

/*?*/ sal_Bool SfxLibraryContainer_Impl::hasElements()
/*?*/ 	throw(RuntimeException)
/*?*/ {
        sal_Bool bRet = maNameContainer.hasElements();
        return bRet;
/*?*/ }

// Methods XNameAccess
/*N*/ Any SfxLibraryContainer_Impl::getByName( const OUString& aName )
/*N*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	Any aRetAny = maNameContainer.getByName( aName ) ;
/*N*/ 	return aRetAny;
/*N*/ }

/*N*/ Sequence< OUString > SfxLibraryContainer_Impl::getElementNames()
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	return maNameContainer.getElementNames();
/*N*/ }

/*N*/ sal_Bool SfxLibraryContainer_Impl::hasByName( const OUString& aName )
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	sal_Bool bRet = maNameContainer.hasByName( aName ) ;
/*N*/ 	return bRet;
/*N*/ }

// Methods XLibraryContainer
/*N*/ Reference< XNameContainer > SAL_CALL SfxLibraryContainer_Impl::createLibrary( const OUString& Name )
/*N*/ 		throw(IllegalArgumentException, ElementExistException, RuntimeException)
/*N*/ {
/*N*/ 	SfxLibrary_Impl* pNewLib = implCreateLibrary();
/*N*/     pNewLib->maLibElementFileExtension = maLibElementFileExtension;
/*N*/ 	Reference< XNameAccess > xNameAccess = static_cast< XNameAccess* >( pNewLib );
/*N*/ 	Any aElement;
/*N*/ 	aElement <<= xNameAccess;
/*N*/ 	maNameContainer.insertByName( Name, aElement );
/*N*/ 	mbModified = sal_True;
/*N*/     Reference< XNameContainer > xRet( xNameAccess, UNO_QUERY );
/*N*/ 	return xRet;
/*N*/ }

/*N*/ Reference< XNameAccess > SAL_CALL SfxLibraryContainer_Impl::createLibraryLink
/*N*/ 	( const OUString& Name, const OUString& StorageURL, sal_Bool ReadOnly )
/*N*/ 		throw(IllegalArgumentException, ElementExistException, RuntimeException)
/*N*/ {
/*N*/ 	// TODO: Check other reasons to force ReadOnly status
/*N*/ 	//if( !ReadOnly )
/*N*/ 	//{
/*N*/ 	//}
/*N*/ 
/*N*/     OUString aLibInfoFileURL;
/*N*/     OUString aLibDirURL;
/*N*/     OUString aUnexpandedStorageURL;
/*N*/     checkStorageURL( StorageURL, aLibInfoFileURL, aLibDirURL, aUnexpandedStorageURL );
/*N*/ 
/*N*/ 
/*N*/ 	SfxLibrary_Impl* pNewLib = implCreateLibraryLink( aLibInfoFileURL, aLibDirURL, ReadOnly );
/*N*/     pNewLib->maLibElementFileExtension = maLibElementFileExtension;
/*N*/     pNewLib->maUnexpandedStorageURL = aUnexpandedStorageURL;
/*N*/ 
/*N*/     OUString aInitFileName;
/*N*/     SotStorageRef xDummyStor;
/*N*/     ::xmlscript::LibDescriptor aLibDesc;
/*N*/     sal_Bool bReadIndexFile = implLoadLibraryIndexFile( pNewLib, aLibDesc, xDummyStor, aInitFileName );
/*N*/     implImportLibDescriptor( pNewLib, aLibDesc );
/*N*/ 
/*N*/ 	Reference< XNameAccess > xRet = static_cast< XNameAccess* >( pNewLib );
/*N*/ 	Any aElement;
/*N*/ 	aElement <<= xRet;
/*N*/ 	maNameContainer.insertByName( Name, aElement );
/*N*/ 	mbModified = sal_True;
/*N*/ 	return xRet;
/*N*/ }

/*?*/ void SAL_CALL SfxLibraryContainer_Impl::removeLibrary( const OUString& Name )
/*?*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

/*N*/ sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryLoaded( const OUString& Name )
/*N*/ 	throw(NoSuchElementException, RuntimeException)
/*N*/ {
/*N*/     SfxLibrary_Impl* pImplLib = getImplLib( Name );
/*N*/ 	sal_Bool bRet = pImplLib->mbLoaded;
/*N*/ 	return bRet;
/*N*/ }


/*N*/ void SAL_CALL SfxLibraryContainer_Impl::loadLibrary( const OUString& Name )
/*N*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	Any aLibAny = maNameContainer.getByName( Name ) ;
/*N*/ 	Reference< XNameAccess > xNameAccess;
/*N*/ 	aLibAny >>= xNameAccess;
/*N*/ 	SfxLibrary_Impl* pImplLib = static_cast< SfxLibrary_Impl* >( xNameAccess.get() );
/*N*/ 
/*N*/     sal_Bool bLoaded = pImplLib->mbLoaded;
/*N*/ 	pImplLib->mbLoaded = sal_True;
/*N*/ 	if( !bLoaded && xNameAccess->hasElements() )
/*N*/ 	{
/*N*/         if( pImplLib->mbPasswordProtected )
/*N*/         {
/*N*/             implLoadPasswordLibrary( pImplLib, Name );
/*N*/             return;
/*N*/         }
/*N*/ 
/*N*/ 		sal_Bool bLink = pImplLib->mbLink;
/*N*/ 		sal_Bool bStorage = mxStorage.Is() && !bLink;
/*N*/ 
/*N*/ 		SotStorageRef xLibrariesStor;
/*N*/ 		SotStorageRef xLibraryStor;
/*N*/ 		SotStorageStreamRef xElementStream;
/*N*/ 		if( bStorage )
/*N*/ 		{
/*?*/             xLibrariesStor = mxStorage->OpenSotStorage( maLibrariesDir, STREAM_READ | STREAM_NOCREATE );
/*?*/ 			if( xLibrariesStor.Is() && xLibrariesStor->GetError() == ERRCODE_NONE )
/*?*/ 			{
/*?*/                 xLibraryStor = xLibrariesStor->OpenSotStorage( Name, STREAM_READ | STREAM_NOCREATE );
/*?*/ 			}
/*?*/ 			if( !xLibraryStor.Is() || xLibraryStor->GetError() != ERRCODE_NONE )
/*?*/ 			{
/*?*/ 				OSL_ENSURE( 0, "### couln't open sub storage for library\n" );
/*?*/ 				return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		Sequence< OUString > aNames = pImplLib->getElementNames();
/*N*/ 		sal_Int32 nNameCount = aNames.getLength();
/*N*/ 		const OUString* pNames = aNames.getConstArray();
/*N*/ 		for( sal_Int32 i = 0 ; i < nNameCount ; i++ )
/*N*/ 		{
/*N*/ 			OUString aElementName = pNames[ i ];
/*N*/ 
/*N*/ 			OUString aFile;
/*N*/ 			if( bStorage )
/*N*/ 			{
/*?*/ 				aFile = aElementName;
/*?*/ 				aFile += String( RTL_CONSTASCII_USTRINGPARAM(".xml") );
/*?*/ 
/*?*/ 				xElementStream = xLibraryStor->OpenSotStream( aFile, STREAM_READ );
/*?*/ 				if( xElementStream->GetError() != ERRCODE_NONE )
/*?*/ 				{
/*?*/ 					// Check for EA2 document version with wrong extensions
/*?*/ 					aFile = aElementName;
/*?*/ 					aFile += String( RTL_CONSTASCII_USTRINGPARAM(".") );
/*?*/ 					aFile += maLibElementFileExtension;
/*?*/ 					xElementStream = xLibraryStor->OpenSotStream( aFile, STREAM_READ );
/*?*/ 				}
/*?*/ 
/*?*/ 				if( !xElementStream.Is() || xElementStream->GetError() != ERRCODE_NONE )
/*?*/ 				{
/*?*/ 					OSL_ENSURE( 0, "### couln't open library element stream\n" );
/*?*/ 					return;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 		        String aLibDirPath = pImplLib->maStorageURL;
/*N*/ 				INetURLObject aElementInetObj( aLibDirPath );
/*N*/ 				aElementInetObj.insertName( aElementName, sal_False,
/*N*/ 					INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::ENCODE_ALL );
/*N*/ 				aElementInetObj.setExtension( maLibElementFileExtension );
/*N*/ 				aFile = aElementInetObj.GetMainURL( INetURLObject::NO_DECODE );
/*N*/ 			}
/*N*/ 
/*N*/ 			Any aAny = importLibraryElement( aFile, xElementStream );
/*N*/ 			if( pImplLib->hasByName( aElementName ) )
/*N*/             {
/*N*/                 if( aAny.hasValue() )
/*N*/ 				    pImplLib->maNameContainer.replaceByName( aElementName, aAny );
/*N*/             }
/*N*/ 			else
/*N*/             {
/*?*/ 				pImplLib->maNameContainer.insertByName( aElementName, aAny );
/*N*/             }
/*N*/ 		}
/*N*/ 
/*N*/         pImplLib->mbModified = sal_False;
/*N*/ 	}
/*N*/ }

// Methods XLibraryContainer2
sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryLink( const OUString& Name )
    throw (NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
    sal_Bool bRet = pImplLib->mbLink;
    return bRet;
}

/*?*/ OUString SAL_CALL SfxLibraryContainer_Impl::getLibraryLinkURL( const OUString& Name )
/*?*/     throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); OUString aRetStr;return aRetStr;//STRIP001 
/*?*/ }

sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryReadOnly( const OUString& Name )
    throw (NoSuchElementException, RuntimeException)
{
    SfxLibrary_Impl* pImplLib = getImplLib( Name );
    sal_Bool bRet = pImplLib->mbReadOnly || (pImplLib->mbLink && pImplLib->mbReadOnlyLink);
    return bRet;
}

/*?*/ void SAL_CALL SfxLibraryContainer_Impl::setLibraryReadOnly( const OUString& Name, sal_Bool bReadOnly )
/*?*/     throw (NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*?*/ }

/*?*/ void SAL_CALL SfxLibraryContainer_Impl::renameLibrary( const OUString& Name, const OUString& NewName )
/*?*/     throw (NoSuchElementException, ElementExistException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*?*/ }


// Methods XLibraryContainerPassword
/*?*/ sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryPasswordProtected( const OUString& Name )
/*?*/     throw (NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*?*/ }

/*?*/ sal_Bool SAL_CALL SfxLibraryContainer_Impl::isLibraryPasswordVerified( const OUString& Name )
/*?*/     throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;
/*?*/ }

/*?*/ sal_Bool SAL_CALL SfxLibraryContainer_Impl::verifyLibraryPassword
/*?*/     ( const OUString& Name, const OUString& Password )
/*?*/         throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;
/*?*/ }

/*?*/ void SAL_CALL SfxLibraryContainer_Impl::changeLibraryPassword( const OUString& Name,
/*?*/     const OUString& OldPassword, const OUString& NewPassword )
/*?*/         throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP");
/*?*/ }

// Methods XContainer
/*N*/ void SAL_CALL SfxLibraryContainer_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
/*N*/ 	throw (RuntimeException)
/*N*/ {
/*N*/ 	maNameContainer.setEventSource( static_cast< XInterface* >( (OWeakObject*)this ) );
/*N*/ 	maNameContainer.addContainerListener( xListener );
/*N*/ }

/*N*/ void SAL_CALL SfxLibraryContainer_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
/*N*/ 	throw (RuntimeException)
/*N*/ {
/*N*/ 	maNameContainer.removeContainerListener( xListener );
/*N*/ }


/*N*/ OUString SfxLibraryContainer_Impl::expand_url( const OUString& url )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/     if (0 == url.compareToAscii( RTL_CONSTASCII_STRINGPARAM(EXPAND_PROTOCOL ":") ))
/*?*/     {
/*?*/         if( !mxMacroExpander.is() )
/*?*/         {
/*?*/             Reference< XPropertySet > xProps( mxMSF, UNO_QUERY );
/*?*/             OSL_ASSERT( xProps.is() );
/*?*/             if( xProps.is() )
/*?*/             {
/*?*/                 Reference< XComponentContext > xContext;
/*?*/                 xProps->getPropertyValue(
/*?*/                     OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
/*?*/                 OSL_ASSERT( xContext.is() );
/*?*/                 if( xContext.is() )
/*?*/                 {
/*?*/                     Reference< util::XMacroExpander > xExpander;
/*?*/                     xContext->getValueByName(
/*?*/                         OUSTR("/singletons/com.sun.star.util.theMacroExpander") ) >>= xExpander;
/*?*/                     if(! xExpander.is())
/*?*/                     {
/*?*/                         throw DeploymentException(
/*?*/                             OUSTR("no macro expander singleton available!"), Reference< XInterface >() );
/*?*/                     }
/*?*/                     MutexGuard guard( Mutex::getGlobalMutex() );
/*?*/                     if( !mxMacroExpander.is() )
/*?*/                     {
/*?*/                         mxMacroExpander = xExpander;
/*?*/                     }
/*?*/                 }
/*?*/             }
/*?*/         }
/*?*/ 
/*?*/         if( !mxMacroExpander.is() )
/*?*/             return url;
/*?*/ 
/*?*/         // cut protocol
/*?*/         OUString macro( url.copy( sizeof (EXPAND_PROTOCOL ":") -1 ) );
/*?*/         // decode uric class chars
/*?*/         macro = Uri::decode( macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
/*?*/         // expand macro string
/*?*/         OUString ret( mxMacroExpander->expandMacros( macro ) );
/*?*/         return ret;
/*?*/     }
    else if( mxStringSubstitution.is() )
    {
        return mxStringSubstitution->substituteVariables( url, false );
    }
/*N*/     else
/*N*/     {
/*N*/         return url;
/*N*/     }
/*N*/ }


//============================================================================

// Implementation class SfxLibrary_Impl

// Ctor
/*N*/ SfxLibrary_Impl::SfxLibrary_Impl( Type aType,
/*N*/                                   Reference< XMultiServiceFactory > xMSF,
/*N*/                                   Reference< XSimpleFileAccess > xSFI )
/*N*/ 	: OComponentHelper( m_mutex )
/*N*/     , mxMSF( xMSF )
/*N*/     , mxSFI( xSFI )
/*N*/ 	, maNameContainer( aType )
/*N*/ 	, mbLoaded( sal_True )
/*N*/ 	, mbModified( sal_True )
/*N*/     , mbInitialised( sal_False )
/*N*/     , mbLink( sal_False )
/*N*/ 	, mbReadOnly( sal_False )
/*N*/     , mbReadOnlyLink( sal_False )
/*N*/ 	, mbPasswordProtected( sal_False )
/*N*/ 	, mbPasswordVerified( sal_False )
/*N*/     , mbDoc50Password( sal_False )
/*N*/ 	, mbSharedIndexFile( sal_False )
/*N*/ {
/*N*/ }

/*N*/ SfxLibrary_Impl::SfxLibrary_Impl( Type aType,
/*N*/                                   Reference< XMultiServiceFactory > xMSF,
/*N*/                                   Reference< XSimpleFileAccess > xSFI ,
/*N*/                                   const OUString& aLibInfoFileURL,
/*N*/                                   const OUString& aStorageURL, sal_Bool ReadOnly )
/*N*/ 	: OComponentHelper( m_mutex )
/*N*/     , mxMSF( xMSF )
/*N*/     , mxSFI( xSFI )
/*N*/ 	, maNameContainer( aType )
/*N*/ 	, mbLoaded( sal_False )
/*N*/ 	, mbModified( sal_True )
/*N*/     , mbInitialised( sal_False )
/*N*/     , maLibInfoFileURL( aLibInfoFileURL )
/*N*/ 	, maStorageURL( aStorageURL )
/*N*/     , mbLink( sal_True )
/*N*/ 	, mbReadOnly( sal_False )
/*N*/     , mbReadOnlyLink( ReadOnly )
/*N*/ 	, mbPasswordProtected( sal_False )
/*N*/ 	, mbPasswordVerified( sal_False )
/*N*/     , mbDoc50Password( sal_False )
/*N*/ 	, mbSharedIndexFile( sal_False )
/*N*/ {
/*N*/ }


// Methods XInterface
/*N*/ Any SAL_CALL SfxLibrary_Impl::queryInterface( const Type& rType )
/*N*/ 	throw( RuntimeException )
/*N*/ {
/*N*/ 	Any aRet;

    /*
    if( mbReadOnly )
    {
        aRet = Any( ::cppu::queryInterface( rType,
            static_cast< XContainer * >( this ),
            static_cast< XNameAccess * >( this ) ) );
    }
    else
    {
    */
/*N*/ 		aRet = Any( ::cppu::queryInterface( rType,
/*N*/ 			static_cast< XContainer * >( this ),
/*N*/ 			static_cast< XNameContainer * >( this ),
/*N*/ 			static_cast< XNameAccess * >( this ) ) );
/*N*/ 	//}
/*N*/ 	if( !aRet.hasValue() )
/*N*/ 		aRet = OComponentHelper::queryInterface( rType );
/*N*/ 	return aRet;
/*N*/ }

// Methods XElementAccess
/*?*/ Type SfxLibrary_Impl::getElementType()
/*?*/ 	throw(RuntimeException)
/*?*/ {
        return maNameContainer.getElementType();
/*?*/ }

/*N*/ sal_Bool SfxLibrary_Impl::hasElements()
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	sal_Bool bRet = maNameContainer.hasElements();
/*N*/ 	return bRet;
/*N*/ }

// Methods XNameAccess
Any SfxLibrary_Impl::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Any aRetAny;
    if ( !mbPasswordProtected || mbPasswordVerified )    
        aRetAny = maNameContainer.getByName( aName );
    return aRetAny;
}

/*N*/ Sequence< OUString > SfxLibrary_Impl::getElementNames()
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	return maNameContainer.getElementNames();
/*N*/ }

/*N*/ sal_Bool SfxLibrary_Impl::hasByName( const OUString& aName )
/*N*/ 	throw(RuntimeException)
/*N*/ {
/*N*/ 	sal_Bool bRet = maNameContainer.hasByName( aName );
/*N*/ 	return bRet;
/*N*/ }

// Methods XNameReplace
/*?*/ void SfxLibrary_Impl::replaceByName( const OUString& aName, const Any& aElement )
/*?*/ 	throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }


// Methods XNameContainer
/*N*/ void SfxLibrary_Impl::insertByName( const OUString& aName, const Any& aElement )
/*N*/ 	throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
/*N*/ {
/*N*/ 	if( mbReadOnly || (mbLink && mbReadOnlyLink) )
/*N*/         throw RuntimeException();
/*N*/ 
/*N*/ 	maNameContainer.insertByName( aName, aElement );
/*N*/ 	mbModified = sal_True;
/*N*/ }

/*?*/ void SfxLibrary_Impl::removeByName( const OUString& Name )
/*?*/ 	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

// XTypeProvider
/*?*/ Sequence< Type > SfxLibrary_Impl::getTypes()
/*?*/ 	throw( RuntimeException )
/*?*/ {
    static OTypeCollection * s_pTypes_NameContainer = 0;
    static OTypeCollection * s_pTypes_NameAccess = 0;
    {
        if( !s_pTypes_NameContainer )
        {
            MutexGuard aGuard( Mutex::getGlobalMutex() );
            if( !s_pTypes_NameContainer )
            {
                static OTypeCollection s_aTypes_NameContainer(
                    ::getCppuType( (const Reference< XNameContainer > *)0 ),
                    ::getCppuType( (const Reference< XContainer > *)0 ),
                    OComponentHelper::getTypes() );
                s_pTypes_NameContainer = &s_aTypes_NameContainer;
            }
        }
        return s_pTypes_NameContainer->getTypes();
    }

/*?*/ }


/*?*/ Sequence< sal_Int8 > SfxLibrary_Impl::getImplementationId()
/*?*/ 	throw( RuntimeException )
/*?*/ {
    static OImplementationId * s_pId_NameContainer = 0;
    static OImplementationId * s_pId_NameAccess = 0;
    {
        if( !s_pId_NameContainer )
        {
            MutexGuard aGuard( Mutex::getGlobalMutex() );
            if( !s_pId_NameContainer )
            {
                static OImplementationId s_aId_NameContainer;
                s_pId_NameContainer = &s_aId_NameContainer;
            }
        }
        return s_pId_NameContainer->getImplementationId();
    }

/*?*/ }


//============================================================================

// Methods XContainer
/*N*/ void SAL_CALL SfxLibrary_Impl::addContainerListener( const Reference< XContainerListener >& xListener )
/*N*/ 	throw (RuntimeException)
/*N*/ {
/*N*/ 	maNameContainer.setEventSource( static_cast< XInterface* >( (OWeakObject*)this ) );
/*N*/ 	maNameContainer.addContainerListener( xListener );
/*N*/ }

/*?*/ void SAL_CALL SfxLibrary_Impl::removeContainerListener( const Reference< XContainerListener >& xListener )
/*?*/ 	throw (RuntimeException)
/*?*/ {
        maNameContainer.removeContainerListener( xListener );
/*?*/ }

//============================================================================

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
