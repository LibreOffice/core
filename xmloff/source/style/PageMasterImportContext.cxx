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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include "PageMasterPropHdl.hxx"
#include "PagePropertySetContext.hxx"
#include "PageHeaderFooterContext.hxx"
#include "PageMasterPropMapper.hxx"
#include "PageMasterImportPropMapper.hxx"
#include <xmloff/PageMasterStyleMap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>

//
#include <xmlsdtypes.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <xmloff/xmlerror.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//
using namespace ::com::sun::star::beans;

void PageStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( XML_NAMESPACE_STYLE == nPrefixKey && IsXMLToken( rLocalName, XML_PAGE_USAGE ) )
    {
        sPageUsage = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


PageStyleContext::PageStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles,
        bool bDefaultStyle) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_PAGE_MASTER, bDefaultStyle),
    sPageUsage(),
    m_bIsFillStyleAlreadyConverted(false) //
{
}

PageStyleContext::~PageStyleContext()
{
}

SvXMLImportContextRef PageStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    if( XML_NAMESPACE_STYLE == nPrefix &&
        ((IsXMLToken(rLocalName, XML_HEADER_STYLE )) ||
         (IsXMLToken(rLocalName, XML_FOOTER_STYLE )) ) )
    {
        bool bHeader = IsXMLToken(rLocalName, XML_HEADER_STYLE);
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
            return new PageHeaderFooterContext(GetImport(), nPrefix, rLocalName,
                            xAttrList, GetProperties(), xImpPrMap, nStartIndex, nEndIndex, bHeader);
        }
    }

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken(rLocalName, XML_PAGE_LAYOUT_PROPERTIES) )
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
            return new PagePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    XML_TYPE_PROP_PAGE_LAYOUT,
                                                    GetProperties(),
                                                    xImpPrMap, 0, nEndIndex, Page);
        }
    }

    return XMLPropStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);
}

void PageStyleContext::FillPropertySet(const uno::Reference<beans::XPropertySet > & rPropSet)
{
    // need to filter out old fill definitions when the new ones are used. The new
    // ones are used when a FillStyle is defined
    if(!m_bIsFillStyleAlreadyConverted && !GetProperties().empty())
    {
        static OUString s_FillStyle("FillStyle");
        static OUString s_HeaderFillStyle("HeaderFillStyle");
        static OUString s_FooterFillStyle("FooterFillStyle");

        if(doNewDrawingLayerFillStyleDefinitionsExist(s_FillStyle))
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
            { CTF_PM_FILLGRADIENTNAME, -1 },
            { CTF_PM_FILLTRANSNAME, -1 },
            { CTF_PM_FILLHATCHNAME, -1 },
            { CTF_PM_FILLBITMAPNAME, -1 },

            // also need to special handling for header entries
            { CTF_PM_HEADERFILLGRADIENTNAME, -1 },
            { CTF_PM_HEADERFILLTRANSNAME, -1 },
            { CTF_PM_HEADERFILLHATCHNAME, -1 },
            { CTF_PM_HEADERFILLBITMAPNAME, -1 },

            // also need to special handling for footer entries
            { CTF_PM_FOOTERFILLGRADIENTNAME, -1 },
            { CTF_PM_FOOTERFILLTRANSNAME, -1 },
            { CTF_PM_FOOTERFILLHATCHNAME, -1 },
            { CTF_PM_FOOTERFILLBITMAPNAME, -1 },

            {-1, -1}
        };

        // the style families associated with the same index modulo 4
        static sal_uInt16 aFamilies[] =
        {
            XML_STYLE_FAMILY_SD_GRADIENT_ID,
            XML_STYLE_FAMILY_SD_GRADIENT_ID,
            XML_STYLE_FAMILY_SD_HATCH_ID,
            XML_STYLE_FAMILY_SD_FILL_IMAGE_ID
        };

        // Fill PropertySet, but let it handle special properties not itself
        xImpPrMap->FillPropertySet(GetProperties(), rPropSet, aContextIDs);

        // get property set mapper
        const rtl::Reference< XMLPropertySetMapper >& rMapper = xImpPrMap->getPropertySetMapper();
        Reference< XPropertySetInfo > xInfo;

        // handle special attributes which have MID_FLAG_NO_PROPERTY_IMPORT set
        for(sal_uInt16 i = 0; aContextIDs[i].nContextID != -1; i++)
        {
            sal_Int32 nIndex = aContextIDs[i].nIndex;

            if(nIndex != -1)
            {
                switch(aContextIDs[i].nContextID)
                {
                    case CTF_PM_FILLGRADIENTNAME:
                    case CTF_PM_FILLTRANSNAME:
                    case CTF_PM_FILLHATCHNAME:
                    case CTF_PM_FILLBITMAPNAME:

                    case CTF_PM_HEADERFILLGRADIENTNAME:
                    case CTF_PM_HEADERFILLTRANSNAME:
                    case CTF_PM_HEADERFILLHATCHNAME:
                    case CTF_PM_HEADERFILLBITMAPNAME:

                    case CTF_PM_FOOTERFILLGRADIENTNAME:
                    case CTF_PM_FOOTERFILLTRANSNAME:
                    case CTF_PM_FOOTERFILLHATCHNAME:
                    case CTF_PM_FOOTERFILLBITMAPNAME:
                    {
                        struct XMLPropertyState& rState = GetProperties()[nIndex];
                        OUString sStyleName;
                        rState.maValue >>= sStyleName;

                        // translate the used name from ODF intern to the name used in the Model
                        sStyleName = GetImport().GetStyleDisplayName(aFamilies[i%4], sStyleName);

                        try
                        {
                            // set property
                            const OUString& rPropertyName = rMapper->GetEntryAPIName(rState.mnIndex);

                            if(!xInfo.is())
                            {
                                xInfo = rPropSet->getPropertySetInfo();
                            }

                            if(xInfo->hasPropertyByName(rPropertyName))
                            {
                                rPropSet->setPropertyValue(rPropertyName,Any(sStyleName));
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

    // old code, replaced by above stuff
    // XMLPropStyleContext::FillPropertySet(rPropSet);

    if (!sPageUsage.isEmpty())
    {
        uno::Any aPageUsage;
        XMLPMPropHdl_PageStyleLayout aPageUsageHdl;
        if (aPageUsageHdl.importXML(sPageUsage, aPageUsage, GetImport().GetMM100UnitConverter()))
            rPropSet->setPropertyValue("PageStyleLayout", aPageUsage);
    }
}

// text grid enhancement for better CJK support
//set default page layout style
void PageStyleContext::SetDefaults( )
{
    Reference < XMultiServiceFactory > xFactory ( GetImport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        Reference < XInterface > xInt = xFactory->createInstance( "com.sun.star.text.Defaults" );
        Reference < beans::XPropertySet > xProperties ( xInt, UNO_QUERY );
        if ( xProperties.is() )
            FillPropertySet ( xProperties );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
