/*************************************************************************
 *
 *  $RCSfile: xmleohlp.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _PERSIST_HXX
#include <so3/persist.hxx>
#endif
#ifndef _FACTORY_HXX
#include <so3/factory.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <so3/embobj.hxx>
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
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

#define XML_CONTAINERSTORAGE_NAME       "Objects"
#define XML_EMBEDDEDOBJECT_URL_BASE     "vnd.sun.star.EmbeddedObject:"

// -----------------------------------------------------------------------------

class InputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<stario::XInputStream>
{
    ::osl::Mutex    maMutex;
    SvStorageRef xStor;
    Reference < XInputStream > xIn;
    TempFile aTempFile;

public:
    InputStorageWrapper_Impl( SvPersist *pPersist );
    virtual ~InputStorageWrapper_Impl();

    virtual sal_Int32   SAL_CALL    readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
};


InputStorageWrapper_Impl::InputStorageWrapper_Impl(
        SvPersist *pPersist )
{
    SvStream *pStream = 0;
    aTempFile.EnableKillingFile();
    pStream = aTempFile.GetStream( STREAM_READWRITE );
    SvStorageRef aTempStor = new SvStorage( sal_False, *pStream );
    if( pPersist->DoSaveAs( aTempStor ) )
    {
        aTempStor->Commit();
    }
    else
    {
        aTempStor.Clear();
        pStream = 0;
    }

    if( pStream )
    {
        pStream->Seek( 0 );
        xIn = new OInputStreamWrapper( *pStream );
    }
}

InputStorageWrapper_Impl::~InputStorageWrapper_Impl()
{
    xIn = 0;
    xStor = 0;
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
    xStor = 0;
}

// -----------------------------------------------------------------------------

class OutputStorageWrapper_Impl : public ::cppu::WeakImplHelper1<XOutputStream>
{
    ::osl::Mutex    maMutex;
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

OutputStorageWrapper_Impl::OutputStorageWrapper_Impl() :
    bStreamClosed( sal_False ),
    bCreateStorageFailed( sal_False )
{
    aTempFile.EnableKillingFile();
    SvStream *pStream = aTempFile.GetStream( STREAM_READWRITE );
    xOut = new OOutputStreamWrapper( *pStream );
}

OutputStorageWrapper_Impl::~OutputStorageWrapper_Impl()
{
    xStor = 0;
    xOut = 0;
}

SvStorage *OutputStorageWrapper_Impl::GetStorage()
{
    if( !xStor )
    {
        if( bStreamClosed && !bCreateStorageFailed )
        {
            xStor = new SvStorage( *aTempFile.GetStream( STREAM_READWRITE ) );
            if( xStor->GetError() != 0 )
            {
                xStor = 0;
                bCreateStorageFailed = sal_True;
            }
        }
    }

    return xStor;
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
    maDefaultContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    mpRootStorage( 0 ),
    mpDocPersist( 0 ),
    mpStreamMap( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
{
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( SvPersist& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    WeakComponentImplHelper2< XEmbeddedObjectResolver, XNameAccess >( maMutex ),
    maDefaultContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    mpRootStorage( 0 ),
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
        sal_Bool bInternalToExternal ) const
{
    // internal URL: vnd.sun.star.EmbeddedObject:<object-name>
    //           or: vnd.sun.star.EmbeddedObject:<path>/<object-name>
    // external URL: ./<path>/<object-name>
    //           or: <path>/<object-name>
    //           or: <object-name>
    // currently, path may only consist of a single directory name
    sal_Bool    bRet = sal_False;

    if( !rURLStr.getLength() )
        return sal_False;

    if( bInternalToExternal )
    {
        sal_Int32 nPos = rURLStr.indexOf( ':' );
        if( -1 == nPos ||
            0 != rURLStr.compareToAscii( XML_EMBEDDEDOBJECT_URL_BASE,
                                 sizeof( XML_EMBEDDEDOBJECT_URL_BASE ) -1 ) )
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

SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
        const OUString& rStorageName )
{
    DBG_ASSERT( -1 == rStorageName.indexOf( '/' ) &&
                -1 == rStorageName.indexOf( '\\' ),
                "nested embedded storages aren't supported" );
    if( !mxContainerStorage.Is() ||
        ( rStorageName != maCurContainerStorageName ) )
    {
        if( mxContainerStorage.Is() &&
            maCurContainerStorageName.getLength() > 0 &&
            EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode )
            mxContainerStorage->Commit();

        if( rStorageName.getLength() > 0 && mpRootStorage )
        {
            StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
                                    ? STREAM_STD_READWRITE
                                    : STREAM_STD_READ;
            mxContainerStorage = mpRootStorage->OpenStorage( rStorageName,
                                                             eMode );
        }
        else
        {
            mxContainerStorage = mpRootStorage;
        }
        maCurContainerStorageName = rStorageName;
    }

    return mxContainerStorage;
}

// -----------------------------------------------------------------------------

SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetObjectStorage(
        const OUString& rContainerStorageName,
        const OUString& rObjectStorageName,
        sal_Bool bUCBStorage )
{
    SvStorageRef xObjStor;
    SvStorageRef xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );
    if( xCntnrStor.Is() )
    {
        StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
                                ? STREAM_STD_READWRITE
                                : STREAM_STD_READ;
        xObjStor = xCntnrStor->OpenStorage( rObjectStorageName, eMode );
    }

    return xObjStor;
}

// -----------------------------------------------------------------------------

String SvXMLEmbeddedObjectHelper::ImplGetUniqueName( SvStorage* pStg,
                                              const sal_Char* p ) const
{
    String aP = String( ByteString( p ), RTL_TEXTENCODING_UTF8 );
    String aName;
    static ULONG nId = (ULONG)&aP;
    nId++;
    for( ;; )
    {
        aName = aP;
        sal_Int32 nBitsLeft = 32;
        while( nBitsLeft )
        {
            sal_uInt32 nNumb = ( nId << ( 32 - nBitsLeft ) ) >> 28;
            if ( nNumb > 9 )
                nNumb += 'A' - 10;
            else
                nNumb += '0';
            aName+=(sal_Unicode)((sal_Char)nNumb);
            nBitsLeft -= 4;
        }
        if( !pStg->IsContained( aName ) )
            break;
        nId++;
    }
    return aName;
}

sal_Bool SvXMLEmbeddedObjectHelper::ImplReadObject(
        const OUString& rContainerStorageName,
        OUString& rObjName,
        const SvGlobalName *pClassId,
        SvStorage *pTempStor )
{
    SvStorageRef xDocStor( mpDocPersist->GetStorage() );
    SvStorageRef xCntnrStor( ImplGetContainerStorage(
                                        rContainerStorageName ) );

    if( !xCntnrStor.Is() && !pTempStor )
        return sal_False;

    String aSrcObjName( rObjName );
    if( xDocStor != xCntnrStor || pTempStor )
    {

        // Is the object name unique?
        if( mpDocPersist->GetObjectList() )
        {
            sal_uInt32 nCount = mpDocPersist->GetObjectList()->Count();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                SvInfoObject* pTst = mpDocPersist->GetObjectList()->GetObject(i);
                // TODO: unicode: is this correct?
                if( rObjName.equalsIgnoreAsciiCase( pTst->GetObjName() ) ||
                    rObjName.equalsIgnoreAsciiCase( pTst->GetStorageName() ) )
                {
                    rObjName = ImplGetUniqueName( xDocStor, "Obj" );
                    break;
                }
            }
        }

        if( pTempStor )
        {
            SvStorageRef xDstStor = xDocStor->OpenOLEStorage( rObjName );
            if( !pTempStor->CopyTo( xDstStor ) )
                return sal_False;
            xDstStor->Commit();
        }
        else
        {
            if( !xCntnrStor->CopyTo( aSrcObjName, xDocStor, rObjName ) )
                return sal_False;
        }
    }

    SvGlobalName aClassId;
    if( pClassId )
    {
        // If a class id is specifies, use it.
        aClassId = *pClassId;
    }
    else
    {
        // Otherwise try to get one from the storage. For packages, the
        // class id is derived from the package's mime type. The mime type
        // is stored in the packages manifest and the manifest is read when
        // the stoage is opened. Therfor, the class id is available without
        // realy accessing the storage.
        SvStorageRef xObjStor;
        if( xDocStor == xCntnrStor )
            xObjStor = ImplGetObjectStorage( rContainerStorageName,
                                             rObjName, sal_False );
        else
            xObjStor = xDocStor->OpenStorage( rObjName, STREAM_STD_READ );
        DBG_ASSERT( xObjStor.Is(), "Couldn't open object storage" );
        if( xObjStor.Is() )
            aClassId = xObjStor->GetClassName();
    }

    // For all unkown class id, the OLE object has to be wrapped by an
    // outplace object.
    SvGlobalName aOutClassId( SO3_OUT_CLASSID );
    if( SvGlobalName() == aClassId ||
        ( aOutClassId != aClassId &&
          !SvFactory::IsIntern( aClassId, 0 ) ) )
        aClassId = SvGlobalName( aOutClassId );

    SvInfoObjectRef xInfo = new SvEmbeddedInfoObject( rObjName, aClassId );
    mpDocPersist->Insert( xInfo );

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
        ImplReadObject( aContainerStorageName, aObjectStorageName, pClassId,
                         pOut ? pOut->GetStorage() : 0 );
        sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_EMBEDDEDOBJECT_URL_BASE     ) );
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
        SvStorage *pRootStorage,
        SvPersist& rPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    mpRootStorage = pRootStorage;
    mpDocPersist = &rPersist;
    meCreateMode = eCreateMode;
}

// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        SvStorage& rRootStorage,
        SvPersist& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode,
        sal_Bool bDirect )
{
    SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;

    pThis->acquire();
    pThis->Init( &rRootStorage, rDocPersist, eCreateMode );

    return pThis;
}

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        SvPersist& rDocPersist,
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

// XGraphicObjectResolver
OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL( const OUString& aURL )
    throw(RuntimeException)
{
    MutexGuard          aGuard( maMutex );

    return ImplInsertEmbeddedObjectURL( aURL );
}

// XNameAccess
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
        Reference < XInputStream > xStrm;
        OUString aContainerStorageName, aObjectStorageName;
        if( ImplGetStorageNames( rURLStr, aContainerStorageName,
                                 aObjectStorageName,
                                 sal_True ) )
        {
            SvPersistRef xObj = mpDocPersist->GetObject( aObjectStorageName );
            if( xObj.Is() )
                xStrm = new InputStorageWrapper_Impl( xObj );
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

        return aObjectStorageName.getLength() > 0 &&
               mpDocPersist->Find( aObjectStorageName );
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
        return mpDocPersist->GetObjectList() != 0 &&
               mpDocPersist->GetObjectList()->Count() > 0;
    }
}
