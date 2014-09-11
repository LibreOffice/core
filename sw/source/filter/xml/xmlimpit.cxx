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

#include "xmlimpit.hxx"

#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/memberids.hrc>

#include "hintids.hxx"
#include "unomid.h"
#include <svx/unomid.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include "fmtpdsc.hxx"
#include "fmtornt.hxx"
#include "fmtfsize.hxx"

#include "fmtlsplt.hxx"
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprhdl.hxx>
#include "xmlithlp.hxx"
#include <com/sun/star/uno/Any.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using uno::Any;

SvXMLImportItemMapper::SvXMLImportItemMapper(
                                SvXMLItemMapEntriesRef rMapEntries,
                                sal_uInt16 nUnknWhich ) :
    mrMapEntries( rMapEntries ),
    nUnknownWhich( nUnknWhich )
{
}

SvXMLImportItemMapper::~SvXMLImportItemMapper()
{
}

void
SvXMLImportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

// fills the given itemset with the attributes in the given list
void SvXMLImportItemMapper::importXML( SfxItemSet& rSet,
                                      uno::Reference< xml::sax::XAttributeList > xAttrList,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      const SvXMLNamespaceMap& rNamespaceMap )
{
    sal_Int16 nAttr = xAttrList->getLength();

    SvXMLAttrContainerItem *pUnknownItem = 0;
    for( sal_Int16 i=0; i < nAttr; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName, aPrefix, aNamespace;
        sal_uInt16 nPrefix =
            rNamespaceMap.GetKeyByAttrName( rAttrName, &aPrefix, &aLocalName,
                                            &aNamespace );
        if( XML_NAMESPACE_XMLNS == nPrefix )
            continue;

        const OUString& rValue = xAttrList->getValueByIndex( i );

        // find a map entry for this attribute
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByName( nPrefix, aLocalName );

        if( pEntry )
        {
            // we have a valid map entry here, so lets use it...
            if( 0 == (pEntry->nMemberId & (MID_SW_FLAG_NO_ITEM_IMPORT|
                                       MID_SW_FLAG_ELEMENT_ITEM_IMPORT)) )
            {
                // first get item from itemset
                const SfxPoolItem* pItem = 0;
                SfxItemState eState = rSet.GetItemState( pEntry->nWhichId, true,
                                                         &pItem );

                // if its not set, try the pool
                if(SfxItemState::SET != eState && SFX_WHICH_MAX > pEntry->nWhichId )
                    pItem = &rSet.GetPool()->GetDefaultItem(pEntry->nWhichId);

                // do we have an item?
                if(eState >= SfxItemState::DEFAULT && pItem)
                {
                    SfxPoolItem *pNewItem = pItem->Clone();
                    bool bPut = false;

                    if( 0 == (pEntry->nMemberId&MID_SW_FLAG_SPECIAL_ITEM_IMPORT) )
                    {
                        bPut = PutXMLValue( *pNewItem, rValue,
                                            static_cast<sal_uInt16>( pEntry->nMemberId & MID_SW_FLAG_MASK ),
                                            rUnitConverter );

                    }
                    else
                    {
                        bPut = handleSpecialItem( *pEntry, *pNewItem, rSet,
                                                  rValue, rUnitConverter,
                                                  rNamespaceMap );
                    }

                    if( bPut )
                        rSet.Put( *pNewItem );

                    delete pNewItem;
                }
                else
                {
                    OSL_FAIL( "Could not get a needed item for xml import!" );
                }
            }
            else if( 0 != (pEntry->nMemberId & MID_SW_FLAG_NO_ITEM_IMPORT) )
            {
                handleNoItem( *pEntry, rSet, rValue, rUnitConverter,
                              rNamespaceMap );
            }
        }
        else if( USHRT_MAX != nUnknownWhich )
        {
            if( !pUnknownItem )
            {
                const SfxPoolItem* pItem = 0;
                if( SfxItemState::SET == rSet.GetItemState( nUnknownWhich, true,
                                                       &pItem ) )
                {
                    SfxPoolItem *pNew = pItem->Clone();
                    pUnknownItem = PTR_CAST( SvXMLAttrContainerItem, pNew );
                    OSL_ENSURE( pUnknownItem,
                                "SvXMLAttrContainerItem expected" );
                    if( !pUnknownItem )
                        delete pNew;
                }
                else
                {
                    pUnknownItem = new SvXMLAttrContainerItem( nUnknownWhich );
                }
            }
            if( pUnknownItem )
            {
                if( XML_NAMESPACE_NONE == nPrefix )
                    pUnknownItem->AddAttr( aLocalName, rValue );
                else
                    pUnknownItem->AddAttr( aPrefix, aNamespace, aLocalName,
                                           rValue );
            }
        }
    }

    if( pUnknownItem )
    {
        rSet.Put( *pUnknownItem );
        delete pUnknownItem;
    }

    finished(rSet, rUnitConverter);
}

/** this method is called for every item that has the
    MID_SW_FLAG_SPECIAL_ITEM_IMPORT flag set */
bool
SvXMLImportItemMapper::handleSpecialItem(  const SvXMLItemMapEntry& /*rEntry*/,
                                            SfxPoolItem& /*rItem*/,
                                            SfxItemSet& /*rSet*/,
                                            const OUString& /*rValue*/,
                                            const SvXMLUnitConverter& /*rUnitConverter*/,
                                            const SvXMLNamespaceMap& /*rNamespaceMap*/ )
{
    OSL_FAIL( "unsupported special item in xml import" );
    return false;
}

/** this method is called for every item that has the
    MID_SW_FLAG_NO_ITEM_IMPORT flag set */
bool SvXMLImportItemMapper::handleNoItem( const SvXMLItemMapEntry& /*rEntry*/,
                                           SfxItemSet& /*rSet*/,
                                           const OUString& /*rValue*/,
                                           const SvXMLUnitConverter& /*rUnitConverter*/,
                                           const SvXMLNamespaceMap& /*rNamespaceMap*/ )
{
    OSL_FAIL( "unsupported no item in xml import" );
    return false;
}

void
SvXMLImportItemMapper::finished(SfxItemSet &, SvXMLUnitConverter const&) const
{
    // nothing to do here
}

struct BoxHolder : private boost::noncopyable
{
    SvxBorderLine* pTop;
    SvxBorderLine* pBottom;
    SvxBorderLine* pLeft;
    SvxBorderLine* pRight;

    BoxHolder(SvxBoxItem* pBox)
    {
        pTop    = pBox->GetTop() == NULL ?
            NULL : new SvxBorderLine( *pBox->GetTop() );
        pBottom = pBox->GetBottom() == NULL ?
            NULL : new SvxBorderLine( *pBox->GetBottom() );
        pLeft   = pBox->GetLeft() == NULL ?
            NULL : new SvxBorderLine( *pBox->GetLeft() );
        pRight  = pBox->GetRight() == NULL ?
            NULL : new SvxBorderLine( *pBox->GetRight() );
    }

    ~BoxHolder()
    {
        delete pTop;
        delete pBottom;
        delete pLeft;
        delete pRight;
    }
};

// put an XML-string value into an item
bool SvXMLImportItemMapper::PutXMLValue(
    SfxPoolItem& rItem,
    const OUString& rValue,
    sal_uInt16 nMemberId,
    const SvXMLUnitConverter& rUnitConverter )
{
    bool bOk = false;

    switch (rItem.Which())
        {
        case RES_LR_SPACE:
        {
            SvxLRSpaceItem* pLRSpace = PTR_CAST(SvxLRSpaceItem, &rItem);
            OSL_ENSURE( pLRSpace != NULL, "Wrong Which-ID!" );

            switch( nMemberId )
            {
                case MID_L_MARGIN:
                case MID_R_MARGIN:
                {
                    sal_Int32 nProp = 100;
                    sal_Int32 nAbs = 0;

                    if( rValue.indexOf( '%' ) != -1 )
                        bOk = ::sax::Converter::convertPercent(nProp, rValue);
                    else
                        bOk = rUnitConverter.convertMeasureToCore(nAbs, rValue);

                    if( bOk )
                    {
                        switch( nMemberId )
                        {
                            case MID_L_MARGIN:
                                pLRSpace->SetTxtLeft( (sal_Int32)nAbs, (sal_uInt16)nProp );
                                break;
                            case MID_R_MARGIN:
                                pLRSpace->SetRight( (sal_Int32)nAbs, (sal_uInt16)nProp );
                                break;
                        }
                    }
                }
                break;

                case MID_FIRST_LINE_INDENT:
                {
                    sal_Int32 nProp = 100;
                    sal_Int32 nAbs = 0;

                    if( rValue.indexOf( '%' ) != -1 )
                        bOk = ::sax::Converter::convertPercent(nProp, rValue);
                    else
                        bOk = rUnitConverter.convertMeasureToCore(nAbs, rValue,
                                                             -0x7fff, 0x7fff );

                    pLRSpace->SetTxtFirstLineOfst( (short)nAbs, (sal_uInt16)nProp );
                }
                break;

                case MID_FIRST_AUTO:
                {
                    bool bAutoFirst(false);
                    bOk = ::sax::Converter::convertBool( bAutoFirst, rValue );
                    if( bOk )
                        pLRSpace->SetAutoFirst( bAutoFirst );
                }
                break;

                default:
                    OSL_FAIL( "unknown member id!");
            }
        }
        break;

        case RES_UL_SPACE:
        {
            SvxULSpaceItem* pULSpace = PTR_CAST(SvxULSpaceItem, &rItem);
            OSL_ENSURE( pULSpace != NULL, "Wrong Which-ID!" );

            sal_Int32 nProp = 100;
            sal_Int32 nAbs = 0;

            if( rValue.indexOf( '%' ) != -1 )
                bOk = ::sax::Converter::convertPercent( nProp, rValue );
            else
                bOk = rUnitConverter.convertMeasureToCore( nAbs, rValue );

            switch( nMemberId )
            {
                case MID_UP_MARGIN:
                    pULSpace->SetUpper( (sal_uInt16)nAbs, (sal_uInt16)nProp );
                    break;
                case MID_LO_MARGIN:
                    pULSpace->SetLower( (sal_uInt16)nAbs, (sal_uInt16)nProp );
                    break;
                default:
                    OSL_FAIL("unknown MemberId");
            }
        }
        break;

        case RES_SHADOW:
        {
            SvxShadowItem* pShadow = PTR_CAST(SvxShadowItem, &rItem);
            OSL_ENSURE( pShadow != NULL, "Wrong Which-ID" );

            bool bColorFound = false;
            bool bOffsetFound = false;

            SvXMLTokenEnumerator aTokenEnum( rValue );

            Color aColor( 128,128, 128 );
            pShadow->SetLocation( SVX_SHADOW_BOTTOMRIGHT );

            OUString aToken;
            while( aTokenEnum.getNextToken( aToken ) )
            {
                if( IsXMLToken( aToken, XML_NONE ) )
                {
                    pShadow->SetLocation( SVX_SHADOW_NONE );
                    bOk = true;
                }
                else if( !bColorFound && aToken.startsWith("#") )
                {
                    sal_Int32 nColor(0);
                    bOk = ::sax::Converter::convertColor( nColor, aToken );
                    if( !bOk )
                        return false;

                    aColor.SetColor(nColor);
                    bColorFound = true;
                }
                else if( !bOffsetFound )
                {
                    sal_Int32 nX = 0, nY = 0;

                    bOk = rUnitConverter.convertMeasureToCore( nX, aToken );
                    if( bOk && aTokenEnum.getNextToken( aToken ) )
                        bOk = rUnitConverter.convertMeasureToCore( nY, aToken );

                    if( bOk )
                    {
                        if( nX < 0 )
                        {
                            if( nY < 0 )
                            {
                                pShadow->SetLocation( SVX_SHADOW_TOPLEFT );
                            }
                            else
                            {
                                pShadow->SetLocation( SVX_SHADOW_BOTTOMLEFT );
                            }
                        }
                        else
                        {
                            if( nY < 0 )
                            {
                                pShadow->SetLocation( SVX_SHADOW_TOPRIGHT );
                            }
                            else
                            {
                                pShadow->SetLocation( SVX_SHADOW_BOTTOMRIGHT );
                            }
                        }

                        if( nX < 0 ) nX *= -1;
                        if( nY < 0 ) nY *= -1;

                        pShadow->SetWidth( static_cast< sal_uInt16 >( (nX + nY) >> 1 ) );
                    }
                }
            }

            if( bOk && ( bColorFound || bOffsetFound ) )
            {
                pShadow->SetColor( aColor );
            }
            else
                bOk = false;
        }
        break;

        case RES_BOX:
        {
            SvxBoxItem* pBox = PTR_CAST(SvxBoxItem, &rItem);
            OSL_ENSURE( pBox != NULL, "Wrong WHich-ID" );

            // copy SvxBorderLines
            BoxHolder aBoxes(pBox);

            sal_Int32 nTemp;

            switch( nMemberId )
            {
                case ALL_BORDER_PADDING:
                case LEFT_BORDER_PADDING:
                case RIGHT_BORDER_PADDING:
                case TOP_BORDER_PADDING:
                case BOTTOM_BORDER_PADDING:
                    if (!rUnitConverter.convertMeasureToCore( nTemp, rValue,
                                0, 0xffff ))
                    {
                        return false;
                    }

                    if( nMemberId == LEFT_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        pBox->SetDistance( (sal_uInt16)nTemp, BOX_LINE_LEFT );
                    if( nMemberId == RIGHT_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        pBox->SetDistance( (sal_uInt16)nTemp, BOX_LINE_RIGHT );
                    if( nMemberId == TOP_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        pBox->SetDistance( (sal_uInt16)nTemp, BOX_LINE_TOP );
                    if( nMemberId == BOTTOM_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        pBox->SetDistance( (sal_uInt16)nTemp, BOX_LINE_BOTTOM);
                    break;

                case ALL_BORDER:
                case LEFT_BORDER:
                case RIGHT_BORDER:
                case TOP_BORDER:
                case BOTTOM_BORDER:
                {
                    bool bHasStyle = false;
                    bool bHasWidth = false;
                    bool bHasColor = false;

                    sal_uInt16 nStyle = USHRT_MAX;
                    sal_uInt16 nWidth = 0;
                    sal_uInt16 nNamedWidth = USHRT_MAX;

                    Color aColor( COL_BLACK );

                    if( !sw_frmitems_parseXMLBorder( rValue, rUnitConverter,
                                                      bHasStyle, nStyle,
                                                      bHasWidth, nWidth, nNamedWidth,
                                                      bHasColor, aColor ) )
                        return false;

                    if( TOP_BORDER == nMemberId || ALL_BORDER == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pTop,
                                                   bHasStyle, nStyle,
                                                   bHasWidth, nWidth, nNamedWidth,
                                                   bHasColor, aColor );

                    if( BOTTOM_BORDER == nMemberId || ALL_BORDER == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pBottom,
                                                   bHasStyle, nStyle,
                                                   bHasWidth, nWidth, nNamedWidth,
                                                   bHasColor, aColor );

                    if( LEFT_BORDER == nMemberId || ALL_BORDER == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pLeft,
                                                   bHasStyle, nStyle,
                                                   bHasWidth, nWidth, nNamedWidth,
                                                   bHasColor, aColor );

                    if( RIGHT_BORDER == nMemberId || ALL_BORDER == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pRight,
                                                   bHasStyle, nStyle,
                                                   bHasWidth, nWidth, nNamedWidth,
                                                   bHasColor, aColor );
                }
                break;
                case ALL_BORDER_LINE_WIDTH:
                case LEFT_BORDER_LINE_WIDTH:
                case RIGHT_BORDER_LINE_WIDTH:
                case TOP_BORDER_LINE_WIDTH:
                case BOTTOM_BORDER_LINE_WIDTH:
                {
                    SvXMLTokenEnumerator aTokenEnum( rValue );

                    sal_Int32 nInWidth, nDistance, nOutWidth;

                    OUString aToken;
                    if( !aTokenEnum.getNextToken( aToken ) )
                        return false;

                    if (!rUnitConverter.convertMeasureToCore(nInWidth, aToken))
                        return false;

                    if( !aTokenEnum.getNextToken( aToken ) )
                        return false;

                    if (!rUnitConverter.convertMeasureToCore(nDistance, aToken))
                        return false;

                    if( !aTokenEnum.getNextToken( aToken ) )
                        return false;

                    if (!rUnitConverter.convertMeasureToCore(nOutWidth, aToken))
                        return false;

                    // #i61946: accept line style even it's not part of our "normal" set of line styles
                    sal_uInt16 nWidth = 0;

                    if( TOP_BORDER_LINE_WIDTH == nMemberId ||
                        ALL_BORDER_LINE_WIDTH == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pTop, nWidth,
                                static_cast< sal_uInt16 >( nOutWidth ),
                                static_cast< sal_uInt16 >( nInWidth ),
                                static_cast< sal_uInt16 >( nDistance ) );

                    if( BOTTOM_BORDER_LINE_WIDTH == nMemberId ||
                        ALL_BORDER_LINE_WIDTH == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pBottom, nWidth,
                                static_cast< sal_uInt16 >( nOutWidth ),
                                static_cast< sal_uInt16 >( nInWidth ),
                                static_cast< sal_uInt16 >( nDistance ) );

                    if( LEFT_BORDER_LINE_WIDTH == nMemberId ||
                        ALL_BORDER_LINE_WIDTH == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pLeft, nWidth,
                                static_cast< sal_uInt16 >( nOutWidth ),
                                static_cast< sal_uInt16 >( nInWidth ),
                                static_cast< sal_uInt16 >( nDistance ) );

                    if( RIGHT_BORDER_LINE_WIDTH == nMemberId ||
                        ALL_BORDER_LINE_WIDTH == nMemberId )
                        sw_frmitems_setXMLBorder( aBoxes.pRight, nWidth,
                                static_cast< sal_uInt16 >( nOutWidth ),
                                static_cast< sal_uInt16 >( nInWidth ),
                                static_cast< sal_uInt16 >( nDistance ) );
                }
                break;
            }

            pBox->SetLine( aBoxes.pTop,    BOX_LINE_TOP    );
            pBox->SetLine( aBoxes.pBottom, BOX_LINE_BOTTOM );
            pBox->SetLine( aBoxes.pLeft,   BOX_LINE_LEFT   );
            pBox->SetLine( aBoxes.pRight,  BOX_LINE_RIGHT  );

            bOk = true;
        }
        break;

        case RES_BREAK:
        {
            SvxFmtBreakItem* pFmtBreak = PTR_CAST(SvxFmtBreakItem, &rItem);
            OSL_ENSURE( pFmtBreak != NULL, "Wrong Which-ID" );

            sal_uInt16 eEnum;

            if( !SvXMLUnitConverter::convertEnum( eEnum, rValue, psXML_BreakType ) )
                return false;

            if( eEnum == 0 )
            {
                pFmtBreak->SetValue( SVX_BREAK_NONE );
                bOk = true;
            }
            else
            {
                switch( nMemberId )
                {
                    case MID_BREAK_BEFORE:
                        pFmtBreak->SetValue( static_cast< sal_uInt16 >((eEnum == 1) ?
                                             SVX_BREAK_COLUMN_BEFORE :
                                             SVX_BREAK_PAGE_BEFORE) );
                        break;
                    case MID_BREAK_AFTER:
                        pFmtBreak->SetValue( static_cast< sal_uInt16 >((eEnum == 1) ?
                                             SVX_BREAK_COLUMN_AFTER :
                                             SVX_BREAK_PAGE_AFTER) );
                        break;
                }
                bOk = true;
            }
        }
        break;

        case RES_KEEP:
        {
            SvxFmtKeepItem* pFmtKeep = PTR_CAST(SvxFmtKeepItem, &rItem);
            OSL_ENSURE( pFmtKeep != NULL, "Wrong Which-ID" );

            if( IsXMLToken( rValue, XML_ALWAYS ) ||
                 IsXMLToken( rValue, XML_TRUE ) )
            {
                pFmtKeep->SetValue( true );
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_AUTO ) ||
                     IsXMLToken( rValue, XML_FALSE ) )
            {
                pFmtKeep->SetValue( false );
                bOk = true;
            }
        }
        break;

        case RES_BACKGROUND:
        {
            SvxBrushItem* pBrush = PTR_CAST(SvxBrushItem, &rItem);
            OSL_ENSURE( pBrush != NULL, "Wrong Which-ID" );

            sal_Int32 nTempColor(0);
            switch( nMemberId )
                {
                case MID_BACK_COLOR:
                    if( IsXMLToken( rValue, XML_TRANSPARENT ) )
                    {
                        pBrush->GetColor().SetTransparency(0xff);
                        bOk = true;
                    }
                    else if (::sax::Converter::convertColor(nTempColor, rValue))
                    {
                        Color aTempColor(nTempColor);
                        aTempColor.SetTransparency(0);
                        pBrush->SetColor( aTempColor );
                        bOk = true;
                    }
                    break;

                case MID_GRAPHIC_LINK:
                {
                    SvxGraphicPosition eOldGraphicPos = pBrush->GetGraphicPos();
                    uno::Any aAny;
                    aAny <<= rValue;
                    pBrush->PutValue( aAny, MID_GRAPHIC_URL );
                    if( GPOS_NONE == eOldGraphicPos &&
                        GPOS_NONE != pBrush->GetGraphicPos() )
                        pBrush->SetGraphicPos( GPOS_TILED );
                    bOk = true;
                }
                break;

                case MID_GRAPHIC_REPEAT:
                {
                    SvxGraphicPosition eGraphicPos = pBrush->GetGraphicPos();
                    sal_uInt16 nPos = GPOS_NONE;
                    if( SvXMLUnitConverter::convertEnum( nPos, rValue,
                                                    psXML_BrushRepeat ) )
                    {
                        if( GPOS_MM != nPos || GPOS_NONE == eGraphicPos ||
                            GPOS_AREA == eGraphicPos || GPOS_TILED == eGraphicPos )
                            pBrush->SetGraphicPos( (SvxGraphicPosition)nPos );
                        bOk = true;
                    }
                }
                break;

                case MID_GRAPHIC_POSITION:
                {
                    SvxGraphicPosition ePos = GPOS_NONE, eTmp;
                    sal_uInt16 nTmp;
                    SvXMLTokenEnumerator aTokenEnum( rValue );
                    OUString aToken;
                    bool bHori = false, bVert = false;
                    bOk = true;
                    while( bOk && aTokenEnum.getNextToken( aToken ) )
                    {
                        if( bHori && bVert )
                        {
                            bOk = false;
                        }
                        else if( -1 != aToken.indexOf( '%' ) )
                        {
                            sal_Int32 nPrc = 50;
                            if (::sax::Converter::convertPercent(nPrc, aToken))
                            {
                                if( !bHori )
                                {
                                    ePos = nPrc < 25 ? GPOS_LT :
                                               (nPrc < 75 ? GPOS_MM : GPOS_RB);
                                    bHori = true;
                                }
                                else
                                {
                                    eTmp = nPrc < 25 ? GPOS_LT:
                                               (nPrc < 75 ? GPOS_LM : GPOS_LB);
                                    sw_frmitems_MergeXMLVertPos( ePos, eTmp );
                                    bVert = true;
                                }
                            }
                            else
                            {
                                // wrong percentage
                                bOk = false;
                            }
                        }
                        else if( IsXMLToken( aToken, XML_CENTER ) )
                        {
                            if( bHori )
                                sw_frmitems_MergeXMLVertPos( ePos, GPOS_MM );
                            else if ( bVert )
                                sw_frmitems_MergeXMLHoriPos( ePos, GPOS_MM );
                            else
                                ePos = GPOS_MM;
                        }
                        else if( SvXMLUnitConverter::convertEnum( nTmp, aToken,
                                                         psXML_BrushHoriPos ) )
                        {
                            if( bVert )
                                sw_frmitems_MergeXMLHoriPos(
                                    ePos, (SvxGraphicPosition)nTmp );
                            else if( !bHori )
                                ePos = (SvxGraphicPosition)nTmp;
                            else
                                bOk = false;
                            bHori = true;
                        }
                        else if( SvXMLUnitConverter::convertEnum( nTmp, aToken,
                                                         psXML_BrushVertPos ) )
                        {
                            if( bHori )
                                sw_frmitems_MergeXMLVertPos(
                                    ePos, (SvxGraphicPosition)nTmp );
                            else if( !bVert )
                                ePos = (SvxGraphicPosition)nTmp;
                            else
                                bOk = false;
                            bVert = true;
                        }
                        else
                        {
                            bOk = false;
                        }
                    }

                    if( GPOS_NONE == ePos ) bOk = false;
                    if( bOk )
                        pBrush->SetGraphicPos( ePos );
                }
                break;

                case MID_GRAPHIC_FILTER:
                    pBrush->SetGraphicFilter( rValue );
                    bOk = true;
                    break;
                }
        }
        break;

        case RES_PAGEDESC:
        {
            SwFmtPageDesc* pPageDesc = PTR_CAST(SwFmtPageDesc, &rItem);
            OSL_ENSURE( pPageDesc != NULL, "Wrong Which-ID" );

            if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
            {
                sal_Int32 nVal;
                bOk = ::sax::Converter::convertNumber(
                        nVal, rValue, 0, USHRT_MAX);
                if( bOk )
                    pPageDesc->SetNumOffset( (sal_uInt16)nVal );
            }
        }
        break;

        case RES_LAYOUT_SPLIT:
        case RES_ROW_SPLIT:
        {
            SfxBoolItem* pSplit = PTR_CAST(SfxBoolItem, &rItem);
            OSL_ENSURE( pSplit != NULL, "Wrong Which-ID" );

            if( IsXMLToken( rValue, XML_AUTO ) ||
                 IsXMLToken( rValue, XML_TRUE ) )
            {
                pSplit->SetValue( true );
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_ALWAYS ) ||
                     IsXMLToken( rValue, XML_FALSE ) )
            {
                pSplit->SetValue( false );
                bOk = true;
            }
        }
        break;

        case RES_HORI_ORIENT:
        {
            SwFmtHoriOrient* pHoriOrient = PTR_CAST(SwFmtHoriOrient, &rItem);
            OSL_ENSURE( pHoriOrient != NULL, "Wrong Which-ID" );

            sal_uInt16 nValue;
            bOk = SvXMLUnitConverter::convertEnum( nValue, rValue,
                                              aXMLTableAlignMap );
            if( bOk )
                pHoriOrient->SetHoriOrient( nValue );
        }
        break;

        case RES_VERT_ORIENT:
        {
            SwFmtVertOrient* pVertOrient = PTR_CAST(SwFmtVertOrient, &rItem);
            OSL_ENSURE( pVertOrient != NULL, "Wrong Which-ID" );

            sal_uInt16 nValue;
            bOk = SvXMLUnitConverter::convertEnum( nValue, rValue,
                                              aXMLTableVAlignMap );
            if( bOk )
                pVertOrient->SetVertOrient( nValue );
            //#i8855# text::VertOrientation::NONE is stored as empty string and should be applied here
            else if(rValue.isEmpty())
            {
                pVertOrient->SetVertOrient( text::VertOrientation::NONE );
                bOk = true;
            }
        }
        break;

        case RES_FRM_SIZE:
        {
            SwFmtFrmSize* pFrmSize = PTR_CAST(SwFmtFrmSize, &rItem);
            OSL_ENSURE( pFrmSize != NULL, "Wrong Which-ID" );

            bool bSetHeight = false;
            bool bSetWidth = false;
            bool bSetSizeType = false;
            SwFrmSize eSizeType = ATT_VAR_SIZE;
            sal_Int32 nMin = MINLAY;

            switch( nMemberId )
                {
                case MID_FRMSIZE_REL_WIDTH:
                {
                    sal_Int32 nValue;
                    bOk = ::sax::Converter::convertPercent( nValue, rValue );
                    if( bOk )
                    {
                        if( nValue < 1 )
                            nValue = 1;
                        else if( nValue > 100 )
                            nValue = 100;

                        pFrmSize->SetWidthPercent( (sal_Int8)nValue );
                    }
                }
                break;
                case MID_FRMSIZE_WIDTH:
                    bSetWidth = true;
                    break;
                case MID_FRMSIZE_MIN_HEIGHT:
                    eSizeType = ATT_MIN_SIZE;
                    bSetHeight = true;
                    nMin = 1;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_FIX_HEIGHT:
                    eSizeType = ATT_FIX_SIZE;
                    bSetHeight = true;
                    nMin = 1;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_COL_WIDTH:
                    eSizeType = ATT_FIX_SIZE;
                    bSetWidth = true;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_REL_COL_WIDTH:
                {
                    sal_Int32 nPos = rValue.indexOf( '*' );
                    if( -1L != nPos )
                    {
                        sal_Int32 nValue = rValue.toInt32();
                        if( nValue < MINLAY )
                            nValue = MINLAY;
                        else if( nValue > USHRT_MAX )
                            nValue = USHRT_MAX;

                        pFrmSize->SetWidth( (sal_uInt16)nValue );
                        pFrmSize->SetHeightSizeType( ATT_VAR_SIZE );
                        bOk = true;
                    }
                }
                break;
                }

            sal_Int32 nValue;
            if( bSetHeight || bSetWidth )
            {
                bOk = rUnitConverter.convertMeasureToCore(nValue, rValue, nMin,
                                                     USHRT_MAX );
                if( bOk )
                {
                    if( bSetWidth )
                        pFrmSize->SetWidth( (sal_uInt16)nValue );
                    if( bSetHeight )
                        pFrmSize->SetHeight( (sal_uInt16)nValue );
                    if( bSetSizeType )
                        pFrmSize->SetHeightSizeType( eSizeType );
                }
            }
        }
        break;

        case RES_FRAMEDIR:
        {
            const XMLPropertyHandler* pWritingModeHandler =
                XMLPropertyHandlerFactory::CreatePropertyHandler(
                    XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT );
            if( pWritingModeHandler != NULL )
            {
                Any aAny;
                bOk = pWritingModeHandler->importXML( rValue, aAny,
                                                      rUnitConverter );
                if( bOk )
                    bOk = rItem.PutValue( aAny );

                delete pWritingModeHandler;
            }
        }
        break;

        case RES_COLLAPSING_BORDERS:
        {
            SfxBoolItem* pBorders = PTR_CAST(SfxBoolItem, &rItem);
            OSL_ENSURE( pBorders != NULL, "Wrong Which-ID" );

            if( IsXMLToken( rValue, XML_COLLAPSING ) )
            {
                pBorders->SetValue( true );
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_SEPARATING ) )
            {
                pBorders->SetValue( false );
                bOk = true;
            }
            else
                bOk = false;
        }
        break;

        default:
            OSL_FAIL("Item not implemented!");
        break;
   }

    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
