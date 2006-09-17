/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ximppage.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:32:00 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_NUMBERSTYLESIMPORT_HXX
#include "XMLNumberStylesImport.hxx"
#endif

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

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

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGenericPageContext, SvXMLImportContext );

SdXMLGenericPageContext::SdXMLGenericPageContext(
    SvXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList>&,
    Reference< drawing::XShapes >& rShapes)
:   SvXMLImportContext( rImport, nPrfx, rLocalName ),
    mxShapes( rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGenericPageContext::~SdXMLGenericPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGenericPageContext::StartElement( const Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    GetImport().GetShapeImport()->pushGroupForSorting( mxShapes );

#ifndef SVX_LIGHT
    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->startPage( Reference< drawing::XDrawPage >::query( mxShapes ) );
#endif
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGenericPageContext::CreateChildContext( USHORT nPrefix,
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

    if( GetImport().IsFormsSupported() )
        GetImport().GetFormImport()->endPage();

    if( maUseHeaderDeclName.getLength() || maUseFooterDeclName.getLength() || maUseDateTimeDeclName.getLength() )
    {
        try
        {
            Reference <beans::XPropertySet> xSet(mxShapes, uno::UNO_QUERY_THROW );
            Reference< beans::XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );

            if( maUseHeaderDeclName.getLength() )
            {
                const OUString aStrHeaderTextProp( RTL_CONSTASCII_USTRINGPARAM( "HeaderText" ) );
                if( xInfo->hasPropertyByName( aStrHeaderTextProp ) )
                    xSet->setPropertyValue( aStrHeaderTextProp,
                                            makeAny( GetSdImport().GetHeaderDecl( maUseHeaderDeclName ) ) );
            }

            if( maUseFooterDeclName.getLength() )
            {
                const OUString aStrFooterTextProp( RTL_CONSTASCII_USTRINGPARAM( "FooterText" ) );
                if( xInfo->hasPropertyByName( aStrFooterTextProp ) )
                    xSet->setPropertyValue( aStrFooterTextProp,
                                        makeAny( GetSdImport().GetFooterDecl( maUseFooterDeclName ) ) );
            }

            if( maUseDateTimeDeclName.getLength() )
            {
                const OUString aStrDateTimeTextProp( RTL_CONSTASCII_USTRINGPARAM( "DateTimeText" ) );
                if( xInfo->hasPropertyByName( aStrDateTimeTextProp ) )
                {
                    sal_Bool bFixed;
                    OUString aDateTimeFormat;
                    const OUString aText( GetSdImport().GetDateTimeDecl( maUseDateTimeDeclName, bFixed, aDateTimeFormat ) );

                    xSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("IsDateTimeFixed") ),
                                        makeAny( bFixed ) );

                    if( bFixed )
                    {
                        xSet->setPropertyValue( aStrDateTimeTextProp, makeAny( aText ) );
                    }
                    else if( aDateTimeFormat.getLength() )
                    {
                        const SdXMLStylesContext* pStyles = dynamic_cast< const SdXMLStylesContext* >( GetSdImport().GetShapeImport()->GetStylesContext() );
                        if( pStyles )
                        {
                            const SdXMLNumberFormatImportContext* pSdNumStyle =
                                dynamic_cast< const SdXMLNumberFormatImportContext* >( pStyles->FindStyleChildContext( XML_STYLE_FAMILY_DATA_STYLE, aDateTimeFormat, sal_True ) );

                            if( pSdNumStyle )
                            {
                                xSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("DateTimeFormat") ),
                                                                    makeAny( pSdNumStyle->GetDrawKey() ) );
                            }
                        }
                    }
                }
            }
        }
        catch( uno::Exception& e )
        {
            (void)e;
            DBG_ERROR("xmloff::SdXMLGenericPageContext::EndElement(), unexpected exception cought!");
        }
    }
}

void SdXMLGenericPageContext::SetStyle( rtl::OUString& rStyleName )
{
    // set PageProperties?
    if(rStyleName.getLength())
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

                            const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
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
                                            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Background"))));
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
        catch( uno::Exception )
        {
            DBG_ERROR( "SdXMLGenericPageContext::SetStyle(): uno::Exception catched!" );
        }
    }
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
                OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("Layout"));
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
