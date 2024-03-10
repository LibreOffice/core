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

#include <sal/config.h>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/mediadescriptor.hxx>
#include <osl/diagnose.h>
#include <rtl/uri.hxx>
#include <memory>
#include <mutex>
#include <set>

#include <oox/core/filterbase.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/ole/oleobjecthelper.hxx>
#include <oox/ole/vbaproject.hxx>

namespace oox::core {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::XNameAccess;
using utl::MediaDescriptor;
using ::comphelper::SequenceAsHashMap;
using ::oox::ole::OleObjectHelper;
using ::oox::ole::VbaProject;

namespace {

struct UrlPool
{
    std::mutex             maMutex;
    ::std::set< OUString > maUrls;
};

UrlPool& StaticUrlPool()
{
    static UrlPool SINGLETON;
    return SINGLETON;
}

/** This guard prevents recursive loading/saving of the same document. */
class DocumentOpenedGuard
{
public:
    explicit            DocumentOpenedGuard( const OUString& rUrl );
                        ~DocumentOpenedGuard();
                        DocumentOpenedGuard(const DocumentOpenedGuard&) = delete;
    DocumentOpenedGuard& operator=(const DocumentOpenedGuard&) = delete;

    bool         isValid() const { return mbValid; }

private:
    OUString            maUrl;
    bool                mbValid;
};

DocumentOpenedGuard::DocumentOpenedGuard( const OUString& rUrl )
{
    UrlPool& rUrlPool = StaticUrlPool();
    std::scoped_lock aGuard( rUrlPool.maMutex );
    mbValid = rUrl.isEmpty() || (rUrlPool.maUrls.count( rUrl ) == 0);
    if( mbValid && !rUrl.isEmpty() )
    {
        rUrlPool.maUrls.insert( rUrl );
        maUrl = rUrl;
    }
}

DocumentOpenedGuard::~DocumentOpenedGuard()
{
    UrlPool& rUrlPool = StaticUrlPool();
    std::scoped_lock aGuard( rUrlPool.maMutex );
    if( !maUrl.isEmpty() )
        rUrlPool.maUrls.erase( maUrl );
}

/** Specifies whether this filter is an import or export filter. */
enum FilterDirection
{
    FILTERDIRECTION_UNKNOWN,
    FILTERDIRECTION_IMPORT,
    FILTERDIRECTION_EXPORT
};

} // namespace

struct FilterBaseImpl
{
    typedef std::shared_ptr< GraphicHelper >        GraphicHelperRef;
    typedef std::shared_ptr< ModelObjectHelper >    ModelObjHelperRef;
    typedef std::shared_ptr< OleObjectHelper >      OleObjHelperRef;
    typedef std::shared_ptr< VbaProject >           VbaProjectRef;

    FilterDirection     meDirection;
    SequenceAsHashMap   maArguments;
    SequenceAsHashMap   maFilterData;
    MediaDescriptor     maMediaDesc;
    OUString            maFileUrl;
    StorageRef          mxStorage;
    OoxmlVersion        meVersion;

    GraphicHelperRef    mxGraphicHelper;        /// Graphic and graphic object handling.
    ModelObjHelperRef   mxModelObjHelper;       /// Tables to create new named drawing objects.
    std::map<css::uno::Reference<css::lang::XMultiServiceFactory>, ModelObjHelperRef>
        mxModelObjHelpers;
    OleObjHelperRef     mxOleObjHelper;         /// OLE object handling.
    VbaProjectRef       mxVbaProject;           /// VBA project manager.

    Reference< XComponentContext >      mxComponentContext;
    Reference< XModel >                 mxModel;
    Reference< XMultiServiceFactory >   mxModelFactory;
    Reference< XFrame >                 mxTargetFrame;
    Reference< XInputStream >           mxInStream;
    Reference< XStream >                mxOutStream;
    Reference< XStatusIndicator >       mxStatusIndicator;
    Reference< XInteractionHandler >    mxInteractionHandler;
    Reference< XShape >                 mxParentShape;

    bool mbExportVBA;

    bool mbExportTemplate;

    /// @throws RuntimeException
    explicit            FilterBaseImpl( const Reference< XComponentContext >& rxContext );

    /// @throws IllegalArgumentException
    void                setDocumentModel( const Reference< XComponent >& rxComponent );
};

FilterBaseImpl::FilterBaseImpl( const Reference< XComponentContext >& rxContext ) :
    meDirection( FILTERDIRECTION_UNKNOWN ),
    meVersion(ECMA_376_1ST_EDITION),
    mxComponentContext( rxContext, UNO_SET_THROW ),
    mbExportVBA(false),
    mbExportTemplate(false)
{
}

void FilterBaseImpl::setDocumentModel( const Reference< XComponent >& rxComponent )
{
    try
    {
        mxModel.set( rxComponent, UNO_QUERY_THROW );
        mxModelFactory.set( rxComponent, UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
        throw IllegalArgumentException();
    }
}

FilterBase::FilterBase( const Reference< XComponentContext >& rxContext ) :
    mxImpl( new FilterBaseImpl( rxContext ) )
{
}

FilterBase::~FilterBase()
{
}

bool FilterBase::isImportFilter() const
{
    return mxImpl->meDirection == FILTERDIRECTION_IMPORT;
}

bool FilterBase::isExportFilter() const
{
    return mxImpl->meDirection == FILTERDIRECTION_EXPORT;
}

OoxmlVersion FilterBase::getVersion() const
{
    return mxImpl->meVersion;
}

const Reference< XComponentContext >& FilterBase::getComponentContext() const
{
    return mxImpl->mxComponentContext;
}

const Reference< XModel >& FilterBase::getModel() const
{
    return mxImpl->mxModel;
}

const Reference< XMultiServiceFactory >& FilterBase::getModelFactory() const
{
    return mxImpl->mxModelFactory;
}

const Reference< XFrame >& FilterBase::getTargetFrame() const
{
    return mxImpl->mxTargetFrame;
}

const Reference< XStatusIndicator >& FilterBase::getStatusIndicator() const
{
    return mxImpl->mxStatusIndicator;
}

MediaDescriptor& FilterBase::getMediaDescriptor() const
{
    return mxImpl->maMediaDesc;
}

SequenceAsHashMap& FilterBase::getFilterData() const
{
    return mxImpl->maFilterData;
}

const OUString& FilterBase::getFileUrl() const
{
    return mxImpl->maFileUrl;
}

namespace {

bool lclIsDosDrive( std::u16string_view rUrl, size_t nPos = 0 )
{
    return
        (rUrl.size() >= nPos + 3) &&
        ((('A' <= rUrl[ nPos ]) && (rUrl[ nPos ] <= 'Z')) || (('a' <= rUrl[ nPos ]) && (rUrl[ nPos ] <= 'z'))) &&
        (rUrl[ nPos + 1 ] == ':') &&
        (rUrl[ nPos + 2 ] == '/');
}

} // namespace

OUString FilterBase::getAbsoluteUrl( const OUString& rUrl ) const
{
    // handle some special cases before calling ::rtl::Uri::convertRelToAbs()

    static constexpr OUString aFileSchema = u"file:"_ustr;
    static constexpr OUString aFilePrefix = u"file:///"_ustr;
    const sal_Int32 nFilePrefixLen = aFilePrefix.getLength();
    static constexpr OUString aUncPrefix = u"//"_ustr;

    /*  (1) convert all backslashes to slashes, and check that passed URL is
        not empty. */
    OUString aUrl = rUrl.replace( '\\', '/' );
    if( aUrl.isEmpty() )
        return aUrl;

    /*  (2) add 'file:///' to absolute Windows paths, e.g. convert
        'C:/path/file' to 'file:///c:/path/file'. */
    if( lclIsDosDrive( aUrl ) )
        return aFilePrefix + aUrl;

    /*  (3) add 'file:' to UNC paths, e.g. convert '//server/path/file' to
        'file://server/path/file'. */
    if( aUrl.match( aUncPrefix ) )
        return aFileSchema + aUrl;

    /*  (4) remove additional slashes from UNC paths, e.g. convert
        'file://///server/path/file' to 'file://server/path/file'. */
    if( (aUrl.getLength() >= nFilePrefixLen + 2) &&
        aUrl.match( aFilePrefix ) &&
        aUrl.match( aUncPrefix, nFilePrefixLen ) )
    {
        return aFileSchema + aUrl.subView( nFilePrefixLen );
    }

    /*  (5) handle URLs relative to current drive, e.g. the URL '/path1/file1'
        relative to the base URL 'file:///C:/path2/file2' does not result in
        the expected 'file:///C:/path1/file1', but in 'file:///path1/file1'. */
    if( aUrl.startsWith("/") &&
        mxImpl->maFileUrl.match( aFilePrefix ) &&
        lclIsDosDrive( mxImpl->maFileUrl, nFilePrefixLen ) )
    {
        return OUString::Concat(mxImpl->maFileUrl.subView( 0, nFilePrefixLen + 3 )) + aUrl.subView( 1 );
    }

    try
    {
        return ::rtl::Uri::convertRelToAbs( mxImpl->maFileUrl, aUrl );
    }
    catch( ::rtl::MalformedUriException& )
    {
    }
    return aUrl;
}

StorageRef const & FilterBase::getStorage() const
{
    return mxImpl->mxStorage;
}

Reference< XInputStream > FilterBase::openInputStream( const OUString& rStreamName ) const
{
    if (!mxImpl->mxStorage)
        throw RuntimeException();
    return mxImpl->mxStorage->openInputStream( rStreamName );
}

Reference< XOutputStream > FilterBase::openOutputStream( const OUString& rStreamName ) const
{
    return mxImpl->mxStorage->openOutputStream( rStreamName );
}

void FilterBase::commitStorage() const
{
    mxImpl->mxStorage->commit();
}

// helpers

GraphicHelper& FilterBase::getGraphicHelper() const
{
    if( !mxImpl->mxGraphicHelper )
        mxImpl->mxGraphicHelper.reset( implCreateGraphicHelper() );
    return *mxImpl->mxGraphicHelper;
}

ModelObjectHelper& FilterBase::getModelObjectHelper() const
{
    if( !mxImpl->mxModelObjHelper )
        mxImpl->mxModelObjHelper = std::make_shared<ModelObjectHelper>( mxImpl->mxModelFactory );
    return *mxImpl->mxModelObjHelper;
}

ModelObjectHelper& FilterBase::getModelObjectHelperForModel(
    const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory) const
{
    if (!mxImpl->mxModelObjHelpers.count(xFactory))
        mxImpl->mxModelObjHelpers[xFactory] = std::make_shared<ModelObjectHelper>(xFactory);
    return *mxImpl->mxModelObjHelpers[xFactory];
}

OleObjectHelper& FilterBase::getOleObjectHelper() const
{
    if( !mxImpl->mxOleObjHelper )
        mxImpl->mxOleObjHelper = std::make_shared<OleObjectHelper>(mxImpl->mxModelFactory, mxImpl->mxModel);
    return *mxImpl->mxOleObjHelper;
}

VbaProject& FilterBase::getVbaProject() const
{
    if( !mxImpl->mxVbaProject )
        mxImpl->mxVbaProject.reset( implCreateVbaProject() );
    return *mxImpl->mxVbaProject;
}

bool FilterBase::importBinaryData( StreamDataSequence & orDataSeq, const OUString& rStreamName )
{
    OSL_ENSURE( !rStreamName.isEmpty(), "FilterBase::importBinaryData - empty stream name" );
    if( rStreamName.isEmpty() )
        return false;

    // try to open the stream (this may fail - do not assert)
    Reference<XInputStream> xInStream = openInputStream( rStreamName );
    if (!xInStream)
        return false;

    // copy the entire stream to the passed sequence
    sal_Int32 nBytesRead = xInStream->readBytes( orDataSeq, SAL_MAX_INT32);
    return nBytesRead != -1 && nBytesRead != 0;
}

// com.sun.star.lang.XServiceInfo interface

sal_Bool SAL_CALL FilterBase::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL FilterBase::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExportFilter" };
}

// com.sun.star.lang.XInitialization interface

void SAL_CALL FilterBase::initialize( const Sequence< Any >& rArgs )
{
    if( rArgs.getLength() >= 2 ) try
    {
        mxImpl->maArguments << rArgs[ 1 ];
    }
    catch( Exception& )
    {
    }

    if (!rArgs.hasElements())
        return;

    Sequence<css::beans::PropertyValue> aSeq;
    rArgs[0] >>= aSeq;
    for (const auto& rVal : aSeq)
    {
        if (rVal.Name == "UserData")
        {
            css::uno::Sequence<OUString> aUserDataSeq;
            rVal.Value >>= aUserDataSeq;
            if (comphelper::findValue(aUserDataSeq, "macro-enabled") != -1)
                mxImpl->mbExportVBA = true;
        }
        else if (rVal.Name == "Flags")
        {
            sal_Int32 nFlags(0);
            rVal.Value >>= nFlags;
            mxImpl->mbExportTemplate = bool(static_cast<SfxFilterFlags>(nFlags) & SfxFilterFlags::TEMPLATE);
        }
    }
}

// com.sun.star.document.XImporter interface

void SAL_CALL FilterBase::setTargetDocument( const Reference< XComponent >& rxDocument )
{
    mxImpl->setDocumentModel( rxDocument );
    mxImpl->meDirection = FILTERDIRECTION_IMPORT;
}

// com.sun.star.document.XExporter interface

void SAL_CALL FilterBase::setSourceDocument( const Reference< XComponent >& rxDocument )
{
    mxImpl->setDocumentModel( rxDocument );
    mxImpl->meDirection = FILTERDIRECTION_EXPORT;
}

// com.sun.star.document.XFilter interface

sal_Bool SAL_CALL FilterBase::filter( const Sequence< PropertyValue >& rMediaDescSeq )
{
    if( !mxImpl->mxModel.is() || !mxImpl->mxModelFactory.is() || (mxImpl->meDirection == FILTERDIRECTION_UNKNOWN) )
        throw RuntimeException();

    bool bRet = false;
    setMediaDescriptor( rMediaDescSeq );
    DocumentOpenedGuard aOpenedGuard( mxImpl->maFileUrl );
    if( aOpenedGuard.isValid() || mxImpl->maFileUrl.isEmpty() )
    {
        Reference<XModel> xTempModel = mxImpl->mxModel;
        xTempModel->lockControllers();
        comphelper::ScopeGuard const lockControllersGuard([xTempModel]() {
            xTempModel->unlockControllers();
        });

        switch( mxImpl->meDirection )
        {
            case FILTERDIRECTION_UNKNOWN:
            break;
            case FILTERDIRECTION_IMPORT:
                if( mxImpl->mxInStream.is() )
                {
                    mxImpl->mxStorage = implCreateStorage( mxImpl->mxInStream );
                    bRet = mxImpl->mxStorage && importDocument();
                }
            break;
            case FILTERDIRECTION_EXPORT:
                if( mxImpl->mxOutStream.is() )
                {
                    mxImpl->mxStorage = implCreateStorage( mxImpl->mxOutStream );
                    bRet = mxImpl->mxStorage && exportDocument() && implFinalizeExport( getMediaDescriptor() );
                }
            break;
        }
    }
    return bRet;
}

void SAL_CALL FilterBase::cancel()
{
}

// protected

Reference< XInputStream > FilterBase::implGetInputStream( MediaDescriptor& rMediaDesc ) const
{
    return rMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INPUTSTREAM, Reference< XInputStream >() );
}

Reference< XStream > FilterBase::implGetOutputStream( MediaDescriptor& rMediaDesc ) const
{
    return rMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STREAMFOROUTPUT, Reference< XStream >() );
}

bool FilterBase::implFinalizeExport( MediaDescriptor& /*rMediaDescriptor*/ )
{
    return true;
}

Reference< XStream > const & FilterBase::getMainDocumentStream( ) const
{
    return mxImpl->mxOutStream;
}

// private

void FilterBase::setMediaDescriptor( const Sequence< PropertyValue >& rMediaDescSeq )
{
    mxImpl->maMediaDesc << rMediaDescSeq;

    switch( mxImpl->meDirection )
    {
        case FILTERDIRECTION_UNKNOWN:
            OSL_FAIL( "FilterBase::setMediaDescriptor - invalid filter direction" );
        break;
        case FILTERDIRECTION_IMPORT:
            mxImpl->maMediaDesc.addInputStream();
            mxImpl->mxInStream = implGetInputStream( mxImpl->maMediaDesc );
            OSL_ENSURE( mxImpl->mxInStream.is(), "FilterBase::setMediaDescriptor - missing input stream" );
        break;
        case FILTERDIRECTION_EXPORT:
            mxImpl->mxOutStream = implGetOutputStream( mxImpl->maMediaDesc );
            OSL_ENSURE( mxImpl->mxOutStream.is(), "FilterBase::setMediaDescriptor - missing output stream" );
        break;
    }

    mxImpl->maFileUrl = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL, OUString() );
    mxImpl->mxTargetFrame = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_FRAME, Reference< XFrame >() );
    mxImpl->mxStatusIndicator = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STATUSINDICATOR, Reference< XStatusIndicator >() );
    mxImpl->mxInteractionHandler = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INTERACTIONHANDLER, Reference< XInteractionHandler >() );
    mxImpl->mxParentShape = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "ParentShape", mxImpl->mxParentShape );
    mxImpl->maFilterData = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "FilterData", Sequence< PropertyValue >() );

    // Check for ISO OOXML
    OUString sFilterName = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "FilterName", OUString() );
    try
    {
        Reference<XMultiServiceFactory> xFactory(getComponentContext()->getServiceManager(), UNO_QUERY_THROW);
        Reference<XNameAccess> xFilters(xFactory->createInstance("com.sun.star.document.FilterFactory" ), UNO_QUERY_THROW );
        Any aValues = xFilters->getByName( sFilterName );
        Sequence<PropertyValue > aPropSeq;
        aValues >>= aPropSeq;
        SequenceAsHashMap aProps( aPropSeq );

        sal_Int32 nVersion = aProps.getUnpackedValueOrDefault( "FileFormatVersion", sal_Int32( 0 ) );
        mxImpl->meVersion = OoxmlVersion( nVersion );
    }
    catch ( const Exception& )
    {
        // Not ISO OOXML
    }
}

GraphicHelper* FilterBase::implCreateGraphicHelper() const
{
    // default: return base implementation without any special behaviour
    return new GraphicHelper( mxImpl->mxComponentContext, mxImpl->mxTargetFrame, mxImpl->mxStorage );
}

bool FilterBase::exportVBA() const
{
    return mxImpl->mbExportVBA;
}

bool FilterBase::isExportTemplate() const
{
    return mxImpl->mbExportTemplate;
}

} // namespace oox::core

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
