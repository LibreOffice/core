/*************************************************************************
 *
 *  $RCSfile: xmlxtimp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:52:08 $
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
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HDL_
#include <com/sun/star/io/XSeekable.hdl>
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
using namespace ::xmloff::token;
using namespace cppu;

sal_Char __READONLY_DATA sXML_np__office_ooo[] = "___office";
sal_Char __READONLY_DATA sXML_np__style_ooo[] = "___style";
sal_Char __READONLY_DATA sXML_np__text_ooo[] = "___text";
sal_Char __READONLY_DATA sXML_np__table_ooo[] = "___table";
sal_Char __READONLY_DATA sXML_np__draw_ooo[] = "___draw";
sal_Char __READONLY_DATA sXML_np__dr3d_ooo[] = "___dr3d";
sal_Char __READONLY_DATA sXML_np__meta_ooo[] = "___meta";
sal_Char __READONLY_DATA sXML_np__number_ooo[] = "___number";
sal_Char __READONLY_DATA sXML_np__chart_ooo[] = "___chart";
sal_Char __READONLY_DATA sXML_np__script_ooo[] = "___script";
sal_Char __READONLY_DATA sXML_np__config_ooo[] = "___config";
sal_Char __READONLY_DATA sXML_np__form_ooo[] = "___form";

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
        catch( uno::Exception& )
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
    XMLMarkerStyleImport aMarkerStyle( GetImport() );
    aMarkerStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importDash( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    XMLDashStyleImport aDashStyle( GetImport() );
    aDashStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importHatch( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    XMLHatchStyleImport aHatchStyle( GetImport() );
    aHatchStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importGradient( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    XMLGradientStyleImport aGradientStyle( GetImport() );
    aGradientStyle.importXML( xAttrList, rAny, rName );
}

void SvxXMLTableImportContext::importBitmap( USHORT nPrfx, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    XMLImageStyle aImageStyle;
    aImageStyle.importXML( xAttrList, rAny, rName, GetImport() );
}

///////////////////////////////////////////////////////////////////////

// #110680#
SvxXMLXTableImport::SvxXMLXTableImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const Reference< XNameContainer > & rTable,
    Reference< XGraphicObjectResolver >& xGrfResolver )
:   SvXMLImport(xServiceFactory, 0),
    mrTable( rTable )
{
    SetGraphicResolver( xGrfResolver );

    // OOo namespaces
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office_ooo ) ),
                        GetXMLToken(XML_N_OFFICE_OOO),
                        XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__draw_ooo ) ),
                        GetXMLToken(XML_N_DRAW_OOO),
                        XML_NAMESPACE_DRAW );
}

SvxXMLXTableImport::~SvxXMLXTableImport() throw ()
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
                aParserInput.aInputStream = aMedium.GetInputStream();
                Reference< io::XSeekable > xSeek( aParserInput.aInputStream, UNO_QUERY );
                if ( xSeek.is() )
                    xSeek->seek( 0 );
            }

            if( xSource.is() )
            {
                Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
                xSourceControl->start();
            }

            // #110680#
            // Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xTable, xGrfResolver ) );
            Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xServiceFactory, xTable, xGrfResolver ) );

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

