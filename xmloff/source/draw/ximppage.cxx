/*************************************************************************
 *
 *  $RCSfile: ximppage.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:21:50 $
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XIMPPAGE_HXX
#include "ximppage.hxx"
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

#ifndef _XMLOFF_ANIMIMP_HXX
#include "animimp.hxx"
#endif

#ifndef _XMLOFF_FORMSIMP_HXX
#include "formsimp.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGenericPageContext, SvXMLImportContext );

SdXMLGenericPageContext::SdXMLGenericPageContext(
    SvXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SvXMLImportContext( rImport, nPrfx, rLocalName ),
    mxShapes( rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGenericPageContext::~SdXMLGenericPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGenericPageContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    GetImport().GetShapeImport()->pushGroupForSorting( mxShapes );

#ifndef SVX_LIGHT
    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->startPage( uno::Reference< drawing::XDrawPage >::query( mxShapes ) );
#endif
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGenericPageContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0L;

    if( nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_ANIMATIONS ) )
    {
        pContext = new XMLAnimationsContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_FORMS ) )
    {
#ifndef SVX_LIGHT
        if( GetImport().IsFormsSupported() )
            pContext = GetImport().GetFormImport()->createOfficeFormsContext( GetImport(), nPrefix, rLocalName );
#endif
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

#ifndef SVX_LIGHT
    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->endPage();
#endif
}

void SdXMLGenericPageContext::SetLayout()
{
    // set PresentationPageLayout?
    if(GetSdImport().IsImpress() && maPageLayoutName.getLength())
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
            uno::Reference< container::XNameAccess > xPageLayouts( GetSdImport().getPageLayouts() );
            if( xPageLayouts.is() )
            {
                if( xPageLayouts->hasByName( maPageLayoutName ) )
                    xPageLayouts->getByName( maPageLayoutName ) >>= nType;
            }

        }

        if( -1 != nType )
        {
            uno::Reference <beans::XPropertySet> xPropSet(mxShapes, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("Layout"));
                uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
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
        uno::Reference< drawing::XShape > xShape;
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
                uno::Reference< drawing::XDrawPage > xMasterPage(GetLocalShapesContext(), uno::UNO_QUERY);
                if(xMasterPage.is())
                {
                    // set sizes for this masterpage
                    uno::Reference <beans::XPropertySet> xPropSet(xMasterPage, uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        uno::Any aAny;

                        aAny <<= pPageMasterContext->GetBorderBottom();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom")), aAny);

                        aAny <<= pPageMasterContext->GetBorderLeft();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderLeft")), aAny);

                        aAny <<= pPageMasterContext->GetBorderRight();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderRight")), aAny);

                        aAny <<= pPageMasterContext->GetBorderTop();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("BorderTop")), aAny);

                        aAny <<= pPageMasterContext->GetWidth();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), aAny);

                        aAny <<= pPageMasterContext->GetHeight();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), aAny);

                        aAny <<= pPageMasterContext->GetOrientation();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")), aAny);
                    }
                }
            }
        }

    }
}
