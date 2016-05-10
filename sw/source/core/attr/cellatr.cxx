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
#include <format.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <node.hxx>
#include <rolbck.hxx>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <calbck.hxx>
#include <swtable.hxx>

SwTableBoxNumFormat::SwTableBoxNumFormat( sal_uInt32 nFormat, bool bFlag )
    : SfxUInt32Item( RES_BOXATR_FORMAT, nFormat ), bAuto( bFlag )
{
}

bool SwTableBoxNumFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetValue() == static_cast<const SwTableBoxNumFormat&>(rAttr).GetValue() &&
           bAuto == static_cast<const SwTableBoxNumFormat&>(rAttr).bAuto;
}

SfxPoolItem* SwTableBoxNumFormat::Clone( SfxItemPool* ) const
{
    return new SwTableBoxNumFormat( GetValue(), bAuto );
}

SwTableBoxFormula::SwTableBoxFormula( const OUString& rFormula )
    : SfxPoolItem( RES_BOXATR_FORMULA ),
    SwTableFormula( rFormula ),
    pDefinedIn( nullptr )
{
}

bool SwTableBoxFormula::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetFormula() == static_cast<const SwTableBoxFormula&>(rAttr).GetFormula() &&
           pDefinedIn == static_cast<const SwTableBoxFormula&>(rAttr).pDefinedIn;
}

SfxPoolItem* SwTableBoxFormula::Clone( SfxItemPool* ) const
{
    // switch to external rendering
    SwTableBoxFormula* pNew = new SwTableBoxFormula( GetFormula() );
    pNew->SwTableFormula::operator=( *this );
    return pNew;
}

/** Get node type of the node containing this formula

    E.g. TextFeld -> TextNode, or
     BoxAttribute -> BoxStartNode

    Caution: Must override when inheriting.
*/
const SwNode* SwTableBoxFormula::GetNodeOfFormula() const
{
    const SwNode* pRet = nullptr;
    if( pDefinedIn )
    {
        SwTableBox* pBox = SwIterator<SwTableBox,SwModify>( *pDefinedIn ).First();
        if( pBox )
            pRet = pBox->GetSttNd();
    }
    return pRet;
}

SwTableBox* SwTableBoxFormula::GetTableBox()
{
    SwTableBox* pBox = nullptr;
    if( pDefinedIn )
        pBox = SwIterator<SwTableBox,SwModify>( *pDefinedIn ).First();
    return pBox;
}

void SwTableBoxFormula::ChangeState( const SfxPoolItem* pItem )
{
    if( !pDefinedIn )
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
    if( pNd && &pNd->GetNodes() == &pNd->GetDoc()->GetNodes() &&
        nullptr != ( pTableNd = pNd->FindTableNode() ))
    {
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
            SAL_FALLTHROUGH;
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
}

void SwTableBoxFormula::Calc( SwTableCalcPara& rCalcPara, double& rValue )
{
    if( !rCalcPara.rCalc.IsCalcError() )
    {
        // create pointers from box names
        BoxNmToPtr( rCalcPara.pTable );
        const OUString sFormula( MakeFormula( rCalcPara ));
        if( !rCalcPara.rCalc.IsCalcError() )
            rValue = rCalcPara.rCalc.Calculate( sFormula ).GetDouble();
        else
            rValue = DBL_MAX;
        ChgValid( !rCalcPara.IsStackOverflow() ); // value is now valid again
    }
}

SwTableBoxValue::SwTableBoxValue()
    : SfxPoolItem( RES_BOXATR_VALUE ), nValue( 0 )
{
}

SwTableBoxValue::SwTableBoxValue( const double nVal )
    : SfxPoolItem( RES_BOXATR_VALUE ), nValue( nVal )
{
}

bool SwTableBoxValue::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    SwTableBoxValue const& rOther( static_cast<SwTableBoxValue const&>(rAttr) );
    // items with NaN should be equal to enable pooling
    return ::rtl::math::isNan( nValue )
        ?   ::rtl::math::isNan( rOther.nValue )
        :   ( nValue == rOther.nValue );
}

SfxPoolItem* SwTableBoxValue::Clone( SfxItemPool* ) const
{
    return new SwTableBoxValue( nValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
