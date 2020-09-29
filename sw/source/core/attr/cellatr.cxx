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

#include <calc.hxx>
#include <cellatr.hxx>
#include <doc.hxx>
#include <float.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <node.hxx>
#include <rolbck.hxx>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <calbck.hxx>
#include <swtable.hxx>

// The % SV_COUNTRY_LANGUAGE_OFFSET result checks if nFormat is a mere built-in
// @ Text format of *any* locale and if so uses the default text format. Text
// is text, the locale doesn't matter for Writer's number formatting purposes.
// The advantage is that this is the pool's default item value and some places
// benefit from this special treatment in that they don't have to handle/store
// attribute specifics, especially when writing a document.
SwTableBoxNumFormat::SwTableBoxNumFormat( sal_uInt32 nFormat )
    : SfxUInt32Item( RES_BOXATR_FORMAT,
            (((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == getSwDefaultTextFormat()) ?
             getSwDefaultTextFormat() : nFormat))
{
}

bool SwTableBoxNumFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetValue() == static_cast<const SwTableBoxNumFormat&>(rAttr).GetValue();
}

SwTableBoxNumFormat* SwTableBoxNumFormat::Clone( SfxItemPool* ) const
{
    return new SwTableBoxNumFormat( GetValue() );
}

SwTableBoxFormula::SwTableBoxFormula( const OUString& rFormula )
    : SfxPoolItem( RES_BOXATR_FORMULA ),
    SwTableFormula( rFormula ),
    m_pDefinedIn( nullptr )
{
}

bool SwTableBoxFormula::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetFormula() == static_cast<const SwTableBoxFormula&>(rAttr).GetFormula() &&
           m_pDefinedIn == static_cast<const SwTableBoxFormula&>(rAttr).m_pDefinedIn;
}

SwTableBoxFormula* SwTableBoxFormula::Clone( SfxItemPool* ) const
{
    // switch to external rendering
    SwTableBoxFormula* pNew = new SwTableBoxFormula( GetFormula() );
    pNew->SwTableFormula::operator=( *this );
    return pNew;
}

/** Get node type of the node containing this formula

    E.g. TextField -> TextNode, or
      BoxAttribute -> BoxStartNode

    Caution: Must override when inheriting.
*/
const SwNode* SwTableBoxFormula::GetNodeOfFormula() const
{
    const SwNode* pRet = nullptr;
    if( m_pDefinedIn )
    {
        SwTableBox* pBox = SwIterator<SwTableBox,SwModify>( *m_pDefinedIn ).First();
        if( pBox )
            pRet = pBox->GetSttNd();
    }
    return pRet;
}

SwTableBox* SwTableBoxFormula::GetTableBox()
{
    SwTableBox* pBox = nullptr;
    if( m_pDefinedIn )
        pBox = SwIterator<SwTableBox,SwModify>( *m_pDefinedIn ).First();
    return pBox;
}

void SwTableBoxFormula::ChangeState( const SfxPoolItem* pItem )
{
    if( !m_pDefinedIn )
        return ;

    SwTableFormulaUpdate* pUpdateField;
    if( !pItem || RES_TABLEFML_UPDATE != pItem->Which() )
    {
        // reset value flag
        ChgValid( false );
        return ;
    }

    pUpdateField = const_cast<SwTableFormulaUpdate*>(static_cast<const SwTableFormulaUpdate*>(pItem));

    // detect table that contains this attribute
    const SwTableNode* pTableNd;
    const SwNode* pNd = GetNodeOfFormula();
    if (!pNd || &pNd->GetNodes() != &pNd->GetDoc().GetNodes())
        return;
    pTableNd = pNd->FindTableNode();
    if( pTableNd == nullptr )
        return;

    switch( pUpdateField->m_eFlags )
    {
    case TBL_CALC:
        // reset value flag
        ChgValid( false );
        break;
    case TBL_BOXNAME:
        if( &pTableNd->GetTable() == pUpdateField->m_pTable )
            // use external rendering
            PtrToBoxNm( pUpdateField->m_pTable );
        break;
    case TBL_BOXPTR:
        // internal rendering
        BoxNmToPtr( &pTableNd->GetTable() );
        break;
    case TBL_RELBOXNAME:
        if( &pTableNd->GetTable() == pUpdateField->m_pTable )
            // relative rendering
            ToRelBoxNm( pUpdateField->m_pTable );
        break;

    case TBL_SPLITTBL:
        if( &pTableNd->GetTable() == pUpdateField->m_pTable )
        {
            sal_uInt16 nLnPos = SwTableFormula::GetLnPosInTable(
                                    pTableNd->GetTable(), GetTableBox() );
            pUpdateField->m_bBehindSplitLine = USHRT_MAX != nLnPos &&
                                        pUpdateField->m_nSplitLine <= nLnPos;
        }
        else
            pUpdateField->m_bBehindSplitLine = false;
        [[fallthrough]];
    case TBL_MERGETBL:
        if( pUpdateField->m_pHistory )
        {
            // for a history record the unchanged formula is needed
            SwTableBoxFormula aCopy( *this );
            pUpdateField->m_bModified = false;
            ToSplitMergeBoxNm( *pUpdateField );

            if( pUpdateField->m_bModified )
            {
                // external rendering
                aCopy.PtrToBoxNm( &pTableNd->GetTable() );
                pUpdateField->m_pHistory->Add(
                    &aCopy,
                    &aCopy,
                    pNd->FindTableBoxStartNode()->GetIndex());
            }
        }
        else
            ToSplitMergeBoxNm( *pUpdateField );
        break;
    }
}

void SwTableBoxFormula::Calc( SwTableCalcPara& rCalcPara, double& rValue )
{
    if( !rCalcPara.m_rCalc.IsCalcError() )
    {
        // create pointers from box names
        BoxNmToPtr( rCalcPara.m_pTable );
        const OUString sFormula( MakeFormula( rCalcPara ));
        if( !rCalcPara.m_rCalc.IsCalcError() )
            rValue = rCalcPara.m_rCalc.Calculate( sFormula ).GetDouble();
        else
            rValue = DBL_MAX;
        ChgValid( !rCalcPara.IsStackOverflow() ); // value is now valid again
    }
}

SwTableBoxValue::SwTableBoxValue()
    : SfxPoolItem( RES_BOXATR_VALUE ), m_nValue( 0 )
{
}

SwTableBoxValue::SwTableBoxValue( const double nVal )
    : SfxPoolItem( RES_BOXATR_VALUE ), m_nValue( nVal )
{
}

bool SwTableBoxValue::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwTableBoxValue const& rOther( static_cast<SwTableBoxValue const&>(rAttr) );
    // items with NaN should be equal to enable pooling
    return std::isnan( m_nValue )
        ?   std::isnan( rOther.m_nValue )
        :   ( m_nValue == rOther.m_nValue );
}

SwTableBoxValue* SwTableBoxValue::Clone( SfxItemPool* ) const
{
    return new SwTableBoxValue( m_nValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
