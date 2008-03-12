/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ximpbody.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:37:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#ifndef _XIMPBODY_HXX
#include "ximpbody.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
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

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
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

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODESUPPLIER_HPP_
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#endif

#include "unointerfacetouniqueidentifiermapper.hxx"

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif

#ifndef _XMLOFF_XIMPSHOW_HXX
#include "ximpshow.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _XMLOFF_ANIMATIONIMPORT_HXX
#include "animationimport.hxx"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdXMLDrawPageContext::SdXMLDrawPageContext( SdXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGenericPageContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
,   mbHadSMILNodes( false )
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

            case XML_TOK_DRAWPAGE_ID:
            {
                uno::Reference< uno::XInterface > xRef( rShapes.get() );
                GetImport().getInterfaceToIdentifierMapper().registerReference( sValue, xRef );
                break;
            }
            case XML_TOK_DRAWPAGE_HREF:
            {
                maHREF = sValue;
                break;
            }
        }
    }

    GetImport().GetShapeImport()->startPage( rShapes );

    uno::Reference< drawing::XDrawPage > xShapeDrawPage(rShapes, uno::UNO_QUERY);

    // set PageName?
    if(maName.getLength())
    {
        if(xShapeDrawPage.is())
        {
            uno::Reference < container::XNamed > xNamed(xShapeDrawPage, uno::UNO_QUERY);
            if(xNamed.is())
                xNamed->setName(maName);
        }
    }

    // set MasterPage?
    if(maMasterPageName.getLength())
    {
        // #85906# Code for setting masterpage needs complete rework
        // since GetSdImport().GetMasterStylesContext() gives always ZERO
        // because of content/style file split. Now the nechanism is to
        // compare the wanted masterpage-name with the existing masterpages
        // which were loaded and created in the styles section loading.
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);
        uno::Reference < drawing::XMasterPageTarget > xDrawPage(rShapes, uno::UNO_QUERY);
        uno::Reference< drawing::XDrawPage > xMasterPage;

        if(xDrawPage.is() && xMasterPages.is())
        {
            sal_Bool bDone(FALSE);
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

                        if(sMasterPageName.getLength() &&
                            sMasterPageName.equals(sDisplayName))
                        {
                            xDrawPage->setMasterPage(xMasterPage);
                            bDone = TRUE;
                        }
                    }
                }
            }

            DBG_ASSERT( bDone, "xmloff::SdXMLDrawPageContext::SdXMLDrawPageContext(), could not find a slide master!" );
        }
    }

    SetStyle( maStyleName );

    if( maHREF.getLength() )
    {
        uno::Reference< beans::XPropertySet > xProps( xShapeDrawPage, uno::UNO_QUERY );
        if( xProps.is() )
        {
            sal_Int32 nIndex = maHREF.lastIndexOf( (sal_Unicode)'#' );
            if( nIndex != -1 )
            {
                OUString aFileName( maHREF.copy( 0, nIndex ) );
                OUString aBookmarkName( maHREF.copy( nIndex+1 ) );

                maHREF = GetImport().GetAbsoluteReference( aFileName );
                maHREF += String( (sal_Unicode)'#' );
                maHREF += aBookmarkName;
            }

            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "BookmarkURL" ) ), uno::makeAny( maHREF ) );
        }
    }

    SetLayout();

    DeleteAllShapes();
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
                            pContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName, xAttrList, xNewShapes);
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
                    pContext = new xmloff::AnimationNodeContext( xNodeSupplier->getAnimationNode(), GetSdImport(), nPrefix, rLocalName, xAttrList );
                    mbHadSMILNodes = true;
                }
            }
            break;
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SdXMLGenericPageContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
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
            xmloff::AnimationNodeContext::postProcessRootNode( GetSdImport(), xNodeSupplier->getAnimationNode(), xPageProps );
    }
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
        case XML_TOK_BODY_HEADER_DECL:
        case XML_TOK_BODY_FOOTER_DECL:
        case XML_TOK_BODY_DATE_TIME_DECL:
        {
            pContext = new SdXMLHeaderFooterDeclContext( GetImport(), nPrefix, rLocalName, xAttrList );
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
                        pContext = new SdXMLDrawPageContext(GetSdImport(), nPrefix, rLocalName, xAttrList,
                            xNewShapes);
                    }
                }
            }
            break;
        }
        case XML_TOK_BODY_SETTINGS:
        {
            pContext = new SdXMLShowsContext( GetSdImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    // call parent when no own context was created
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}
