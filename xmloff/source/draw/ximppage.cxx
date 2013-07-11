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

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <cppuhelper/implbase1.hxx>
#include <sax/tools/converter.hxx>
#include "XMLNumberStylesImport.hxx"
#include <xmloff/xmlstyle.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "ximppage.hxx"
#include "ximpshap.hxx"
#include "animimp.hxx"
#include "XMLStringBufferImportContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include "ximpstyl.hxx"
#include <xmloff/prstylei.hxx>
#include "PropertySetMerger.hxx"

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


//////////////////////////////////////////////////////////////////////////////

class DrawAnnotationContext : public SvXMLImportContext
{

public:
    DrawAnnotationContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,const Reference< xml::sax::XAttributeList>& xAttrList, const Reference< XAnnotationAccess >& xAnnotationAccess );

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();

private:
    Reference< XAnnotation > mxAnnotation;
    Reference< XTextCursor > mxCursor;

    OUStringBuffer maAuthorBuffer;
    OUStringBuffer maDateBuffer;
};

DrawAnnotationContext::DrawAnnotationContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,const Reference< xml::sax::XAttributeList>& xAttrList, const Reference< XAnnotationAccess >& xAnnotationAccess )
: SvXMLImportContext( rImport, nPrfx, rLocalName )
, mxAnnotation( xAnnotationAccess->createAndInsertAnnotation() )
{
    if( mxAnnotation.is() )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

        RealPoint2D aPosition;
        RealSize2D aSize;

        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sValue( xAttrList->getValueByIndex( i ) );
            OUString sAttrName( xAttrList->getNameByIndex( i ) );
            OUString aLocalName;
            switch( GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName ) )
            {
            case XML_NAMESPACE_SVG:
                if( IsXMLToken( aLocalName, XML_X ) )
                {
                    sal_Int32 x;
                    GetImport().GetMM100UnitConverter().convertMeasureToCore(
                            x, sValue);
                    aPosition.X = static_cast<double>(x) / 100.0;
                }
                else if( IsXMLToken( aLocalName, XML_Y ) )
                {
                    sal_Int32 y;
                    GetImport().GetMM100UnitConverter().convertMeasureToCore(
                            y, sValue);
                    aPosition.Y = static_cast<double>(y) / 100.0;
                }
                else if( IsXMLToken( aLocalName, XML_WIDTH ) )
                {
                    sal_Int32 w;
                    GetImport().GetMM100UnitConverter().convertMeasureToCore(
                            w, sValue);
                    aSize.Width = static_cast<double>(w) / 100.0;
                }
                else if( IsXMLToken( aLocalName, XML_HEIGHT ) )
                {
                    sal_Int32 h;
                    GetImport().GetMM100UnitConverter().convertMeasureToCore(
                            h, sValue);
                    aSize.Height = static_cast<double>(h) / 100.0;
                }
                break;
            default:
                break;
            }
        }

        mxAnnotation->setPosition( aPosition );
        mxAnnotation->setSize( aSize );
    }
}

SvXMLImportContext * DrawAnnotationContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    if( mxAnnotation.is() )
    {
        if( XML_NAMESPACE_DC == nPrefix )
        {
            if( IsXMLToken( rLocalName, XML_CREATOR ) )
                pContext = new XMLStringBufferImportContext(GetImport(), nPrefix, rLocalName, maAuthorBuffer);
            else if( IsXMLToken( rLocalName, XML_DATE ) )
                pContext = new XMLStringBufferImportContext(GetImport(), nPrefix, rLocalName, maDateBuffer);
        }
        else
        {
            // create text cursor on demand
            if( !mxCursor.is() )
            {
                uno::Reference< text::XText > xText( mxAnnotation->getTextRange() );
                if( xText.is() )
                {
                    UniReference < XMLTextImportHelper > xTxtImport = GetImport().GetTextImport();
                    mxCursor = xText->createTextCursor();
                    if( mxCursor.is() )
                        xTxtImport->SetCursor( mxCursor );
                }
            }

            // if we have a text cursor, lets  try to import some text
            if( mxCursor.is() )
            {
                pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
            }
        }
    }

    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

void DrawAnnotationContext::EndElement()
{
    if(mxCursor.is())
    {
        // delete addition newline
        const OUString aEmpty;
        mxCursor->gotoEnd( sal_False );
        mxCursor->goLeft( 1, sal_True );
        mxCursor->setString( aEmpty );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if( mxAnnotation.is() )
    {
        mxAnnotation->setAuthor( maAuthorBuffer.makeStringAndClear() );

        util::DateTime aDateTime;
        if (::sax::Converter::parseDateTime(aDateTime, 0,
                maDateBuffer.makeStringAndClear()))
        {
            mxAnnotation->setDateTime(aDateTime);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGenericPageContext, SvXMLImportContext );

SdXMLGenericPageContext::SdXMLGenericPageContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList>& xAttrList,
    Reference< drawing::XShapes >& rShapes)
: SvXMLImportContext( rImport, nPrfx, rLocalName )
, mxShapes( rShapes )
, mxAnnotationAccess( rShapes, UNO_QUERY )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if( (nPrefix == XML_NAMESPACE_DRAW) && IsXMLToken( aLocalName, XML_NAV_ORDER ) )
        {
            msNavOrder = xAttrList->getValueByIndex( i );
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGenericPageContext::~SdXMLGenericPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGenericPageContext::StartElement( const Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    GetImport().GetShapeImport()->pushGroupForSorting( mxShapes );

    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->startPage( Reference< drawing::XDrawPage >::query( mxShapes ) );
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGenericPageContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0L;

    if( nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_ANIMATIONS ) )
    {
        pContext = new XMLAnimationsContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_FORMS ) )
    {
        if( GetImport().IsFormsSupported() )
            pContext = GetImport().GetFormImport()->createOfficeFormsContext( GetImport(), nPrefix, rLocalName );
    }
    else if( ((nPrefix == XML_NAMESPACE_OFFICE) || (nPrefix == XML_NAMESPACE_OFFICE_EXT)) && IsXMLToken( rLocalName, XML_ANNOTATION ) )
    {
        if( mxAnnotationAccess.is() )
            pContext = new DrawAnnotationContext( GetImport(), nPrefix, rLocalName, xAttrList, mxAnnotationAccess );
    }
    else
    {
        // call GroupChildContext function at common ShapeImport
        pContext = GetImport().GetShapeImport()->CreateGroupChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxShapes);
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGenericPageContext::EndElement()
{
    GetImport().GetShapeImport()->popGroupAndSort();

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
                    sal_Bool bFixed;
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
                                dynamic_cast< const SdXMLNumberFormatImportContext* >( pStyles->FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE, aDateTimeFormat, sal_True ) );

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
            OSL_FAIL("xmloff::SdXMLGenericPageContext::EndElement(), unexpected exception cought!");
        }
    }

    SetNavigationOrder();
}

void SdXMLGenericPageContext::SetStyle( OUString& rStyleName )
{
    // set PageProperties?
    if(!rStyleName.isEmpty())
    {
        try
        {
            const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetAutoStylesContext();

            if( pContext && pContext->ISA( SvXMLStyleContext ) )
            {
                const SdXMLStylesContext* pStyles = (SdXMLStylesContext*)pContext;
                if(pStyles)
                {
                    const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext(
                        XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, rStyleName);

                    if(pStyle && pStyle->ISA(XMLPropStyleContext))
                    {
                        XMLPropStyleContext* pPropStyle = (XMLPropStyleContext*)pStyle;

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
                                        xBackgroundSet = Reference< beans::XPropertySet >::query(
                                            xServiceFact->createInstance(
                                            OUString("com.sun.star.drawing.Background")));
                                    }
                                }

                                if( xBackgroundSet.is() )
                                    xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xBackgroundSet );
                            }

                            if(xPropSet.is())
                            {
                                pPropStyle->FillPropertySet(xPropSet);

                                if( xBackgroundSet.is() )
                                    xPropSet1->setPropertyValue( aBackground, uno::makeAny( xBackgroundSet ) );
                            }
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
}

void SdXMLGenericPageContext::SetLayout()
{
    // set PresentationPageLayout?
    if(GetSdImport().IsImpress() && !maPageLayoutName.isEmpty())
    {
        sal_Int32 nType = -1;

        const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();

        if( pContext && pContext->ISA( SvXMLStyleContext ) )
        {
            const SdXMLStylesContext* pStyles = (SdXMLStylesContext*)pContext;
            if(pStyles)
            {
                const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext( XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID, maPageLayoutName);

                if(pStyle && pStyle->ISA(SdXMLPresentationPageLayoutContext))
                {
                    SdXMLPresentationPageLayoutContext* pLayout = (SdXMLPresentationPageLayoutContext*)pStyle;
                    nType = pLayout->GetTypeId();
                }
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
                    xPropSet->setPropertyValue(aPropName, uno::makeAny( (sal_Int16)nType ) );
            }
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
        uno::Any aAny(mxShapes->getByIndex(0L));

        aAny >>= xShape;

        if(xShape.is())
        {
            mxShapes->remove(xShape);
        }
    }
}

void SdXMLGenericPageContext::SetPageMaster( OUString& rsPageMasterName )
{
    if( GetSdImport().GetShapeImport()->GetStylesContext() )
    {
        // look for PageMaster with this name

        // #80012# GetStylesContext() replaced with GetAutoStylesContext()
        const SvXMLStylesContext* pAutoStyles = GetSdImport().GetShapeImport()->GetAutoStylesContext();

        const SvXMLStyleContext* pStyle = pAutoStyles ? pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID, rsPageMasterName) : NULL;

        if(pStyle && pStyle->ISA(SdXMLPageMasterContext))
        {
            const SdXMLPageMasterContext* pPageMaster = (SdXMLPageMasterContext*)pStyle;
            const SdXMLPageMasterStyleContext* pPageMasterContext = pPageMaster->GetPageMasterStyle();

            if(pPageMasterContext)
            {
                Reference< drawing::XDrawPage > xMasterPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xMasterPage.is())
                {
                    // set sizes for this masterpage
                    Reference <beans::XPropertySet> xPropSet(xMasterPage, uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        uno::Any aAny;

                        aAny <<= pPageMasterContext->GetBorderBottom();
                        xPropSet->setPropertyValue("BorderBottom", aAny);

                        aAny <<= pPageMasterContext->GetBorderLeft();
                        xPropSet->setPropertyValue("BorderLeft", aAny);

                        aAny <<= pPageMasterContext->GetBorderRight();
                        xPropSet->setPropertyValue("BorderRight", aAny);

                        aAny <<= pPageMasterContext->GetBorderTop();
                        xPropSet->setPropertyValue("BorderTop", aAny);

                        aAny <<= pPageMasterContext->GetWidth();
                        xPropSet->setPropertyValue("Width", aAny);

                        aAny <<= pPageMasterContext->GetHeight();
                        xPropSet->setPropertyValue("Height", aAny);

                        aAny <<= pPageMasterContext->GetOrientation();
                        xPropSet->setPropertyValue("Orientation", aAny);
                    }
                }
            }
        }

    }
}

class XoNavigationOrderAccess : public ::cppu::WeakImplHelper1< XIndexAccess >
{
public:
    XoNavigationOrderAccess( std::vector< Reference< XShape > >& rShapes );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

private:
    std::vector< Reference< XShape > > maShapes;
};

XoNavigationOrderAccess::XoNavigationOrderAccess( std::vector< Reference< XShape > >& rShapes )
{
    maShapes.swap( rShapes );
}

// XIndexAccess
sal_Int32 SAL_CALL XoNavigationOrderAccess::getCount(  ) throw (RuntimeException)
{
    return static_cast< sal_Int32 >( maShapes.size() );
}

Any SAL_CALL XoNavigationOrderAccess::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if( (Index < 0) || (Index > getCount()) )
        throw IndexOutOfBoundsException();

    return Any( maShapes[Index] );
}

// XElementAccess
Type SAL_CALL XoNavigationOrderAccess::getElementType(  ) throw (RuntimeException)
{
    return XShape::static_type();
}

sal_Bool SAL_CALL XoNavigationOrderAccess::hasElements(  ) throw (RuntimeException)
{
    return maShapes.empty() ? sal_False : sal_True;
}

void SdXMLGenericPageContext::SetNavigationOrder()
{
    if( !msNavOrder.isEmpty() ) try
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

            aShapes[nIndex] = Reference< XShape >( rIdMapper.getReference( sId ), UNO_QUERY );
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
        OSL_FAIL("xmloff::SdXMLGenericPageContext::SetNavigationOrder(), unexpected exception cought while importing shape navigation order!");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
