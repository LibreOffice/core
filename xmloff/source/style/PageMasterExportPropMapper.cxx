/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "PageMasterExportPropMapper.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <xmloff/PageMasterStyleMap.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/extract.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::xmloff::token;


//______________________________________________________________________________

static inline sal_Bool lcl_HasSameLineWidth( const table::BorderLine2& rLine1, const table::BorderLine2& rLine2 )
{
    return  (rLine1.InnerLineWidth == rLine2.InnerLineWidth) &&
            (rLine1.OuterLineWidth == rLine2.OuterLineWidth) &&
            (rLine1.LineDistance == rLine2.LineDistance) &&
            (rLine1.LineWidth == rLine2.LineWidth);
}

inline sal_Bool operator==( const table::BorderLine2& rLine1, const table::BorderLine2& rLine2 )
{
    return  (rLine1.Color == rLine2.Color) &&
            lcl_HasSameLineWidth( rLine1, rLine2 ) &&
            ( rLine1.LineStyle == rLine2.LineStyle );
}

static inline void lcl_RemoveState( XMLPropertyState* pState )
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

static void lcl_AddState(::std::vector< XMLPropertyState >& rPropState, sal_Int32 nIndex, const rtl::OUString& rProperty, uno::Reference< beans::XPropertySet >& xProps)
{
    if(::cppu::any2bool(xProps->getPropertyValue(rProperty)))
        rPropState.push_back(XMLPropertyState (nIndex, cppu::bool2any(sal_True)));
}

//______________________________________________________________________________
// helper struct to handle equal XMLPropertyState's for page, header and footer

struct XMLPropertyStateBuffer
{
    XMLPropertyState*       pPMMarginAll;
    XMLPropertyState*       pPMMarginTop;
    XMLPropertyState*       pPMMarginBottom;
    XMLPropertyState*       pPMMarginLeft;
    XMLPropertyState*       pPMMarginRight;

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
    :   pPMMarginAll( NULL )
    ,   pPMMarginTop( NULL )
    ,   pPMMarginBottom( NULL )
    ,   pPMMarginLeft( NULL )
    ,   pPMMarginRight( NULL )
    ,
        pPMBorderAll( NULL ),
        pPMBorderTop( NULL ),
        pPMBorderBottom( NULL ),
        pPMBorderLeft( NULL ),
        pPMBorderRight( NULL ),

        pPMBorderWidthAll( NULL ),
        pPMBorderWidthTop( NULL ),
        pPMBorderWidthBottom( NULL ),
        pPMBorderWidthLeft( NULL ),
        pPMBorderWidthRight( NULL ),

        pPMPaddingAll( NULL ),
        pPMPaddingTop( NULL ),
        pPMPaddingBottom( NULL ),
        pPMPaddingLeft( NULL ),
        pPMPaddingRight( NULL )
{
}

void XMLPropertyStateBuffer::ContextFilter( ::std::vector< XMLPropertyState >& )
{
    if (pPMMarginAll)
    {
        if (pPMMarginTop && pPMMarginBottom && pPMMarginLeft && pPMMarginRight)
        {
            sal_Int32 nTop = 0, nBottom = 0, nLeft = 0, nRight = 0;

            pPMMarginTop->maValue >>= nTop;
            pPMMarginBottom->maValue >>= nBottom;
            pPMMarginLeft->maValue >>= nLeft;
            pPMMarginRight->maValue >>= nRight;

            // Don't remove the Margin Left,Right,Top,Bottom for backward compatibility
            if ((nTop != nBottom) || (nBottom != nLeft) || (nLeft != nRight))
                lcl_RemoveState( pPMMarginAll );
        }
        else
            lcl_RemoveState( pPMMarginAll );
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

//______________________________________________________________________________

XMLPageMasterExportPropMapper::XMLPageMasterExportPropMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
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
        sal_uInt16 /*nFlags*/,
        const ::std::vector< XMLPropertyState >* pProperties,
        sal_uInt32 nIdx ) const
{
    XMLPageMasterExportPropMapper* pThis = (XMLPageMasterExportPropMapper*) this;

    sal_uInt32 nContextId = getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex );
    switch( nContextId )
    {
        case CTF_PM_GRAPHICURL:
        case CTF_PM_HEADERGRAPHICURL:
        case CTF_PM_FOOTERGRAPHICURL:
            {
                DBG_ASSERT( pProperties && (nIdx >= 2), "property vector missing" );
                sal_Int32 nPos;
                sal_Int32 nFilter;
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
                    nPos = 0;  // TODO What values should this be?
                    nFilter = 0;
                }
                const Any*  pPos    = NULL;
                const Any*  pFilter = NULL;
                if( pProperties && (nIdx >= 2) )
                {
                    const XMLPropertyState& rPos = (*pProperties)[nIdx - 2];
                    DBG_ASSERT( getPropertySetMapper()->GetEntryContextId( rPos.mnIndex ) == nPos,
                                "invalid property map: pos expected" );
                    if( getPropertySetMapper()->GetEntryContextId( rPos.mnIndex ) == nPos )
                        pPos = &rPos.maValue;

                    const XMLPropertyState& rFilter = (*pProperties)[nIdx - 1];
                    DBG_ASSERT( getPropertySetMapper()->GetEntryContextId( rFilter.mnIndex ) == nFilter,
                                "invalid property map: filter expected" );
                    if( getPropertySetMapper()->GetEntryContextId( rFilter.mnIndex ) == nFilter )
                        pFilter = &rFilter.maValue;
                }
                sal_uInt32 nPropIndex = rProperty.mnIndex;
                pThis->aBackgroundImageExport.exportXML( rProperty.maValue, pPos, pFilter, NULL,
                    getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                    getPropertySetMapper()->GetEntryXMLName( nPropIndex ) );
            }
            break;
        case CTF_PM_TEXTCOLUMNS:
            pThis->aTextColumnsExport.exportXML( rProperty.maValue );
            break;
        case CTF_PM_FTN_LINE_WEIGTH:
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
        ::std::vector< XMLPropertyState >& rPropState,
        Reference< XPropertySet > rPropSet ) const
{
    XMLPropertyStateBuffer  aPageBuffer;
    XMLPropertyStateBuffer  aHeaderBuffer;
    XMLPropertyStateBuffer  aFooterBuffer;

    XMLPropertyState*       pPMHeaderHeight     = NULL;
    XMLPropertyState*       pPMHeaderMinHeight  = NULL;
    XMLPropertyState*       pPMHeaderDynamic    = NULL;

    XMLPropertyState*       pPMFooterHeight     = NULL;
    XMLPropertyState*       pPMFooterMinHeight  = NULL;
    XMLPropertyState*       pPMFooterDynamic    = NULL;

    XMLPropertyState*       pPMScaleTo          = NULL;
    XMLPropertyState*       pPMScaleToPages     = NULL;
    XMLPropertyState*       pPMScaleToX         = NULL;
    XMLPropertyState*       pPMScaleToY         = NULL;
    XMLPropertyState*       pPMStandardMode     = NULL;
    XMLPropertyState*       pPMGridBaseWidth    = NULL;
    // same as pPMGridSnapTo but for backward compatibility only
    XMLPropertyState*       pPMGridSnapToChars  = NULL;
    XMLPropertyState*       pPMGridSnapTo       = NULL;

    XMLPropertyState*       pPrint              = NULL;

    UniReference < XMLPropertySetMapper > aPropMapper(getPropertySetMapper());

    for( ::std::vector< XMLPropertyState >::iterator aIter = rPropState.begin(); aIter != rPropState.end(); ++aIter )
    {
        XMLPropertyState *pProp = &(*aIter);
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
            case CTF_PM_MARGINTOP:          pBuffer->pPMMarginTop           = pProp;    break;
            case CTF_PM_MARGINBOTTOM:       pBuffer->pPMMarginBottom        = pProp;    break;
            case CTF_PM_MARGINLEFT:         pBuffer->pPMMarginLeft          = pProp;    break;
            case CTF_PM_MARGINRIGHT:        pBuffer->pPMMarginRight         = pProp;    break;
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
        }
        if (nPrintId == CTF_PM_PRINTMASK)
        {
            pPrint = pProp;
            lcl_RemoveState(pPrint);
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

    if( pPMHeaderHeight && (!pPMHeaderDynamic || (pPMHeaderDynamic && getBOOL( pPMHeaderDynamic->maValue ))) )
        lcl_RemoveState( pPMHeaderHeight );
    if( pPMHeaderMinHeight && pPMHeaderDynamic && !getBOOL( pPMHeaderDynamic->maValue ) )
        lcl_RemoveState( pPMHeaderMinHeight );
    if( pPMHeaderDynamic )
        lcl_RemoveState( pPMHeaderDynamic );

    if( pPMFooterHeight && (!pPMFooterDynamic || (pPMFooterDynamic && getBOOL( pPMFooterDynamic->maValue ))) )
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
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_ANNOTATIONS), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintAnnotations")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_CHARTS), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintCharts")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_DRAWING), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintDrawing")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_FORMULAS), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintFormulas")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_GRID), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintGrid")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_HEADERS), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintHeaders")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_OBJECTS), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintObjects")), rPropSet);
        lcl_AddState(rPropState, aPropMapper->FindEntryIndex(CTF_PM_PRINT_ZEROVALUES), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrintZeroValues")), rPropSet);
    }

    SvXMLExportPropertyMapper::ContextFilter(rPropState,rPropSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
