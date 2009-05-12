/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filterbase.cxx,v $
 * $Revision: 1.5.20.1 $
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

#include "oox/core/filterbase.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <rtl/uri.hxx>
#include <comphelper/mediadescriptor.hxx>

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::task::XInteractionHandler;
using ::comphelper::MediaDescriptor;

namespace oox {
namespace core {

// ============================================================================

struct FilterBaseImpl
{
    MediaDescriptor     maDescriptor;
    OUString            maFileUrl;
    StorageRef          mxStorage;

    Sequence< Any >                     maArguments;
    Reference< XMultiServiceFactory >   mxFactory;
    Reference< XModel >                 mxModel;
    Reference< XInputStream >           mxInStream;
    Reference< XStream >                mxStream;
    Reference< XStatusIndicator >       mxStatusIndicator;
    Reference< XInteractionHandler >    mxInteractionHandler;

    explicit            FilterBaseImpl( const Reference< XMultiServiceFactory >& rxFactory );
    void                setMediaDescriptor( const Sequence< PropertyValue >& rDescriptor );
};

// ----------------------------------------------------------------------------

FilterBaseImpl::FilterBaseImpl( const Reference< XMultiServiceFactory >& rxFactory ) :
    mxFactory( rxFactory )
{
    OSL_ENSURE( mxFactory.is(), "FilterBaseImpl::FilterBaseImpl - missing service factory" );
}

void FilterBaseImpl::setMediaDescriptor( const Sequence< PropertyValue >& rDescriptor )
{
    maDescriptor = rDescriptor;

    maFileUrl = maDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), maFileUrl );
    mxInStream = maDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_INPUTSTREAM(), mxInStream );
    mxStream = maDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_STREAMFOROUTPUT(), mxStream );
    mxStatusIndicator = maDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_STATUSINDICATOR(), mxStatusIndicator );
    mxInteractionHandler = maDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_INTERACTIONHANDLER(), mxInteractionHandler );

    if( mxInStream.is() )
        maDescriptor.addInputStream();
}

// ============================================================================

FilterBase::FilterBase( const Reference< XMultiServiceFactory >& rxFactory ) :
    mxImpl( new FilterBaseImpl( rxFactory ) )
{
}

FilterBase::~FilterBase()
{
}

bool FilterBase::isImportFilter() const
{
    return mxImpl->mxInStream.is();
}

bool FilterBase::isExportFilter() const
{
    return mxImpl->mxStream.is();
}

// ----------------------------------------------------------------------------

const Sequence< Any >& FilterBase::getArguments() const
{
    return mxImpl->maArguments;
}

const Reference< XMultiServiceFactory >& FilterBase::getGlobalFactory() const
{
    return mxImpl->mxFactory;
}

const Reference< XModel >& FilterBase::getModel() const
{
    return mxImpl->mxModel;
}

const Reference< XStatusIndicator >& FilterBase::getStatusIndicator() const
{
    return mxImpl->mxStatusIndicator;
}

const Reference< XInteractionHandler >& FilterBase::getInteractionHandler() const
{
    return mxImpl->mxInteractionHandler;
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

    const OUString aFileSchema = CREATE_OUSTRING( "file:" );
    const OUString aFilePrefix = CREATE_OUSTRING( "file:///" );
    const sal_Int32 nFilePrefixLen = aFilePrefix.getLength();
    const OUString aUncPrefix = CREATE_OUSTRING( "//" );

    /*  (1) convert all backslashes to slashes. */
    OUString aUrl = rUrl.replace( '\\', '/' );

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
    if( (aUrl.getLength() >= 1) && (aUrl[ 0 ] == '/') &&
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

StorageRef FilterBase::openSubStorage( const OUString& rStorageName, bool bCreate ) const
{
    return mxImpl->mxStorage->openSubStorage( rStorageName, bCreate );
}

Reference< XInputStream > FilterBase::openInputStream( const OUString& rStreamName ) const
{
    return mxImpl->mxStorage->openInputStream( rStreamName );
}

Reference< XOutputStream > FilterBase::openOutputStream( const OUString& rStreamName ) const
{
    return mxImpl->mxStorage->openOutputStream( rStreamName );
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL FilterBase::getImplementationName() throw( RuntimeException )
{
    return implGetImplementationName();
}

sal_Bool SAL_CALL FilterBase::supportsService( const OUString& rServiceName ) throw( RuntimeException )
{
    return
        (rServiceName == CREATE_OUSTRING( "com.sun.star.document.ImportFilter" )) ||
        (rServiceName == CREATE_OUSTRING( "com.sun.star.document.ExportFilter" ));
}

Sequence< OUString > SAL_CALL FilterBase::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    aServiceNames[ 1 ] = CREATE_OUSTRING( "com.sun.star.document.ExportFilter" );
    return aServiceNames;
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL FilterBase::initialize( const Sequence< Any >& rArgs ) throw( Exception, RuntimeException )
{
    mxImpl->maArguments = rArgs;
}

// com.sun.star.document.XImporter interface ----------------------------------

void SAL_CALL FilterBase::setTargetDocument( const Reference< XComponent >& rxDocument ) throw( IllegalArgumentException, RuntimeException )
{
    mxImpl->mxModel.set( rxDocument, UNO_QUERY );
    if( !mxImpl->mxModel.is() )
        throw IllegalArgumentException();
}

// com.sun.star.document.XExporter interface ----------------------------------

void SAL_CALL FilterBase::setSourceDocument( const Reference< XComponent >& rxDocument ) throw( IllegalArgumentException, RuntimeException )
{
    mxImpl->mxModel.set( rxDocument, UNO_QUERY );
    if( !mxImpl->mxModel.is() )
        throw IllegalArgumentException();
}

// com.sun.star.document.XFilter interface ------------------------------------

sal_Bool SAL_CALL FilterBase::filter( const Sequence< PropertyValue >& rDescriptor ) throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
    if( mxImpl->mxFactory.is() && mxImpl->mxModel.is() )
    {
        mxImpl->setMediaDescriptor( rDescriptor );
        mxImpl->mxStorage = implCreateStorage( mxImpl->mxInStream, mxImpl->mxStream );
        if( mxImpl->mxStorage.get() )
        {
            mxImpl->mxModel->lockControllers();
            if( mxImpl->mxInStream.is() )
                bRet = importDocument();
            else if( mxImpl->mxStream.is() )
                bRet = exportDocument();
            mxImpl->mxModel->unlockControllers();
        }
    }
    return bRet;
}

void SAL_CALL FilterBase::cancel() throw( RuntimeException )
{
}

// ============================================================================

} // namespace core
} // namespace oox

