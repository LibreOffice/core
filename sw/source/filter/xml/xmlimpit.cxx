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
#include <boost/noncopyable.hpp>

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

    SvXMLAttrContainerItem *pUnknownItem = nullptr;
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
                const SfxPoolItem* pItem = nullptr;
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
                const SfxPoolItem* pItem = nullptr;
                if( SfxItemState::SET == rSet.GetItemState( nUnknownWhich, true,
                                                       &pItem ) )
                {
                    SfxPoolItem *pNew = pItem->Clone();
                    pUnknownItem = dynamic_cast<SvXMLAttrContainerItem*>( pNew  );
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

    explicit BoxHolder(SvxBoxItem& rBox)
    {
        pTop    = rBox.GetTop() == nullptr ?
            nullptr : new SvxBorderLine( *rBox.GetTop() );
        pBottom = rBox.GetBottom() == nullptr ?
            nullptr : new SvxBorderLine( *rBox.GetBottom() );
        pLeft   = rBox.GetLeft() == nullptr ?
            nullptr : new SvxBorderLine( *rBox.GetLeft() );
        pRight  = rBox.GetRight() == nullptr ?
            nullptr : new SvxBorderLine( *rBox.GetRight() );
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
            SvxLRSpaceItem& rLRSpace = dynamic_cast<SvxLRSpaceItem&>(rItem);

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
                                rLRSpace.SetTextLeft( (sal_Int32)nAbs, (sal_uInt16)nProp );
                                break;
                            case MID_R_MARGIN:
                                rLRSpace.SetRight( (sal_Int32)nAbs, (sal_uInt16)nProp );
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

                    rLRSpace.SetTextFirstLineOfst( (short)nAbs, (sal_uInt16)nProp );
                }
                break;

                case MID_FIRST_AUTO:
                {
                    bool bAutoFirst(false);
                    bOk = ::sax::Converter::convertBool( bAutoFirst, rValue );
                    if( bOk )
                        rLRSpace.SetAutoFirst( bAutoFirst );
                }
                break;

                default:
                    OSL_FAIL( "unknown member id!");
            }
        }
        break;

        case RES_UL_SPACE:
        {
            SvxULSpaceItem& rULSpace = dynamic_cast<SvxULSpaceItem&>(rItem);

            sal_Int32 nProp = 100;
            sal_Int32 nAbs = 0;

            if( rValue.indexOf( '%' ) != -1 )
                bOk = ::sax::Converter::convertPercent( nProp, rValue );
            else
                bOk = rUnitConverter.convertMeasureToCore( nAbs, rValue );

            switch( nMemberId )
            {
                case MID_UP_MARGIN:
                    rULSpace.SetUpper( (sal_uInt16)nAbs, (sal_uInt16)nProp );
                    break;
                case MID_LO_MARGIN:
                    rULSpace.SetLower( (sal_uInt16)nAbs, (sal_uInt16)nProp );
                    break;
                default:
                    OSL_FAIL("unknown MemberId");
            }
        }
        break;

        case RES_SHADOW:
        {
            SvxShadowItem& rShadow = dynamic_cast<SvxShadowItem&>(rItem);

            bool bColorFound = false;
            bool bOffsetFound = false;

            SvXMLTokenEnumerator aTokenEnum( rValue );

            Color aColor( 128,128, 128 );
            rShadow.SetLocation( SVX_SHADOW_BOTTOMRIGHT );

            OUString aToken;
            while( aTokenEnum.getNextToken( aToken ) )
            {
                if( IsXMLToken( aToken, XML_NONE ) )
                {
                    rShadow.SetLocation( SVX_SHADOW_NONE );
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
                                rShadow.SetLocation( SVX_SHADOW_TOPLEFT );
                            }
                            else
                            {
                                rShadow.SetLocation( SVX_SHADOW_BOTTOMLEFT );
                            }
                        }
                        else
                        {
                            if( nY < 0 )
                            {
                                rShadow.SetLocation( SVX_SHADOW_TOPRIGHT );
                            }
                            else
                            {
                                rShadow.SetLocation( SVX_SHADOW_BOTTOMRIGHT );
                            }
                        }

                        if( nX < 0 ) nX *= -1;
                        if( nY < 0 ) nY *= -1;

                        rShadow.SetWidth( static_cast< sal_uInt16 >( (nX + nY) >> 1 ) );
                    }
                }
            }

            if( bOk && ( bColorFound || bOffsetFound ) )
            {
                rShadow.SetColor(aColor);
            }
            else
                bOk = false;
        }
        break;

        case RES_BOX:
        {
            SvxBoxItem& rBox = dynamic_cast<SvxBoxItem&>(rItem);

            // copy SvxBorderLines
            BoxHolder aBoxes(rBox);

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
                        rBox.SetDistance( (sal_uInt16)nTemp, SvxBoxItemLine::LEFT );
                    if( nMemberId == RIGHT_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( (sal_uInt16)nTemp, SvxBoxItemLine::RIGHT );
                    if( nMemberId == TOP_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( (sal_uInt16)nTemp, SvxBoxItemLine::TOP );
                    if( nMemberId == BOTTOM_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( (sal_uInt16)nTemp, SvxBoxItemLine::BOTTOM);
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

            rBox.SetLine( aBoxes.pTop,    SvxBoxItemLine::TOP    );
            rBox.SetLine( aBoxes.pBottom, SvxBoxItemLine::BOTTOM );
            rBox.SetLine( aBoxes.pLeft,   SvxBoxItemLine::LEFT   );
            rBox.SetLine( aBoxes.pRight,  SvxBoxItemLine::RIGHT  );

            bOk = true;
        }
        break;

        case RES_BREAK:
        {
            SvxFormatBreakItem& rFormatBreak = dynamic_cast<SvxFormatBreakItem&>(rItem);
            sal_uInt16 eEnum;

            if( !SvXMLUnitConverter::convertEnum( eEnum, rValue, psXML_BreakType ) )
                return false;

            if( eEnum == 0 )
            {
                rFormatBreak.SetValue( SVX_BREAK_NONE );
                bOk = true;
            }
            else
            {
                switch( nMemberId )
                {
                    case MID_BREAK_BEFORE:
                        rFormatBreak.SetValue( static_cast< sal_uInt16 >((eEnum == 1) ?
                                             SVX_BREAK_COLUMN_BEFORE :
                                             SVX_BREAK_PAGE_BEFORE) );
                        break;
                    case MID_BREAK_AFTER:
                        rFormatBreak.SetValue( static_cast< sal_uInt16 >((eEnum == 1) ?
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
            SvxFormatKeepItem& rFormatKeep = dynamic_cast<SvxFormatKeepItem&>(rItem);

            if( IsXMLToken( rValue, XML_ALWAYS ) ||
                 IsXMLToken( rValue, XML_TRUE ) )
            {
                rFormatKeep.SetValue( true );
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_AUTO ) ||
                     IsXMLToken( rValue, XML_FALSE ) )
            {
                rFormatKeep.SetValue( false );
                bOk = true;
            }
        }
        break;

        case RES_BACKGROUND:
        {
            SvxBrushItem& rBrush = dynamic_cast<SvxBrushItem&>(rItem);

            sal_Int32 nTempColor(0);
            switch( nMemberId )
                {
                case MID_BACK_COLOR:
                    if( IsXMLToken( rValue, XML_TRANSPARENT ) )
                    {
                        rBrush.GetColor().SetTransparency(0xff);
                        bOk = true;
                    }
                    else if (::sax::Converter::convertColor(nTempColor, rValue))
                    {
                        Color aTempColor(nTempColor);
                        aTempColor.SetTransparency(0);
                        rBrush.SetColor( aTempColor );
                        bOk = true;
                    }
                    break;

                case MID_GRAPHIC_LINK:
                {
                    SvxGraphicPosition eOldGraphicPos = rBrush.GetGraphicPos();
                    rBrush.PutValue( Any(rValue), MID_GRAPHIC_URL );
                    if( GPOS_NONE == eOldGraphicPos &&
                        GPOS_NONE != rBrush.GetGraphicPos() )
                        rBrush.SetGraphicPos( GPOS_TILED );
                    bOk = true;
                }
                break;

                case MID_GRAPHIC_REPEAT:
                {
                    SvxGraphicPosition eGraphicPos = rBrush.GetGraphicPos();
                    sal_uInt16 nPos = GPOS_NONE;
                    if( SvXMLUnitConverter::convertEnum( nPos, rValue,
                                                    psXML_BrushRepeat ) )
                    {
                        if( GPOS_MM != nPos || GPOS_NONE == eGraphicPos ||
                            GPOS_AREA == eGraphicPos || GPOS_TILED == eGraphicPos )
                            rBrush.SetGraphicPos( (SvxGraphicPosition)nPos );
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
                        rBrush.SetGraphicPos( ePos );
                }
                break;

                case MID_GRAPHIC_FILTER:
                    rBrush.SetGraphicFilter( rValue );
                    bOk = true;
                    break;
                }
        }
        break;

        case RES_PAGEDESC:
        {
            SwFormatPageDesc& rPageDesc = dynamic_cast<SwFormatPageDesc&>(rItem);

            if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
            {
                sal_Int32 nVal;
                bOk = ::sax::Converter::convertNumber(
                        nVal, rValue, 0, USHRT_MAX);
                if( bOk )
                    rPageDesc.SetNumOffset( (sal_uInt16)nVal );
            }
        }
        break;

        case RES_LAYOUT_SPLIT:
        case RES_ROW_SPLIT:
        {
            SfxBoolItem& rSplit = dynamic_cast<SfxBoolItem&>(rItem);

            if( IsXMLToken( rValue, XML_AUTO ) ||
                 IsXMLToken( rValue, XML_TRUE ) )
            {
                rSplit.SetValue( true );
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_ALWAYS ) ||
                     IsXMLToken( rValue, XML_FALSE ) )
            {
                rSplit.SetValue( false );
                bOk = true;
            }
        }
        break;

        case RES_HORI_ORIENT:
        {
            SwFormatHoriOrient& rHoriOrient = dynamic_cast<SwFormatHoriOrient&>(rItem);

            sal_uInt16 nValue;
            bOk = SvXMLUnitConverter::convertEnum( nValue, rValue,
                                              aXMLTableAlignMap );
            if( bOk )
                rHoriOrient.SetHoriOrient( nValue );
        }
        break;

        case RES_VERT_ORIENT:
        {
            SwFormatVertOrient& rVertOrient = dynamic_cast<SwFormatVertOrient&>(rItem);

            sal_uInt16 nValue;
            bOk = SvXMLUnitConverter::convertEnum( nValue, rValue,
                                              aXMLTableVAlignMap );
            if( bOk )
                rVertOrient.SetVertOrient( nValue );
            //#i8855# text::VertOrientation::NONE is stored as empty string and should be applied here
            else if(rValue.isEmpty())
            {
                rVertOrient.SetVertOrient( text::VertOrientation::NONE );
                bOk = true;
            }
        }
        break;

        case RES_FRM_SIZE:
        {
            SwFormatFrameSize& rFrameSize = dynamic_cast<SwFormatFrameSize&>(rItem);

            bool bSetHeight = false;
            bool bSetWidth = false;
            bool bSetSizeType = false;
            SwFrameSize eSizeType = ATT_VAR_SIZE;
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

                        rFrameSize.SetWidthPercent( (sal_Int8)nValue );
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

                        rFrameSize.SetWidth( (sal_uInt16)nValue );
                        rFrameSize.SetHeightSizeType( ATT_VAR_SIZE );
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
                        rFrameSize.SetWidth( (sal_uInt16)nValue );
                    if( bSetHeight )
                        rFrameSize.SetHeight( (sal_uInt16)nValue );
                    if( bSetSizeType )
                        rFrameSize.SetHeightSizeType( eSizeType );
                }
            }
        }
        break;

        case RES_FRAMEDIR:
        {
            const XMLPropertyHandler* pWritingModeHandler =
                XMLPropertyHandlerFactory::CreatePropertyHandler(
                    XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT );
            if( pWritingModeHandler != nullptr )
            {
                Any aAny;
                bOk = pWritingModeHandler->importXML( rValue, aAny,
                                                      rUnitConverter );
                if( bOk )
                    bOk = rItem.PutValue( aAny, 0 );

                delete pWritingModeHandler;
            }
        }
        break;

        case RES_COLLAPSING_BORDERS:
        {
            SfxBoolItem& rBorders = dynamic_cast<SfxBoolItem&>(rItem);

            if( IsXMLToken( rValue, XML_COLLAPSING ) )
            {
                rBorders.SetValue(true);
                bOk = true;
            }
            else if( IsXMLToken( rValue, XML_SEPARATING ) )
            {
                rBorders.SetValue(false);
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
