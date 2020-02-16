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

#include "ximpbody.hxx"
#include <xmloff/xmlnamespace.hxx>
#include "ximpnote.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include "ximpstyl.hxx"
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/families.hxx>
#include "ximpshow.hxx"
#include "layerimp.hxx"
#include <animationimport.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SdXMLDrawPageContext::SdXMLDrawPageContext( SdXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLGenericPageContext( rImport, xAttrList, rShapes )
,   mbHadSMILNodes( false )
{
    bool bHaveXmlId( false );
    OUString sXmlId, sStyleName, sContextName, sMasterPageName, sHREF;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_NAME):
            {
                sContextName = sValue;
                break;
            }
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            {
                sStyleName = sValue;
                break;
            }
            case XML_ELEMENT(DRAW, XML_MASTER_PAGE_NAME):
            {
                sMasterPageName = sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_PRESENTATION_PAGE_LAYOUT_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_PRESENTATION_PAGE_LAYOUT_NAME):
            {
                maPageLayoutName =  sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_HEADER_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_HEADER_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_HEADER_NAME):
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_FOOTER_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_FOOTER_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_FOOTER_NAME):
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_USE_DATE_TIME_NAME):
            case XML_ELEMENT(PRESENTATION_SO52, XML_USE_DATE_TIME_NAME):
            case XML_ELEMENT(PRESENTATION_OOO, XML_USE_DATE_TIME_NAME):
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }
            case XML_ELEMENT(DRAW, XML_ID):
            {
                if (!bHaveXmlId) { sXmlId = sValue; }
            }
            break;
            case XML_ELEMENT(XML, XML_ID):
            {
                sXmlId = sValue;
                bHaveXmlId = true;
            }
            break;
            case XML_ELEMENT(XLINK, XML_HREF):
            {
                sHREF = sValue;
                break;
            }
        }
    }

    if (!sXmlId.isEmpty())
    {
        uno::Reference< uno::XInterface > const xRef( rShapes.get() );
        GetImport().getInterfaceToIdentifierMapper().registerReference(
            sXmlId, xRef );
    }
    GetImport().GetShapeImport()->startPage( rShapes );

    uno::Reference< drawing::XDrawPage > xShapeDrawPage(rShapes, uno::UNO_QUERY);

    // set PageName?
    if(!sContextName.isEmpty())
    {
        if(xShapeDrawPage.is())
        {
            uno::Reference < container::XNamed > xNamed(xShapeDrawPage, uno::UNO_QUERY);
            if(xNamed.is())
                xNamed->setName(sContextName);
        }
    }

    // set MasterPage?
    if(!sMasterPageName.isEmpty())
    {
        // #85906# Code for setting masterpage needs complete rework
        // since GetSdImport().GetMasterStylesContext() gives always ZERO
        // because of content/style file split. Now the mechanism is to
        // compare the wanted masterpage-name with the existing masterpages
        // which were loaded and created in the styles section loading.
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);
        uno::Reference < drawing::XMasterPageTarget > xDrawPage(rShapes, uno::UNO_QUERY);
        uno::Reference< drawing::XDrawPage > xMasterPage;

        if(xDrawPage.is() && xMasterPages.is())
        {
            bool bDone(false);
            OUString sDisplayName( rImport.GetStyleDisplayName(
                            XmlStyleFamily::MASTER_PAGE, sMasterPageName ) );

            for(sal_Int32 a = 0; !bDone && a < xMasterPages->getCount(); a++)
            {
                uno::Any aAny(xMasterPages->getByIndex(a));
                aAny >>= xMasterPage;

                if(xMasterPage.is())
                {
                    uno::Reference < container::XNamed > xMasterNamed(xMasterPage, uno::UNO_QUERY);
                    if(xMasterNamed.is())
                    {
                        OUString sLoopMasterPageName = xMasterNamed->getName();

                        if(!sLoopMasterPageName.isEmpty() && sLoopMasterPageName == sDisplayName)
                        {
                            xDrawPage->setMasterPage(xMasterPage);
                            bDone = true;
                        }
                    }
                }
            }

            SAL_WARN_IF( !bDone, "xmloff", "xmloff::SdXMLDrawPageContext::SdXMLDrawPageContext(), could not find a master slide!" );
        }
    }

    SetStyle( sStyleName );

    if( !sHREF.isEmpty() )
    {
        uno::Reference< beans::XPropertySet > xProps( xShapeDrawPage, uno::UNO_QUERY );
        if( xProps.is() )
        {
            sal_Int32 nIndex = sHREF.lastIndexOf( '#' );
            if( nIndex != -1 )
            {
                OUString aFileName( sHREF.copy( 0, nIndex ) );
                OUString aBookmarkName( sHREF.copy( nIndex+1 ) );

                sHREF = GetImport().GetAbsoluteReference( aFileName ) + "#"
                    + aBookmarkName;
            }

            xProps->setPropertyValue("BookmarkURL", uno::makeAny( sHREF ) );
        }
    }

    SetLayout();

    DeleteAllShapes();
}

SdXMLDrawPageContext::~SdXMLDrawPageContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler >  SdXMLDrawPageContext::createFastChildContext(sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    // some special objects inside draw:page context
    switch(nElement)
    {
        case XML_ELEMENT(PRESENTATION, XML_NOTES):
        case XML_ELEMENT(PRESENTATION_SO52, XML_NOTES):
        case XML_ELEMENT(PRESENTATION_OOO, XML_NOTES):
        {
            if( GetSdImport().IsImpress() )
            {
                // get notes page
                uno::Reference< presentation::XPresentationPage > xPresPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesDrawPage = xPresPage->getNotesPage();
                    if(xNotesDrawPage.is())
                    {
                        // presentation:notes inside draw:page context
                        return new SdXMLNotesContext( GetSdImport(), xAttrList, xNotesDrawPage);
                    }
                }
            }
            break;
        }
        case XML_ELEMENT(ANIMATION, XML_PAR):
        case XML_ELEMENT(ANIMATION_OOO, XML_PAR):
        case XML_ELEMENT(ANIMATION, XML_SEQ):
        case XML_ELEMENT(ANIMATION_OOO, XML_SEQ):
        {
            if( GetSdImport().IsImpress() )
            {
                uno::Reference< animations::XAnimationNodeSupplier > xNodeSupplier(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xNodeSupplier.is())
                {
                    mbHadSMILNodes = true;
                    return new xmloff::AnimationNodeContext( xNodeSupplier->getAnimationNode(), GetSdImport(), nElement, xAttrList );
                }
            }
            break;
        }
        case XML_ELEMENT(DRAW, XML_LAYER_SET):
        {
            return new SdXMLLayerSetContext( GetSdImport() );
        }
    }

    // call parent when no own context was created
    return SdXMLGenericPageContext::createFastChildContext(nElement, xAttrList);
}

void SdXMLDrawPageContext::endFastElement(sal_Int32 nElement)
{
    SdXMLGenericPageContext::endFastElement(nElement);
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());

    if( mbHadSMILNodes )
    {
        uno::Reference< animations::XAnimationNodeSupplier > xNodeSupplier(GetLocalShapesContext(), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > xPageProps( GetLocalShapesContext(), uno::UNO_QUERY );
        if(xNodeSupplier.is())
            xmloff::AnimationNodeContext::postProcessRootNode( xNodeSupplier->getAnimationNode(), xPageProps );
    }
}

SdXMLBodyContext::SdXMLBodyContext( SdXMLImport& rImport )
:   SvXMLImportContext( rImport )
{
}

SdXMLBodyContext::~SdXMLBodyContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLBodyContext::createFastChildContext(
     sal_Int32 nElement,
     const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch (nElement)
    {
        case XML_ELEMENT(PRESENTATION, XML_SETTINGS):
        case XML_ELEMENT(PRESENTATION_SO52, XML_SETTINGS):
        case XML_ELEMENT(PRESENTATION_OOO, XML_SETTINGS):
        {
            return new SdXMLShowsContext( GetSdImport(), xAttrList );
        }
        case XML_ELEMENT(DRAW, XML_PAGE):
        {
            // only read the first page in preview mode
            if( (GetSdImport().GetNewPageCount() == 0) || !GetSdImport().IsPreview() )
            {
                // import this page
                uno::Reference< drawing::XDrawPage > xNewDrawPage;
                uno::Reference< drawing::XDrawPages > xDrawPages(GetSdImport().GetLocalDrawPages(), uno::UNO_QUERY);

                if( !xDrawPages.is() )
                    break;

                if(GetSdImport().GetNewPageCount() + 1 > xDrawPages->getCount())
                {
                    // new page, create and insert
                    xNewDrawPage = xDrawPages->insertNewByIndex(xDrawPages->getCount());
                }
                else
                {
                    // existing page, use it
                    uno::Any aAny(xDrawPages->getByIndex(GetSdImport().GetNewPageCount()));
                    aAny >>= xNewDrawPage;
                }

                // increment global import page counter
                GetSdImport().IncrementNewPageCount();

                if(xNewDrawPage.is())
                {
                    // draw:page inside office:body context
                    return new SdXMLDrawPageContext(GetSdImport(), xAttrList, xNewDrawPage);
                }
            }
            break;
        }
        case XML_ELEMENT(PRESENTATION, XML_HEADER_DECL):
        case XML_ELEMENT(PRESENTATION, XML_FOOTER_DECL):
        case XML_ELEMENT(PRESENTATION, XML_DATE_TIME_DECL):
        {
            return new SdXMLHeaderFooterDeclContext( GetImport(), xAttrList );
            break;
        }
        default:
            assert(false);
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
