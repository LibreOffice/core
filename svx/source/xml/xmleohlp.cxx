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
#include "precompiled_svx.hxx"

#include <stdio.h>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#ifndef _COM_SUN_STAR_EMBED_XEMBED_PERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
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

#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#include <map>
#include "svx/xmleohlp.hxx"

// -----------
// - Defines -
// -----------

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

// -----------------------------------------------------------------------------

class OutputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<XOutputStream>
{
    ::osl::Mutex    maMutex;
    Reference < XOutputStream > xOut;
    TempFile aTempFile;
    sal_Bool bStreamClosed : 1;
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
    : bStreamClosed( sal_False )
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
    bStreamClosed = sal_True;
}

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
DBG_NAME(SvXMLEmbeddedObjectHelper)
SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    maReplacementGraphicsContainerStorageName60( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME_60) ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
    DBG_CTOR(SvXMLEmbeddedObjectHelper,NULL);
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( ::comphelper::IEmbeddedHelper& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    maReplacementGraphicsContainerStorageName60( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME_60) ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
    DBG_CTOR(SvXMLEmbeddedObjectHelper,NULL);
    Init( 0, rDocPersist, eCreateMode );
}


// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
{
    DBG_DTOR(SvXMLEmbeddedObjectHelper,NULL);
    if( mpStreamMap )
    {
        SvXMLEmbeddedObjectHelper_Impl::iterator aIter = mpStreamMap->begin();
        SvXMLEmbeddedObjectHelper_Impl::iterator aEnd = mpStreamMap->end();
        for( ; aIter != aEnd; aIter++ )
        {
            if( aIter->second )
            {
                aIter->second->release();
                aIter->second = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLEmbeddedObjectHelper::disposing()
{
    Flush();
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLEmbeddedObjectHelper::ImplGetStorageNames(
        const ::rtl::OUString& rURLStr,
        ::rtl::OUString& rContainerStorageName,
        ::rtl::OUString& rObjectStorageName,
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

    if( !rURLStr.getLength() )
        return sal_False;

    // get rid of arguments
    sal_Int32 nPos = rURLStr.indexOf( '?' );
    ::rtl::OUString aURLNoPar;
    if ( nPos == -1 )
        aURLNoPar = rURLStr;
    else
    {
        aURLNoPar = rURLStr.copy( 0, nPos );

        // check the arguments
        nPos++;
        while( nPos >= 0 && nPos < rURLStr.getLength() )
        {
            ::rtl::OUString aToken = rURLStr.getToken( 0, ',', nPos );
            if ( aToken.equalsIgnoreAsciiCase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "oasis=false" ) ) ) )
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
        sal_Bool bObjUrl =
            0 == aURLNoPar.compareToAscii( XML_EMBEDDEDOBJECT_URL_BASE,
                                 sizeof( XML_EMBEDDEDOBJECT_URL_BASE ) -1 );
        sal_Bool bGrUrl = !bObjUrl &&
              0 == aURLNoPar.compareToAscii( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE,
                         sizeof( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE ) -1 );
        if( !(bObjUrl || bGrUrl) )
            return sal_False;

        sal_Int32 nPathStart = nPos + 1;
        nPos = aURLNoPar.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = ::rtl::OUString();
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
            sal_Bool bOASIS = mxRootStorage.is() &&
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
        DBG_ASSERT( '#' != aURLNoPar[0], "invalid object URL" );

        sal_Int32 _nPos = aURLNoPar.lastIndexOf( '/' );
        if( -1 == _nPos )
        {
            rContainerStorageName = ::rtl::OUString();
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
                if( 0 == aURLNoPar.compareToAscii( "./", 2 ) )
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

    if( -1 != rContainerStorageName.indexOf( '/' ) )
    {
        DBG_ERROR( "SvXMLEmbeddedObjectHelper: invalid path name" );
        return sal_False;
    }

    return sal_True;
}


// -----------------------------------------------------------------------------

uno::Reference < embed::XStorage > SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
        const ::rtl::OUString& rStorageName )
{
    DBG_ASSERT( -1 == rStorageName.indexOf( '/' ) &&
                -1 == rStorageName.indexOf( '\\' ),
                "nested embedded storages aren't supported" );
    if( !mxContainerStorage.is() ||
        ( rStorageName != maCurContainerStorageName ) )
    {
        if( mxContainerStorage.is() &&
            maCurContainerStorageName.getLength() > 0 &&
            EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode )
        {
            uno::Reference < embed::XTransactedObject > xTrans( mxContainerStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
        }

        if( rStorageName.getLength() > 0 && mxRootStorage.is() )
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

// -----------------------------------------------------------------------------

sal_Bool SvXMLEmbeddedObjectHelper::ImplReadObject(
        const ::rtl::OUString& rContainerStorageName,
        ::rtl::OUString& rObjName,
        const SvGlobalName *pClassId,
        SvStream* pTemp )
{
    (void)pClassId;

    uno::Reference < embed::XStorage > xDocStor( mpDocPersist->getStorage() );
    uno::Reference < embed::XStorage > xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );

    if( !xCntnrStor.is() && !pTemp )
        return sal_False;

    String aSrcObjName( rObjName );
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
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.oleobject" ) ) ) );

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
    ::rtl::OUString aName( rObjName );

    // TODO/LATER: The provided pClassId is ignored for now.
    //             The stream contains OLE storage internally and this storage already has a class id specifying the
    //             server that was used to create the object. pClassId could be used to specify the server that should
    //             be used for the next opening, but this information seems to be out of the file format responsibility
    //             area.
    rContainer.GetEmbeddedObject( aName );

    return sal_True;
}

// -----------------------------------------------------------------------------

::rtl::OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
        const ::rtl::OUString& rURLStr )
{
    ::rtl::OUString sRetURL;

    ::rtl::OUString aContainerStorageName, aObjectStorageName;
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
        sRetURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(XML_EMBEDDEDOBJECT_URL_BASE) );
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
        sRetURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("./") );
        if( aContainerStorageName.getLength() )
        {
            sRetURL += aContainerStorageName;
            sRetURL += ::rtl::OUString( '/' );
        }
        sRetURL += aObjectStorageName;
    }

    return sRetURL;
}

// -----------------------------------------------------------------------------

uno::Reference< io::XInputStream > SvXMLEmbeddedObjectHelper::ImplGetReplacementImage(
                                            const uno::Reference< embed::XEmbeddedObject >& xObj )
{
    uno::Reference< io::XInputStream > xStream;

    if( xObj.is() )
    {
        try
        {
            sal_Bool bSwitchBackToLoaded = sal_False;
            sal_Int32 nCurState = xObj->getCurrentState();
            if ( nCurState == embed::EmbedStates::LOADED || nCurState == embed::EmbedStates::RUNNING )
            {
                // means that the object is not active
                // copy replacement image from old to new container
                ::rtl::OUString aMediaType;
                xStream = mpDocPersist->getEmbeddedObjectContainer().GetGraphicStream( xObj, &aMediaType );
            }

            if ( !xStream.is() )
            {
                // the image must be regenerated
                // TODO/LATER: another aspect could be used
                if ( nCurState == embed::EmbedStates::LOADED )
                    bSwitchBackToLoaded = sal_True;

                ::rtl::OUString aMediaType;
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

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Init(
        const uno::Reference < embed::XStorage >& rRootStorage,
        ::comphelper::IEmbeddedHelper& rPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    mxRootStorage = rRootStorage;
    mpDocPersist = &rPersist;
    meCreateMode = eCreateMode;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Destroy(
        SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper )
{
    if( pSvXMLEmbeddedObjectHelper )
    {
        pSvXMLEmbeddedObjectHelper->dispose();
        pSvXMLEmbeddedObjectHelper->release();
    }
}

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Flush()
{
    if( mxTempStorage.is() )
    {
        Reference < XComponent > xComp( mxTempStorage, UNO_QUERY );
        xComp->dispose();
    }
}

// XGraphicObjectResolver: alien objects!
::rtl::OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL( const ::rtl::OUString& aURL )
    throw(RuntimeException)
{
    MutexGuard          aGuard( maMutex );

    return ImplInsertEmbeddedObjectURL( aURL );
}

// XNameAccess: alien objects!
Any SAL_CALL SvXMLEmbeddedObjectHelper::getByName(
        const ::rtl::OUString& rURLStr )
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
        ::rtl::OUString aContainerStorageName, aObjectStorageName;
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
                            aEmbDescr[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StoreVisualReplacement" ) );
                               aEmbDescr[0].Value <<= (sal_Bool)(!bOasisFormat);
                            if ( !bOasisFormat )
                            {
                                ::rtl::OUString aMimeType;
                                uno::Reference< io::XInputStream > xGrInStream = ImplGetReplacementImage( xObj );
                                if ( xGrInStream.is() )
                                {
                                    aEmbDescr.realloc( 2 );
                                    aEmbDescr[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VisualReplacement" ) );
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

Sequence< ::rtl::OUString > SAL_CALL SvXMLEmbeddedObjectHelper::getElementNames()
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    return Sequence< ::rtl::OUString >(0);
}

sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasByName( const ::rtl::OUString& rURLStr )
    throw (RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        return sal_True;
    }
    else
    {
        ::rtl::OUString aContainerStorageName, aObjectStorageName;
        if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                                  aObjectStorageName,
                                  sal_True ) )
            return sal_False;

        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();
        return aObjectStorageName.getLength() > 0 &&
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

