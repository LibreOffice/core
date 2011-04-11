/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hdl>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/docfile.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "xmloff/nmspmap.hxx"

#include "xmloff/xmltoken.hxx"
#include "xmloff/xmlmetae.hxx"
#include "xmloff/DashStyle.hxx"
#include "xmloff/GradientStyle.hxx"
#include "xmloff/HatchStyle.hxx"
#include "xmloff/ImageStyle.hxx"
#include "xmloff/MarkerStyle.hxx"
#include <xmloff/xmlictxt.hxx>
#include "svx/xmlgrhlp.hxx"
#include "xmloff/attrlist.hxx"

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

sal_Char const sXML_np__office[] = "__office";
sal_Char const sXML_np__office_ooo[] = "___office";
sal_Char const sXML_np__draw[] = "__draw";
sal_Char const sXML_np__draw_ooo[] = "___draw";
sal_Char const sXML_np__ooo[] = "__ooo";
sal_Char const sXML_np__xlink[] = "__xlink";

///////////////////////////////////////////////////////////////////////

enum SvxXMLTableImportContextEnum { stice_unknown, stice_color, stice_marker, stice_dash, stice_hatch, stice_gradient, stice_bitmap };

///////////////////////////////////////////////////////////////////////

class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >& xAttrList, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable,
        sal_Bool bOOoFormat );
    virtual ~SvxXMLTableImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList );

    SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }

protected:
    void importColor( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importMarker( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importDash( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importHatch( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importGradient( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importBitmap( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );

private:
    uno::Reference< XNameContainer > mxTable;
    SvxXMLTableImportContextEnum meContext;
    sal_Bool mbOOoFormat;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >&, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable, sal_Bool bOOoFormat )
: SvXMLImportContext( rImport, nPrfx, rLName ), mxTable( xTable ), meContext( eContext ),
    mbOOoFormat( bOOoFormat )
{
}

SvxXMLTableImportContext::~SvxXMLTableImportContext()
{
}

SvXMLImportContext *SvxXMLTableImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& rAttrList )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        uno::Reference< XAttributeList > xAttrList( rAttrList );
        if( mbOOoFormat &&
             (stice_dash == meContext || stice_hatch == meContext ||
             stice_bitmap == meContext) )
        {
            SvXMLAttributeList *pAttrList = new SvXMLAttributeList( rAttrList );
            xAttrList = pAttrList;
            sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                const OUString& rAttrName = xAttrList->getNameByIndex( i );
                OUString aLocalName;
                sal_uInt16 nPrefix_ =
                    GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
                if( XML_NAMESPACE_XLINK == nPrefix_ &&
                    stice_bitmap == meContext &&
                    IsXMLToken( aLocalName, XML_HREF ) )
                {
                    const OUString rValue = xAttrList->getValueByIndex( i );
                    if( rValue.getLength() && '#' == rValue[0] )
                        pAttrList->SetValueByIndex( i, rValue.copy( 1 ) );
                }
                else if( XML_NAMESPACE_DRAW == nPrefix_ &&
                          ( ( stice_dash == meContext &&
                              (IsXMLToken( aLocalName, XML_DOTS1_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DOTS2_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DISTANCE )) ) ||
                            ( stice_hatch == meContext &&
                              IsXMLToken( aLocalName, XML_HATCH_DISTANCE ) ) ) )
                {
                    const OUString rValue = xAttrList->getValueByIndex( i );
                    sal_Int32 nPos = rValue.getLength();
                    while( nPos && rValue[nPos-1] <= ' ' )
                        --nPos;
                    if( nPos > 2 &&
                        ('c'==rValue[nPos-2] || 'C'==rValue[nPos-2]) &&
                        ('h'==rValue[nPos-1] || 'H'==rValue[nPos-1]) )
                    {
                        pAttrList->SetValueByIndex( i, rValue.copy( 0, nPos-2 ) );
                    }
                }
            }
        }
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
            case stice_unknown:
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

void SvxXMLTableImportContext::importColor( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aLocalName );


        if( XML_NAMESPACE_DRAW == nPrefix )
        {
            if( aLocalName == GetXMLToken(XML_NAME) )
            {
                rName = xAttrList->getValueByIndex( i );
            }
            else if( aLocalName == GetXMLToken(XML_COLOR) )
            {
                Color aColor;
                SvXMLUnitConverter::convertColor(aColor, xAttrList->getValueByIndex( i ));
                rAny <<= (sal_Int32)aColor.GetColor();
            }
        }
    }
}

void SvxXMLTableImportContext::importMarker( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    try
    {
        XMLMarkerStyleImport aMarkerStyle( GetImport() );
        aMarkerStyle.importXML( xAttrList, rAny, rName );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxXMLTableImportContext::importMarker(), exception caught!");
    }
}

void SvxXMLTableImportContext::importDash( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    try
    {
        XMLDashStyleImport aDashStyle( GetImport() );
        aDashStyle.importXML( xAttrList, rAny, rName );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxXMLTableImportContext::importDash(), exception caught!");
    }
}

void SvxXMLTableImportContext::importHatch( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    try
    {
        XMLHatchStyleImport aHatchStyle( GetImport() );
        aHatchStyle.importXML( xAttrList, rAny, rName );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxXMLTableImportContext::importHatch(), exception caught!");
    }
}

void SvxXMLTableImportContext::importGradient( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    try
    {
        XMLGradientStyleImport aGradientStyle( GetImport() );
        aGradientStyle.importXML( xAttrList, rAny, rName );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxXMLTableImportContext::importGradient(), exception caught!");
    }
}

void SvxXMLTableImportContext::importBitmap( sal_uInt16 nPrfx, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    (void)nPrfx;
    (void)rLocalName;

    try
    {
        XMLImageStyle aImageStyle;
        aImageStyle.importXML( xAttrList, rAny, rName, GetImport() );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxXMLTableImportContext::importBitmap(), exception caught!");
    }
}

///////////////////////////////////////////////////////////////////////

// #110680#
SvxXMLXTableImport::SvxXMLXTableImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const uno::Reference< XNameContainer > & rTable,
    uno::Reference< XGraphicObjectResolver >& xGrfResolver )
:   SvXMLImport(xServiceFactory, 0),
    mrTable( rTable )
{
    SetGraphicResolver( xGrfResolver );

    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__ooo ) ), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office ) ), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__draw ) ), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__xlink ) ), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );

    // OOo namespaces for reading OOo 1.1 files
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

sal_Bool SvxXMLXTableImport::load( const OUString& rUrl, const uno::Reference< XNameContainer >& xTable ) throw()
{
    sal_Bool bRet = sal_True;

    uno::Reference< XGraphicObjectResolver >    xGrfResolver;
    SvXMLGraphicHelper* pGraphicHelper = 0;

    try
    {
        do
        {
            SfxMedium aMedium( rUrl, STREAM_READ | STREAM_NOCREATE, sal_True );

            uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
            if( !xServiceFactory.is() )
            {
                OSL_FAIL( "SvxXMLXTableImport::load: got no service manager" );
                break;
            }

            uno::Reference< xml::sax::XParser > xParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ), uno::UNO_QUERY_THROW );
            uno::Reference < io::XStream > xIStm;
            uno::Reference< io::XActiveDataSource > xSource;

            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aMedium.GetName();

            if( aMedium.IsStorage() )
            {
                uno::Reference < embed::XStorage > xStorage( aMedium.GetStorage( sal_False ), uno::UNO_QUERY_THROW );

                const String aContentStmName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
                xIStm.set( xStorage->openStreamElement( aContentStmName, embed::ElementModes::READ ), uno::UNO_QUERY_THROW );
                if( !xIStm.is() )
                {
                    OSL_FAIL( "could not open Content stream" );
                    break;
                }

                aParserInput.aInputStream = xIStm->getInputStream();
                pGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_READ );
                xGrfResolver = pGraphicHelper;
            }
            else
            {
                aParserInput.aInputStream = aMedium.GetInputStream();
                uno::Reference< io::XSeekable > xSeek( aParserInput.aInputStream, uno::UNO_QUERY_THROW );
                xSeek->seek( 0 );
            }

            if( xSource.is() )
            {
                uno::Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY_THROW );
                xSourceControl->start();
            }

            // #110680#
            // uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xTable, xGrfResolver ) );
            uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xServiceFactory, xTable, xGrfResolver ) );

            xParser->setDocumentHandler( xHandler );
            xParser->parseStream( aParserInput );
        }
        while(0);

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );
    }
    catch( uno::Exception& )
    {
//      CL: I disabled this assertion since its an error, but it happens
//          each time you load a document with property tables that are not
//          on the current machine. Maybe a better fix would be to place
//          a file exists check before importing...
//      OSL_FAIL("svx::SvxXMLXTableImport::load(), exception caught!");
        bRet = sal_False;
    }

    return bRet;
}

SvXMLImportContext *SvxXMLXTableImport::CreateContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList )
{
    if( XML_NAMESPACE_OOO == nPrefix ||
        XML_NAMESPACE_OFFICE == nPrefix )
    {
        sal_Bool bOOoFormat = (XML_NAMESPACE_OFFICE == nPrefix);
        Type aType = mrTable->getElementType();

        if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "color-table" ) ) )
        {
            if( aType == ::getCppuType((const sal_Int32*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_color, mrTable, bOOoFormat );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "marker-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_marker, mrTable, bOOoFormat );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "dash-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::LineDash*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_dash, mrTable, bOOoFormat );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "hatch-table" ) ) )
        {
            if( aType == ::getCppuType((const drawing::Hatch*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_hatch, mrTable, bOOoFormat );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "gradient-table" ) ) )
        {
            if( aType == ::getCppuType((const awt::Gradient*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_gradient, mrTable, bOOoFormat );
        }
        else if( rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "bitmap-table" ) ) )
        {
            if( aType == ::getCppuType((const OUString*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_bitmap, mrTable, bOOoFormat );
        }
    }

    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
