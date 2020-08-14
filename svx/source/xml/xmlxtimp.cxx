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
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <sax/tools/converter.hxx>
#include <sfx2/docfile.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/namespacemap.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/DashStyle.hxx>
#include <xmloff/GradientStyle.hxx>
#include <xmloff/HatchStyle.hxx>
#include <xmloff/ImageStyle.hxx>
#include <xmloff/MarkerStyle.hxx>
#include <xmloff/xmlictxt.hxx>
#include <svx/xmlgrhlp.hxx>
#include <xmloff/attrlist.hxx>

#include <xmlxtimp.hxx>
#include <tools/diagnose_ex.h>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;
using namespace cppu;

namespace {

enum class SvxXMLTableImportContextEnum { Color, Marker, Dash, Hatch, Gradient, Bitmap };

class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable,
        bool bOOoFormat );

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext(sal_Int32 Element,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

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

}

SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, SvxXMLTableImportContextEnum eContext, const uno::Reference< XNameContainer >& xTable, bool bOOoFormat )
: SvXMLImportContext( rImport ), mxTable( xTable ), meContext( eContext ),
    mbOOoFormat( bOOoFormat )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler >
        SvxXMLTableImportContext::createFastChildContext(sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW_OOO) )
    {
        SvXMLAttributeList *pAttrList = new SvXMLAttributeList;
        for (auto& aIter : sax_fastparser::castToFastAttributeList( rAttrList ))
            pAttrList->AddAttribute(
                SvXMLImport::getNamespacePrefixFromToken(aIter.getToken(), nullptr) + ":" +
                GetXMLToken(static_cast<XMLTokenEnum>(aIter.getToken() & TOKEN_MASK)),
                aIter.toString());
        if( mbOOoFormat &&
             (SvxXMLTableImportContextEnum::Dash == meContext || SvxXMLTableImportContextEnum::Hatch == meContext ||
             SvxXMLTableImportContextEnum::Bitmap == meContext) )
        {
            sal_Int16 nAttrCount = pAttrList->getLength();
            for( sal_Int16 i=0; i < nAttrCount; i++ )
            {
                const OUString& rAttrName = pAttrList->getNameByIndex( i );
                OUString aLocalName;
                sal_uInt16 nPrefix_ =
                    GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
                if( XML_NAMESPACE_XLINK == nPrefix_ &&
                    SvxXMLTableImportContextEnum::Bitmap == meContext &&
                    IsXMLToken( aLocalName, XML_HREF ) )
                {
                    const OUString rValue = pAttrList->getValueByIndex( i );
                    if( !rValue.isEmpty() && '#' == rValue[0] )
                        pAttrList->SetValueByIndex( i, rValue.copy( 1 ) );
                }
                else if( (XML_NAMESPACE_DRAW == nPrefix_ || XML_NAMESPACE_DRAW_OOO == nPrefix_) &&
                          ( ( SvxXMLTableImportContextEnum::Dash == meContext &&
                              (IsXMLToken( aLocalName, XML_DOTS1_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DOTS2_LENGTH ) ||
                               IsXMLToken( aLocalName, XML_DISTANCE )) ) ||
                            ( SvxXMLTableImportContextEnum::Hatch == meContext &&
                              IsXMLToken( aLocalName, XML_DISTANCE ) ) ) )
                {
                    const OUString rValue = pAttrList->getValueByIndex( i );
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
                importColor( pAttrList, aAny, aName );
                break;
            case SvxXMLTableImportContextEnum::Marker:
                importMarker( pAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Dash:
                importDash( pAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Hatch:
                importHatch( pAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Gradient:
                importGradient( pAttrList, aAny, aName  );
                break;
            case SvxXMLTableImportContextEnum::Bitmap:
                importBitmap( pAttrList, aAny, aName  );
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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        return new SvXMLImportContext( GetImport() );
    }
    return nullptr;
}

void SvxXMLTableImportContext::importColor( const uno::Reference< XAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aLocalName );


        if( XML_NAMESPACE_DRAW == nPrefix || XML_NAMESPACE_DRAW_OOO == nPrefix )
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
        uno::Any aGraphicAny;
        XMLImageStyle::importXML(xAttrList, aGraphicAny, rName, GetImport());
        if (aGraphicAny.has<uno::Reference<graphic::XGraphic>>())
        {
            auto xGraphic = aGraphicAny.get<uno::Reference<graphic::XGraphic>>();
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            if (xBitmap.is())
                rAny <<= xBitmap;
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("SvxXMLTableImportContext::importBitmap(), exception caught!");
    }
}


SvxXMLXTableImport::SvxXMLXTableImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const uno::Reference< XNameContainer > & rTable,
    uno::Reference<XGraphicStorageHandler> const & xGraphicStorageHandler)
:   SvXMLImport(rContext, "", SvXMLImportFlags::NONE),
    mrTable( rTable )
{
    SetGraphicStorageHandler(xGraphicStorageHandler);

    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK);

    GetNamespaceMap().Add( "__ooo", GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
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
                               rtl::Reference<SvXMLGraphicHelper>& rxGraphicHelper,
                               const uno::Reference < embed::XStorage >& xStorage )
{
    uno::Reference < io::XStream > xIStm( xStorage->openStreamElement( "Content.xml", embed::ElementModes::READ ), uno::UNO_SET_THROW );
    pParserInput->aInputStream = xIStm->getInputStream();
    rxGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Read );
}

bool SvxXMLXTableImport::load( const OUString &rPath, const OUString &rReferer,
                               const uno::Reference < embed::XStorage > &xStorage,
                               const uno::Reference< XNameContainer >& xTable,
                               bool *bOptLoadedFromStorage ) throw()
{
    bool bRet = true;
    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;

    INetURLObject aURLObj( rPath );
    bool bUseStorage = aURLObj.GetProtocol() == INetProtocol::NotValid; // a relative path

    try
    {
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        xml::sax::InputSource aParserInput;
        comphelper::LifecycleProxy aNasty;

        if( !bUseStorage || !xStorage.is() )
        {
            SfxMedium aMedium( rPath, rReferer, StreamMode::READ | StreamMode::NOCREATE );
            aParserInput.sSystemId = aMedium.GetName();

            if( aMedium.IsStorage() )
            {
                uno::Reference < embed::XStorage > xMediumStorage( aMedium.GetStorage( false ), uno::UNO_SET_THROW );
                openStorageStream( &aParserInput, xGraphicHelper, xMediumStorage );
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
                openStorageStream( &aParserInput, xGraphicHelper, xSubStorage );
            else
            {
                css::uno::Reference< css::io::XStream > xStream = comphelper::OStorageHelper::GetStreamAtPath(
                        xStorage, rPath, embed::ElementModes::READ, aNasty );
                if( !xStream.is() )
                    return false;
                aParserInput.aInputStream = xStream->getInputStream();
            }
            if( bOptLoadedFromStorage )
                *bOptLoadedFromStorage = true;
        }

        uno::Reference<XGraphicStorageHandler> xGraphicStorageHandler;
        if (xGraphicHelper.is())
            xGraphicStorageHandler = xGraphicHelper.get();

        try
        {
            uno::Reference< io::XSeekable > xSeek( aParserInput.aInputStream, uno::UNO_QUERY_THROW );
            xSeek->seek( 0 );
        }
        catch (const uno::Exception&)
        {
        }

        rtl::Reference<SvxXMLXTableImport> xImport(new SvxXMLXTableImport(xContext, xTable, xGraphicStorageHandler));
        xImport->parseStream( aParserInput );

        if( xGraphicHelper )
            xGraphicHelper->dispose();
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

SvXMLImportContext *SvxXMLXTableImport::CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_OOO) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_OFFICE) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_OFFICE_OOO) )
    {
        bool bOOoFormat = IsTokenInNamespace(nElement, XML_NAMESPACE_OFFICE) ||
                          IsTokenInNamespace(nElement, XML_NAMESPACE_OFFICE_OOO);
        Type aType = mrTable->getElementType();
        sal_Int32 nToken = nElement & TOKEN_MASK;

        if ( nToken == XML_COLOR_TABLE )
        {
            if( aType == ::cppu::UnoType<sal_Int32>::get() )
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Color, mrTable, bOOoFormat );
        }
        else if ( nToken == XML_MARKER_TABLE )
        {
            if( aType == cppu::UnoType<drawing::PolyPolygonBezierCoords>::get())
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Marker, mrTable, bOOoFormat );
        }
        else if ( nToken == XML_DASH_TABLE )
        {
            if( aType == cppu::UnoType<drawing::LineDash>::get())
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Dash, mrTable, bOOoFormat );
        }
        else if ( nToken == XML_HATCH_TABLE )
        {
            if( aType == cppu::UnoType<drawing::Hatch>::get())
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Hatch, mrTable, bOOoFormat );
        }
        else if ( nToken == XML_GRADIENT_TABLE )
        {
            if( aType == cppu::UnoType<awt::Gradient>::get())
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Gradient, mrTable, bOOoFormat );
        }
        else if ( nToken == XML_BITMAP_TABLE )
        {
            if( aType == ::cppu::UnoType<awt::XBitmap>::get())
                return new SvxXMLTableImportContext( *this, SvxXMLTableImportContextEnum::Bitmap, mrTable, bOOoFormat );
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
