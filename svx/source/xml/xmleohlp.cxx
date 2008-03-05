/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmleohlp.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:02:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <stdio.h>

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTFACTORY_HPP_
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBED_PERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#include <svtools/embedhlp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>

#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#include <map>

#ifndef _XMLEOHLP_HXX
#include "xmleohlp.hxx"
#endif

// -----------
// - Defines -
// -----------

using namespace ::rtl;
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

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    maReplacementGraphicsContainerStorageName60( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME_60) ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( ::comphelper::IEmbeddedHelper& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    maReplacementGraphicsContainerStorageName60( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME_60) ),
    mpDocPersist( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ ),
    mpStreamMap( 0 )
{
    Init( 0, rDocPersist, eCreateMode );
}


// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
{
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
            rContainerStorageName = OUString();
            rObjectStorageName = aURLNoPar;
        }
        else
        {
            sal_Int32 nPathStart = 0;
            if( 0 == aURLNoPar.compareToAscii( "./", 2 ) )
                nPathStart = 2;
            if( _nPos >= nPathStart )
                rContainerStorageName = aURLNoPar.copy( nPathStart, _nPos-nPathStart);
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
        const OUString& rStorageName )
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

OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
        const OUString& rURLStr )
{
    OUString sRetURL;

    OUString    aContainerStorageName, aObjectStorageName;
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
        sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_EMBEDDEDOBJECT_URL_BASE) );
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
        sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM("./") );
        if( aContainerStorageName.getLength() )
        {
            sRetURL += aContainerStorageName;
            sRetURL += OUString( '/' );
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
                OUString aMediaType;
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
                            aEmbDescr[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StoreVisualReplacement" ) );
                               aEmbDescr[0].Value <<= (sal_Bool)(!bOasisFormat);
                            if ( !bOasisFormat )
                            {
                                OUString aMimeType;
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
        OUString    aContainerStorageName, aObjectStorageName;
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
