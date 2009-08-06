/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlfilterbase.cxx,v $
 * $Revision: 1.5.6.2 $
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

#include "oox/core/xmlfilterbase.hxx"
#include <stdio.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <sax/fshelper.hxx>
#include "properties.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/core/fasttokenhandler.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/recordparser.hxx"
#include "oox/core/relationshandler.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::StringPair;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::embed::XRelationshipAccess;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::xml::sax::XFastParser;
using ::com::sun::star::xml::sax::XFastTokenHandler;
using ::com::sun::star::xml::sax::XFastDocumentHandler;
using ::com::sun::star::xml::sax::InputSource;
using ::com::sun::star::xml::sax::SAXException;
using ::sax_fastparser::FastSerializerHelper;
using ::sax_fastparser::FSHelperPtr;

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

    explicit            XmlFilterBaseImpl();
};

// ----------------------------------------------------------------------------

XmlFilterBaseImpl::XmlFilterBaseImpl() :
    maBinSuffix( CREATE_OUSTRING( ".bin" ) ),
    mxTokenHandler( new FastTokenHandler )
{
}

// ============================================================================

XmlFilterBase::XmlFilterBase( const Reference< XMultiServiceFactory >& rxGlobalFactory ) :
    FilterBase( rxGlobalFactory ),
    mxImpl( new XmlFilterBaseImpl ),
    mnRelId( 1 ),
    mnMaxDocId( 0 )
{
}

XmlFilterBase::~XmlFilterBase()
{
}

// ----------------------------------------------------------------------------

OUString XmlFilterBase::getFragmentPathFromFirstType( const OUString& rType )
{
    // importRelations() caches the relations map for subsequence calls
    return importRelations( OUString() )->getFragmentPathFromFirstType( rType );
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

    // try to import binary streams (fragment extension must be '.bin')
    sal_Int32 nBinSuffixPos = aFragmentPath.getLength() - mxImpl->maBinSuffix.getLength();
    if( (nBinSuffixPos >= 0) && aFragmentPath.match( mxImpl->maBinSuffix, nBinSuffixPos ) )
    {
        try
        {
            // try to open the fragment stream (this may fail - do not assert)
            Reference< XInputStream > xInStrm( openInputStream( aFragmentPath ), UNO_SET_THROW );

            // create the record parser
            RecordParser aParser;
            aParser.setFragmentHandler( rxHandler );

            // create the input source and parse the stream
            RecordInputSource aSource;
            aSource.mxInStream.reset( new BinaryXInputStream( xInStrm, true ) );
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
        // try to open the fragment stream (this may fail - do not assert)
        Reference< XInputStream > xInStrm( rxHandler->openFragmentStream(), UNO_SET_THROW );

        // create the fast parser
        Reference< XFastParser > xParser( getGlobalFactory()->createInstance(
            CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ) ), UNO_QUERY_THROW );
        xParser->setFastDocumentHandler( xDocHandler );
        xParser->setTokenHandler( mxImpl->mxTokenHandler );

        // register XML namespaces
        xParser->registerNamespace( CREATE_OUSTRING( "http://www.w3.org/XML/1998/namespace" ), NMSP_XML );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships" ), NMSP_PACKAGE_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships" ), NMSP_RELATIONSHIPS );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/main" ), NMSP_DRAWINGML );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/diagram" ), NMSP_DIAGRAM );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/chart" ), NMSP_CHART );

        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:vml" ), NMSP_VML );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:office" ), NMSP_OFFICE );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:word" ), NMSP_VML_DOC );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:excel" ), NMSP_VML_XLS );
        xParser->registerNamespace( CREATE_OUSTRING( "urn:schemas-microsoft-com:office:powerpoint" ), NMSP_VML_PPT );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.microsoft.com/office/2006/activeX" ), NMSP_AX );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/spreadsheetml/2006/main"), NMSP_XLS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing" ), NMSP_XDR );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.microsoft.com/office/excel/2006/main" ), NMSP_XM );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/presentationml/2006/main"), NMSP_PPT );

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
        importFragment( new RelationsFragment( *this, rxRelations ) );
    }
    return rxRelations;
}

Reference< XOutputStream > XmlFilterBase::openFragmentStream( const OUString& rStreamName, const OUString& rMediaType )
{
    Reference< XOutputStream > xOutputStream = openOutputStream( rStreamName );
    PropertySet aPropSet( xOutputStream );
    aPropSet.setProperty( PROP_MediaType, rMediaType );
    return xOutputStream;
}

FSHelperPtr XmlFilterBase::openFragmentStreamWithSerializer( const OUString& rStreamName, const OUString& rMediaType )
{
    return FSHelperPtr( new FastSerializerHelper( openFragmentStream( rStreamName, rMediaType ) ) );
}

namespace {

OUString lclAddRelation( const Reference< XRelationshipAccess > xRelations, sal_Int32 nId, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    OUString sId = OUStringBuffer().appendAscii( "rId" ).append( nId ).makeStringAndClear();

    Sequence< StringPair > aEntry( bExternal ? 3 : 2 );
    aEntry[0].First = CREATE_OUSTRING( "Type" );
    aEntry[0].Second = rType;
    aEntry[1].First = CREATE_OUSTRING( "Target" );
    aEntry[1].Second = rTarget;
    if( bExternal )
    {
        aEntry[2].First = CREATE_OUSTRING( "TargetMode" );
        aEntry[2].Second = CREATE_OUSTRING( "External" );
    }
    xRelations->insertRelationshipByID( sId, aEntry, sal_True );

    return sId;
}

} // namespace

OUString XmlFilterBase::addRelation( const OUString& rType, const OUString& rTarget, bool bExternal )
{
    Reference< XRelationshipAccess > xRelations( getStorage()->getXStorage(), UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, mnRelId ++, rType, rTarget, bExternal );

    return OUString();
}

OUString XmlFilterBase::addRelation( const Reference< XOutputStream > xOutputStream, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    sal_Int32 nId = 0;

    PropertySet aPropSet( xOutputStream );
    if( aPropSet.is() )
        aPropSet.getProperty( nId, PROP_RelId );
    else
        nId = mnRelId++;

    Reference< XRelationshipAccess > xRelations( xOutputStream, UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, nId, rType, rTarget, bExternal );

    return OUString();
}

StorageRef XmlFilterBase::implCreateStorage(
        Reference< XInputStream >& rxInStream, Reference< XStream >& rxOutStream ) const
{
    StorageRef xStorage;
    if( rxInStream.is() )
        xStorage.reset( new ZipStorage( getGlobalFactory(), rxInStream ) );
    else if( rxOutStream.is() )
        xStorage.reset( new ZipStorage( getGlobalFactory(), rxOutStream ) );

    return xStorage;
}

// ============================================================================

} // namespace core
} // namespace oox

