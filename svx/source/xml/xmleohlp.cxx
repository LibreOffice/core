/*************************************************************************
 *
 *  $RCSfile: xmleohlp.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
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

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#include <map>

#ifndef _XMLEOHLP_HXX
#include "xmleohlp.hxx"
#endif

#include <sfx2/objsh.hxx>

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
class InputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<stario::XInputStream>
{
    ::osl::Mutex    maMutex;
    Reference < XInputStream > xIn;
    TempFile aTempFile;

public:
    InputStorageWrapper_Impl( const uno::Reference < embed::XStorage >&, const ::rtl::OUString& );
    virtual ~InputStorageWrapper_Impl();

    virtual sal_Int32   SAL_CALL    readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
};

InputStorageWrapper_Impl::InputStorageWrapper_Impl( const uno::Reference < embed::XStorage >& xDocStor, const ::rtl::OUString& aObjectStorageName )
{
    SvStream *pStream = 0;
    aTempFile.EnableKillingFile();
    uno::Reference < container::XNameAccess > xAccess( xDocStor, uno::UNO_QUERY );
    if ( xAccess.is() && xAccess->hasByName( aObjectStorageName ) )
    {
        try
        {
            if ( xDocStor->isStreamElement( aObjectStorageName ) )
            {
                xIn = xDocStor->openStreamElement( aObjectStorageName, embed::ElementModes::READ )->getInputStream();
            }
            else
            {
                DBG_ERROR("InputStorageWrapper for an own object?!");
                uno::Reference < embed::XStorage > xStg = xDocStor->openStorageElement( aObjectStorageName, embed::ElementModes::READ );
                pStream = aTempFile.GetStream( STREAM_READWRITE );
                uno::Reference < io::XStream > xStream = new OStreamWrapper( *pStream );
                uno::Reference < embed::XStorage > xStor = ::comphelper::OStorageHelper::GetStorageFromStream( xStream );
                xStg->copyToStorage( xStor );
                xIn = new OSeekableInputStreamWrapper( *pStream );
            }
        }
        catch ( uno::Exception& )
        {
        }
    }

    if ( !xIn.is() )
    {
        if ( !pStream )
            pStream = aTempFile.GetStream( STREAM_READWRITE );
        xIn = new OSeekableInputStreamWrapper( *pStream );
    }
}

InputStorageWrapper_Impl::~InputStorageWrapper_Impl()
{
    xIn = 0;
}

sal_Int32 SAL_CALL InputStorageWrapper_Impl::readBytes(
        Sequence< sal_Int8 >& aData,
        sal_Int32 nBytesToRead)
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    return xIn->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL InputStorageWrapper_Impl::readSomeBytes(
        Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead)
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    return xIn->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL InputStorageWrapper_Impl::skipBytes( sal_Int32 nBytesToSkip )
    throw(NotConnectedException, BufferSizeExceededException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    xIn->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL InputStorageWrapper_Impl::available()
    throw(NotConnectedException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    return xIn->available();
}

void SAL_CALL InputStorageWrapper_Impl::closeInput()
    throw(NotConnectedException, RuntimeException)
{
    MutexGuard          aGuard( maMutex );
    xIn->closeInput();
    xIn = 0;
}

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
    mpStreamMap( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
{
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( SfxObjectShell& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maReplacementGraphicsContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    maReplacementGraphicsContainerStorageName60( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME_60) ),
    mpDocPersist( 0 ),
    mpStreamMap( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
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
        for( aIter; aIter != aEnd; aIter++ )
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
        sal_Bool *pGraphicRepl ) const
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
    sal_Bool    bRet = sal_False;
    if( pGraphicRepl )
        *pGraphicRepl = sal_False;

    if( !rURLStr.getLength() )
        return sal_False;

    if( bInternalToExternal )
    {
        sal_Int32 nPos = rURLStr.indexOf( ':' );
        if( -1 == nPos )
            return sal_False;
        sal_Bool bObjUrl =
            0 == rURLStr.compareToAscii( XML_EMBEDDEDOBJECT_URL_BASE,
                                 sizeof( XML_EMBEDDEDOBJECT_URL_BASE ) -1 );
        sal_Bool bGrUrl = !bObjUrl &&
              0 == rURLStr.compareToAscii( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE,
                         sizeof( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE ) -1 );
        if( !(bObjUrl || bGrUrl) )
            return sal_False;

        sal_Int32 nPathStart = nPos + 1;
        nPos = rURLStr.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = OUString();
            rObjectStorageName = rURLStr.copy( nPathStart );
        }
        else if( nPos > nPathStart )
        {
            rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = rURLStr.copy( nPos+1 );
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
        DBG_ASSERT( '#' != rURLStr[0], "invalid object URL" );

        sal_Int32 nPos = rURLStr.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = OUString();
            rObjectStorageName = rURLStr;
        }
        else
        {
            sal_Int32 nPathStart = 0;
            if( 0 == rURLStr.compareToAscii( "./", 2 ) )
                nPathStart = 2;
            if( nPos >= nPathStart )
                rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = rURLStr.copy( nPos+1 );
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
    uno::Reference < embed::XStorage > xDocStor( mpDocPersist->GetStorage() );
    uno::Reference < embed::XStorage > xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );

    if( !xCntnrStor.is() && !pTemp )
        return sal_False;

    String aSrcObjName( rObjName );
    comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->GetEmbeddedObjectContainer();
    if( xDocStor != xCntnrStor || pTemp )
    {
        // Is the object name unique?
        // TODO/LATER: make this alltogether a method in the EmbeddedObjectContainer
        if( rContainer.HasEmbeddedObject( rObjName ) )
            rObjName = rContainer.CreateUniqueObjectName();

        if( pTemp )
        {
            try
            {
                uno::Reference < io::XStream > xStm = xDocStor->openStreamElement( rObjName,
                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xStm );
                *pTemp >> *pStream;
                delete pStream;
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
        // Objects are written using SfxObjectShell::SaveAs
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

void SvXMLEmbeddedObjectHelper::Init(
        const uno::Reference < embed::XStorage >& rRootStorage,
        SfxObjectShell& rPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    mxRootStorage = rRootStorage;
    mpDocPersist = &rPersist;
    meCreateMode = eCreateMode;
}

// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        const uno::Reference < embed::XStorage >& rRootStorage,
        SfxObjectShell& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode,
        sal_Bool bDirect )
{
    SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;

    pThis->acquire();
    pThis->Init( rRootStorage, rDocPersist, eCreateMode );

    return pThis;
}

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        SfxObjectShell& rDocPersist,
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
        Reference < XInputStream > xStrm;
        OUString aContainerStorageName, aObjectStorageName;
        if( ImplGetStorageNames( rURLStr, aContainerStorageName,
                                 aObjectStorageName,
                                 sal_True,
                                    &bGraphicRepl ) )
        {
            try
            {
                if( bGraphicRepl )
                {
                    Reference < embed::XEmbeddedObject > xObj =
                        mpDocPersist->GetEmbeddedObjectContainer().
                            GetEmbeddedObject( aObjectStorageName );
                    DBG_ASSERT( xObj.is(), "Didn't get object" );
                    if( xObj.is() )
                    {
                        OUString aMimeType;
                        xStrm = mpDocPersist->GetEmbeddedObjectContainer().
                                                    GetGraphicStream( xObj,
                                                    &aMimeType );
                    }
                }
                else
                {
                    uno::Reference < embed::XStorage > xDocStor( mpDocPersist->GetStorage() );
                    if ( xDocStor->isStreamElement( aObjectStorageName ) )
                    {
                        uno::Reference < io::XStream > xStream = xDocStor->openStreamElement( aObjectStorageName, embed::ElementModes::READ );
                        xStrm = xStream->getInputStream();
                    }
                    else
                    {
                        xStrm = new InputStorageWrapper_Impl( xDocStor, aObjectStorageName );
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

        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->GetEmbeddedObjectContainer();
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
        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->GetEmbeddedObjectContainer();
        return rContainer.HasEmbeddedObjects();
    }
}
