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
using namespace ::xmloff::token;
using namespace cppu;

enum class SvxXMLTableImportContextEnum { Color, Marker, Dash, Hatch, Gradient, Bitmap };


class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable,
        bool bOOoFormat );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList ) override;

protected:
    void importColor( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importMarker( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importDash( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importHatch( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importGradient( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importBitmap( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName );

private:
    uno::Reference< XNameContainer > mxTable;
    SvxXMLTableImportContextEnum meContext;
    bool mbOOoFormat;
};


SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable, bool bOOoFormat )
: SvXMLImportContext( rImport, nPrfx, rLName ), mxTable( xTable ), meContext( eContext ),
    mbOOoFormat( bOOoFormat )
{
}

SvXMLImportContextRef SvxXMLTableImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& rAttrList )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        uno::Reference< XAttributeList > xAttrList( rAttrList );
        if( mbOOoFormat &&
             (SvxXMLTableImportContextEnum::Dash == meContext || SvxXMLTableImportContextEnum::Hatch == meContext ||
             SvxXMLTableImportContextEnum::Bitmap == meContext) )
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
                    SvxXMLTableImportContextEnum::Bitmap == meContext &&
                    IsXMLToken( aLocalName, XML_HREF ) )
                {
                    const OUString rValue = xAttrList->getValueByIndex( i );
                    if( !rValue.isEmpty() && '#' == rValue[0] )
                        pAttrList->SetValueByIndex( i, rValue.copy( 1 ) );
                }
                else if( XML_NAMESPACE_DRAW == nPrefix_ &&
                          ( ( SvxXMLTableImportContextEnum::Dash == meContext &&
                              (IsXMLToken( aLocalName, XML_DOTS1_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DOTS2_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DISTANCE )) ) ||
                            ( SvxXMLTableImportContextEnum::Hatch == meContext &&
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
            case SvxXMLTableImportContextEnum::Color:
                importColor( xAttrList, aAny, aName );
                break;
            case SvxXMLTableImportContextEnum::Marker:
                importMarker( xAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Dash:
                importDash( xAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Hatch:
                importHatch( xAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Gradient:
                importGradient( xAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Bitmap:
                importBitmap( xAttrList, aAny, aName  );
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

void SvxXMLTableImportContext::importColor( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
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

void SvxXMLTableImportContext::importMarker( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
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

void SvxXMLTableImportContext::importDash( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
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

void SvxXMLTableImportContext::importHatch( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
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

void SvxXMLTableImportContext::importGradient( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
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

void SvxXMLTableImportContext::importBitmap( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    try
    {
        XMLImageStyle aImageStyle;
        XMLImageStyle::importXML( xAttrList, rAny, rName, GetImport() );
    }
    catch (const Exception&)
    {
        OSL_FAIL("SvxXMLTableImportContext::importBitmap(), exception caught!");
    }
}


SvxXMLXTableImport::SvxXMLXTableImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const uno::Reference< XNameContainer > & rTable,
    uno::Reference< XGraphicObjectResolver > const & xGrfResolver )
:   SvXMLImport(rContext, "", SvXMLImportFlags::NONE),
    mrTable( rTable )
{
    SetGraphicResolver( xGrfResolver );

    GetNamespaceMap().Add( "__ooo", GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap().Add( "__office", GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( "__draw", GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    GetNamespaceMap().Add( "__xlink", GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );

    // OOo namespaces for reading OOo 1.1 files
    GetNamespaceMap().Add( "___office",
                        GetXMLToken(XML_N_OFFICE_OOO),
                        XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( "___draw",
                        GetXMLToken(XML_N_DRAW_OOO),
                        XML_NAMESPACE_DRAW );
}

SvxXMLXTableImport::~SvxXMLXTableImport() throw ()
{
}

static void openStorageStream( xml::sax::InputSource *pParserInput,
                               SvXMLGraphicHelper   **ppGraphicHelper,
                               const uno::Reference < embed::XStorage >& xStorage )
{
    uno::Reference < io::XStream > xIStm;
    xIStm.set( xStorage->openStreamElement( "Content.xml", embed::ElementModes::READ ), uno::UNO_QUERY_THROW );
    if( !xIStm.is() )
    {
        OSL_FAIL( "could not open Content stream" );
        return;
    }
    pParserInput->aInputStream = xIStm->getInputStream();
    *ppGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Read, "" );
}

bool SvxXMLXTableImport::load( const OUString &rPath, const OUString &rReferer,
                               const uno::Reference < embed::XStorage > &xStorage,
                               const uno::Reference< XNameContainer >& xTable,
                               bool *bOptLoadedFromStorage ) throw()
{
    bool bRet = true;
    SvXMLGraphicHelper* pGraphicHelper = nullptr;

    INetURLObject aURLObj( rPath );
    bool bUseStorage = aURLObj.GetProtocol() == INetProtocol::NotValid; // a relative path

    try
    {
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

        xml::sax::InputSource aParserInput;
        comphelper::LifecycleProxy aNasty;

        if( !bUseStorage || !xStorage.is() )
        {
            SfxMedium aMedium( rPath, rReferer, StreamMode::READ | StreamMode::NOCREATE );
            aParserInput.sSystemId = aMedium.GetName();

            if( aMedium.IsStorage() )
            {
                uno::Reference < embed::XStorage > xMediumStorage( aMedium.GetStorage( false ), uno::UNO_QUERY_THROW );
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
                css::uno::Reference< css::io::XStream > xStream;
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
    catch (...)
    {
//      thrown each time you load a document with property tables that are not
//      on the current machine. FIXME: would be better to check a file exists
//      before importing ...
        bRet = false;
    }

    return bRet;
}

SvXMLImportContext *SvxXMLXTableImport::CreateContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& /*xAttrList*/ )
{
    if( XML_NAMESPACE_OOO == nPrefix ||
        XML_NAMESPACE_OFFICE == nPrefix )
    {
        bool bOOoFormat = (XML_NAMESPACE_OFFICE == nPrefix);
        Type aType = mrTable->getElementType();

        if ( rLocalName == "color-table" )
        {
            if( aType == ::cppu::UnoType<sal_Int32>::get() )
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Color, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "marker-table" )
        {
            if( aType == cppu::UnoType<drawing::PolyPolygonBezierCoords>::get())
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Marker, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "dash-table" )
        {
            if( aType == cppu::UnoType<drawing::LineDash>::get())
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Dash, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "hatch-table" )
        {
            if( aType == cppu::UnoType<drawing::Hatch>::get())
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Hatch, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "gradient-table" )
        {
            if( aType == cppu::UnoType<awt::Gradient>::get())
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Gradient, mrTable, bOOoFormat );
        }
        else if ( rLocalName == "bitmap-table" )
        {
            if( aType == ::cppu::UnoType<OUString>::get())
                return new SvxXMLTableImportContext( *this, nPrefix, rLocalName, SvxXMLTableImportContextEnum::Bitmap, mrTable, bOOoFormat );
        }
    }

    return new SvXMLImportContext( *this, nPrefix, rLocalName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
