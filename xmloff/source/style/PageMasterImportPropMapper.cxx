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


#include "PageMasterImportPropMapper.hxx"
#include "PageMasterPropMapper.hxx"
#include <xmloff/PageMasterStyleMap.hxx>
#include <xmloff/maptype.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <xmloff/xmlimp.hxx>
#include <memory>

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

PageMasterImportPropertyMapper::PageMasterImportPropertyMapper(
        const rtl::Reference< XMLPropertySetMapper >& rMapper,
        SvXMLImport& rImp ) :
    SvXMLImportPropertyMapper( rMapper, rImp ),
    rImport( rImp )
{
}

PageMasterImportPropertyMapper::~PageMasterImportPropertyMapper()
{
}

bool PageMasterImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        std::vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    bool bRet = false;
    sal_Int16 nContextID =
            getPropertySetMapper()->GetEntryContextId(rProperty.mnIndex);

    if( CTF_PM_REGISTER_STYLE==nContextID )
    {
        OUString sDisplayName( rImport.GetStyleDisplayName(
                    XML_STYLE_FAMILY_TEXT_PARAGRAPH, rValue ) );
        Reference < XNameContainer > xParaStyles =
            rImport.GetTextImport()->GetParaStyles();
        if( xParaStyles.is() && xParaStyles->hasByName( sDisplayName ) )
        {
            rProperty.maValue <<= sDisplayName;
            bRet = true;
        }
    }
    else
    {
        bRet = SvXMLImportPropertyMapper::handleSpecialItem(
                    rProperty, rProperties, rValue,
                    rUnitConverter, rNamespaceMap );
    }

    return bRet;
}


void PageMasterImportPropertyMapper::finished(std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    SvXMLImportPropertyMapper::finished(rProperties, nStartIndex, nEndIndex);
    XMLPropertyState* pAllPaddingProperty = nullptr;
    XMLPropertyState* pPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pNewPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllBorderProperty = nullptr;
    XMLPropertyState* pBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pNewBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllBorderWidthProperty = nullptr;
    XMLPropertyState* pBorderWidths[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllHeaderPaddingProperty = nullptr;
    XMLPropertyState* pHeaderPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pHeaderNewPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllHeaderBorderProperty = nullptr;
    XMLPropertyState* pHeaderBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pHeaderNewBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllHeaderBorderWidthProperty = nullptr;
    XMLPropertyState* pHeaderBorderWidths[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllFooterPaddingProperty = nullptr;
    XMLPropertyState* pFooterPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pFooterNewPadding[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllFooterBorderProperty = nullptr;
    XMLPropertyState* pFooterBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pFooterNewBorders[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pAllFooterBorderWidthProperty = nullptr;
    XMLPropertyState* pFooterBorderWidths[4] = { nullptr, nullptr, nullptr, nullptr };
    XMLPropertyState* pHeaderHeight = nullptr;
    XMLPropertyState* pHeaderMinHeight = nullptr;
    std::unique_ptr<XMLPropertyState> xHeaderDynamic;
    XMLPropertyState* pFooterHeight = nullptr;
    XMLPropertyState* pFooterMinHeight = nullptr;
    std::unique_ptr<XMLPropertyState> xFooterDynamic;
    XMLPropertyState* pAllMarginProperty = nullptr;
    XMLPropertyState* pMargins[4] = { nullptr, nullptr, nullptr, nullptr };
    std::unique_ptr<XMLPropertyState> pNewMargins[4];
    XMLPropertyState* pAllHeaderMarginProperty = nullptr;
    XMLPropertyState* pHeaderMargins[4] = { nullptr, nullptr, nullptr, nullptr };
    std::unique_ptr<XMLPropertyState> pNewHeaderMargins[4];
    XMLPropertyState* pAllFooterMarginProperty = nullptr;
    XMLPropertyState* pFooterMargins[4] = { nullptr, nullptr, nullptr, nullptr };
    std::unique_ptr<XMLPropertyState> pNewFooterMargins[4];

    for (auto& rProp : rProperties)
    {
        XMLPropertyState *property = &rProp;
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(property->mnIndex);
        if (property->mnIndex >= nStartIndex && property->mnIndex < nEndIndex)
        {
            switch (nContextID)
            {
                case CTF_PM_PADDINGALL                  : pAllPaddingProperty = property; break;
                case CTF_PM_PADDINGLEFT                 : pPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_PADDINGRIGHT                : pPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_PADDINGTOP                  : pPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_PADDINGBOTTOM               : pPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_BORDERALL                   : pAllBorderProperty = property; break;
                case CTF_PM_BORDERLEFT                  : pBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_BORDERRIGHT                 : pBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_BORDERTOP                   : pBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_BORDERBOTTOM                : pBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_BORDERWIDTHALL              : pAllBorderWidthProperty = property; break;
                case CTF_PM_BORDERWIDTHLEFT             : pBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_BORDERWIDTHRIGHT            : pBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_BORDERWIDTHTOP              : pBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_BORDERWIDTHBOTTOM           : pBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERPADDINGALL            : pAllHeaderPaddingProperty = property; break;
                case CTF_PM_HEADERPADDINGLEFT           : pHeaderPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERPADDINGRIGHT          : pHeaderPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERPADDINGTOP            : pHeaderPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERPADDINGBOTTOM         : pHeaderPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERBORDERALL             : pAllHeaderBorderProperty = property; break;
                case CTF_PM_HEADERBORDERLEFT            : pHeaderBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERBORDERRIGHT           : pHeaderBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERBORDERTOP             : pHeaderBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERBORDERBOTTOM          : pHeaderBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERBORDERWIDTHALL        : pAllHeaderBorderWidthProperty = property; break;
                case CTF_PM_HEADERBORDERWIDTHLEFT       : pHeaderBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERBORDERWIDTHRIGHT      : pHeaderBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERBORDERWIDTHTOP        : pHeaderBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERBORDERWIDTHBOTTOM     : pHeaderBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERPADDINGALL            : pAllFooterPaddingProperty = property; break;
                case CTF_PM_FOOTERPADDINGLEFT           : pFooterPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERPADDINGRIGHT          : pFooterPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERPADDINGTOP            : pFooterPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERPADDINGBOTTOM         : pFooterPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERBORDERALL             : pAllFooterBorderProperty = property; break;
                case CTF_PM_FOOTERBORDERLEFT            : pFooterBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERBORDERRIGHT           : pFooterBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERBORDERTOP             : pFooterBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERBORDERBOTTOM          : pFooterBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHALL        : pAllFooterBorderWidthProperty = property; break;
                case CTF_PM_FOOTERBORDERWIDTHLEFT       : pFooterBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHRIGHT      : pFooterBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHTOP        : pFooterBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHBOTTOM     : pFooterBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERHEIGHT                : pHeaderHeight = property; break;
                case CTF_PM_HEADERMINHEIGHT             : pHeaderMinHeight = property; break;
                case CTF_PM_FOOTERHEIGHT                : pFooterHeight = property; break;
                case CTF_PM_FOOTERMINHEIGHT             : pFooterMinHeight = property; break;
                case CTF_PM_MARGINALL   :
                      pAllMarginProperty = property; break;
                case CTF_PM_MARGINTOP   :
                      pMargins[XML_LINE_TOP] = property; break;
                case CTF_PM_MARGINBOTTOM:
                      pMargins[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_MARGINLEFT  :
                      pMargins[XML_LINE_LEFT] = property; break;
                case CTF_PM_MARGINRIGHT :
                      pMargins[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERMARGINALL   :
                      pAllHeaderMarginProperty = property; break;
                case CTF_PM_HEADERMARGINTOP   :
                      pHeaderMargins[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERMARGINBOTTOM:
                      pHeaderMargins[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERMARGINLEFT  :
                      pHeaderMargins[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERMARGINRIGHT :
                      pHeaderMargins[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERMARGINALL   :
                      pAllFooterMarginProperty = property; break;
                case CTF_PM_FOOTERMARGINTOP   :
                      pFooterMargins[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERMARGINBOTTOM:
                      pFooterMargins[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERMARGINLEFT  :
                      pFooterMargins[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERMARGINRIGHT :
                      pFooterMargins[XML_LINE_RIGHT] = property; break;
            }
        }
    }

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pAllMarginProperty && !pMargins[i])
        {
            pNewMargins[i].reset(new XMLPropertyState(
                pAllMarginProperty->mnIndex + 1 + i,
                pAllMarginProperty->maValue));
        }
        if (pAllHeaderMarginProperty && !pHeaderMargins[i])
        {
            pNewHeaderMargins[i].reset(new XMLPropertyState(
                pAllHeaderMarginProperty->mnIndex + 1 + i,
                pAllHeaderMarginProperty->maValue));
        }
        if (pAllFooterMarginProperty && !pFooterMargins[i])
        {
            pNewFooterMargins[i].reset(new XMLPropertyState(
                pAllFooterMarginProperty->mnIndex + 1 + i,
                pAllFooterMarginProperty->maValue));
        }
        if (pAllPaddingProperty && !pPadding[i])
            pNewPadding[i] = new XMLPropertyState(pAllPaddingProperty->mnIndex + 1 + i, pAllPaddingProperty->maValue);
        if (pAllBorderProperty && !pBorders[i])
        {
            pNewBorders[i] = new XMLPropertyState(pAllBorderProperty->mnIndex + 1 + i, pAllBorderProperty->maValue);
            pBorders[i] = pNewBorders[i];
        }
        if( !pBorderWidths[i] )
            pBorderWidths[i] = pAllBorderWidthProperty;
        else
            pBorderWidths[i]->mnIndex = -1;
        if( pBorders[i] )
        {
            table::BorderLine2 aBorderLine;
            pBorders[i]->maValue >>= aBorderLine;
            if( pBorderWidths[i] )
            {
                table::BorderLine2 aBorderLineWidth;
                pBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                aBorderLine.LineWidth = aBorderLineWidth.LineWidth;
                pBorders[i]->maValue <<= aBorderLine;
            }
        }
        if (pAllHeaderPaddingProperty && !pHeaderPadding[i])
            pHeaderNewPadding[i] = new XMLPropertyState(pAllHeaderPaddingProperty->mnIndex + 1 + i, pAllHeaderPaddingProperty->maValue);
        if (pAllHeaderBorderProperty && !pHeaderBorders[i])
            pHeaderNewBorders[i] = new XMLPropertyState(pAllHeaderBorderProperty->mnIndex + 1 + i, pAllHeaderBorderProperty->maValue);
        if( !pHeaderBorderWidths[i] )
            pHeaderBorderWidths[i] = pAllHeaderBorderWidthProperty;
        else
            pHeaderBorderWidths[i]->mnIndex = -1;
        if( pHeaderBorders[i] )
        {
            table::BorderLine2 aBorderLine;
            pHeaderBorders[i]->maValue >>= aBorderLine;
            if( pHeaderBorderWidths[i] )
            {
                table::BorderLine2 aBorderLineWidth;
                pHeaderBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                aBorderLine.LineWidth = aBorderLineWidth.LineWidth;
                pHeaderBorders[i]->maValue <<= aBorderLine;
            }
        }
        if (pAllFooterPaddingProperty && !pFooterPadding[i])
            pFooterNewPadding[i] = new XMLPropertyState(pAllFooterPaddingProperty->mnIndex + 1 + i, pAllFooterPaddingProperty->maValue);
        if (pAllFooterBorderProperty && !pFooterBorders[i])
            pFooterNewBorders[i] = new XMLPropertyState(pAllFooterBorderProperty->mnIndex + 1 + i, pAllFooterBorderProperty->maValue);
        if( !pFooterBorderWidths[i] )
            pFooterBorderWidths[i] = pAllFooterBorderWidthProperty;
        else
            pFooterBorderWidths[i]->mnIndex = -1;
        if( pFooterBorders[i] )
        {
            table::BorderLine2 aBorderLine;
            pFooterBorders[i]->maValue >>= aBorderLine;
            if( pFooterBorderWidths[i] )
            {
                table::BorderLine2 aBorderLineWidth;
                pFooterBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                aBorderLine.LineWidth = aBorderLineWidth.LineWidth;
                pFooterBorders[i]->maValue <<= aBorderLine;
            }
        }
    }

    if (pHeaderHeight)
    {
        xHeaderDynamic.reset(new XMLPropertyState(pHeaderHeight->mnIndex + 2, Any(false)));
    }
    if (pHeaderMinHeight)
    {
        xHeaderDynamic.reset(new XMLPropertyState(pHeaderMinHeight->mnIndex + 1, Any(true)));
    }
    if (pFooterHeight)
    {
        xFooterDynamic.reset(new XMLPropertyState(pFooterHeight->mnIndex + 2, Any(false)));
    }
    if (pFooterMinHeight)
    {
        xFooterDynamic.reset(new XMLPropertyState(pFooterMinHeight->mnIndex + 1, Any(true)));
    }

    // fdo#38056: nerf the various AllFoo properties so they do not override
    // the individual Foo properties later on
    if (pAllPaddingProperty)
    {
        pAllPaddingProperty->mnIndex = -1;
    }
    if (pAllBorderProperty)
    {
        pAllBorderProperty->mnIndex = -1;
    }
    if (pAllBorderWidthProperty)
    {
        pAllBorderWidthProperty->mnIndex = -1;
    }
    if (pAllHeaderPaddingProperty)
    {
        pAllHeaderPaddingProperty->mnIndex = -1;
    }
    if (pAllHeaderBorderProperty)
    {
        pAllHeaderBorderProperty->mnIndex = -1;
    }
    if (pAllHeaderBorderWidthProperty)
    {
        pAllHeaderBorderWidthProperty->mnIndex = -1;
    }
    if (pAllFooterPaddingProperty)
    {
        pAllFooterPaddingProperty->mnIndex = -1;
    }
    if (pAllFooterBorderProperty)
    {
        pAllFooterBorderProperty->mnIndex = -1;
    }
    if (pAllFooterBorderWidthProperty)
    {
        pAllFooterBorderWidthProperty->mnIndex = -1;
    }
    if (pAllMarginProperty)
    {
        pAllMarginProperty->mnIndex = -1;
    }
    if (pAllHeaderMarginProperty)
    {
        pAllHeaderMarginProperty->mnIndex = -1;
    }
    if (pAllFooterMarginProperty)
    {
        pAllFooterMarginProperty->mnIndex = -1;
    }

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pNewMargins[i])
        {
            rProperties.push_back(*pNewMargins[i]);
        }
        if (pNewHeaderMargins[i])
        {
            rProperties.push_back(*pNewHeaderMargins[i]);
        }
        if (pNewFooterMargins[i])
        {
            rProperties.push_back(*pNewFooterMargins[i]);
        }
        if (pNewPadding[i])
        {
            rProperties.push_back(*pNewPadding[i]);
            delete pNewPadding[i];
        }
        if (pNewBorders[i])
        {
            rProperties.push_back(*pNewBorders[i]);
            delete pNewBorders[i];
        }
        if (pHeaderNewPadding[i])
        {
            rProperties.push_back(*pHeaderNewPadding[i]);
            delete pHeaderNewPadding[i];
        }
        if (pHeaderNewBorders[i])
        {
            rProperties.push_back(*pHeaderNewBorders[i]);
            delete pHeaderNewBorders[i];
        }
        if (pFooterNewPadding[i])
        {
            rProperties.push_back(*pFooterNewPadding[i]);
            delete pFooterNewPadding[i];
        }
        if (pFooterNewBorders[i])
        {
            rProperties.push_back(*pFooterNewBorders[i]);
            delete pFooterNewBorders[i];
        }
    }
    if(xHeaderDynamic)
    {
        rProperties.push_back(*xHeaderDynamic);
        xHeaderDynamic.reset();
    }
    if(xFooterDynamic)
    {
        rProperties.push_back(*xFooterDynamic);
        xFooterDynamic.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
