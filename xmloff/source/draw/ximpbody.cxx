/*************************************************************************
 *
 *  $RCSfile: ximpbody.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-08 12:19:53 $
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

#pragma hdrstop

#ifndef _XIMPBODY_HXX
#include "ximpbody.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdXMLDrawPageContext::SdXMLDrawPageContext( SdXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGroupShapeContext( rImport, nPrfx, rLocalName, rShapes )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        USHORT nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetDrawPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_DRAWPAGE_NAME:
            {
                maName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_STYLE_NAME:
            {
                maStyleName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_MASTER_PAGE_NAME:
            {
                maMasterPageName = sValue;
                break;
            }
            case XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME:
            {
                maPageLayoutName = sValue;
                break;
            }
        }
    }

    // set PageName?
    if(maName.getLength())
    {
        uno::Reference< drawing::XDrawPage > xDrawPage(rShapes, uno::UNO_QUERY);
        if(xDrawPage.is())
        {
            uno::Reference < container::XNamed > xNamed(xDrawPage, uno::UNO_QUERY);
            if(xNamed.is())
                xNamed->setName(maName);
        }
    }

    // set PageProperties?
    if(maStyleName.getLength())
    {
        const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();

        if( pContext && pContext->ISA( SvXMLStyleContext ) )
        {
            const SdXMLStylesContext* pStyles = (SdXMLStylesContext*)pContext;
            if(pStyles)
            {
                const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, maStyleName);

                if(pStyle && pStyle->ISA(XMLPropStyleContext))
                {
                    XMLPropStyleContext* pPropStyle = (XMLPropStyleContext*)pStyle;


                    uno::Reference <beans::XPropertySet> xPropSet1(rShapes, uno::UNO_QUERY);
                    if(xPropSet1.is())
                    {
                        // since the background items are in a different propertyset
                        // which itself is a property of the pages property set
                        // we now merge these two propertysets if possible to simulate
                        // a single propertyset with all draw page properties
                        const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
                        uno::Reference< beans::XPropertySet > xPropSet2;
                        uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
                        if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
                        {
                            uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetSdImport().GetModel(), uno::UNO_QUERY);
                            if(xServiceFact.is())
                            {
                                uno::Reference< beans::XPropertySet > xTempSet(
                                    xServiceFact->createInstance(
                                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Background"))),
                                    uno::UNO_QUERY);

                                xPropSet2 = xTempSet;
                            }
                        }

                        uno::Reference< beans::XPropertySet > xPropSet;
                        if( xPropSet2.is() )
                            xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xPropSet2 );
                        else
                            xPropSet = xPropSet1;

                        if(xPropSet.is())
                        {
                            pPropStyle->FillPropertySet(xPropSet);
                        }

                        if( xPropSet2.is() )
                        {
                            uno::Any aAny;
                            aAny <<= xPropSet2;
                            xPropSet1->setPropertyValue( aBackground, aAny );
                        }
                    }
                }
            }
        }
    }

    // set MasterPage?
    if(maMasterPageName.getLength())
    {
        const SdXMLMasterStylesContext* pStyles = GetSdImport().GetMasterStylesContext();
        if(pStyles)
        {
            sal_Bool bDone(FALSE);

            for(sal_uInt32 a(0L); !bDone && a < pStyles->GetMasterPageList().Count(); a++)
            {
                const SdXMLMasterPageContext* pMaster = pStyles->GetMasterPageList().GetObject(a);

                if(pMaster && pMaster->GetName().getLength() && pMaster->GetName().equals(maMasterPageName))
                {
                    uno::Reference < drawing::XMasterPageTarget > xDrawPage(rShapes, uno::UNO_QUERY);
                    if(xDrawPage.is())
                    {
                        uno::Reference< drawing::XDrawPage > xMasterPage(
                            pMaster->GetLocalShapesContext(), uno::UNO_QUERY);

                        if(xMasterPage.is())
                        {
                            xDrawPage->setMasterPage(xMasterPage);
                            bDone = TRUE;
                        }
                    }
                }
            }
        }
    }

    // set PresentationPageLayout?
    if(maPageLayoutName.getLength())
    {
        const SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();

        if( pContext && pContext->ISA( SvXMLStyleContext ) )
        {
            const SdXMLStylesContext* pStyles = (SdXMLStylesContext*)pContext;
            if(pStyles)
            {
                const SvXMLStyleContext* pStyle = pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID, maPageLayoutName);

                if(pStyle && pStyle->ISA(SdXMLPresentationPageLayoutContext))
                {
                    SdXMLPresentationPageLayoutContext* pLayout = (SdXMLPresentationPageLayoutContext*)pStyle;
                    uno::Reference <beans::XPropertySet> xPropSet(rShapes, uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        uno::Any aAny;
                        aAny <<= pLayout->GetTypeId();
                        xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Layout")), aAny);
                    }
                }
            }
        }
    }

    // now delete all up-to-now contained shapes; they have been created
    // when setting the presentation page layout.
    while(rShapes->getCount())
    {
        uno::Reference< drawing::XShape > xShape;
        uno::Any aAny(rShapes->getByIndex(0L));

        aAny >>= xShape;

        if(xShape.is())
        {
            rShapes->remove(xShape);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLDrawPageContext::~SdXMLDrawPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLDrawPageContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0L;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetDrawPageElemTokenMap();

    // some special objects inside draw:page context
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_DRAWPAGE_NOTES:
        {
            // presentation:notes inside draw:page context
            pContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName,
                GetLocalShapesContext());
            break;
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SdXMLGroupShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLBodyContext::SdXMLBodyContext( SdXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName )
:   SvXMLImportContext( rImport, nPrfx, rLocalName )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLBodyContext::~SdXMLBodyContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLBodyContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0L;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetBodyElemTokenMap();

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_BODY_PAGE:
        {
            uno::Reference< drawing::XDrawPage > xNewDrawPage;
            uno::Reference< drawing::XDrawPages > xDrawPages(GetSdImport().GetLocalDrawPages(), uno::UNO_QUERY);

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
                    pContext = new SdXMLDrawPageContext(GetSdImport(), nPrefix, rLocalName, xAttrList,
                        xNewShapes);
                }
            }
            break;
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}


