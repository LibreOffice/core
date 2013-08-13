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

#include "oox/core/filterbase.hxx"

#include <set>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <rtl/uri.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/modelobjecthelper.hxx"
#include "oox/ole/oleobjecthelper.hxx"
#include "oox/ole/vbaproject.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::XNameAccess;
using ::comphelper::MediaDescriptor;
using ::comphelper::SequenceAsHashMap;
using ::oox::ole::OleObjectHelper;
using ::oox::ole::VbaProject;

// ============================================================================

namespace {

struct UrlPool
{
    ::osl::Mutex        maMutex;
    ::std::set< OUString > maUrls;
};

struct StaticUrlPool : public ::rtl::Static< UrlPool, StaticUrlPool > {};

// ----------------------------------------------------------------------------

/** This guard prevents recursive loading/saving of the same document. */
class DocumentOpenedGuard
{
public:
    explicit            DocumentOpenedGuard( const OUString& rUrl );
                        ~DocumentOpenedGuard();

    inline bool         isValid() const { return mbValid; }

private:
                        DocumentOpenedGuard( const DocumentOpenedGuard& );
    DocumentOpenedGuard& operator=( const DocumentOpenedGuard& );

    OUString            maUrl;
    bool                mbValid;
};

DocumentOpenedGuard::DocumentOpenedGuard( const OUString& rUrl )
{
    UrlPool& rUrlPool = StaticUrlPool::get();
    ::osl::MutexGuard aGuard( rUrlPool.maMutex );
    mbValid = rUrl.isEmpty() || (rUrlPool.maUrls.count( rUrl ) == 0);
    if( mbValid && !rUrl.isEmpty() )
    {
        rUrlPool.maUrls.insert( rUrl );
        maUrl = rUrl;
    }
}

DocumentOpenedGuard::~DocumentOpenedGuard()
{
    UrlPool& rUrlPool = StaticUrlPool::get();
    ::osl::MutexGuard aGuard( rUrlPool.maMutex );
    if( !maUrl.isEmpty() )
        rUrlPool.maUrls.erase( maUrl );
}

} // namespace

// ============================================================================

/** Specifies whether this filter is an import or export filter. */
enum FilterDirection
{
    FILTERDIRECTION_UNKNOWN,
    FILTERDIRECTION_IMPORT,
    FILTERDIRECTION_EXPORT
};

// ----------------------------------------------------------------------------

struct FilterBaseImpl
{
    typedef ::boost::shared_ptr< GraphicHelper >        GraphicHelperRef;
    typedef ::boost::shared_ptr< ModelObjectHelper >    ModelObjHelperRef;
    typedef ::boost::shared_ptr< OleObjectHelper >      OleObjHelperRef;
    typedef ::boost::shared_ptr< VbaProject >           VbaProjectRef;

    FilterDirection     meDirection;
    SequenceAsHashMap   maArguments;
    SequenceAsHashMap   maFilterData;
    MediaDescriptor     maMediaDesc;
    OUString            maFileUrl;
    StorageRef          mxStorage;
    OoxmlVersion        meVersion;

    GraphicHelperRef    mxGraphicHelper;        /// Graphic and graphic object handling.
    ModelObjHelperRef   mxModelObjHelper;       /// Tables to create new named drawing objects.
    OleObjHelperRef     mxOleObjHelper;         /// OLE object handling.
    VbaProjectRef       mxVbaProject;           /// VBA project manager.

    Reference< XComponentContext >      mxComponentContext;
    Reference< XMultiComponentFactory > mxComponentFactory;
    Reference< XModel >                 mxModel;
    Reference< XMultiServiceFactory >   mxModelFactory;
    Reference< XFrame >                 mxTargetFrame;
    Reference< XInputStream >           mxInStream;
    Reference< XStream >                mxOutStream;
    Reference< XStatusIndicator >       mxStatusIndicator;
    Reference< XInteractionHandler >    mxInteractionHandler;
    Reference< XShape >                 mxParentShape;

    explicit            FilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException );

    void                setDocumentModel( const Reference< XComponent >& rxComponent ) throw( IllegalArgumentException );

    void                initializeFilter();
    void                finalizeFilter();
};

// ----------------------------------------------------------------------------

FilterBaseImpl::FilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    meDirection( FILTERDIRECTION_UNKNOWN ),
    meVersion( ECMA_DIALECT ),
    mxComponentContext( rxContext, UNO_SET_THROW ),
    mxComponentFactory( rxContext->getServiceManager(), UNO_SET_THROW )
{
}

void FilterBaseImpl::setDocumentModel( const Reference< XComponent >& rxComponent ) throw( IllegalArgumentException )
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

void FilterBaseImpl::initializeFilter()
{
    try
    {
        // lock the model controllers
        mxModel->lockControllers();
    }
    catch( Exception& )
    {
    }
}

void FilterBaseImpl::finalizeFilter()
{
    try
    {
        // writing back the FilterData to the MediaDescriptor
        maMediaDesc["FilterData"] = makeAny(maFilterData.getAsConstPropertyValueList());
        // write the descriptor back to the document model (adds the passwords)
        mxModel->attachResource( maFileUrl, maMediaDesc.getAsConstPropertyValueList() );
        // unlock the model controllers
        mxModel->unlockControllers();
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

FilterBase::FilterBase( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
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

const Reference< XShape >& FilterBase::getParentShape() const
{
    return mxImpl->mxParentShape;
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

inline bool lclIsDosDrive( const OUString& rUrl, sal_Int32 nPos = 0 )
{
    return
        (rUrl.getLength() >= nPos + 3) &&
        ((('A' <= rUrl[ nPos ]) && (rUrl[ nPos ] <= 'Z')) || (('a' <= rUrl[ nPos ]) && (rUrl[ nPos ] <= 'z'))) &&
        (rUrl[ nPos + 1 ] == ':') &&
        (rUrl[ nPos + 2 ] == '/');
}

} // namespace

OUString FilterBase::getAbsoluteUrl( const OUString& rUrl ) const
{
    // handle some special cases before calling ::rtl::Uri::convertRelToAbs()

    const OUString aFileSchema = "file:";
    const OUString aFilePrefix = "file:///";
    const sal_Int32 nFilePrefixLen = aFilePrefix.getLength();
    const OUString aUncPrefix = "//";

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
        return aFileSchema + aUrl.copy( nFilePrefixLen );
    }

    /*  (5) handle URLs relative to current drive, e.g. the URL '/path1/file1'
        relative to the base URL 'file:///C:/path2/file2' does not result in
        the expected 'file:///C:/path1/file1', but in 'file:///path1/file1'. */
    if( !aUrl.isEmpty() && (aUrl[ 0 ] == '/') &&
        mxImpl->maFileUrl.match( aFilePrefix ) &&
        lclIsDosDrive( mxImpl->maFileUrl, nFilePrefixLen ) )
    {
        return mxImpl->maFileUrl.copy( 0, nFilePrefixLen + 3 ) + aUrl.copy( 1 );
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

StorageRef FilterBase::getStorage() const
{
    return mxImpl->mxStorage;
}

Reference< XInputStream > FilterBase::openInputStream( const OUString& rStreamName ) const
{
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

// helpers --------------------------------------------------------------------

GraphicHelper& FilterBase::getGraphicHelper() const
{
    if( !mxImpl->mxGraphicHelper )
        mxImpl->mxGraphicHelper.reset( implCreateGraphicHelper() );
    return *mxImpl->mxGraphicHelper;
}

ModelObjectHelper& FilterBase::getModelObjectHelper() const
{
    if( !mxImpl->mxModelObjHelper )
        mxImpl->mxModelObjHelper.reset( new ModelObjectHelper( mxImpl->mxModelFactory ) );
    return *mxImpl->mxModelObjHelper;
}

OleObjectHelper& FilterBase::getOleObjectHelper() const
{
    if( !mxImpl->mxOleObjHelper )
        mxImpl->mxOleObjHelper.reset( new OleObjectHelper( mxImpl->mxModelFactory ) );
    return *mxImpl->mxOleObjHelper;
}

VbaProject& FilterBase::getVbaProject() const
{
    if( !mxImpl->mxVbaProject )
        mxImpl->mxVbaProject.reset( implCreateVbaProject() );
    return *mxImpl->mxVbaProject;
}

bool FilterBase::importBinaryData( StreamDataSequence& orDataSeq, const OUString& rStreamName )
{
    OSL_ENSURE( !rStreamName.isEmpty(), "FilterBase::importBinaryData - empty stream name" );
    if( rStreamName.isEmpty() )
        return false;

    // try to open the stream (this may fail - do not assert)
    BinaryXInputStream aInStrm( openInputStream( rStreamName ), true );
    if( aInStrm.isEof() )
        return false;

    // copy the entire stream to the passed sequence
    SequenceOutputStream aOutStrm( orDataSeq );
    aInStrm.copyToStream( aOutStrm );
    return true;
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL FilterBase::getImplementationName() throw( RuntimeException )
{
    return implGetImplementationName();
}

sal_Bool SAL_CALL FilterBase::supportsService( const OUString& rServiceName ) throw( RuntimeException )
{
    return
        (rServiceName == "com.sun.star.document.ImportFilter" ) ||
        (rServiceName == "com.sun.star.document.ExportFilter" );
}

Sequence< OUString > SAL_CALL FilterBase::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[ 0 ] = "com.sun.star.document.ImportFilter";
    aServiceNames[ 1 ] = "com.sun.star.document.ExportFilter";
    return aServiceNames;
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL FilterBase::initialize( const Sequence< Any >& rArgs ) throw( Exception, RuntimeException )
{
    if( rArgs.getLength() >= 2 ) try
    {
        mxImpl->maArguments << rArgs[ 1 ];
    }
    catch( Exception& )
    {
    }
}

// com.sun.star.document.XImporter interface ----------------------------------

void SAL_CALL FilterBase::setTargetDocument( const Reference< XComponent >& rxDocument ) throw( IllegalArgumentException, RuntimeException )
{
    mxImpl->setDocumentModel( rxDocument );
    mxImpl->meDirection = FILTERDIRECTION_IMPORT;
}

// com.sun.star.document.XExporter interface ----------------------------------

void SAL_CALL FilterBase::setSourceDocument( const Reference< XComponent >& rxDocument ) throw( IllegalArgumentException, RuntimeException )
{
    mxImpl->setDocumentModel( rxDocument );
    mxImpl->meDirection = FILTERDIRECTION_EXPORT;
}

// com.sun.star.document.XFilter interface ------------------------------------

sal_Bool SAL_CALL FilterBase::filter( const Sequence< PropertyValue >& rMediaDescSeq ) throw( RuntimeException )
{
    if( !mxImpl->mxModel.is() || !mxImpl->mxModelFactory.is() || (mxImpl->meDirection == FILTERDIRECTION_UNKNOWN) )
        throw RuntimeException();

    sal_Bool bRet = sal_False;
    setMediaDescriptor( rMediaDescSeq );
    DocumentOpenedGuard aOpenedGuard( mxImpl->maFileUrl );
    if( aOpenedGuard.isValid() || mxImpl->maFileUrl.isEmpty() )
    {
        mxImpl->initializeFilter();
        switch( mxImpl->meDirection )
        {
            case FILTERDIRECTION_UNKNOWN:
            break;
            case FILTERDIRECTION_IMPORT:
                if( mxImpl->mxInStream.is() )
                {
                    mxImpl->mxStorage = implCreateStorage( mxImpl->mxInStream );
                    bRet = mxImpl->mxStorage.get() && importDocument();
                }
            break;
            case FILTERDIRECTION_EXPORT:
                if( mxImpl->mxOutStream.is() )
                {
                    mxImpl->mxStorage = implCreateStorage( mxImpl->mxOutStream );
                    bRet = mxImpl->mxStorage.get() && exportDocument() && implFinalizeExport( getMediaDescriptor() );
                }
            break;
        }
        mxImpl->mxModel->unlockControllers();
    }
    return bRet;
}

void SAL_CALL FilterBase::cancel() throw( RuntimeException )
{
}

// protected ------------------------------------------------------------------

Reference< XInputStream > FilterBase::implGetInputStream( MediaDescriptor& rMediaDesc ) const
{
    return rMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INPUTSTREAM(), Reference< XInputStream >() );
}

Reference< XStream > FilterBase::implGetOutputStream( MediaDescriptor& rMediaDesc ) const
{
    return rMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STREAMFOROUTPUT(), Reference< XStream >() );
}

bool FilterBase::implFinalizeExport( MediaDescriptor& /*rMediaDescriptor*/ )
{
    return true;
}

Reference< XStream > FilterBase::getMainDocumentStream( ) const
{
    return mxImpl->mxOutStream;
}

// private --------------------------------------------------------------------

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

    mxImpl->maFileUrl = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), OUString() );
    mxImpl->mxTargetFrame = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_FRAME(), Reference< XFrame >() );
    mxImpl->mxStatusIndicator = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >() );
    mxImpl->mxInteractionHandler = mxImpl->maMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INTERACTIONHANDLER(), Reference< XInteractionHandler >() );
    mxImpl->mxParentShape = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "ParentShape", mxImpl->mxParentShape );
    mxImpl->maFilterData = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "FilterData", Sequence< PropertyValue >() );

    // Check for ISO OOXML
    OUString sFilterName = mxImpl->maMediaDesc.getUnpackedValueOrDefault( "FilterName", OUString() );
    try
    {
        Reference< XNameAccess > xFilters( Reference<XMultiServiceFactory>(getComponentContext()->getServiceManager(), UNO_QUERY_THROW)->createInstance("com.sun.star.document.FilterFactory" ), UNO_QUERY_THROW );
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

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
