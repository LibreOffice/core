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
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/ColorStop.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <sax/tools/converter.hxx>
#include <sfx2/docfile.hxx>
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/DashStyle.hxx>
#include <xmloff/GradientStyle.hxx>
#include <xmloff/HatchStyle.hxx>
#include <xmloff/ImageStyle.hxx>
#include <xmloff/MarkerStyle.hxx>
#include <xmloff/xmlictxt.hxx>
#include <svx/xmlgrhlp.hxx>

#include <xmlxtimp.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;
using namespace cppu;

namespace {

enum class SvxXMLTableImportContextEnum { Color, Marker, Dash, Hatch, Gradient, Bitmap };

class SvxXMLTableImportContext : public SvXMLImportContext
{
public:
    SvxXMLTableImportContext( SvXMLImport& rImport, SvxXMLTableImportContextEnum eContext, uno::Reference< XNameContainer > xTable,
        bool bOOoFormat );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext(sal_Int32 Element,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

protected:
    static void importColor( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importMarker( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importDash( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importHatch( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName );
    void importBitmap( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName );

private:
    uno::Reference< XNameContainer > mxTable;
    SvxXMLTableImportContextEnum meContext;
    bool mbOOoFormat;
};

}

SvxXMLTableImportContext::SvxXMLTableImportContext( SvXMLImport& rImport, SvxXMLTableImportContextEnum eContext, uno::Reference< XNameContainer > xTable, bool bOOoFormat )
: SvXMLImportContext( rImport ), mxTable(std::move( xTable )), meContext( eContext ),
    mbOOoFormat( bOOoFormat )
{
}

namespace
{
    // MCGR: Helper ImportContext to be able to parse sub-content
    // entries like XMLGradientStopContext which are allowed now
    // for importing Gradients
    class XMLGradientHelperContext : public SvXMLImportContext
    {
    private:
        uno::Reference< XNameContainer > mxTable;
        css::uno::Any          maAny;
        OUString               maStrName;
        std::vector<css::awt::ColorStop> maColorStopVec;

    public:
        XMLGradientHelperContext(
            SvXMLImport& rImport,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
            const css::uno::Reference< XNameContainer >& rxTable);
        virtual ~XMLGradientHelperContext() override;
        virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
            sal_Int32 nElement,
            const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;
        virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
    };

    XMLGradientHelperContext::XMLGradientHelperContext(
        SvXMLImport& rImport,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
        const uno::Reference< XNameContainer >& rxTable)
    :   SvXMLImportContext(rImport),
        mxTable(rxTable)
    {
        try
        {
            // Import GradientStyle
            XMLGradientStyleImport aGradientStyle( GetImport() );
            aGradientStyle.importXML( xAttrList, maAny, maStrName );
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    XMLGradientHelperContext::~XMLGradientHelperContext()
    {
        // if GradientStyle was imported, add to List
        if( !maStrName.isEmpty() && maAny.hasValue() )
        {
            if( mxTable->hasByName( maStrName ) )
            {
                mxTable->replaceByName( maStrName, maAny );
            }
            else
            {
                mxTable->insertByName( maStrName, maAny );
            }
        }
    }

    css::uno::Reference<css::xml::sax::XFastContextHandler> XMLGradientHelperContext::createFastChildContext(
            sal_Int32 nElement,
            const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
    {
        // be prepared & import GradientStop entries
        if (nElement == XML_ELEMENT(LO_EXT, xmloff::token::XML_GRADIENT_STOP))
        {
            return new XMLGradientStopContext(GetImport(), nElement, xAttrList, maColorStopVec);
        }

        return nullptr;
    }

    void XMLGradientHelperContext::endFastElement(sal_Int32 )
    {
        // correcting invalid StopOffset values is done at the model. Therefore we import them here
        // without any change.
        if (!maColorStopVec.empty())
        {
            awt::Gradient2 aGradient;
            maAny >>= aGradient;
            aGradient.ColorStops = comphelper::containerToSequence(maColorStopVec);
            maAny <<= aGradient;
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler >
        SvxXMLTableImportContext::createFastChildContext(sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    if( !(IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW) ||
          IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW_OOO) ))
            return nullptr;

    std::vector<std::pair<sal_Int32, OString>> aTmpAttrList;
    for (auto& aIter : sax_fastparser::castToFastAttributeList( rAttrList ))
        aTmpAttrList.push_back({aIter.getToken(), OString(aIter.toCString())});
    if( mbOOoFormat &&
         (SvxXMLTableImportContextEnum::Dash == meContext || SvxXMLTableImportContextEnum::Hatch == meContext ||
         SvxXMLTableImportContextEnum::Bitmap == meContext) )
    {
        for( auto & aIter : aTmpAttrList )
        {
            sal_Int32 aLocalAttr = aIter.first & TOKEN_MASK;
            if( aIter.first == XML_ELEMENT(XLINK, XML_HREF) &&
                SvxXMLTableImportContextEnum::Bitmap == meContext )
            {
                OString& rValue = aIter.second;
                if( !rValue.isEmpty() && '#' == rValue[0] )
                    rValue = rValue.copy( 1 );
            }
            else if( (IsTokenInNamespace(aIter.first, XML_NAMESPACE_DRAW) || IsTokenInNamespace(aIter.first, XML_NAMESPACE_DRAW_OOO)) &&
                      ( ( SvxXMLTableImportContextEnum::Dash == meContext &&
                          ( aLocalAttr == XML_DOTS1_LENGTH ||
                            aLocalAttr == XML_DOTS2_LENGTH ||
                            aLocalAttr == XML_DISTANCE ) ) ||
                        ( SvxXMLTableImportContextEnum::Hatch == meContext &&
                          ( aLocalAttr == XML_DISTANCE ) ) ) )
            {
                OString& rValue = aIter.second;
                sal_Int32 nPos = rValue.getLength();
                while( nPos && rValue[nPos-1] <= ' ' )
                    --nPos;
                if( nPos > 2 &&
                    ('c'==rValue[nPos-2] || 'C'==rValue[nPos-2]) &&
                    ('h'==rValue[nPos-1] || 'H'==rValue[nPos-1]) )
                {
                    rValue = rValue.copy( 0, nPos-2 );
                }
            }
        }
    }

    if (nElement == XML_ELEMENT(DRAW, XML_GRADIENT))
    {
        // MCGR: for Gradients, no longer use fixed import but use an own
        // ImportContext to be able to import now possible sub-entries like
        // ColorStop entries
        return new XMLGradientHelperContext( GetImport(), rAttrList, mxTable );
    }

    try
    {
        rtl::Reference<sax_fastparser::FastAttributeList> xFastList = new sax_fastparser::FastAttributeList(nullptr);
        for (const auto& aIter : aTmpAttrList)
            xFastList->add(aIter.first, aIter.second);

        Any aAny;
        OUString aName;

        switch( meContext )
        {
        case SvxXMLTableImportContextEnum::Color:
            importColor( xFastList, aAny, aName );
            break;
        case SvxXMLTableImportContextEnum::Marker:
            importMarker( xFastList, aAny, aName  );
            break;
        case SvxXMLTableImportContextEnum::Dash:
            importDash( xFastList, aAny, aName  );
            break;
        case SvxXMLTableImportContextEnum::Hatch:
            importHatch( xFastList, aAny, aName  );
            break;
        case SvxXMLTableImportContextEnum::Bitmap:
            importBitmap( xFastList, aAny, aName  );
            break;
        default:
            // SvxXMLTableImportContextEnum::Gradient
            // is no longer imported as 'fixed content'
            // but dynamically using an own ImportContext
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

void SvxXMLTableImportContext::importColor( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_NAME):
            case XML_ELEMENT(DRAW_OOO, XML_NAME):
                rName = aIter.toString();
                break;
            case XML_ELEMENT(DRAW, XML_COLOR):
            case XML_ELEMENT(DRAW_OOO, XML_COLOR):
            {
                sal_Int32 nColor(0);
                ::sax::Converter::convertColor(nColor, aIter.toView());
                rAny <<= nColor;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SvxXMLTableImportContext::importMarker( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    try
    {
        XMLMarkerStyleImport aMarkerStyle( GetImport() );
        aMarkerStyle.importXML( xAttrList, rAny, rName );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }
}

void SvxXMLTableImportContext::importDash( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    try
    {
        XMLDashStyleImport aDashStyle( GetImport() );
        aDashStyle.importXML( xAttrList, rAny, rName );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }
}

void SvxXMLTableImportContext::importHatch( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName )
{
    try
    {
        XMLHatchStyleImport aHatchStyle( GetImport() );
        aHatchStyle.importXML( xAttrList, rAny, rName );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }
}

void SvxXMLTableImportContext::importBitmap( const uno::Reference< XFastAttributeList >& xAttrList, Any& rAny, OUString& rName )
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
        TOOLS_WARN_EXCEPTION("svx", "");
    }
}


SvxXMLXTableImport::SvxXMLXTableImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const uno::Reference< XNameContainer > & rTable,
    uno::Reference<XGraphicStorageHandler> const & xGraphicStorageHandler)
:   SvXMLImport(rContext, u""_ustr, SvXMLImportFlags::NONE),
    mrTable( rTable )
{
    SetGraphicStorageHandler(xGraphicStorageHandler);

    GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW);
    GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK);

    GetNamespaceMap().Add( u"__ooo"_ustr, GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap().Add( u"__xlink"_ustr, GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );

    // OOo namespaces for reading OOo 1.1 files
    GetNamespaceMap().Add( u"___office"_ustr,
                        GetXMLToken(XML_N_OFFICE_OOO),
                        XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( u"___draw"_ustr,
                        GetXMLToken(XML_N_DRAW_OOO),
                        XML_NAMESPACE_DRAW );
    GetNamespaceMap().Add( u"___loext"_ustr,
                        GetXMLToken(XML_N_LO_EXT),
                        XML_NAMESPACE_LO_EXT);
}

SvxXMLXTableImport::~SvxXMLXTableImport() noexcept
{
}

static void openStorageStream( xml::sax::InputSource *pParserInput,
                               rtl::Reference<SvXMLGraphicHelper>& rxGraphicHelper,
                               const uno::Reference < embed::XStorage >& xStorage )
{
    uno::Reference < io::XStream > xIStm( xStorage->openStreamElement( u"Content.xml"_ustr, embed::ElementModes::READ ), uno::UNO_SET_THROW );
    pParserInput->aInputStream = xIStm->getInputStream();
    rxGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Read );
}

bool SvxXMLXTableImport::load( const OUString &rPath, const OUString &rReferer,
                               const uno::Reference < embed::XStorage > &xStorage,
                               const uno::Reference< XNameContainer >& xTable,
                               bool *bOptLoadedFromStorage ) noexcept
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
