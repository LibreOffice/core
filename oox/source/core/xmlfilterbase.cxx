/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfilterbase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:16:03 $
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

#include "oox/core/xmlfilterbase.hxx"
#include <set>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/core/fasttokenhandler.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/recordparser.hxx"
#include "oox/core/relationshandler.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::embed::XTransactedObject;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::document::XDocumentSubStorageSupplier;
using ::com::sun::star::xml::sax::XFastParser;
using ::com::sun::star::xml::sax::XFastTokenHandler;
using ::com::sun::star::xml::sax::XFastDocumentHandler;
using ::com::sun::star::xml::sax::InputSource;
using ::com::sun::star::xml::sax::SAXException;

namespace oox {
namespace core {

// ============================================================================

struct XmlFilterBaseImpl
{
    typedef RefMap< OUString, Relations > RelationsMap;

    OUString            maBinSuffix;
    Reference< XFastTokenHandler >
                        mxTokenHandler;
    RelationsMap        maRelationsMap;
    ::std::set< OUString > maPictureSet;        /// Already copied picture stream names.
    Reference< XStorage > mxPictureStorage;     /// Target model picture storage.
    Reference< XNameContainer > mxMarkerTable;  /// Table to create new arrow names.

    explicit            XmlFilterBaseImpl();
};

// ----------------------------------------------------------------------------

XmlFilterBaseImpl::XmlFilterBaseImpl() :
    maBinSuffix( CREATE_OUSTRING( ".bin" ) ),
    mxTokenHandler( new FastTokenHandler )
{
}

// ============================================================================

XmlFilterBase::XmlFilterBase( const Reference< XMultiServiceFactory >& rxFactory ) :
    FilterBase( rxFactory ),
    mxImpl( new XmlFilterBaseImpl )
{
}

XmlFilterBase::~XmlFilterBase()
{
}

// ----------------------------------------------------------------------------

OUString XmlFilterBase::getFragmentPathFromType( const OUString& rType )
{
    return importRelations( OUString() )->getTargetFromType( rType );
}

bool XmlFilterBase::importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler )
{
    OSL_ENSURE( rxHandler.is(), "XmlFilterBase::importFragment - missing fragment handler" );
    if( !rxHandler.is() )
        return false;

    // fragment handler must contain path to fragment stream
    OUString aFragmentPath = rxHandler->getFragmentPath();
    OSL_ENSURE( aFragmentPath.getLength() > 0, "XmlFilterBase::importFragment - missing fragment path" );
    if( aFragmentPath.getLength() == 0 )
        return false;

    // try to open the fragment stream (this may fail - do not assert)
    Reference< XInputStream > xInStrm = openInputStream( aFragmentPath );
    if( !xInStrm.is() )
        return false;

    // try to import binary streams (fragment extension must be '.bin')
    sal_Int32 nBinSuffixPos = aFragmentPath.getLength() - mxImpl->maBinSuffix.getLength();
    if( (nBinSuffixPos >= 0) && aFragmentPath.match( mxImpl->maBinSuffix, nBinSuffixPos ) )
    {
        try
        {
            // create the record parser
            RecordParser aParser;
            aParser.setFragmentHandler( rxHandler );

            // create the input source and parse the stream
            RecordInputSource aSource;
            aSource.mxInStream.reset( new BinaryInputStream( xInStrm, true ) );
            aSource.maSystemId = aFragmentPath;
            aParser.parseStream( aSource );
            return true;
        }
        catch( Exception& )
        {
        }
        return false;
    }

    // get the XFastDocumentHandler interface from the fragment handler
    Reference< XFastDocumentHandler > xDocHandler( rxHandler.get() );
    if( !xDocHandler.is() )
        return false;

    // try to import XML stream
    try
    {
        // create the fast parser
        Reference< XFastParser > xParser( getServiceFactory()->createInstance(
            CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ) ), UNO_QUERY_THROW );
        xParser->setFastDocumentHandler( xDocHandler );
        xParser->setTokenHandler( mxImpl->mxTokenHandler );

        // register XML namespaces
        xParser->registerNamespace( CREATE_OUSTRING( "http://www.w3.org/XML/1998/namespace" ), NMSP_XML );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships" ), NMSP_PACKAGE_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships" ), NMSP_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:office" ), NMSP_OFFICE );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/main" ), NMSP_DRAWINGML );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/diagram" ), NMSP_DIAGRAM );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/chart" ), NMSP_CHART );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:vml" ), NMSP_VML );

        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:word" ), NMSP_WORD );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/spreadsheetml/2006/main"), NMSP_XLS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing" ), NMSP_XDR );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.microsoft.com/office/excel/2006/main" ), NMSP_XM );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:excel" ), NMSP_EXCEL );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/presentationml/2006/main"), NMSP_PPT );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:powerpoint" ), NMSP_POWERPOINT );

        // create the input source and parse the stream
        InputSource aSource;
        aSource.aInputStream = xInStrm;
        aSource.sSystemId = aFragmentPath;
        xParser->parseStream( aSource );
        return true;
    }
    catch( Exception& )
    {
    }
    return false;
}

RelationsRef XmlFilterBase::importRelations( const OUString& rFragmentPath )
{
    // try to find cached relations
    RelationsRef& rxRelations = mxImpl->maRelationsMap[ rFragmentPath ];
    if( !rxRelations )
    {
        // import and cache relations
        rxRelations.reset( new Relations( rFragmentPath ) );
        importFragment( new RelationsFragmentHandler( *this, rxRelations ) );
    }
    return rxRelations;
}

OUString XmlFilterBase::copyPictureStream( const OUString& rPicturePath )
{
    // split source path into source storage path and stream name
    sal_Int32 nIndex = rPicturePath.lastIndexOf( sal_Unicode( '/' ) );
    OUString sPictureName;
    OUString sSourceStorageName;
    if( nIndex < 0 )
    {
        // root stream
        sPictureName = rPicturePath;
    }
    else
    {
        // sub stream
        sPictureName = rPicturePath.copy( nIndex + 1 );
        sSourceStorageName = rPicturePath.copy( 0, nIndex );
    }

    // check if we already copied this one!
    if( mxImpl->maPictureSet.find( rPicturePath ) == mxImpl->maPictureSet.end() ) try
    {
        // ok, not yet, copy stream to documents picture storage

        // first get the picture storage from our target model
        if( !mxImpl->mxPictureStorage.is() )
        {
            static const OUString sPictures = CREATE_OUSTRING( "Pictures" );
            Reference< XDocumentSubStorageSupplier > xDSSS( getModel(), UNO_QUERY_THROW );
            mxImpl->mxPictureStorage.set( xDSSS->getDocumentSubStorage(
                sPictures, ::com::sun::star::embed::ElementModes::WRITE ), UNO_QUERY_THROW );
        }

        StorageRef xSourceStorage = openSubStorage( sSourceStorageName, false );
        if( xSourceStorage.get() )
        {
            Reference< XStorage > xSourceXStorage = xSourceStorage->getXStorage();
            if( xSourceXStorage.is() )
            {
                xSourceXStorage->copyElementTo( sPictureName, mxImpl->mxPictureStorage, sPictureName );
                Reference< XTransactedObject > xTO( mxImpl->mxPictureStorage, UNO_QUERY_THROW );
                xTO->commit();
            }
        }
    }
    catch( Exception& )
    {
    }

    static const OUString sUrlPrefix = CREATE_OUSTRING( "vnd.sun.star.Package:Pictures/" );
    return sUrlPrefix + sPictureName;
}

Reference< XNameContainer >& XmlFilterBase::getMarkerTable() const
{
    if( !mxImpl->mxMarkerTable.is() ) try
    {
        mxImpl->mxMarkerTable.set( getServiceFactory()->createInstance(
            CREATE_OUSTRING( "com.sun.star.drawing.MarkerTable" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    return mxImpl->mxMarkerTable;
}

StorageRef XmlFilterBase::implCreateStorage(
        Reference< XInputStream >& rxInStream, Reference< XOutputStream >& rxOutStream ) const
{
    StorageRef xStorage;
    if( rxInStream.is() )
        xStorage.reset( new ZipStorage( getServiceFactory(), rxInStream ) );
    else if( rxOutStream.is() )
        xStorage.reset( new ZipStorage( getServiceFactory(), rxOutStream ) );
    return xStorage;
}

// ============================================================================

} // namespace core
} // namespace oox

