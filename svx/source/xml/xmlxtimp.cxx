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

#include <tools/urlobj.hxx>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/streamwrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <sfx2/docfile.hxx>
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

#include <cstdio>

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
        bool bOOoFormat );
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
    bool mbOOoFormat;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >&, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable, bool bOOoFormat )
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
                    if( !rValue.isEmpty() && '#' == rValue[0] )
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

            if( !aName.isEmpty() && aAny.hasValue() )
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
        catch (const uno::Exception&)
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
                sal_Int32 nColor(0);
                ::sax::Converter::convertColor(nColor,
                        xAttrList->getValueByIndex( i ));
                rAny <<= nColor;
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
    catch (const Exception&)
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
    catch (const Exception&)
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
    catch (const Exception&)
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
    catch (const Exception&)
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
    catch (const Exception&)
    {
        OSL_FAIL("SvxXMLTableImportContext::importBitmap(), exception caught!");
    }
}

///////////////////////////////////////////////////////////////////////

// #110680#
SvxXMLXTableImport::SvxXMLXTableImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
    const uno::Reference< XNameContainer > & rTable,
    uno::Reference< XGraphicObjectResolver >& xGrfResolver )
:   SvXMLImport(xContext, 0),
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

static void openStorageStream( xml::sax::InputSource *pParserInput,
                               SvXMLGraphicHelper   **ppGraphicHelper,
                               uno::Reference < embed::XStorage > xStorage )
{
    uno::Reference < io::XStream > xIStm;
    xIStm.set( xStorage->openStreamElement( "Content.xml", embed::ElementModes::READ ), uno::UNO_QUERY_THROW );
    if( !xIStm.is() )
    {
        OSL_FAIL( "could not open Content stream" );
        return;
    }
    pParserInput->aInputStream = xIStm->getInputStream();
    *ppGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_READ );
}

bool SvxXMLXTableImport::load( const OUString &rPath,
                               const uno::Reference < embed::XStorage > &xStorage,
                               const uno::Reference< XNameContainer >& xTable,
                               bool *bOptLoadedFromStorage ) throw()
{
    bool bRet = true;
    SvXMLGraphicHelper* pGraphicHelper = 0;

    INetURLObject aURLObj( rPath );
    bool bUseStorage = aURLObj.GetProtocol() == INET_PROT_NOT_VALID; // a relative path

    try
    {
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

        xml::sax::InputSource aParserInput;
        comphelper::LifecycleProxy aNasty;

        if( !bUseStorage || !xStorage.is() )
        {
            SfxMedium aMedium( rPath, STREAM_READ | STREAM_NOCREATE );
            aParserInput.sSystemId = aMedium.GetName();

            if( aMedium.IsStorage() )
            {
                uno::Reference < embed::XStorage > xMediumStorage( aMedium.GetStorage( sal_False ), uno::UNO_QUERY_THROW );
                openStorageStream( &aParserInput, &pGraphicHelper, xMediumStorage );
            }
            else
                aParserInput.aInputStream = aMedium.GetInputStream();
        }
        else // relative URL into a storage
        {
            uno::Reference< embed::XStorage > xSubStorage;
            try
            {
                xSubStorage = comphelper::OStorageHelper::GetStorageAtPath(
                        xStorage, rPath, embed::ElementModes::READ, aNasty );
            }
            catch (const uno::Exception&)
            {
            }
            if( xSubStorage.is() )
                openStorageStream( &aParserInput, &pGraphicHelper, xSubStorage );
            else
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream;
                xStream = comphelper::OStorageHelper::GetStreamAtPath(
                        xStorage, rPath, embed::ElementModes::READ, aNasty );
                if( !xStream.is() )
                    return false;
                aParserInput.aInputStream = xStream->getInputStream();
            }
            if( bOptLoadedFromStorage )
                *bOptLoadedFromStorage = true;
        }

        uno::Reference< XGraphicObjectResolver > xGrfResolver;
        if (pGraphicHelper)
            xGrfResolver = pGraphicHelper;

        try
        {
            uno::Reference< io::XSeekable > xSeek( aParserInput.aInputStream, uno::UNO_QUERY_THROW );
            xSeek->seek( 0 );
        }
        catch (const uno::Exception&)
        {
        }

        uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTableImport( xContext, xTable, xGrfResolver ) );
        xParser->setDocumentHandler( xHandler );
        xParser->parseStream( aParserInput );

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );
    }
    catch (const uno::Exception&)
    {
//      thrown each time you load a document with property tables that are not
//      on the current machine. FIXME: would be better to check a file exists
//      before importing ...
//        fprintf (stderr, "parsing etc. exception '%s'\n",
//                 OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        bRet = false;
    }

    return bRet;
}

SvXMLImportContext *SvxXMLXTableImport::CreateContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList )
{
    if( XML_NAMESPACE_OOO == nPrefix ||
        XML_NAMESPACE_OFFICE == nPrefix )
    {
        bool bOOoFormat = (XML_NAMESPACE_OFFICE == nPrefix);
        Type aType = mrTable->getElementType();

        if ( rLocalName == "color-table" )
        {
            if( aType == ::getCppuType((const sal_Int32*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_color, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "marker-table" )
        {
            if( aType == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_marker, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "dash-table" )
        {
            if( aType == ::getCppuType((const drawing::LineDash*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_dash, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "hatch-table" )
        {
            if( aType == ::getCppuType((const drawing::Hatch*)0) )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_hatch, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "gradient-table" )
        {
            if( aType == ::getCppuType((const awt::Gradient*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_gradient, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "bitmap-table" )
        {
            if( aType == ::getCppuType((const OUString*)0))
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, xAttrList, stice_bitmap, mrTable, bOOoFormat );
        }
    }

    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
