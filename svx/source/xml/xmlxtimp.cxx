/*************************************************************************
 *
 *  $RCSfile: xmlxtimp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-23 21:35:17 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XGRAPHICOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmloff/xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "xmloff/nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmloff/xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmloff/xmlmetae.hxx"
#endif

#ifndef _XMLOFF_DASHSTYLE_HXX
#include "xmloff/DashStyle.hxx"
#endif

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include "xmloff/GradientStyle.hxx"
#endif

#ifndef _XMLOFF_HATCHSTYLE_HXX
#include "xmloff/HatchStyle.hxx"
#endif

#ifndef _XMLOFF_IMAGESTYLE_HXX
#include "xmloff/ImageStyle.hxx"
#endif

#ifndef _XMLOFF_MARKERSTYLE_HXX
#include "xmloff/MarkerStyle.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLGRHLP_HXX
#include "xmlgrhlp.hxx"
#endif

#include "xmlxtimp.hxx"

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::rtl;
using namespace cppu;

///////////////////////////////////////////////////////////////////////

enum SvxXMLTableImportContextEnum { stice_unknown, stice_color, stice_marker, stice_dash, stice_hatch, stice_gradient, stice_bitmap };

///////////////////////////////////////////////////////////////////////

class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvxXMLTableImportContextEnum eContext, const Reference< XNameContainer >& xTable );
    virtual ~SvxXMLTableImportContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList );

    SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }

protected:
    void importColor( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importMarker( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importDash( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importHatch( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importGradient( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importBitmap( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );

private:
    Reference< XNameContainer > mxTable;
    SvxXMLTableImportContextEnum meContext;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, USHORT nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvxXMLTableImportContextEnum eContext, const Reference< XNameContainer >& xTable )
: SvXMLImportContext( rImport, nPrfx, rLName ), meContext( eContext ), mxTable( xTable )
{
}

SvxXMLTableImportContext::~SvxXMLTableImportContext()
{
}

SvXMLImportContext *SvxXMLTableImportContext::CreateChildContext( USHORT nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        try
        {
            Any aAny;
            OUString aName;

            switch( meContext )
            {
            case stice_color:
                importColor( nPrefix, rLocalName, xAttrList, aAny, aName );
                break;
            case stice_marker:
                importMarker( nPrefix, rLocalName, xAttrList, aAny, aName  );
                break;
            case stice_dash:
                importDash( nPrefix, rLocalName, xAttrList, aAny, aName  );
                break;
            case stice_hatch:
                importHatch( nPrefix, rLocalName, xAttrList, aAny, aName  );
                break;
            case stice_gradient:
                importGradient( nPrefix, rLocalName, xAttrList, aAny, aName  );
                break;
            case stice_bitmap:
                importBitmap( nPrefix, rLocalName, xAttrList, aAny, aName  );
                break;
            }

            if( aName.getLength() && aAny.hasValue() )
            {
                if( mxTable->hasByName( aName ) )
                {
                    mxTable->replaceByName( aName, aAny );
                }
                else
                {
                    mxTable->insertByName( aName, aAny );
                }
            }
        }
        catch( uno::Exception& e )
        {
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void SvxXMLTableImportContext::importColor( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aLocalName );


        if( XML_NAMESPACE_DRAW == nPrefix )
        {
            if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_name ) ) )
            {
                rName = xAttrList->getValueByIndex( i );
            }
            else if( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_color ) ) )
            {
                Color aColor;
                SvXMLUnitConverter::convertColor(aColor, xAttrList->getValueByIndex( i ));
                rAny <<= (sal_Int32)aColor.GetColor();
            }
        }
    }
}

void SvxXMLTableImportContext::importMarker( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    Reference< XDocumentHandler > xHandler( (XDocumentHandler*)&GetImport() );
    XMLMarkerStyle aMarkerStyle( xHandler, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aMarkerStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importDash( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    Reference< XDocumentHandler > xHandler( (XDocumentHandler*)&GetImport() );
    XMLDashStyle aDashStyle( &xHandler, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aDashStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importHatch( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    Reference< XDocumentHandler > xHandler( (XDocumentHandler*)&GetImport() );
    XMLHatchStyle aHatchStyle( xHandler, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aHatchStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importGradient( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    Reference< XDocumentHandler > xHandler( (XDocumentHandler*)&GetImport() );
    XMLGradientStyle aGradientStyle( &xHandler, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aGradientStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importBitmap( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    XMLImageStyle aImageStyle;
    aImageStyle.importXML( xAttrList, rAny, rName, GetImport() );
}

///////////////////////////////////////////////////////////////////////

SvxXMLXTableImport::SvxXMLXTableImport( const Reference< XNameContainer > & rTable, Reference< XGraphicObjectResolver >& xGrfResolver )
: mrTable( rTable )
{
    SetGraphicResolver( xGrfResolver );
}

SvxXMLXTableImport::~SvxXMLXTableImport()
{
}

sal_Bool SvxXMLXTableImport::load( const OUString& rUrl, const Reference< XNameContainer >& xTable ) throw()
{
    sal_Bool bRet = sal_True;

    Reference< XGraphicObjectResolver > xGrfResolver;
    SvXMLGraphicHelper* pGraphicHelper = 0;

    try
    {
        do
        {
            SfxMedium aMedium( rUrl, STREAM_READ | STREAM_NOCREATE, TRUE );

            uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
            if( !xServiceFactory.is() )
            {
                DBG_ERROR( "SvxXMLXTableImport::load: got no service manager" );
                break;
            }

            uno::Reference< xml::sax::XParser > xParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ), uno::UNO_QUERY );
            if( !xParser.is() )
            {
                DBG_ERROR( "com.sun.star.xml.sax.Parser service missing" );
                break;
            }

            SvStorageStreamRef                      xIStm;
            uno::Reference< io::XActiveDataSource > xSource;

            SvStorage*                              pStorage = aMedium.GetStorage();

            xml::sax::InputSource                   aParserInput;
            aParserInput.sSystemId = aMedium.GetName();

            if( pStorage )
            {
                const String aContentStmName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );

                xIStm = pStorage->OpenStream( aContentStmName, STREAM_READ | STREAM_NOCREATE );
                if( !xIStm.Is() )
                {
                    DBG_ERROR( "could not open Content stream" );
                    break;
                }

                xIStm->SetBufferSize( 16 * 1024 );
                aParserInput.aInputStream = new utl::OInputStreamWrapper( *xIStm );

                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_READ );
                xGrfResolver = pGraphicHelper;
            }
            else
            {
                Reference< XInterface > xPipe;

                aMedium.GetInStream()->Seek( 0 );
                xSource = aMedium.GetDataSource();

                if( !xSource.is() )
                {
                    DBG_ERROR( "got no data source from medium" );
                    break;
                }

                xPipe = xServiceFactory->createInstance(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe") ) );
                if( !xPipe.is() )
                {
                    DBG_ERROR( "XMLReader::Read: com.sun.star.io.Pipe service missing" );
                    break;
                }

                // connect pipe's output stream to the data source
                xSource->setOutputStream( Reference< io::XOutputStream >::query( xPipe ) );

                aParserInput.aInputStream = uno::Reference< io::XInputStream >::query( xPipe );
            }

            if( xSource.is() )
            {
                Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
                xSourceControl->start();
            }

            Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xTable, xGrfResolver ) );

            xParser->setDocumentHandler( xHandler );
            xParser->parseStream( aParserInput );
        }
        while(0);

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );
    }
    catch( uno::Exception& e )
    {
        bRet = sal_False;
    }

    return bRet;
}

SvXMLImportContext *SvxXMLXTableImport::CreateContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    if( XML_NAMESPACE_OFFICE == nPrefix )
    {
        Type aType = mrTable->getElementType();

        if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "color-table" ) ) )
        {
            if( aType == ::getCppuType((const sal_Int32*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_color, mrTable );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "marker-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_marker, mrTable );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "dash-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::LineDash*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_dash, mrTable );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "hatch-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::Hatch*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_hatch, mrTable );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "gradient-table" ) ) )
        {
            if( aType == ::getCppuType((const awt::Gradient*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_gradient, mrTable );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "bitmap-table" ) ) )
        {
            if( aType == ::getCppuType((const OUString*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_bitmap, mrTable );
        }
    }

    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

