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


#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <osl/diagnose.h>
#include <sot/storage.hxx>
#include <tools/debug.hxx>
#include <sal/log.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>

#include <svtools/embedhlp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>

#include <comphelper/fileformat.h>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <svx/xmleohlp.hxx>
#include <map>
#include <memory>
#include <mutex>

using namespace ::cppu;
using namespace ::utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

constexpr OUStringLiteral XML_CONTAINERSTORAGE_NAME_60 = u"Pictures";
constexpr OUStringLiteral XML_CONTAINERSTORAGE_NAME = u"ObjectReplacements";
constexpr OUString XML_EMBEDDEDOBJECT_URL_BASE = u"vnd.sun.star.EmbeddedObject:"_ustr;
constexpr OUStringLiteral XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE = u"vnd.sun.star.GraphicObject:";


class OutputStorageWrapper_Impl : public ::cppu::WeakImplHelper<XOutputStream>
{
    std::mutex    maMutex;
    Reference < XOutputStream > xOut;
    TempFileFast aTempFile;
    bool bStreamClosed : 1;
    SvStream* pStream;

public:
    OutputStorageWrapper_Impl();

// css::io::XOutputStream
    virtual void SAL_CALL writeBytes(const Sequence< sal_Int8 >& aData) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;

    SvStream*   GetStream();
};

OutputStorageWrapper_Impl::OutputStorageWrapper_Impl()
    : bStreamClosed( false )
    , pStream(nullptr)
{
    pStream = aTempFile.GetStream( StreamMode::READWRITE );
    xOut = new OOutputStreamWrapper( *pStream );
}

SvStream *OutputStorageWrapper_Impl::GetStream()
{
    if( bStreamClosed )
        return pStream;
    return nullptr;
}

void SAL_CALL OutputStorageWrapper_Impl::writeBytes(
        const Sequence< sal_Int8 >& aData)
{
    std::scoped_lock          aGuard( maMutex );
    xOut->writeBytes( aData );
}

void SAL_CALL OutputStorageWrapper_Impl::flush()
{
    std::scoped_lock          aGuard( maMutex );
    xOut->flush();
}

void SAL_CALL OutputStorageWrapper_Impl::closeOutput()
{
    std::scoped_lock          aGuard( maMutex );
    xOut->closeOutput();
    bStreamClosed = true;
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
    mpDocPersist( nullptr ),
    meCreateMode( SvXMLEmbeddedObjectHelperMode::Read )
{
}

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper( ::comphelper::IEmbeddedHelper& rDocPersist, SvXMLEmbeddedObjectHelperMode eCreateMode ) :
    mpDocPersist( nullptr ),
    meCreateMode( SvXMLEmbeddedObjectHelperMode::Read )
{
    Init( nullptr, rDocPersist, eCreateMode );
}

SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
{
}

void SvXMLEmbeddedObjectHelper::disposing(std::unique_lock<std::mutex>&)
{
    if( mxTempStorage.is() )
    {
        mxTempStorage->dispose();
        mxTempStorage.clear();
    }
}

void SvXMLEmbeddedObjectHelper::splitObjectURL(const OUString& _aURLNoPar,
    OUString& rContainerStorageName,
    OUString& rObjectStorageName)
{
    DBG_ASSERT(_aURLNoPar.isEmpty() || '#' != _aURLNoPar[0], "invalid object URL" );
    OUString aURLNoPar = _aURLNoPar;

    sal_Int32 _nPos = aURLNoPar.lastIndexOf( '/' );
    if( -1 == _nPos )
    {
        rContainerStorageName.clear();
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

bool SvXMLEmbeddedObjectHelper::ImplGetStorageNames(
        const OUString& rURLStr,
        OUString& rContainerStorageName,
        OUString& rObjectStorageName,
        bool bInternalToExternal,
        bool *pGraphicRepl,
        bool *pOasisFormat ) const
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
        *pGraphicRepl = false;

    if( pOasisFormat )
        *pOasisFormat = true; // the default value

    if( rURLStr.isEmpty() )
        return false;

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
            if ( aToken.equalsIgnoreAsciiCase( "oasis=false" ) )
            {
                if ( pOasisFormat )
                    *pOasisFormat = false;
                break;
            }
            else
            {
                SAL_WARN( "svx", "invalid arguments was found in URL!" );
            }
        }
    }

    if( bInternalToExternal )
    {
        nPos = aURLNoPar.indexOf( ':' );
        if( -1 == nPos )
            return false;
        bool bObjUrl = aURLNoPar.startsWith( XML_EMBEDDEDOBJECT_URL_BASE );
        bool bGrUrl = !bObjUrl &&
              aURLNoPar.startsWith( XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE );
        if( !(bObjUrl || bGrUrl) )
            return false;

        sal_Int32 nPathStart = nPos + 1;
        nPos = aURLNoPar.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName.clear();
            rObjectStorageName = aURLNoPar.copy( nPathStart );
        }
        else if( nPos > nPathStart )
        {
            rContainerStorageName = aURLNoPar.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = aURLNoPar.copy( nPos+1 );
        }
        else
            return false;

        if( bGrUrl )
        {
            bool bOASIS = mxRootStorage.is() &&
                ( SotStorage::GetVersion( mxRootStorage ) > SOFFICE_FILEFORMAT_60 );
            if (bOASIS)
                rContainerStorageName = XML_CONTAINERSTORAGE_NAME;
            else
                rContainerStorageName = XML_CONTAINERSTORAGE_NAME_60;

            if( pGraphicRepl )
                *pGraphicRepl = true;
        }


    }
    else
    {
        splitObjectURL(aURLNoPar, rContainerStorageName, rObjectStorageName);
    }

    if( -1 != rContainerStorageName.indexOf( '/' ) )
    {
        OSL_FAIL( "SvXMLEmbeddedObjectHelper: invalid path name" );
        return false;
    }

    return true;
}

uno::Reference < embed::XStorage > const & SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
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
            SvXMLEmbeddedObjectHelperMode::Write == meCreateMode )
        {
            uno::Reference < embed::XTransactedObject > xTrans( mxContainerStorage, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
        }

        if( !rStorageName.isEmpty() && mxRootStorage.is() )
        {
            sal_Int32 nMode = SvXMLEmbeddedObjectHelperMode::Write == meCreateMode
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

void SvXMLEmbeddedObjectHelper::ImplReadObject(
        const OUString& rContainerStorageName,
        OUString& rObjName,
        const SvGlobalName *, // pClassId, see "TODO/LATER" below
        SvStream* pTemp )
{
    uno::Reference < embed::XStorage > xDocStor( mpDocPersist->getStorage() );
    uno::Reference < embed::XStorage > xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );

    if( !xCntnrStor.is() && !pTemp )
        return;

    OUString aSrcObjName( rObjName );
    comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();

    // Is the object name unique?
    // if the object is already instantiated by GetEmbeddedObject
    // that means that the duplication is being loaded
    bool bDuplicate = rContainer.HasInstantiatedEmbeddedObject( rObjName );
    DBG_ASSERT( !bDuplicate, "An object in the document is referenced twice!" );

    if( xDocStor != xCntnrStor || pTemp || bDuplicate )
    {
        // TODO/LATER: make this altogether a method in the EmbeddedObjectContainer

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
                std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream( xStm ));
                pTemp->ReadStream( *pStream );
                pStream.reset();

                // TODO/LATER: what to do when other types of objects are based on substream persistence?
                // This is an ole object
                uno::Reference< beans::XPropertySet > xProps( xStm, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    "MediaType",
                    uno::Any( OUString( "application/vnd.sun.star.oleobject" ) ) );

                xStm->getOutputStream()->closeOutput();
            }
            catch ( uno::Exception& )
            {
                return;
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
                return;
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
    OUString const baseURL(mpDocPersist->getDocumentBaseURL());
    rContainer.GetEmbeddedObject(aName, &baseURL);
}

OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
        const OUString& rURLStr )
{
    OUString sRetURL;

    OUString aContainerStorageName, aObjectStorageName;
    if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                              aObjectStorageName,
                              SvXMLEmbeddedObjectHelperMode::Write == meCreateMode ) )
        return sRetURL;

    if( SvXMLEmbeddedObjectHelperMode::Read == meCreateMode )
    {
        OutputStorageWrapper_Impl *pOut = nullptr;
        std::map< OUString, rtl::Reference<OutputStorageWrapper_Impl> >::iterator aIter;

        if( mxStreamMap )
        {
            aIter = mxStreamMap->find( rURLStr );
            if( aIter != mxStreamMap->end() && aIter->second.is() )
                pOut = aIter->second.get();
        }

        SvGlobalName aClassId, *pClassId = nullptr;
        sal_Int32 nPos = aObjectStorageName.lastIndexOf( '!' );
        if( -1 != nPos && aClassId.MakeId( aObjectStorageName.subView( nPos+1 ) ) )
        {
            aObjectStorageName = aObjectStorageName.copy( 0, nPos );
            pClassId = &aClassId;
        }

        ImplReadObject( aContainerStorageName, aObjectStorageName, pClassId, pOut ? pOut->GetStream() : nullptr );
        sRetURL = XML_EMBEDDEDOBJECT_URL_BASE + aObjectStorageName;

        if( pOut )
        {
            mxStreamMap->erase( aIter );
        }
    }
    else
    {
        // Objects are written using ::comphelper::IEmbeddedHelper::SaveAs
        sRetURL = "./";
        if( !aContainerStorageName.isEmpty() )
        {
            sRetURL += aContainerStorageName + "/";
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

rtl::Reference<SvXMLEmbeddedObjectHelper> SvXMLEmbeddedObjectHelper::Create(
        const uno::Reference < embed::XStorage >& rRootStorage,
        ::comphelper::IEmbeddedHelper& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    rtl::Reference<SvXMLEmbeddedObjectHelper> pThis(new SvXMLEmbeddedObjectHelper);

    pThis->Init( rRootStorage, rDocPersist, eCreateMode );

    return pThis;
}

rtl::Reference<SvXMLEmbeddedObjectHelper> SvXMLEmbeddedObjectHelper::Create(
        ::comphelper::IEmbeddedHelper& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode )
{
    rtl::Reference<SvXMLEmbeddedObjectHelper> pThis(new SvXMLEmbeddedObjectHelper);

    pThis->Init( nullptr, rDocPersist, eCreateMode );

    return pThis;
}

OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL(const OUString& rURL)
{
    std::unique_lock          aGuard( m_aMutex );

    OUString sRet;
    try
    {
        sRet = ImplInsertEmbeddedObjectURL(rURL);
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetRuntimeException(
            "SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL non-RuntimeException",
            getXWeak(), anyEx);
    }
    return sRet;
}

// XNameAccess: alien objects!
Any SAL_CALL SvXMLEmbeddedObjectHelper::getByName(
        const OUString& rURLStr )
{
    std::unique_lock          aGuard( m_aMutex );
    Any aRet;
    if( SvXMLEmbeddedObjectHelperMode::Read == meCreateMode )
    {
        Reference < XOutputStream > xStrm;
        if( mxStreamMap )
        {
            auto aIter = mxStreamMap->find( rURLStr );
            if( aIter != mxStreamMap->end() && aIter->second.is() )
                xStrm = aIter->second.get();
        }
        if( !xStrm.is() )
        {
            rtl::Reference<OutputStorageWrapper_Impl> xOut = new OutputStorageWrapper_Impl;
            if( !mxStreamMap )
                mxStreamMap.emplace();
            (*mxStreamMap)[rURLStr] = xOut;
            xStrm = xOut.get();
        }

        aRet <<= xStrm;
    }
    else
    {
        bool bGraphicRepl = false;
        bool bOasisFormat = true;
        Reference < XInputStream > xStrm;
        OUString aContainerStorageName, aObjectStorageName;
        if( ImplGetStorageNames( rURLStr, aContainerStorageName,
                                 aObjectStorageName,
                                 true,
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
                            Sequence < beans::PropertyValue > aDummy,
                                aEmbDescr{ comphelper::makePropertyValue("StoreVisualReplacement",
                                                                         !bOasisFormat) };
                            if ( !bOasisFormat )
                            {
                                uno::Reference< io::XInputStream > xGrInStream = ImplGetReplacementImage( xObj );
                                if ( xGrInStream.is() )
                                {
                                    aEmbDescr.realloc( 2 );
                                    auto pEmbDescr = aEmbDescr.getArray();
                                    pEmbDescr[1].Name = "VisualReplacement";
                                    pEmbDescr[1].Value <<= xGrInStream;
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
{
   return {};
}

sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasByName( const OUString& rURLStr )
{
    std::unique_lock          aGuard( m_aMutex );
    if( SvXMLEmbeddedObjectHelperMode::Read == meCreateMode )
    {
        return true;
    }
    else
    {
        OUString aContainerStorageName, aObjectStorageName;
        if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                                  aObjectStorageName,
                                  true ) )
            return false;

        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();
        return !aObjectStorageName.isEmpty() &&
               rContainer.HasEmbeddedObject( aObjectStorageName );
    }
}

// XNameAccess
Type SAL_CALL SvXMLEmbeddedObjectHelper::getElementType()
{
    std::unique_lock          aGuard( m_aMutex );
    if( SvXMLEmbeddedObjectHelperMode::Read == meCreateMode )
        return cppu::UnoType<XOutputStream>::get();
    else
        return cppu::UnoType<XInputStream>::get();
}

sal_Bool SAL_CALL SvXMLEmbeddedObjectHelper::hasElements()
{
    std::unique_lock          aGuard( m_aMutex );
    if( SvXMLEmbeddedObjectHelperMode::Read == meCreateMode )
    {
        return true;
    }
    else
    {
        comphelper::EmbeddedObjectContainer& rContainer = mpDocPersist->getEmbeddedObjectContainer();
        return rContainer.HasEmbeddedObjects();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
