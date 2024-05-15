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

#include <PageMasterImportContext.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "PageMasterPropHdl.hxx"
#include "PagePropertySetContext.hxx"
#include "PageHeaderFooterContext.hxx"
#include <PageMasterStyleMap.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>

//
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//
using namespace ::com::sun::star::beans;

void PageStyleContext::SetAttribute( sal_Int32 nElement,
                                        const OUString& rValue )
{
    if( nElement == XML_ELEMENT(STYLE, XML_PAGE_USAGE) )
    {
        sPageUsage = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nElement, rValue );
    }
}


PageStyleContext::PageStyleContext( SvXMLImport& rImport,
        SvXMLStylesContext& rStyles,
        bool bDefaultStyle) :
    XMLPropStyleContext( rImport, rStyles, XmlStyleFamily::PAGE_MASTER, bDefaultStyle),
    m_bIsFillStyleAlreadyConverted(false) //
{
}

PageStyleContext::~PageStyleContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > PageStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_HEADER_STYLE) ||
        nElement == XML_ELEMENT(STYLE, XML_FOOTER_STYLE) )
    {
        bool bHeader = nElement == XML_ELEMENT(STYLE, XML_HEADER_STYLE);
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {
            const rtl::Reference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
            sal_Int32 nFlag;
            if (bHeader)
                nFlag = CTF_PM_HEADERFLAG;
            else
                nFlag = CTF_PM_FOOTERFLAG;
            sal_Int32 nStartIndex (-1);
            sal_Int32 nEndIndex (-1);
            bool bFirst(false);
            bool bEnd(false);
            sal_Int32 nIndex = 0;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                if ((rMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK) == nFlag)
                {
                    if (!bFirst)
                    {
                        bFirst = true;
                        nStartIndex = nIndex;
                    }
                }
                else if (bFirst)
                {
                    bEnd = true;
                    nEndIndex = nIndex;
                }
                nIndex++;
            }
            if (!bEnd)
                nEndIndex = nIndex;
            return new PageHeaderFooterContext(GetImport(),
                            GetProperties(), xImpPrMap, nStartIndex, nEndIndex, bHeader);
        }
    }

    if( nElement == XML_ELEMENT(STYLE, XML_PAGE_LAYOUT_PROPERTIES) )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {
            const rtl::Reference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
            sal_Int32 nEndIndex (-1);
            bool bEnd(false);
            sal_Int32 nIndex = 0;
            sal_Int16 nContextID;
            while ( nIndex < rMapper->GetEntryCount() && !bEnd)
            {
                nContextID = rMapper->GetEntryContextId( nIndex );
                if (nContextID && ((nContextID & CTF_PM_FLAGMASK) != XML_PM_CTF_START))
                {
                    nEndIndex = nIndex;
                    bEnd = true;
                }
                nIndex++;
            }
            if (!bEnd)
                nEndIndex = nIndex;
            return new PagePropertySetContext( GetImport(), nElement,
                                                    xAttrList,
                                                    XML_TYPE_PROP_PAGE_LAYOUT,
                                                    GetProperties(),
                                                    xImpPrMap, 0, nEndIndex, Page);
        }
    }

    return XMLPropStyleContext::createFastChildContext(nElement, xAttrList);
}

void PageStyleContext::FillPropertySet(const uno::Reference<beans::XPropertySet > &)
{
    assert(false); // don't call this virtual, call function below
}

void PageStyleContext::FillPropertySet_PageStyle(
        const uno::Reference<beans::XPropertySet> & xPropSet,
        XMLPropStyleContext *const pDrawingPageStyle)
{
    // need to filter out old fill definitions when the new ones are used. The new
    // ones are used when a FillStyle is defined
    if(!m_bIsFillStyleAlreadyConverted && !GetProperties().empty())
    {
        static constexpr OUString s_FillStyle(u"FillStyle"_ustr);
        static constexpr OUStringLiteral s_HeaderFillStyle(u"HeaderFillStyle");
        static constexpr OUStringLiteral s_FooterFillStyle(u"FooterFillStyle");

        // note: the function must only check by property name, not any id/flag!
        if (doNewDrawingLayerFillStyleDefinitionsExist(s_FillStyle)
            || (pDrawingPageStyle && pDrawingPageStyle->doNewDrawingLayerFillStyleDefinitionsExist(s_FillStyle)))
        {
            deactivateOldFillStyleDefinitions(getStandardSet());
        }

        if(doNewDrawingLayerFillStyleDefinitionsExist(s_HeaderFillStyle))
        {
            deactivateOldFillStyleDefinitions(getHeaderSet());
        }

        if(doNewDrawingLayerFillStyleDefinitionsExist(s_FooterFillStyle))
        {
            deactivateOldFillStyleDefinitions(getFooterSet());
        }

        m_bIsFillStyleAlreadyConverted = true;
    }

    // do not use XMLPropStyleContext::FillPropertySet, we need to handle this ourselves since
    // we have properties which use the MID_FLAG_NO_PROPERTY_IMPORT flag since they need some special
    // handling
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = GetStyles()->GetImportPropertyMapper(GetFamily());

    if(xImpPrMap.is())
    {
        // properties that need special handling because they need the used name to be translated first
        struct ContextID_Index_Pair aContextIDs[] =
        {
            { CTF_PM_FILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT },
            { CTF_PM_FILLTRANSNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
            { CTF_PM_FILLHATCHNAME, -1, drawing::FillStyle::FillStyle_HATCH },
            { CTF_PM_FILLBITMAPNAME, -1, drawing::FillStyle::FillStyle_BITMAP },

            // also need to special handling for header entries
            { CTF_PM_HEADERFILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT },
            { CTF_PM_HEADERFILLTRANSNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
            { CTF_PM_HEADERFILLHATCHNAME, -1, drawing::FillStyle::FillStyle_HATCH },
            { CTF_PM_HEADERFILLBITMAPNAME, -1, drawing::FillStyle::FillStyle_BITMAP },

            // also need to special handling for footer entries
            { CTF_PM_FOOTERFILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT },
            { CTF_PM_FOOTERFILLTRANSNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
            { CTF_PM_FOOTERFILLHATCHNAME, -1, drawing::FillStyle::FillStyle_HATCH },
            { CTF_PM_FOOTERFILLBITMAPNAME, -1, drawing::FillStyle::FillStyle_BITMAP },

            {-1, -1, drawing::FillStyle::FillStyle_GRADIENT}
        };

        // the style families associated with the same index modulo 4
        static const XmlStyleFamily aFamilies[] =
        {
            XmlStyleFamily::SD_GRADIENT_ID,
            XmlStyleFamily::SD_GRADIENT_ID,
            XmlStyleFamily::SD_HATCH_ID,
            XmlStyleFamily::SD_FILL_IMAGE_ID
        };

        // Fill PropertySet, but let it handle special properties not itself
        xImpPrMap->FillPropertySet(GetProperties(), xPropSet, aContextIDs);

        // get property set mapper
        const rtl::Reference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
        Reference<XPropertySetInfo> const xInfo(xPropSet->getPropertySetInfo());

        // don't look at the attributes, look at the property, could
        // theoretically be inherited and we don't want to delete erroneously
        drawing::FillStyle fillStyle{drawing::FillStyle_NONE};
        drawing::FillStyle fillStyleHeader{drawing::FillStyle_NONE};
        drawing::FillStyle fillStyleFooter{drawing::FillStyle_NONE};
        if (xInfo->hasPropertyByName(u"FillStyle"_ustr)) // SwXTextDefaults lacks it?
        {
            xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= fillStyle;
            xPropSet->getPropertyValue(u"HeaderFillStyle"_ustr) >>= fillStyleHeader;
            xPropSet->getPropertyValue(u"FooterFillStyle"_ustr) >>= fillStyleFooter;
        }

        // handle special attributes which have MID_FLAG_NO_PROPERTY_IMPORT set
        for(sal_uInt16 i = 0; aContextIDs[i].nContextID != -1; i++)
        {
            sal_Int32 nIndex = aContextIDs[i].nIndex;

            if(nIndex != -1)
            {
                drawing::FillStyle const* pFillStyle(nullptr);
                switch(aContextIDs[i].nContextID)
                {
                    case CTF_PM_FILLGRADIENTNAME:
                    case CTF_PM_FILLTRANSNAME:
                    case CTF_PM_FILLHATCHNAME:
                    case CTF_PM_FILLBITMAPNAME:
                        pFillStyle = &fillStyle;
                        [[fallthrough]];
                    case CTF_PM_HEADERFILLGRADIENTNAME:
                    case CTF_PM_HEADERFILLTRANSNAME:
                    case CTF_PM_HEADERFILLHATCHNAME:
                    case CTF_PM_HEADERFILLBITMAPNAME:
                        if (!pFillStyle) { pFillStyle = &fillStyleHeader; }
                        [[fallthrough]];
                    case CTF_PM_FOOTERFILLGRADIENTNAME:
                    case CTF_PM_FOOTERFILLTRANSNAME:
                    case CTF_PM_FOOTERFILLHATCHNAME:
                    case CTF_PM_FOOTERFILLBITMAPNAME:
                    {
                        if (!pFillStyle) { pFillStyle = &fillStyleFooter; }
                        struct XMLPropertyState& rState = GetProperties()[nIndex];
                        OUString sStyleName;
                        rState.maValue >>= sStyleName;

                        if (aContextIDs[i].nExpectedFillStyle != drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE
                            && aContextIDs[i].nExpectedFillStyle != *pFillStyle)
                        {
                            SAL_INFO("xmloff.style", "PageStyleContext: dropping fill named item: " << sStyleName);
                            break; // ignore it, it's not used
                        }
                        // translate the used name from ODF intern to the name used in the Model
                        sStyleName = GetImport().GetStyleDisplayName(aFamilies[i%4], sStyleName);

                        try
                        {
                            // set property
                            const OUString& rPropertyName = rMapper->GetEntryAPIName(rState.mnIndex);

                            if(xInfo->hasPropertyByName(rPropertyName))
                            {
                                xPropSet->setPropertyValue(rPropertyName,Any(sStyleName));
                            }
                        }
                        catch(css::lang::IllegalArgumentException& e)
                        {
                            Sequence<OUString> aSeq { sStyleName };
                            GetImport().SetError(
                                XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                                aSeq,e.Message,nullptr);
                        }
                        break;
                    }
                }
            }
        }
    }
    else
    {
        OSL_ENSURE(xImpPrMap.is(), "Got no SvXMLImportPropertyMapper (!)");
    }

    // pDrawingPageStyle overrides this
    if (pDrawingPageStyle)
    {
        pDrawingPageStyle->FillPropertySet(xPropSet);
    }
    // horrible heuristic to guess BackgroundFullSize for Writer < 7.0
    else if (!IsDefaultStyle() // ignore pool default, only fix existing styles
            && (GetImport().isGeneratorVersionOlderThan(SvXMLImport::AOO_4x, SvXMLImport::LO_7x)
            // also for AOO 4.x, assume there won't ever be a 4.2
            || GetImport().getGeneratorVersion() == SvXMLImport::AOO_4x))
    {
        bool isFullSize(true); // default is current LO default
        drawing::FillStyle fillStyle{drawing::FillStyle_NONE};
        xPropSet->getPropertyValue(u"FillStyle"_ustr) >>= fillStyle;
        if (GetImport().isGeneratorVersionOlderThan(SvXMLImport::AOO_4x, SvXMLImport::LO_63x)
            // also for AOO 4.x, assume there won't ever be a 4.2
            || GetImport().getGeneratorVersion() == SvXMLImport::AOO_4x)
        {
            // before LO 6.3, always inside the margins (but ignore it if NONE)
            if (fillStyle != drawing::FillStyle_NONE)
            {
                isFullSize = false;
            }
        }
        else
        {
            // LO 6.3/6.4: guess depending on fill style/bitmap mode
            // this should work even if the document doesn't contain fill style
            // but only old background attributes
            // (can't use the aContextIDs stuff above because that requires
            //  re-routing through handleSpecialItem())
            switch (fillStyle)
            {
                case drawing::FillStyle_NONE:
                    break;
                case drawing::FillStyle_SOLID:
                case drawing::FillStyle_GRADIENT:
                case drawing::FillStyle_HATCH:
                    isFullSize = true;
                    break;
                case drawing::FillStyle_BITMAP:
                    {
                        drawing::BitmapMode bitmapMode{};
                        xPropSet->getPropertyValue(u"FillBitmapMode"_ustr) >>= bitmapMode;
                        switch (bitmapMode)
                        {
                            case drawing::BitmapMode_REPEAT:
                                isFullSize = true;
                                break;
                            case drawing::BitmapMode_STRETCH:
                            case drawing::BitmapMode_NO_REPEAT:
                                isFullSize = false;
                                break;
                            default:
                                assert(false);
                        }
                    }
                    break;
                default:
                    assert(false);
            }
        }
        // set it explicitly if it's not the default
        if (!isFullSize)
        {
            SAL_INFO("xmloff.style", "FillPropertySet_PageStyle: Heuristically resetting BackgroundFullSize");
            xPropSet->setPropertyValue(u"BackgroundFullSize"_ustr, uno::Any(isFullSize));
        }
    }

    // old code, replaced by above stuff
    // XMLPropStyleContext::FillPropertySet(rPropSet);

    if (!sPageUsage.isEmpty())
    {
        uno::Any aPageUsage;
        XMLPMPropHdl_PageStyleLayout aPageUsageHdl;
        if (aPageUsageHdl.importXML(sPageUsage, aPageUsage, GetImport().GetMM100UnitConverter()))
            xPropSet->setPropertyValue(u"PageStyleLayout"_ustr, aPageUsage);
    }
}

extern ContextID_Index_Pair const g_MasterPageContextIDs[] =
{
    { CTF_PM_FILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT },
    { CTF_PM_FILLTRANSNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
    { CTF_PM_FILLHATCHNAME, -1, drawing::FillStyle::FillStyle_HATCH },
    { CTF_PM_FILLBITMAPNAME, -1, drawing::FillStyle::FillStyle_BITMAP },

    {-1, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE}
};

extern XmlStyleFamily const g_MasterPageFamilies[] =
{
    XmlStyleFamily::SD_GRADIENT_ID,
    XmlStyleFamily::SD_GRADIENT_ID,
    XmlStyleFamily::SD_HATCH_ID,
    XmlStyleFamily::SD_FILL_IMAGE_ID
};

// text grid enhancement for better CJK support
//set default page layout style
void PageStyleContext::SetDefaults( )
{
    Reference < XMultiServiceFactory > xFactory ( GetImport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        Reference < XInterface > xInt = xFactory->createInstance( u"com.sun.star.text.Defaults"_ustr );
        Reference < beans::XPropertySet > xProperties ( xInt, UNO_QUERY );
        if ( xProperties.is() )
            FillPropertySet_PageStyle(xProperties, nullptr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
