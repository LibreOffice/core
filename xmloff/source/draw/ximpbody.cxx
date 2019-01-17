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
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlnmspe.hxx>
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
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/families.hxx>
#include "ximpshow.hxx"
#include "layerimp.hxx"
#include <PropertySetMerger.hxx>
#include <animationimport.hxx>
#include <osl/diagnose.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

SdXMLDrawPageContext::SdXMLDrawPageContext( SdXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
,   mbHadSMILNodes( false )
{
    bool bHaveXmlId( false );
    OUString sXmlId, sStyleName, sContextName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetDrawPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_DRAWPAGE_NAME:
            {
                sContextName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_STYLE_NAME:
            {
                sStyleName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_MASTER_PAGE_NAME:
            {
                maMasterPageName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME:
            {
                maPageLayoutName =  sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_USE_HEADER_NAME:
            {
                maUseHeaderDeclName =  sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_USE_FOOTER_NAME:
            {
                maUseFooterDeclName =  sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_USE_DATE_TIME_NAME:
            {
                maUseDateTimeDeclName =  sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_DRAWID:
            {
                if (!bHaveXmlId) { sXmlId = sValue; }
            }
            break;
            case XML_TOK_DRAWPAGE_XMLID:
            {
                sXmlId = sValue;
                bHaveXmlId = true;
            }
            break;
            case XML_TOK_DRAWPAGE_HREF:
            {
                maHREF = sValue;
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
    if(!maMasterPageName.isEmpty())
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
                            XML_STYLE_FAMILY_MASTER_PAGE, maMasterPageName ) );

            for(sal_Int32 a = 0; !bDone && a < xMasterPages->getCount(); a++)
            {
                uno::Any aAny(xMasterPages->getByIndex(a));
                aAny >>= xMasterPage;

                if(xMasterPage.is())
                {
                    uno::Reference < container::XNamed > xMasterNamed(xMasterPage, uno::UNO_QUERY);
                    if(xMasterNamed.is())
                    {
                        OUString sMasterPageName = xMasterNamed->getName();

                        if(!sMasterPageName.isEmpty() && sMasterPageName == sDisplayName)
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

    if( !maHREF.isEmpty() )
    {
        uno::Reference< beans::XPropertySet > xProps( xShapeDrawPage, uno::UNO_QUERY );
        if( xProps.is() )
        {
            sal_Int32 nIndex = maHREF.lastIndexOf( '#' );
            if( nIndex != -1 )
            {
                OUString aFileName( maHREF.copy( 0, nIndex ) );
                OUString aBookmarkName( maHREF.copy( nIndex+1 ) );

                maHREF = GetImport().GetAbsoluteReference( aFileName ) + "#"
                    + aBookmarkName;
            }

            xProps->setPropertyValue("BookmarkURL", uno::makeAny( maHREF ) );
        }
    }

    SetLayout();

    DeleteAllShapes();
}

SdXMLDrawPageContext::~SdXMLDrawPageContext()
{
}

SvXMLImportContextRef SdXMLDrawPageContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetDrawPageElemTokenMap();

    // some special objects inside draw:page context
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_DRAWPAGE_NOTES:
        {
            if( GetSdImport().IsImpress() )
            {
                // get notes page
                uno::Reference< presentation::XPresentationPage > xPresPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesDrawPage(xPresPage->getNotesPage(), uno::UNO_QUERY);
                    if(xNotesDrawPage.is())
                    {
                        uno::Reference< drawing::XShapes > xNewShapes(xNotesDrawPage, uno::UNO_QUERY);
                        if(xNewShapes.is())
                        {
                            // presentation:notes inside draw:page context
                            xContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName, xAttrList, xNewShapes);
                        }
                    }
                }
            }
            break;
        }
        case XML_TOK_DRAWPAGE_PAR:
        case XML_TOK_DRAWPAGE_SEQ:
        {
            if( GetSdImport().IsImpress() )
            {
                uno::Reference< animations::XAnimationNodeSupplier > xNodeSupplier(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xNodeSupplier.is())
                {
                    xContext = new xmloff::AnimationNodeContext( xNodeSupplier->getAnimationNode(), GetSdImport(), nPrefix, rLocalName, xAttrList );
                    mbHadSMILNodes = true;
                }
            }
            break;
        }
        case XML_TOK_DRAWPAGE_LAYER_SET:
        {
            xContext = new SdXMLLayerSetContext( GetSdImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    // call parent when no own context was created
    if (!xContext)
        xContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

void SdXMLDrawPageContext::EndElement()
{
    SdXMLGenericPageContext::EndElement();
    GetImport().GetShapeImport()->endPage(GetLocalShapesContext());

    if( mbHadSMILNodes )
    {
        uno::Reference< animations::XAnimationNodeSupplier > xNodeSupplier(GetLocalShapesContext(), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > xPageProps( GetLocalShapesContext(), uno::UNO_QUERY );
        if(xNodeSupplier.is())
            xmloff::AnimationNodeContext::postProcessRootNode( xNodeSupplier->getAnimationNode(), xPageProps );
    }
}

SdXMLBodyContext::SdXMLBodyContext( SdXMLImport& rImport,
    const OUString& rLocalName )
:   SvXMLImportContext( rImport, XML_NAMESPACE_OFFICE, rLocalName )
{
}

SdXMLBodyContext::~SdXMLBodyContext()
{
}

SvXMLImportContextRef SdXMLBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetBodyElemTokenMap();

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_BODY_HEADER_DECL:
        case XML_TOK_BODY_FOOTER_DECL:
        case XML_TOK_BODY_DATE_TIME_DECL:
        {
            xContext = new SdXMLHeaderFooterDeclContext( GetImport(), nPrefix, rLocalName, xAttrList );
            break;
        }
        case XML_TOK_BODY_PAGE:
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
                    uno::Reference< drawing::XShapes > xNewShapes(xNewDrawPage, uno::UNO_QUERY);
                    if(xNewShapes.is())
                    {
                        // draw:page inside office:body context
                        xContext = new SdXMLDrawPageContext(GetSdImport(), nPrefix, rLocalName, xAttrList,
                            xNewShapes);
                    }
                }
            }
            break;
        }
        case XML_TOK_BODY_SETTINGS:
        {
            xContext = new SdXMLShowsContext( GetSdImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    // call parent when no own context was created
    if (!xContext)
        xContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return xContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
