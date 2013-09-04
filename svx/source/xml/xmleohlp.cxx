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


#include <stdio.h>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/debug.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>

#include <svtools/embedhlp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>

#include <comphelper/classids.hxx>
#include <map>
#include "svx/xmleohlp.hxx"

using namespace ::osl;
using namespace ::cppu;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

#define XML_CONTAINERSTORAGE_NAME_60        "Pictures"
#define XML_CONTAINERSTORAGE_NAME       "ObjectReplacements"
#define XML_EMBEDDEDOBJECT_URL_BASE     "vnd.sun.star.EmbeddedObject:"
#define XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE      "vnd.sun.star.GraphicObject:"

// -----------------------------------------------------------------------------

class OutputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<XOutputStream>
{
    ::osl::Mutex    maMutex;
    Reference < XOutputStream > xOut;
    TempFile aTempFile;
    bool bStreamClosed : 1;
    SvStream* pStream;

public:
    OutputStorageWrapper_Impl();
    virtual ~OutputStorageWrapper_Impl();

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL flush() throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void SAL_CALL closeOutput() throw(NotConnectedException, BufferSizeExceededException, RuntimeException);

    SvStream*   GetStream();
};

OutputStorageWrapper_Impl::OutputStorageWrapper_Impl()
    : bStreamClosed( false )
    , pStream(0)
{
    aTempFile.EnableKillingFile();
    pStream = aTempFile.GetStream( STREAM_READWRITE );
    xOut = new OOutputStreamWrapper( *pStream );
}

OutputStorageWrapper_Impl::~OutputStorageWrapper_Impl()
{
}

SvStream *OutputStorageWrapper_Impl::GetStream()
{
    if( bStreamClosed )
        return pStream;
    return NULL;
}

void SAL_CALL OutputStorageWrapper_Impl::writeBytes(
        const Sequence< sal_Int8 >& aData)
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    xOut->writeBytes( aData );
}

void SAL_CALL OutputStorageWrapper_Impl::flush()
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    xOut->flush();
}

void SAL_CALL OutputStorageWrapper_Impl::closeOutput()
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    xOut->closeOutput();
    bStreamClosed = true;
}

struct OUStringLess
{
    bool operator() ( const OUString& r1, const OUString& r2 ) const
    {
        return (r1 < r2) != sal_False;
    }
};

DBG_NAME(SvXMLEmbeddedObjectHelper)
SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( XML_CONTAINERSTORAGE_NAME ),
    maReplacementGraphicsContainerStorageName60( XML_CONTAINERSTORAGE_NAME_60 ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
    DBG_CTOR(SvXMLEmbeddedObjectHelper,NULL);
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( ::comphelper::IEmbeddedHelper& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( XML_CONTAINERSTORAGE_NAME ),
    maReplacementGraphicsContainerStorageName60( XML_CONTAINERSTORAGE_NAME_60 ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
    DBG_CTOR(SvXMLEmbeddedObjectHelper,NULL);
    Init( 0, rDocPersist, eCreateMode );
}

SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
{
    DBG_DTOR(SvXMLEmbeddedObjectHelper,NULL);
    if( mpStreamMap )
    {
        SvXMLEmbeddedObjectHelper_Impl::iterator aIter = mpStreamMap->begin();
        SvXMLEmbeddedObjectHelper_Impl::iterator aEnd = mpStreamMap->end();
        for( ; aIter != aEnd; ++aIter )
        {
            if( aIter->second )
            {
                aIter->second->release();
                aIter->second = 0;
            }
        }
        delete mpStreamMap;
    }
}

void SAL_CALL SvXMLEmbeddedObjectHelper::disposing()
{
    Flush();
}


void SvXMLEmbeddedObjectHelper::splitObjectURL(OUString aURLNoPar,
    OUString& rContainerStorageName,
    OUString& rObjectStorageName)
{
    DBG_ASSERT( '#' != aURLNoPar[0], "invalid object URL" );

    sal_Int32 _nPos = aURLNoPar.lastIndexOf( '/' );
    if( -1 == _nPos )
    {
        rContainerStorageName = OUString();
        rObjectStorageName = aURLNoPar;
    }
    else
    {
        //eliminate 'superfluous' slashes at start and end
        //#i103076# load objects with all allowed xlink:href syntaxes
        {
            //eliminate './' at start
            sal_Int32 nStart = 0;
            sal_Int32 nCount = aURLNoPar.getLength();
            if( aURLNoPar.startsWith( "./" ) )
            {
                nStart = 2;
                nCount -= 2;
            }

            //eliminate '/' at end
            sal_Int32 nEnd = aURLNoPar.lastIndexOf( '/' );
            if( nEnd == aURLNoPar.getLength()-1 && nEnd != (nStart-1) )
                nCount--;

            aURLNoPar = aURLNoPar.copy( nStart, nCount );
        }

        _nPos = aURLNoPar.lastIndexOf( '/' );
        if( _nPos >= 0 )
            rContainerStorageName = aURLNoPar.copy( 0, _nPos );
        rObjectStorageName = aURLNoPar.copy( _nPos+1 );
    }
}

sal_Bool SvXMLEmbeddedObjectHelper::ImplGetStorageNames(
        const OUString& rURLStr,
        OUString& rContainerStorageName,
        OUString& rObjectStorageName,
        sal_Bool bInternalToExternal,
        sal_Bool *pGraphicRepl,
        sal_Bool *pOasisFormat ) const
{
    // internal URL: vnd.sun.star.EmbeddedObject:<object-name>
    //           or: vnd.sun.star.EmbeddedObject:<path>/<object-name>
    // internal replacement images:
    //               vnd.sun.star.EmbeddedObjectGraphic:<object-name>
    //           or: vnd.sun.star.EmbeddedObjectGraphic:<path>/<object-name>
    // external URL: ./<path>/<object-name>
    //           or: <path>/<object-name>
    //           or: <object-name>
    // currently, path may only consist of a single directory name
    // it is also possible to have additional arguments at the end of URL: <main URL>[?<name>=<value>[,<name>=<value>]*]

    if( pGraphicRepl )
        *pGraphicRepl = sal_False;

    if( pOasisFormat )
        *pOasisFormat = sal_True; // the default value

    if( rURLStr.isEmpty() )
        return sal_False;

    // get rid of arguments
    sal_Int32 nPos = rURLStr.indexOf( '?' );
    OUString aURLNoPar;
    if ( nPos == -1 )
        aURLNoPar = rURLStr;
    else
    {
        aURLNoPar = rURLStr.copy( 0, nPos );

        // check the arguments
        nPos++;
        while( nPos >= 0 && nPos < rURLStr.getLength() )
        {
            OUString aToken = rURLStr.getToken( 0, ',', nPos );
            if ( aToken.equalsIgnoreAsciiCase( OUString( "oasis=false" ) ) )
            {
                if ( pOasisFormat )
                    *pOasisFormat = sal_False;
                break;
            }
            else
            {
                DBG_ASSERT( sal_False, "invalid arguments was found in URL!" );
            }
        }
    }

    if( bInternalToExternal )
    {
        nPos = aURLNoPar.indexOf( ':' );
        if( -1 == nPos )
            return sal_False;
        sal_Bool bObjUrl = aURLNoPar.startsWith( XML_EMBEDDEDOBJECT_URL_BASE );
        bool bGrUrl = !bObjUrl &&
              aURLNoPar.startsWith( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE );
        if( !(bObjUrl || bGrUrl) )
            return sal_False;

        sal_Int32 nPathStart = nPos + 1;
        nPos = aURLNoPar.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = OUString();
            rObjectStorageName = aURLNoPar.copy( nPathStart );
        }
        else if( nPos > nPathStart )
        {
            rContainerStorageName = aURLNoPar.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = aURLNoPar.copy( nPos+1 );
        }
        else
            return sal_False;

        if( bGrUrl )
        {
            bool bOASIS = mxRootStorage.is() &&
                ( SotStorage::GetVersion( mxRootStorage ) > SOFFICE_FILEFORMAT_60 );
            rContainerStorageName = bOASIS
                    ? maReplacementGraphicsContainerStorageName
                    : maReplacementGraphicsContainerStorageName60;

            if( pGraphicRepl )
                *pGraphicRepl = sal_True;
        }


    }
    else
    {
        splitObjectURL(aURLNoPar, rContainerStorageName, rObjectStorageName);
    }

    if( -1 != rContainerStorageName.indexOf( '/' ) )
    {
        OSL_FAIL( "SvXMLEmbeddedObjectHelper: invalid path name" );
        return sal_False;
    }

    return sal_True;
}

uno::Reference < embed::XStorage > SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
        const OUString& rStorageName )
{
    DBG_ASSERT( -1 == rStorageName.indexOf( '/' ) &&
                -1 == rStorageName.indexOf( '\\' ),
                "nested embedded storages aren't supported" );
    if( !mxContainerStorage.is() ||
        ( rStorageName != maCurContainerStorageName ) )
    {
        if( mxContainerStorage.is() &&
            !maCurContainerStorageName.isEmpty() &&
            EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode )
        {
            uno::Reference < embed::XTransactedObject > xTrans( mxContainerStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
        }

        if( !rStorageName.isEmpty() && mxRootStorage.is() )
        {
            sal_Int32 nMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
                                    ? ::embed::ElementModes::READWRITE
                                    : ::embed::ElementModes::READ;
            mxContainerStorage = mxRootStorage->openStorageElement( rStorageName,
                                                             nMode );
        }
        else
        {
            mxContainerStorage = mxRootStorage;
        }
        maCurContainerStorageName = rStorageName;
    }

    return mxContainerStorage;
}

sal_Bool SvXMLEmbeddedObjectHelper::ImplReadObject(
        const OUString& rContainerStorageName,
        OUString& rObjName,
        const SvGlobalName *pClassId,
        SvStream* pTemp )
{
    (void)pClassId;

    uno::Reference < embed::XStorage > xDocStor( mpDocPersist->getStorage() );
    uno::Reference < embed::XStorage > xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );

    if( !xCntnrStor.is() && !pTemp )
        return sal_False;

    OUString aSrcObjName( rObjName );
    comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();

    // Is the object name unique?
    // if the object is already instantiated by GetEmbeddedObject
    // that means that the duplication is being loaded
    sal_Bool bDuplicate = rContainer.HasInstantiatedEmbeddedObject( rObjName );
    DBG_ASSERT( !bDuplicate, "An object in the document is referenced twice!" );

    if( xDocStor != xCntnrStor || pTemp || bDuplicate )
    {
        // TODO/LATER: make this alltogether a method in the EmbeddedObjectContainer

        // create a unique name for the duplicate object
        if( bDuplicate )
            rObjName = rContainer.CreateUniqueObjectName();

        if( pTemp )
        {
            try
            {
                pTemp->Seek( 0 );
                uno::Reference < io::XStream > xStm = xDocStor->openStreamElement( rObjName,
                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xStm );
                *pTemp >> *pStream;
                delete pStream;

                // TODO/LATER: what to do when other types of objects are based on substream persistence?
                // This is an ole object
                uno::Reference< beans::XPropertySet > xProps( xStm, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    OUString( "MediaType" ),
                    uno::makeAny( OUString( "application/vnd.sun.star.oleobject" ) ) );

                xStm->getOutputStream()->closeOutput();
            }
            catch ( uno::Exception& )
            {
                return sal_False;
            }
        }
        else
        {
            try
            {
                xCntnrStor->copyElementTo( aSrcObjName, xDocStor, rObjName );
            }
            catch ( uno::Exception& )
            {
                return sal_False;
            }
        }
    }

    // make object known to the container
    // TODO/LATER: could be done a little bit more efficient!
    OUString aName( rObjName );

    // TODO/LATER: The provided pClassId is ignored for now.
    //             The stream contains OLE storage internally and this storage already has a class id specifying the
    //             server that was used to create the object. pClassId could be used to specify the server that should
    //             be used for the next opening, but this information seems to be out of the file format responsibility
    //             area.
    rContainer.GetEmbeddedObject( aName );

    return sal_True;
}

OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
        const OUString& rURLStr )
{
    OUString sRetURL;

    OUString aContainerStorageName, aObjectStorageName;
    if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                              aObjectStorageName,
                              EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode ) )
        return sRetURL;

    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        OutputStorageWrapper_Impl *pOut = 0;
        SvXMLEmbeddedObjectHelper_Impl::iterator aIter;

        if( mpStreamMap )
        {
            aIter = mpStreamMap->find( rURLStr );
            if( aIter != mpStreamMap->end() && aIter->second )
                pOut = aIter->second;
        }

        SvGlobalName aClassId, *pClassId = 0;
        sal_Int32 nPos = aObjectStorageName.lastIndexOf( '!' );
        if( -1 != nPos && aClassId.MakeId( aObjectStorageName.copy( nPos+1 ) ) )
        {
            aObjectStorageName = aObjectStorageName.copy( 0, nPos );
            pClassId = &aClassId;
        }

        ImplReadObject( aContainerStorageName, aObjectStorageName, pClassId, pOut ? pOut->GetStream() : 0 );
        sRetURL = OUString( XML_EMBEDDEDOBJECT_URL_BASE );
        sRetURL += aObjectStorageName;

        if( pOut )
        {
            mpStreamMap->erase( aIter );
            pOut->release();
        }
    }
    else
    {
        // Objects are written using ::comphelper::IEmbeddedHelper::SaveAs
        sRetURL = OUString("./");
        if( !aContainerStorageName.isEmpty() )
        {
            sRetURL += aContainerStorageName;
            sRetURL += OUString( '/' );
        }
        sRetURL += aObjectStorageName;
    }

    return sRetURL;
}

uno::Reference< io::XInputStream > SvXMLEmbeddedObjectHelper::ImplGetReplacementImage(
                                            const uno::Reference< embed::XEmbeddedObject >& xObj )
{
    uno::Reference< io::XInputStream > xStream;

    if( xObj.is() )
    {
        try
        {
            bool bSwitchBackToLoaded = false;
            sal_Int32 nCurState = xObj->getCurrentState();
            if ( nCurState == embed::EmbedStates::LOADED || nCurState == embed::EmbedStates::RUNNING )
            {
                // means that the object is not active
                // copy replacement image from old to new container
                OUString aMediaType;
                xStream = mpDocPersist->getEmbeddedObjectContainer().GetGraphicStream( xObj, &aMediaType );
            }

            if ( !xStream.is() )
            {
                // the image must be regenerated
                // TODO/LATER: another aspect could be used
                if ( nCurState == embed::EmbedStates::LOADED )
                    bSwitchBackToLoaded = true;

                OUString aMediaType;
                xStream = svt::EmbeddedObjectRef::GetGraphicReplacementStream(
                                                    embed::Aspects::MSOLE_CONTENT,
                                                    xObj,
                                                    &aMediaType );
            }

            if ( bSwitchBackToLoaded )
                // switch back to loaded state; that way we have a minimum cache confusion
                xObj->changeState( embed::EmbedStates::LOADED );
        }
        catch( uno::Exception& )
        {}
    }

    return xStream;
}

void SvXMLEmbeddedObjectHelper::Init(
        const uno::Reference < embed::XStorage >& rRootStorage,
        ::comphelper::IEmbeddedHelper& rPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    mxRootStorage = rRootStorage;
    mpDocPersist = &rPersist;
    meCreateMode = eCreateMode;
}

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        const uno::Reference < embed::XStorage >& rRootStorage,
        ::comphelper::IEmbeddedHelper& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode,
        sal_Bool bDirect )
{
    (void)bDirect;

    SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;

    pThis->acquire();
    pThis->Init( rRootStorage, rDocPersist, eCreateMode );

    return pThis;
}

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        ::comphelper::IEmbeddedHelper& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;

    pThis->acquire();
    pThis->Init( 0, rDocPersist, eCreateMode );

    return pThis;
}

void SvXMLEmbeddedObjectHelper::Destroy(
        SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper )
{
    if( pSvXMLEmbeddedObjectHelper )
    {
        pSvXMLEmbeddedObjectHelper->dispose();
        pSvXMLEmbeddedObjectHelper->release();
    }
}

void SvXMLEmbeddedObjectHelper::Flush()
{
    if( mxTempStorage.is() )
    {
        Reference < XComponent > xComp( mxTempStorage, UNO_QUERY );
        xComp->dispose();
    }
}

// XGraphicObjectResolver: alien objects!
OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL( const OUString& aURL )
    throw(RuntimeException)
{
    MutexGuard          aGuard( maMutex );

    return ImplInsertEmbeddedObjectURL( aURL );
}

// XNameAccess: alien objects!
Any SAL_CALL SvXMLEmbeddedObjectHelper::getByName(
        const OUString& rURLStr )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    Any aRet;
    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        Reference < XOutputStream > xStrm;
        if( mpStreamMap )
        {
            SvXMLEmbeddedObjectHelper_Impl::iterator aIter =
                mpStreamMap->find( rURLStr );
            if( aIter != mpStreamMap->end() && aIter->second )
                xStrm = aIter->second;
        }
        if( !xStrm.is() )
        {
            OutputStorageWrapper_Impl *pOut = new OutputStorageWrapper_Impl;
            pOut->acquire();
            if( !mpStreamMap )
                mpStreamMap = new SvXMLEmbeddedObjectHelper_Impl;
            (*mpStreamMap)[rURLStr] = pOut;
            xStrm = pOut;
        }

        aRet <<= xStrm;
    }
    else
    {
        sal_Bool bGraphicRepl = sal_False;
        sal_Bool bOasisFormat = sal_True;
        Reference < XInputStream > xStrm;
        OUString aContainerStorageName, aObjectStorageName;
        if( ImplGetStorageNames( rURLStr, aContainerStorageName,
                                 aObjectStorageName,
                                 sal_True,
                                    &bGraphicRepl,
                                 &bOasisFormat ) )
        {
            try
            {
                comphelper::EmbeddedObjectContainer& rContainer =
                        mpDocPersist->getEmbeddedObjectContainer();

                Reference < embed::XEmbeddedObject > xObj = rContainer.GetEmbeddedObject( aObjectStorageName );
                DBG_ASSERT( xObj.is(), "Didn't get object" );

                if( xObj.is() )
                {
                    if( bGraphicRepl )
                    {
                        xStrm = ImplGetReplacementImage( xObj );
                    }
                    else
                    {
                        Reference < embed::XEmbedPersist > xPersist( xObj, UNO_QUERY );
                        if( xPersist.is() )
                        {
                            if( !mxTempStorage.is() )
                                mxTempStorage =
                                    comphelper::OStorageHelper::GetTemporaryStorage();
                            Sequence < beans::PropertyValue > aDummy( 0 ), aEmbDescr( 1 );
                            aEmbDescr[0].Name = OUString( "StoreVisualReplacement" );
                               aEmbDescr[0].Value <<= (sal_Bool)(!bOasisFormat);
                            if ( !bOasisFormat )
                            {
                                uno::Reference< io::XInputStream > xGrInStream = ImplGetReplacementImage( xObj );
                                if ( xGrInStream.is() )
                                {
                                    aEmbDescr.realloc( 2 );
                                    aEmbDescr[1].Name = OUString( "VisualReplacement" );
                                    aEmbDescr[1].Value <<= xGrInStream;
                                }
                            }

                            xPersist->storeToEntry( mxTempStorage, aObjectStorageName,
                                                    aDummy, aEmbDescr );
                            Reference < io::XStream > xStream =
                                mxTempStorage->openStreamElement(
                                                        aObjectStorageName,
                                                        embed::ElementModes::READ);
                            if( xStream.is() )
                                xStrm = xStream->getInputStream();
                        }
                    }
                }
            }
            catch ( uno::Exception& )
            {
            }
        }

        aRet <<= xStrm;
    }

    return aRet;
}

Sequence< OUString > SAL_CALL SvXMLEmbeddedObjectHelper::getElementNames()
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    return Sequence< OUString >(0);
}

sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasByName( const OUString& rURLStr )
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        return sal_True;
    }
    else
    {
        OUString aContainerStorageName, aObjectStorageName;
        if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                                  aObjectStorageName,
                                  sal_True ) )
            return sal_False;

        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();
        return !aObjectStorageName.isEmpty() &&
               rContainer.HasEmbeddedObject( aObjectStorageName );
    }
}

// XNameAccess
Type SAL_CALL SvXMLEmbeddedObjectHelper::getElementType()
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
        return ::getCppuType((const Reference<XOutputStream>*)0);
    else
        return ::getCppuType((const Reference<XInputStream>*)0);
}

sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasElements()
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        return sal_True;
    }
    else
    {
        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();
        return rContainer.HasEmbeddedObjects();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
