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

#include "PageMasterExportPropMapper.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <xmloff/PageMasterStyleMap.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <comphelper/extract.hxx>
#include <xmloff/txtprmap.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::xmloff::token;

static bool lcl_HasSameLineWidth( const table::BorderLine2& rLine1, const table::BorderLine2& rLine2 )
{
    return  (rLine1.InnerLineWidth == rLine2.InnerLineWidth) &&
            (rLine1.OuterLineWidth == rLine2.OuterLineWidth) &&
            (rLine1.LineDistance == rLine2.LineDistance) &&
            (rLine1.LineWidth == rLine2.LineWidth);
}

static void lcl_RemoveState( XMLPropertyState* pState )
{
    pState->mnIndex = -1;
    pState->maValue.clear();
}

static void lcl_RemoveStateIfZero16( XMLPropertyState* pState )
{
    sal_Int16   nValue = sal_Int16();
    if( (pState->maValue >>= nValue) && !nValue )
        lcl_RemoveState( pState );
}

static void lcl_AddState(::std::vector< XMLPropertyState >& rPropState, sal_Int32 nIndex, const OUString& rProperty, const uno::Reference< beans::XPropertySet >& xProps)
{
    if(::cppu::any2bool(xProps->getPropertyValue(rProperty)))
        rPropState.emplace_back(nIndex, css::uno::Any(true));
}

// helper struct to handle equal XMLPropertyState's for page, header and footer

struct XMLPropertyStateBuffer
{
    XMLPropertyState*       pPMMarginAll;

    XMLPropertyState*       pPMBorderAll;
    XMLPropertyState*       pPMBorderTop;
    XMLPropertyState*       pPMBorderBottom;
    XMLPropertyState*       pPMBorderLeft;
    XMLPropertyState*       pPMBorderRight;

    XMLPropertyState*       pPMBorderWidthAll;
    XMLPropertyState*       pPMBorderWidthTop;
    XMLPropertyState*       pPMBorderWidthBottom;
    XMLPropertyState*       pPMBorderWidthLeft;
    XMLPropertyState*       pPMBorderWidthRight;

    XMLPropertyState*       pPMPaddingAll;
    XMLPropertyState*       pPMPaddingTop;
    XMLPropertyState*       pPMPaddingBottom;
    XMLPropertyState*       pPMPaddingLeft;
    XMLPropertyState*       pPMPaddingRight;

                            XMLPropertyStateBuffer();
    void                    ContextFilter( ::std::vector< XMLPropertyState >& rPropState );
};

XMLPropertyStateBuffer::XMLPropertyStateBuffer()
    :   pPMMarginAll( nullptr )
    ,
        pPMBorderAll( nullptr ),
        pPMBorderTop( nullptr ),
        pPMBorderBottom( nullptr ),
        pPMBorderLeft( nullptr ),
        pPMBorderRight( nullptr ),

        pPMBorderWidthAll( nullptr ),
        pPMBorderWidthTop( nullptr ),
        pPMBorderWidthBottom( nullptr ),
        pPMBorderWidthLeft( nullptr ),
        pPMBorderWidthRight( nullptr ),

        pPMPaddingAll( nullptr ),
        pPMPaddingTop( nullptr ),
        pPMPaddingBottom( nullptr ),
        pPMPaddingLeft( nullptr ),
        pPMPaddingRight( nullptr )
{
}

void XMLPropertyStateBuffer::ContextFilter( ::std::vector< XMLPropertyState >& )
{
    if (pPMMarginAll)
    {
        lcl_RemoveState(pPMMarginAll); // #i117696# do not write fo:margin
    }

    if( pPMBorderAll )
    {
        if( pPMBorderTop && pPMBorderBottom && pPMBorderLeft && pPMBorderRight )
        {
            table::BorderLine2 aLineTop, aLineBottom, aLineLeft, aLineRight;

            pPMBorderTop->maValue >>= aLineTop;
            pPMBorderBottom->maValue >>= aLineBottom;
            pPMBorderLeft->maValue >>= aLineLeft;
            pPMBorderRight->maValue >>= aLineRight;

            if( (aLineTop == aLineBottom) && (aLineBottom == aLineLeft) && (aLineLeft == aLineRight) )
            {
                lcl_RemoveState( pPMBorderTop );
                lcl_RemoveState( pPMBorderBottom );
                lcl_RemoveState( pPMBorderLeft );
                lcl_RemoveState( pPMBorderRight );
            }
            else
                lcl_RemoveState( pPMBorderAll );
        }
        else
            lcl_RemoveState( pPMBorderAll );
    }

    if( pPMBorderWidthAll )
    {
        if( pPMBorderWidthTop && pPMBorderWidthBottom && pPMBorderWidthLeft && pPMBorderWidthRight )
        {
            table::BorderLine2 aLineTop, aLineBottom, aLineLeft, aLineRight;

            pPMBorderWidthTop->maValue >>= aLineTop;
            pPMBorderWidthBottom->maValue >>= aLineBottom;
            pPMBorderWidthLeft->maValue >>= aLineLeft;
            pPMBorderWidthRight->maValue >>= aLineRight;

            if( lcl_HasSameLineWidth( aLineTop, aLineBottom ) &&
                lcl_HasSameLineWidth( aLineBottom, aLineLeft ) &&
                lcl_HasSameLineWidth( aLineLeft, aLineRight ) )
            {
                lcl_RemoveState( pPMBorderWidthTop );
                lcl_RemoveState( pPMBorderWidthBottom );
                lcl_RemoveState( pPMBorderWidthLeft );
                lcl_RemoveState( pPMBorderWidthRight );
            }
            else
                lcl_RemoveState( pPMBorderWidthAll );
        }
        else
            lcl_RemoveState( pPMBorderWidthAll );
    }

    if( pPMPaddingAll )
    {
        if( pPMPaddingTop && pPMPaddingBottom && pPMPaddingLeft && pPMPaddingRight )
        {
            sal_Int32 nTop = 0, nBottom = 0, nLeft = 0, nRight = 0;

            pPMPaddingTop->maValue >>= nTop;
            pPMPaddingBottom->maValue >>= nBottom;
            pPMPaddingLeft->maValue >>= nLeft;
            pPMPaddingRight->maValue >>= nRight;

            if( (nTop == nBottom) && (nBottom == nLeft) && (nLeft == nRight) )
            {
                lcl_RemoveState( pPMPaddingTop );
                lcl_RemoveState( pPMPaddingBottom );
                lcl_RemoveState( pPMPaddingLeft );
                lcl_RemoveState( pPMPaddingRight );
            }
            else
                lcl_RemoveState( pPMPaddingAll );
        }
        else
            lcl_RemoveState( pPMPaddingAll );
    }
}

XMLPageMasterExportPropMapper::XMLPageMasterExportPropMapper(
        const rtl::Reference< XMLPropertySetMapper >& rMapper,
        SvXMLExport& rExport ) :
    SvXMLExportPropertyMapper( rMapper ),
    aBackgroundImageExport( rExport ),
    aTextColumnsExport( rExport ),
    aFootnoteSeparatorExport( rExport )
{
}

XMLPageMasterExportPropMapper::~XMLPageMasterExportPropMapper()
{
}

void XMLPageMasterExportPropMapper::handleElementItem(
        SvXMLExport&,
        const XMLPropertyState& rProperty,
        SvXmlExportFlags /*nFlags*/,
        const ::std::vector< XMLPropertyState >* pProperties,
        sal_uInt32 nIdx ) const
{
    XMLPageMasterExportPropMapper* pThis = const_cast<XMLPageMasterExportPropMapper*>(this);

    sal_uInt32 nContextId = getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex );
    switch( nContextId )
    {
        case CTF_PM_GRAPHICURL:
        case CTF_PM_HEADERGRAPHICURL:
        case CTF_PM_FOOTERGRAPHICURL:
            {
                assert(pProperties);
                assert(nIdx >= 2 && "horrible array ordering borked again");
                sal_Int32 nPos(-1);
                sal_Int32 nFilter(-1);
                switch( nContextId  )
                {
                case CTF_PM_GRAPHICURL:
                    nPos = CTF_PM_GRAPHICPOSITION;
                    nFilter = CTF_PM_GRAPHICFILTER;
                    break;
                case CTF_PM_HEADERGRAPHICURL:
                    nPos = CTF_PM_HEADERGRAPHICPOSITION;
                    nFilter = CTF_PM_HEADERGRAPHICFILTER;
                    break;
                case CTF_PM_FOOTERGRAPHICURL:
                    nPos = CTF_PM_FOOTERGRAPHICPOSITION;
                    nFilter = CTF_PM_FOOTERGRAPHICFILTER;
                    break;
                default:
                    assert(false);
                }
                const Any*  pPos    = nullptr;
                const Any*  pFilter = nullptr;
                sal_uInt32 nIndex(nIdx - 1);
                const XMLPropertyState& rFilter = (*pProperties)[nIndex];
                if (getPropertySetMapper()->GetEntryContextId(rFilter.mnIndex) == nFilter)
                {
                    pFilter = &rFilter.maValue;
                    --nIndex;
                }
                const XMLPropertyState& rPos = (*pProperties)[nIndex];
                if (getPropertySetMapper()->GetEntryContextId(rPos.mnIndex) == nPos)
                {
                    pPos = &rPos.maValue;
                    --nIndex;
                }
                sal_uInt32 nPropIndex = rProperty.mnIndex;
                pThis->aBackgroundImageExport.exportXML( rProperty.maValue, pPos, pFilter, nullptr,
                    getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                    getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
            }
            break;
        case CTF_PM_TEXTCOLUMNS:
            pThis->aTextColumnsExport.exportXML( rProperty.maValue );
            break;
        case CTF_PM_FTN_LINE_WEIGHT:
            pThis->aFootnoteSeparatorExport.exportXML( pProperties, nIdx,
                                                       getPropertySetMapper());
            break;
    }
}

void XMLPageMasterExportPropMapper::handleSpecialItem(
        SvXMLAttributeList&,
        const XMLPropertyState&,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&,
        const ::std::vector< XMLPropertyState >*,
        sal_uInt32 /*nIdx*/) const
{
}

void XMLPageMasterExportPropMapper::ContextFilter(
        bool bEnableFoFontFamily,
        ::std::vector< XMLPropertyState >& rPropState,
        const Reference< XPropertySet >& rPropSet ) const
{
    XMLPropertyStateBuffer  aPageBuffer;
    XMLPropertyStateBuffer  aHeaderBuffer;
    XMLPropertyStateBuffer  aFooterBuffer;

    XMLPropertyState*       pPMHeaderHeight     = nullptr;
    XMLPropertyState*       pPMHeaderMinHeight  = nullptr;
    XMLPropertyState*       pPMHeaderDynamic    = nullptr;

    XMLPropertyState*       pPMFooterHeight     = nullptr;
    XMLPropertyState*       pPMFooterMinHeight  = nullptr;
    XMLPropertyState*       pPMFooterDynamic    = nullptr;

    XMLPropertyState*       pPMScaleTo          = nullptr;
    XMLPropertyState*       pPMScaleToPages     = nullptr;
    XMLPropertyState*       pPMScaleToX         = nullptr;
    XMLPropertyState*       pPMScaleToY         = nullptr;
    XMLPropertyState*       pPMStandardMode     = nullptr;
    XMLPropertyState*       pPMGridBaseWidth    = nullptr;
    // same as pPMGridSnapTo but for backward compatibility only
    XMLPropertyState*       pPMGridSnapToChars  = nullptr;
    XMLPropertyState*       pPMGridSnapTo       = nullptr;

    XMLPropertyState*       pPrint              = nullptr;

    XMLPropertyState* pRepeatOffsetX = nullptr;
    XMLPropertyState* pRepeatOffsetY = nullptr;
    XMLPropertyState* pHeaderRepeatOffsetX = nullptr;
    XMLPropertyState* pHeaderRepeatOffsetY = nullptr;
    XMLPropertyState* pFooterRepeatOffsetX = nullptr;
    XMLPropertyState* pFooterRepeatOffsetY = nullptr;

    rtl::Reference < XMLPropertySetMapper > aPropMapper(getPropertySetMapper());

    for( auto& rProp : rPropState )
    {
        XMLPropertyState *pProp = &rProp;
        sal_Int16 nContextId    = aPropMapper->GetEntryContextId( pProp->mnIndex );
        sal_Int16 nFlag         = nContextId & CTF_PM_FLAGMASK;
        sal_Int16 nSimpleId     = nContextId & (~CTF_PM_FLAGMASK | XML_PM_CTF_START);
        sal_Int16 nPrintId      = nContextId & CTF_PM_PRINTMASK;

        XMLPropertyStateBuffer* pBuffer;
        switch( nFlag )
        {
            case CTF_PM_HEADERFLAG:         pBuffer = &aHeaderBuffer;   break;
            case CTF_PM_FOOTERFLAG:         pBuffer = &aFooterBuffer;   break;
            default:                        pBuffer = &aPageBuffer;     break;
        }

        switch( nSimpleId )
        {
            case CTF_PM_MARGINALL:          pBuffer->pPMMarginAll           = pProp;    break;
            case CTF_PM_BORDERALL:          pBuffer->pPMBorderAll           = pProp;    break;
            case CTF_PM_BORDERTOP:          pBuffer->pPMBorderTop           = pProp;    break;
            case CTF_PM_BORDERBOTTOM:       pBuffer->pPMBorderBottom        = pProp;    break;
            case CTF_PM_BORDERLEFT:         pBuffer->pPMBorderLeft          = pProp;    break;
            case CTF_PM_BORDERRIGHT:        pBuffer->pPMBorderRight         = pProp;    break;
            case CTF_PM_BORDERWIDTHALL:     pBuffer->pPMBorderWidthAll      = pProp;    break;
            case CTF_PM_BORDERWIDTHTOP:     pBuffer->pPMBorderWidthTop      = pProp;    break;
            case CTF_PM_BORDERWIDTHBOTTOM:  pBuffer->pPMBorderWidthBottom   = pProp;    break;
            case CTF_PM_BORDERWIDTHLEFT:    pBuffer->pPMBorderWidthLeft     = pProp;    break;
            case CTF_PM_BORDERWIDTHRIGHT:   pBuffer->pPMBorderWidthRight    = pProp;    break;
            case CTF_PM_PADDINGALL:         pBuffer->pPMPaddingAll          = pProp;    break;
            case CTF_PM_PADDINGTOP:         pBuffer->pPMPaddingTop          = pProp;    break;
            case CTF_PM_PADDINGBOTTOM:      pBuffer->pPMPaddingBottom       = pProp;    break;
            case CTF_PM_PADDINGLEFT:        pBuffer->pPMPaddingLeft         = pProp;    break;
            case CTF_PM_PADDINGRIGHT:       pBuffer->pPMPaddingRight        = pProp;    break;
        }

        switch( nContextId )
        {
            case CTF_PM_HEADERHEIGHT:       pPMHeaderHeight     = pProp;    break;
            case CTF_PM_HEADERMINHEIGHT:    pPMHeaderMinHeight  = pProp;    break;
            case CTF_PM_HEADERDYNAMIC:      pPMHeaderDynamic    = pProp;    break;
            case CTF_PM_FOOTERHEIGHT:       pPMFooterHeight     = pProp;    break;
            case CTF_PM_FOOTERMINHEIGHT:    pPMFooterMinHeight  = pProp;    break;
            case CTF_PM_FOOTERDYNAMIC:      pPMFooterDynamic    = pProp;    break;
            case CTF_PM_SCALETO:            pPMScaleTo          = pProp;    break;
            case CTF_PM_SCALETOPAGES:       pPMScaleToPages     = pProp;    break;
            case CTF_PM_SCALETOX:           pPMScaleToX         = pProp;    break;
            case CTF_PM_SCALETOY:           pPMScaleToY         = pProp;    break;
            case CTF_PM_STANDARD_MODE:      pPMStandardMode     = pProp;    break;
            case CTP_PM_GRID_BASE_WIDTH:        pPMGridBaseWidth    = pProp;    break;
            case CTP_PM_GRID_SNAP_TO_CHARS:     pPMGridSnapToChars  = pProp;    break;
            case CTP_PM_GRID_SNAP_TO:       pPMGridSnapTo = pProp;    break;

            case CTF_PM_REPEAT_OFFSET_X:
                pRepeatOffsetX = pProp;
                break;

            case CTF_PM_REPEAT_OFFSET_Y:
                pRepeatOffsetY = pProp;
                break;

            case CTF_PM_HEADERREPEAT_OFFSET_X:
                pHeaderRepeatOffsetX = pProp;
                break;

            case CTF_PM_HEADERREPEAT_OFFSET_Y:
                pHeaderRepeatOffsetY = pProp;
                break;

            case CTF_PM_FOOTERREPEAT_OFFSET_X:
                pFooterRepeatOffsetX = pProp;
                break;

            case CTF_PM_FOOTERREPEAT_OFFSET_Y:
                pFooterRepeatOffsetY = pProp;
                break;

            // Sort out empty entries
            case CTF_PM_FILLGRADIENTNAME:
            case CTF_PM_FILLHATCHNAME:
            case CTF_PM_FILLBITMAPNAME:
            case CTF_PM_FILLTRANSNAME:

            case CTF_PM_HEADERFILLGRADIENTNAME:
            case CTF_PM_HEADERFILLHATCHNAME:
            case CTF_PM_HEADERFILLBITMAPNAME:
            case CTF_PM_HEADERFILLTRANSNAME:

            case CTF_PM_FOOTERFILLGRADIENTNAME:
            case CTF_PM_FOOTERFILLHATCHNAME:
            case CTF_PM_FOOTERFILLBITMAPNAME:
            case CTF_PM_FOOTERFILLTRANSNAME:
            {
                OUString aStr;

                if( (pProp->maValue >>= aStr) && 0 == aStr.getLength() )
                {
                    pProp->mnIndex = -1;
                }

                break;
            }
        }

        if (nPrintId == CTF_PM_PRINTMASK)
        {
            pPrint = pProp;
            lcl_RemoveState(pPrint);
        }
    }

    // These entries need to be reduced to a single one for XML export.
    // Both would be exported as 'draw:tile-repeat-offset' following a percent
    // value and a 'vertical' or 'horizontal' tag as mark. If both would be active
    // and both would be exported this would create an XML error (same property twice)
    if(pRepeatOffsetX && pRepeatOffsetY)
    {
        sal_Int32 nOffset(0);

        if((pRepeatOffsetX->maValue >>= nOffset) && (!nOffset))
        {
            pRepeatOffsetX->mnIndex = -1;
        }
        else
        {
            pRepeatOffsetY->mnIndex = -1;
        }
    }

    // Same as above for Header
    if(pHeaderRepeatOffsetX && pHeaderRepeatOffsetY)
    {
        sal_Int32 nOffset(0);

        if((pHeaderRepeatOffsetX->maValue >>= nOffset) && (!nOffset))
        {
            pHeaderRepeatOffsetX->mnIndex = -1;
        }
        else
        {
            pHeaderRepeatOffsetY->mnIndex = -1;
        }
    }

    // Same as above for Footer
    if(pFooterRepeatOffsetX && pFooterRepeatOffsetY)
    {
        sal_Int32 nOffset(0);

        if((pFooterRepeatOffsetX->maValue >>= nOffset) && (!nOffset))
        {
            pFooterRepeatOffsetX->mnIndex = -1;
        }
        else
        {
            pFooterRepeatOffsetY->mnIndex = -1;
        }
    }

    if( pPMStandardMode && !getBOOL(pPMStandardMode->maValue) )
    {
        lcl_RemoveState(pPMStandardMode);
        if( pPMGridBaseWidth )
            lcl_RemoveState(pPMGridBaseWidth);
        if( pPMGridSnapToChars )
            lcl_RemoveState(pPMGridSnapToChars);
        if (pPMGridSnapTo)
        {
            lcl_RemoveState(pPMGridSnapTo);
        }
    }

    if( pPMGridBaseWidth && pPMStandardMode )
        lcl_RemoveState(pPMStandardMode);

    aPageBuffer.ContextFilter( rPropState );
    aHeaderBuffer.ContextFilter( rPropState );
    aFooterBuffer.ContextFilter( rPropState );

    if( pPMHeaderHeight && (!pPMHeaderDynamic || getBOOL( pPMHeaderDynamic->maValue )) )
        lcl_RemoveState( pPMHeaderHeight );
    if( pPMHeaderMinHeight && pPMHeaderDynamic && !getBOOL( pPMHeaderDynamic->maValue ) )
        lcl_RemoveState( pPMHeaderMinHeight );
    if( pPMHeaderDynamic )
        lcl_RemoveState( pPMHeaderDynamic );

    if( pPMFooterHeight && (!pPMFooterDynamic || getBOOL( pPMFooterDynamic->maValue )) )
        lcl_RemoveState( pPMFooterHeight );
    if( pPMFooterMinHeight && pPMFooterDynamic && !getBOOL( pPMFooterDynamic->maValue ) )
        lcl_RemoveState( pPMFooterMinHeight );
    if( pPMFooterDynamic )
        lcl_RemoveState( pPMFooterDynamic );

    if( pPMScaleTo )
        lcl_RemoveStateIfZero16( pPMScaleTo );
    if( pPMScaleToPages )
        lcl_RemoveStateIfZero16( pPMScaleToPages );
    if( pPMScaleToX )
        lcl_RemoveStateIfZero16( pPMScaleToX );
    if( pPMScaleToY )
        lcl_RemoveStateIfZero16( pPMScaleToY );

    if (pPrint)
    {
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_ANNOTATIONS), "PrintAnnotations", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_CHARTS), "PrintCharts", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_DRAWING), "PrintDrawing", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_FORMULAS), "PrintFormulas", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_GRID), "PrintGrid", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_HEADERS), "PrintHeaders", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_OBJECTS), "PrintObjects", rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_ZEROVALUES), "PrintZeroValues", rPropSet);
    }

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rPropState, rPropSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
