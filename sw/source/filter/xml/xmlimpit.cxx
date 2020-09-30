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

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <utility>
#include <xmloff/xmluconv.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <xmloff/namespacemap.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/memberids.h>
#include <osl/diagnose.h>

#include <hintids.hxx>
#include <unomid.h>
#include <svx/unomid.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdir.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>

#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "xmlithlp.hxx"
#include <com/sun/star/uno/Any.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using uno::Any;

constexpr sal_uInt16 nUnknownWhich = RES_UNKNOWNATR_CONTAINER;

SvXMLImportItemMapper::SvXMLImportItemMapper(
                                SvXMLItemMapEntriesRef const & rMapEntries ) :
    mrMapEntries( rMapEntries )
{
}

SvXMLImportItemMapper::~SvXMLImportItemMapper()
{
}

void
SvXMLImportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = std::move(rMapEntries);
}

// fills the given itemset with the attributes in the given list
void SvXMLImportItemMapper::importXML( SfxItemSet& rSet,
                                      uno::Reference< xml::sax::XFastAttributeList > const & xAttrList,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      const SvXMLNamespaceMap& rNamespaceMap )
{
    std::unique_ptr<SvXMLAttrContainerItem> pUnknownItem;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( IsTokenInNamespace(aIter.getToken(), XML_NAMESPACE_XMLNS) )
            continue;

        sal_Int32 nToken = aIter.getToken();
        const OUString sValue = aIter.toString();

        // find a map entry for this attribute
        sal_Int32 nLookupToken = nToken;
        // compatibility namespaces need to be transformed into current namespace before looking up
        if (IsTokenInNamespace(nLookupToken, XML_NAMESPACE_FO_COMPAT))
            nLookupToken = XML_ELEMENT(FO, (nLookupToken & TOKEN_MASK));
        SvXMLItemMapEntry const * pEntry = mrMapEntries->getByName( nLookupToken );

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

                // if it's not set, try the pool
                if (SfxItemState::SET != eState && SfxItemPool::IsWhich(pEntry->nWhichId))
                    pItem = &rSet.GetPool()->GetDefaultItem(pEntry->nWhichId);

                // do we have an item?
                if(eState >= SfxItemState::DEFAULT && pItem)
                {
                    std::unique_ptr<SfxPoolItem> pNewItem(pItem->Clone());
                    bool bPut = false;

                    if( 0 == (pEntry->nMemberId&MID_SW_FLAG_SPECIAL_ITEM_IMPORT) )
                    {
                        bPut = PutXMLValue( *pNewItem, sValue,
                                            static_cast<sal_uInt16>( pEntry->nMemberId & MID_SW_FLAG_MASK ),
                                            rUnitConverter );

                    }
                    else
                    {
                        bPut = handleSpecialItem( *pEntry, *pNewItem, rSet,
                                                  sValue, rUnitConverter );
                    }

                    if( bPut )
                        rSet.Put( *pNewItem );
                }
                else
                {
                    OSL_FAIL( "Could not get a needed item for xml import!" );
                }
            }
            else if( 0 != (pEntry->nMemberId & MID_SW_FLAG_NO_ITEM_IMPORT) )
            {
                handleNoItem( *pEntry, rSet, sValue, rUnitConverter,
                              rNamespaceMap );
            }
        }
        else
        {
            if( !pUnknownItem )
            {
                const SfxPoolItem* pItem = nullptr;
                if( SfxItemState::SET == rSet.GetItemState( nUnknownWhich, true,
                                                       &pItem ) )
                {
                    pUnknownItem.reset( static_cast<SvXMLAttrContainerItem*>( pItem->Clone() ) );
                }
                else
                {
                    pUnknownItem.reset( new SvXMLAttrContainerItem( nUnknownWhich ) );
                }
            }
            if( pUnknownItem )
            {
                if( IsTokenInNamespace(nToken, XML_NAMESPACE_NONE) )
                    pUnknownItem->AddAttr( SvXMLImport::getNameFromToken( nToken ), sValue );
                else
                {
                    const OUString& rAttrNamespacePrefix = SvXMLImport::getNamespacePrefixFromToken(nToken, &rNamespaceMap);
                    OUString sAttrName = SvXMLImport::getNameFromToken( nToken );
                    if ( !rAttrNamespacePrefix.isEmpty() )
                        sAttrName = rAttrNamespacePrefix + SvXMLImport::aNamespaceSeparator + sAttrName;
                    OUString aLocalName, aPrefix, aNamespace;
                    rNamespaceMap.GetKeyByAttrName( sAttrName, &aPrefix, &aLocalName,
                                                        &aNamespace );
                    if ( !rAttrNamespacePrefix.isEmpty() )
                        pUnknownItem->AddAttr( rAttrNamespacePrefix, aNamespace, aLocalName,
                                               sValue );
                    else
                        pUnknownItem->AddAttr( aLocalName, sValue );
                }
            }
        }
    }

    importXMLUnknownAttributes(rSet, xAttrList, rUnitConverter, pUnknownItem);

    if( pUnknownItem )
    {
        rSet.Put( *pUnknownItem );
    }

    finished(rSet, rUnitConverter);
}

void SvXMLImportItemMapper::importXMLUnknownAttributes( SfxItemSet& rSet,
                                      uno::Reference< xml::sax::XFastAttributeList > const & xAttrList,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      std::unique_ptr<SvXMLAttrContainerItem>& pUnknownItem)
{
    const css::uno::Sequence< css::xml::Attribute > unknownAttributes = xAttrList->getUnknownAttributes();
    for (const auto & rAttribute : unknownAttributes)
    {
        if( !pUnknownItem )
        {
            const SfxPoolItem* pItem = nullptr;
            if( SfxItemState::SET == rSet.GetItemState( nUnknownWhich, true,
                                                   &pItem ) )
            {
                pUnknownItem.reset( static_cast<SvXMLAttrContainerItem*>( pItem->Clone() ) );
            }
            else
            {
                pUnknownItem.reset( new SvXMLAttrContainerItem( nUnknownWhich ) );
            }
        }
        if( pUnknownItem )
        {
            if( rAttribute.NamespaceURL.isEmpty() )
                pUnknownItem->AddAttr( rAttribute.Name, rAttribute.Value );
            else
            {
                OUString sPrefix;
                OUString sName = rAttribute.Name;
                int i = sName.indexOf(':');
                if (i != -1)
                {
                    sPrefix = sName.copy(0, i-1);
                    sName = sName.copy(i+1);
                }
                // the sax parser doesn't reject these, strangely
                if (sName.indexOf(':') == -1)
                    pUnknownItem->AddAttr( sPrefix, rAttribute.NamespaceURL, sName,
                                           rAttribute.Value );
                else
                    SAL_WARN("sw", "ignoring dodgy attribute: " + rAttribute.Name);
            }
        }
    }

    if( pUnknownItem )
    {
        rSet.Put( *pUnknownItem );
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
                                            const SvXMLUnitConverter& /*rUnitConverter*/ )
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

namespace {

struct BoxHolder
{
    std::unique_ptr<SvxBorderLine> pTop;
    std::unique_ptr<SvxBorderLine> pBottom;
    std::unique_ptr<SvxBorderLine> pLeft;
    std::unique_ptr<SvxBorderLine> pRight;

    BoxHolder(BoxHolder const&) = delete;
    BoxHolder& operator=(BoxHolder const&) = delete;

    explicit BoxHolder(SvxBoxItem const & rBox)
    {
        if (rBox.GetTop())
            pTop.reset(new SvxBorderLine( *rBox.GetTop() ));
        if (rBox.GetBottom())
            pBottom.reset(new SvxBorderLine( *rBox.GetBottom() ));
        if (rBox.GetLeft())
            pLeft.reset(new SvxBorderLine( *rBox.GetLeft() ));
        if (rBox.GetRight())
            pRight.reset(new SvxBorderLine( *rBox.GetRight() ));
    }
};

}

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
                                rLRSpace.SetTextLeft( nAbs, static_cast<sal_uInt16>(nProp) );
                                break;
                            case MID_R_MARGIN:
                                rLRSpace.SetRight( nAbs, static_cast<sal_uInt16>(nProp) );
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

                    rLRSpace.SetTextFirstLineOffset( static_cast<short>(nAbs), static_cast<sal_uInt16>(nProp) );
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
                    rULSpace.SetUpper( static_cast<sal_uInt16>(nAbs), static_cast<sal_uInt16>(nProp) );
                    break;
                case MID_LO_MARGIN:
                    rULSpace.SetLower( static_cast<sal_uInt16>(nAbs), static_cast<sal_uInt16>(nProp) );
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
            rShadow.SetLocation( SvxShadowLocation::BottomRight );

            OUString aToken;
            while( aTokenEnum.getNextToken( aToken ) )
            {
                if( IsXMLToken( aToken, XML_NONE ) )
                {
                    rShadow.SetLocation( SvxShadowLocation::NONE );
                    bOk = true;
                }
                else if( !bColorFound && aToken.startsWith("#") )
                {
                    bOk = ::sax::Converter::convertColor( aColor, aToken );
                    if( !bOk )
                        return false;

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
                                rShadow.SetLocation( SvxShadowLocation::TopLeft );
                            }
                            else
                            {
                                rShadow.SetLocation( SvxShadowLocation::BottomLeft );
                            }
                        }
                        else
                        {
                            if( nY < 0 )
                            {
                                rShadow.SetLocation( SvxShadowLocation::TopRight );
                            }
                            else
                            {
                                rShadow.SetLocation( SvxShadowLocation::BottomRight );
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
                        rBox.SetDistance( static_cast<sal_uInt16>(nTemp), SvxBoxItemLine::LEFT );
                    if( nMemberId == RIGHT_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( static_cast<sal_uInt16>(nTemp), SvxBoxItemLine::RIGHT );
                    if( nMemberId == TOP_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( static_cast<sal_uInt16>(nTemp), SvxBoxItemLine::TOP );
                    if( nMemberId == BOTTOM_BORDER_PADDING ||
                        nMemberId == ALL_BORDER_PADDING )
                        rBox.SetDistance( static_cast<sal_uInt16>(nTemp), SvxBoxItemLine::BOTTOM);
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

            rBox.SetLine( aBoxes.pTop.get(),    SvxBoxItemLine::TOP    );
            rBox.SetLine( aBoxes.pBottom.get(), SvxBoxItemLine::BOTTOM );
            rBox.SetLine( aBoxes.pLeft.get(),   SvxBoxItemLine::LEFT   );
            rBox.SetLine( aBoxes.pRight.get(),  SvxBoxItemLine::RIGHT  );

            bOk = true;
        }
        break;

        case RES_BREAK:
        {
            SvxFormatBreakItem& rFormatBreak = dynamic_cast<SvxFormatBreakItem&>(rItem);
            sal_uInt16 eEnum{};

            if( !SvXMLUnitConverter::convertEnum( eEnum, rValue, psXML_BreakType ) )
                return false;

            if( eEnum == 0 )
            {
                rFormatBreak.SetValue( SvxBreak::NONE );
                bOk = true;
            }
            else
            {
                switch( nMemberId )
                {
                    case MID_BREAK_BEFORE:
                        rFormatBreak.SetValue( eEnum == 1 ?
                                               SvxBreak::ColumnBefore :
                                               SvxBreak::PageBefore );
                        break;
                    case MID_BREAK_AFTER:
                        rFormatBreak.SetValue( eEnum == 1 ?
                                               SvxBreak::ColumnAfter :
                                               SvxBreak::PageAfter );
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

                case MID_GRAPHIC_REPEAT:
                {
                    SvxGraphicPosition eGraphicPos = rBrush.GetGraphicPos();
                    SvxGraphicPosition nPos = GPOS_NONE;
                    if( SvXMLUnitConverter::convertEnum( nPos, rValue,
                                                    psXML_BrushRepeat ) )
                    {
                        if( GPOS_MM != nPos || GPOS_NONE == eGraphicPos ||
                            GPOS_AREA == eGraphicPos || GPOS_TILED == eGraphicPos )
                            rBrush.SetGraphicPos( nPos );
                        bOk = true;
                    }
                }
                break;

                case MID_GRAPHIC_POSITION:
                {
                    SvxGraphicPosition ePos = GPOS_NONE, eTmp;
                    SvxGraphicPosition nTmp;
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
                                    ePos, nTmp );
                            else if( !bHori )
                                ePos = nTmp;
                            else
                                bOk = false;
                            bHori = true;
                        }
                        else if( SvXMLUnitConverter::convertEnum( nTmp, aToken,
                                                         psXML_BrushVertPos ) )
                        {
                            if( bHori )
                                sw_frmitems_MergeXMLVertPos(
                                    ePos, nTmp );
                            else if( !bVert )
                                ePos = nTmp;
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
                // i#114163 tdf#77111: OOo < 3.3 had a bug where it wrote
                // "auto" as "0" for tables - now that we support a real offset
                //  0, this fake "0" MUST NOT be imported as offset 0!
                if( bOk && nVal > 0 )
                    rPageDesc.SetNumOffset( static_cast<sal_uInt16>(nVal) );
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

            sal_Int16 nValue;
            bOk = SvXMLUnitConverter::convertEnum( nValue, rValue,
                                              aXMLTableAlignMap );
            if( bOk )
                rHoriOrient.SetHoriOrient( nValue );
        }
        break;

        case RES_VERT_ORIENT:
        {
            SwFormatVertOrient& rVertOrient = dynamic_cast<SwFormatVertOrient&>(rItem);

            sal_Int16 nValue;
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
            SwFrameSize eSizeType = SwFrameSize::Variable;
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

                        rFrameSize.SetWidthPercent( static_cast<sal_Int8>(nValue) );
                    }
                }
                break;
                case MID_FRMSIZE_WIDTH:
                    bSetWidth = true;
                    break;
                case MID_FRMSIZE_MIN_HEIGHT:
                    eSizeType = SwFrameSize::Minimum;
                    bSetHeight = true;
                    nMin = 1;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_FIX_HEIGHT:
                    eSizeType = SwFrameSize::Fixed;
                    bSetHeight = true;
                    nMin = 1;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_COL_WIDTH:
                    eSizeType = SwFrameSize::Fixed;
                    bSetWidth = true;
                    bSetSizeType = true;
                    break;
                case MID_FRMSIZE_REL_COL_WIDTH:
                {
                    sal_Int32 nPos = rValue.indexOf( '*' );
                    if( -1 != nPos )
                    {
                        sal_Int32 nValue = rValue.toInt32();
                        if( nValue < MINLAY )
                            nValue = MINLAY;
                        else if( nValue > SAL_MAX_UINT16 )
                            nValue = SAL_MAX_UINT16;

                        rFrameSize.SetWidth( static_cast<sal_uInt16>(nValue) );
                        rFrameSize.SetHeightSizeType( SwFrameSize::Variable );
                        bOk = true;
                    }
                }
                break;
                }

            if( bSetHeight || bSetWidth )
            {
                sal_Int32 nValue;
                bOk = rUnitConverter.convertMeasureToCore(nValue, rValue, nMin,
                                                     USHRT_MAX );
                if( bOk )
                {
                    if( bSetWidth )
                        rFrameSize.SetWidth( static_cast<sal_uInt16>(nValue) );
                    if( bSetHeight )
                        rFrameSize.SetHeight( static_cast<sal_uInt16>(nValue) );
                    if( bSetSizeType )
                        rFrameSize.SetHeightSizeType( eSizeType );
                }
            }
        }
        break;

        case RES_FRAMEDIR:
        {
            if (IsXMLToken(rValue, XML_BT_LR))
            {
                // Read bt-lr from the extension namespace, handle other values
                // below.
                Any aAny;
                aAny <<= static_cast<sal_uInt16>(SvxFrameDirection::Vertical_LR_BT);
                bOk = rItem.PutValue(aAny, 0);
            }
            else
            {
                std::unique_ptr<XMLPropertyHandler> pWritingModeHandler =
                    XMLPropertyHandlerFactory::CreatePropertyHandler(
                        XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT );
                Any aAny;
                bOk = pWritingModeHandler->importXML( rValue, aAny,
                                                          rUnitConverter );
                if( bOk )
                    bOk = rItem.PutValue( aAny, 0 );
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
