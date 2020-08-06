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

#include <sal/config.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <sax/tools/converter.hxx>
#include <XMLNumberStylesImport.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "ximppage.hxx"
#include <animimp.hxx>
#include <XMLStringBufferImportContext.hxx>
#include <xmloff/xmlictxt.hxx>
#include "ximpstyl.hxx"
#include <xmloff/prstylei.hxx>
#include <PropertySetMerger.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::geometry;

namespace {

class DrawAnnotationContext : public SvXMLImportContext
{

public:
    DrawAnnotationContext( SvXMLImport& rImport, const Reference< xml::sax::XFastAttributeList>& xAttrList, const Reference< XAnnotationAccess >& xAnnotationAccess );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

private:
    Reference< XAnnotation > mxAnnotation;
    Reference< XTextCursor > mxCursor;

    OUStringBuffer maAuthorBuffer;
    OUStringBuffer maInitialsBuffer;
    OUStringBuffer maDateBuffer;
};

}

DrawAnnotationContext::DrawAnnotationContext( SvXMLImport& rImport, const Reference< xml::sax::XFastAttributeList>& xAttrList, const Reference< XAnnotationAccess >& xAnnotationAccess )
: SvXMLImportContext( rImport )
, mxAnnotation( xAnnotationAccess->createAndInsertAnnotation() )
{
    if( !mxAnnotation.is() )
        return;

    RealPoint2D aPosition;
    RealSize2D aSize;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(SVG, XML_X):
            {
                sal_Int32 x;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        x, sValue);
                aPosition.X = static_cast<double>(x) / 100.0;
                break;
            }
            case XML_ELEMENT(SVG, XML_Y):
            {
                sal_Int32 y;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        y, sValue);
                aPosition.Y = static_cast<double>(y) / 100.0;
                break;
            }
            case XML_ELEMENT(SVG, XML_WIDTH):
            {
                sal_Int32 w;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        w, sValue);
                aSize.Width = static_cast<double>(w) / 100.0;
                break;
            }
            case XML_ELEMENT(SVG, XML_HEIGHT):
            {
                sal_Int32 h;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        h, sValue);
                aSize.Height = static_cast<double>(h) / 100.0;
            }
            break;
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);        }
    }

    mxAnnotation->setPosition( aPosition );
    mxAnnotation->setSize( aSize );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > DrawAnnotationContext::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if( mxAnnotation.is() )
    {
        if (IsTokenInNamespace(nElement, XML_NAMESPACE_DC))
        {
            if( (nElement & TOKEN_MASK) == XML_CREATOR )
                return new XMLStringBufferImportContext(GetImport(), maAuthorBuffer);
            else if( (nElement & TOKEN_MASK) == XML_DATE )
                return new XMLStringBufferImportContext(GetImport(), maDateBuffer);
        }
        else if ( nElement == XML_ELEMENT(TEXT, XML_SENDER_INITIALS)
                || nElement == XML_ELEMENT(LO_EXT, XML_SENDER_INITIALS)
                || nElement == XML_ELEMENT(META, XML_CREATOR_INITIALS))
        {
            return new XMLStringBufferImportContext(GetImport(), maInitialsBuffer);
        }
    }
    return nullptr;
}

SvXMLImportContextRef DrawAnnotationContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( mxAnnotation.is() )
    {
        if( XML_NAMESPACE_DC == nPrefix )
        {
            // handled in createFastChildContext
        }
        else if (((XML_NAMESPACE_TEXT == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix)
                    && IsXMLToken(rLocalName, XML_SENDER_INITIALS))
                 || (XML_NAMESPACE_META == nPrefix
                     && IsXMLToken(rLocalName, XML_CREATOR_INITIALS)))
        {
            // handled in createFastChildContext
        }
        else
        {
            // create text cursor on demand
            if( !mxCursor.is() )
            {
                uno::Reference< text::XText > xText( mxAnnotation->getTextRange() );
                if( xText.is() )
                {
                    rtl::Reference < XMLTextImportHelper > xTxtImport = GetImport().GetTextImport();
                    mxCursor = xText->createTextCursor();
                    if( mxCursor.is() )
                        xTxtImport->SetCursor( mxCursor );
                }
            }

            // if we have a text cursor, lets  try to import some text
            if( mxCursor.is() )
            {
                xContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
            }
        }
    }

    return xContext;
}

void DrawAnnotationContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
}

void DrawAnnotationContext::endFastElement(sal_Int32)
{
    if(mxCursor.is())
    {
        // delete addition newline
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( "" );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if( mxAnnotation.is() )
    {
        mxAnnotation->setAuthor( maAuthorBuffer.makeStringAndClear() );
        mxAnnotation->setInitials( maInitialsBuffer.makeStringAndClear() );

        util::DateTime aDateTime;
        if (::sax::Converter::parseDateTime(aDateTime,
                maDateBuffer.makeStringAndClear()))
        {
            mxAnnotation->setDateTime(aDateTime);
        }
    }
}


SdXMLGenericPageContext::SdXMLGenericPageContext(
    SvXMLImport& rImport,
    const Reference< xml::sax::XFastAttributeList>& xAttrList,
    Reference< drawing::XShapes > const & rShapes)
: SvXMLImportContext( rImport )
, mxShapes( rShapes )
, mxAnnotationAccess( rShapes, UNO_QUERY )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(DRAW, XML_NAV_ORDER) )
        {
            msNavOrder = aIter.toString();
            break;
        }
    }
}

SdXMLGenericPageContext::~SdXMLGenericPageContext()
{
}

void SdXMLGenericPageContext::startFastElement( sal_Int32 /*nElement*/, const Reference< css::xml::sax::XFastAttributeList >& )
{
    GetImport().GetShapeImport()->pushGroupForPostProcessing( mxShapes );

    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->startPage( Reference< drawing::XDrawPage >::query( mxShapes ) );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLGenericPageContext::createFastChildContext(
    sal_Int32 nElement,
    const Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    if( nElement == XML_ELEMENT(PRESENTATION, XML_ANIMATIONS) )
    {
        return new XMLAnimationsContext( GetImport() );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_ANNOTATION) || nElement == XML_ELEMENT(OFFICE_EXT, XML_ANNOTATION) )
    {
        if( mxAnnotationAccess.is() )
            return new DrawAnnotationContext( GetImport(), xAttrList, mxAnnotationAccess );
    }
    return nullptr;
}

SvXMLImportContextRef SdXMLGenericPageContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_ANIMATIONS ) )
    {
        // handled in createFastChildContext
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_FORMS ) )
    {
        if( GetImport().IsFormsSupported() )
            xContext = xmloff::OFormLayerXMLImport::createOfficeFormsContext( GetImport(), nPrefix, rLocalName );
    }
    else if( ((nPrefix == XML_NAMESPACE_OFFICE) || (nPrefix == XML_NAMESPACE_OFFICE_EXT)) && IsXMLToken( rLocalName, XML_ANNOTATION ) )
    {
        // handled in createFastChildContext
    }
    else
    {
        // call GroupChildContext function at common ShapeImport
        xContext = GetImport().GetShapeImport()->CreateGroupChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxShapes);
    }

    return xContext;
}

void SdXMLGenericPageContext::endFastElement(sal_Int32 )
{
    GetImport().GetShapeImport()->popGroupAndPostProcess();

    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->endPage();

    if( !maUseHeaderDeclName.isEmpty() || !maUseFooterDeclName.isEmpty() || !maUseDateTimeDeclName.isEmpty() )
    {
        try
        {
            Reference <beans::XPropertySet> xSet(mxShapes, uno::UNO_QUERY_THROW );
            Reference< beans::XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );

            if( !maUseHeaderDeclName.isEmpty() )
            {
                const OUString aStrHeaderTextProp( "HeaderText" );
                if( xInfo->hasPropertyByName( aStrHeaderTextProp ) )
                    xSet->setPropertyValue( aStrHeaderTextProp,
                                            makeAny( GetSdImport().GetHeaderDecl( maUseHeaderDeclName ) ) );
            }

            if( !maUseFooterDeclName.isEmpty() )
            {
                const OUString aStrFooterTextProp( "FooterText" );
                if( xInfo->hasPropertyByName( aStrFooterTextProp ) )
                    xSet->setPropertyValue( aStrFooterTextProp,
                                        makeAny( GetSdImport().GetFooterDecl( maUseFooterDeclName ) ) );
            }

            if( !maUseDateTimeDeclName.isEmpty() )
            {
                const OUString aStrDateTimeTextProp( "DateTimeText" );
                if( xInfo->hasPropertyByName( aStrDateTimeTextProp ) )
                {
                    bool bFixed;
                    OUString aDateTimeFormat;
                    const OUString aText( GetSdImport().GetDateTimeDecl( maUseDateTimeDeclName, bFixed, aDateTimeFormat ) );

                    xSet->setPropertyValue("IsDateTimeFixed",
                                        makeAny( bFixed ) );

                    if( bFixed )
                    {
                        xSet->setPropertyValue( aStrDateTimeTextProp, makeAny( aText ) );
                    }
                    else if( !aDateTimeFormat.isEmpty() )
                    {
                        const SdXMLStylesContext* pStyles = dynamic_cast< const SdXMLStylesContext* >( GetSdImport().GetShapeImport()->GetStylesContext() );
                        if( !pStyles )
                            pStyles = dynamic_cast< const SdXMLStylesContext* >( GetSdImport().GetShapeImport()->GetAutoStylesContext() );

                        if( pStyles )
                        {
                            const SdXMLNumberFormatImportContext* pSdNumStyle =
                                dynamic_cast< const SdXMLNumberFormatImportContext* >( pStyles->FindStyleChildContext( XmlStyleFamily::DATA_STYLE, aDateTimeFormat, true ) );

                            if( pSdNumStyle )
                            {
                                xSet->setPropertyValue("DateTimeFormat",
                                                                    makeAny( pSdNumStyle->GetDrawKey() ) );
                            }
                        }
                    }
                }
            }
        }
        catch(const uno::Exception&)
        {
            OSL_FAIL("xmloff::SdXMLGenericPageContext::EndElement(), unexpected exception caught!");
        }
    }

    SetNavigationOrder();
}

void SdXMLGenericPageContext::SetStyle( OUString const & rStyleName )
{
    // set PageProperties?
    if(rStyleName.isEmpty())
        return;

    try
    {
        const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetAutoStylesContext();

        if (const SdXMLStylesContext* pStyles = dynamic_cast<const SdXMLStylesContext *>(pContext))
        {
            const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext(
                XmlStyleFamily::SD_DRAWINGPAGE_ID, rStyleName);

            if (const XMLPropStyleContext* pPropStyle = dynamic_cast<const XMLPropStyleContext*>(pStyle))
            {
                Reference <beans::XPropertySet> xPropSet1(mxShapes, uno::UNO_QUERY);
                if(xPropSet1.is())
                {
                    Reference< beans::XPropertySet > xPropSet( xPropSet1 );
                    Reference< beans::XPropertySet > xBackgroundSet;

                    const OUString aBackground("Background");
                    if( xPropSet1->getPropertySetInfo()->hasPropertyByName( aBackground ) )
                    {
                        Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
                        if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
                        {
                            Reference< lang::XMultiServiceFactory > xServiceFact(GetSdImport().GetModel(), uno::UNO_QUERY);
                            if(xServiceFact.is())
                            {
                                xBackgroundSet.set(xServiceFact->createInstance("com.sun.star.drawing.Background"), UNO_QUERY);
                            }
                        }

                        if( xBackgroundSet.is() )
                            xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xBackgroundSet );
                    }

                    if(xPropSet.is())
                    {
                        const_cast<XMLPropStyleContext*>(pPropStyle)->FillPropertySet(xPropSet);

                        if( xBackgroundSet.is() )
                            xPropSet1->setPropertyValue( aBackground, uno::makeAny( xBackgroundSet ) );
                    }
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "SdXMLGenericPageContext::SetStyle(): uno::Exception caught!" );
    }
}

void SdXMLGenericPageContext::SetLayout()
{
    // set PresentationPageLayout?
    if(!GetSdImport().IsImpress() || maPageLayoutName.isEmpty())
        return;

    sal_Int32 nType = -1;

    const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();

    if (const SdXMLStylesContext* pStyles = dynamic_cast<const SdXMLStylesContext *>(pContext))
    {
        const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext( XmlStyleFamily::SD_PRESENTATIONPAGELAYOUT_ID, maPageLayoutName);

        if (const SdXMLPresentationPageLayoutContext* pLayout = dynamic_cast<const SdXMLPresentationPageLayoutContext*>(pStyle))
        {
            nType = pLayout->GetTypeId();
        }
    }

    if( -1 == nType )
    {
        Reference< container::XNameAccess > xPageLayouts( GetSdImport().getPageLayouts() );
        if( xPageLayouts.is() )
        {
            if( xPageLayouts->hasByName( maPageLayoutName ) )
                xPageLayouts->getByName( maPageLayoutName ) >>= nType;
        }

    }

    if( -1 != nType )
    {
        Reference <beans::XPropertySet> xPropSet(mxShapes, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            OUString aPropName("Layout");
            Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aPropName ) )
                xPropSet->setPropertyValue(aPropName, uno::makeAny( static_cast<sal_Int16>(nType) ) );
        }
    }
}

void SdXMLGenericPageContext::DeleteAllShapes()
{
    // now delete all up-to-now contained shapes; they have been created
    // when setting the presentation page layout.
    while(mxShapes->getCount())
    {
        Reference< drawing::XShape > xShape;
        uno::Any aAny(mxShapes->getByIndex(0));

        aAny >>= xShape;

        if(xShape.is())
        {
            mxShapes->remove(xShape);
        }
    }
}

void SdXMLGenericPageContext::SetPageMaster( OUString const & rsPageMasterName )
{
    if (!GetSdImport().GetShapeImport()->GetStylesContext())
        return;

    // look for PageMaster with this name

    // #80012# GetStylesContext() replaced with GetAutoStylesContext()
    const SvXMLStylesContext* pAutoStyles = GetSdImport().GetShapeImport()->GetAutoStylesContext();

    const SvXMLStyleContext* pStyle = pAutoStyles ? pAutoStyles->FindStyleChildContext(XmlStyleFamily::SD_PAGEMASTERCONEXT_ID, rsPageMasterName) : nullptr;

    const SdXMLPageMasterContext* pPageMaster = dynamic_cast<const SdXMLPageMasterContext*>(pStyle);
    if (!pPageMaster)
        return;

    const SdXMLPageMasterStyleContext* pPageMasterContext = pPageMaster->GetPageMasterStyle();

    if (!pPageMasterContext)
        return;

    Reference< drawing::XDrawPage > xMasterPage(GetLocalShapesContext(), uno::UNO_QUERY);
    if (!xMasterPage.is())
        return;

    // set sizes for this masterpage
    Reference <beans::XPropertySet> xPropSet(xMasterPage, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        xPropSet->setPropertyValue("BorderBottom", Any(pPageMasterContext->GetBorderBottom()));
        xPropSet->setPropertyValue("BorderLeft", Any(pPageMasterContext->GetBorderLeft()));
        xPropSet->setPropertyValue("BorderRight", Any(pPageMasterContext->GetBorderRight()));
        xPropSet->setPropertyValue("BorderTop", Any(pPageMasterContext->GetBorderTop()));
        xPropSet->setPropertyValue("Width", Any(pPageMasterContext->GetWidth()));
        xPropSet->setPropertyValue("Height", Any(pPageMasterContext->GetHeight()));
        xPropSet->setPropertyValue("Orientation", Any(pPageMasterContext->GetOrientation()));
    }
}

namespace {

class XoNavigationOrderAccess : public ::cppu::WeakImplHelper< XIndexAccess >
{
public:
    explicit XoNavigationOrderAccess( std::vector< Reference< XShape > >& rShapes );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

private:
    std::vector< Reference< XShape > > maShapes;
};

}

XoNavigationOrderAccess::XoNavigationOrderAccess( std::vector< Reference< XShape > >& rShapes )
{
    maShapes.swap( rShapes );
}

// XIndexAccess
sal_Int32 SAL_CALL XoNavigationOrderAccess::getCount(  )
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

Any SAL_CALL XoNavigationOrderAccess::getByIndex( sal_Int32 Index )
{
    if( (Index < 0) || (Index > getCount()) )
        throw IndexOutOfBoundsException();

    return Any( maShapes[Index] );
}

// XElementAccess
Type SAL_CALL XoNavigationOrderAccess::getElementType(  )
{
    return cppu::UnoType<XShape>::get();
}

sal_Bool SAL_CALL XoNavigationOrderAccess::hasElements(  )
{
    return !maShapes.empty();
}

void SdXMLGenericPageContext::SetNavigationOrder()
{
    if( msNavOrder.isEmpty() )
        return;

    try
    {
        sal_uInt32 nIndex;
        const sal_uInt32 nCount = static_cast< sal_uInt32 >( mxShapes->getCount() );
        std::vector< Reference< XShape > > aShapes( nCount );

        ::comphelper::UnoInterfaceToUniqueIdentifierMapper& rIdMapper = GetSdImport().getInterfaceToIdentifierMapper();
        SvXMLTokenEnumerator aEnumerator( msNavOrder );
        OUString sId;
        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            if( !aEnumerator.getNextToken(sId) )
                break;

            aShapes[nIndex].set( rIdMapper.getReference( sId ), UNO_QUERY );
        }

        for( nIndex = 0; nIndex < nCount; ++nIndex )
        {
            if( !aShapes[nIndex].is() )
            {
                OSL_FAIL("xmloff::SdXMLGenericPageContext::SetNavigationOrder(), draw:nav-order attribute incomplete!");
                // todo: warning?
                return;
            }
        }

        Reference< XPropertySet > xSet( mxShapes, UNO_QUERY_THROW );
        xSet->setPropertyValue("NavigationOrder", Any( Reference< XIndexAccess >( new XoNavigationOrderAccess( aShapes ) ) ) );
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("xmloff::SdXMLGenericPageContext::SetNavigationOrder(), unexpected exception caught while importing shape navigation order!");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
