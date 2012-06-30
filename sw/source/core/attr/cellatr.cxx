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

#include <float.h>
#include <rtl/math.hxx>
#include <hintids.hxx>
#include <cellatr.hxx>
#include <calc.hxx>
#include <format.hxx>
#include <doc.hxx>
#include <swtable.hxx>
#include <node.hxx>
#include <hints.hxx>
#include <rolbck.hxx>
#include <switerator.hxx>

// -----------------
// SwTblBoxNumFormat
// -----------------

SwTblBoxNumFormat::SwTblBoxNumFormat( sal_uInt32 nFormat, sal_Bool bFlag )
    : SfxUInt32Item( RES_BOXATR_FORMAT, nFormat ), bAuto( bFlag )
{
}

int SwTblBoxNumFormat::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "no matching attributes" );
    return GetValue() == ( (SwTblBoxNumFormat&) rAttr ).GetValue() &&
           bAuto == ( (SwTblBoxNumFormat&) rAttr ).bAuto;
}

SfxPoolItem* SwTblBoxNumFormat::Clone( SfxItemPool* ) const
{
    return new SwTblBoxNumFormat( GetValue(), bAuto );
}

// -----------------
// SwTblBoxFormula
// -----------------

SwTblBoxFormula::SwTblBoxFormula( const String& rFormula )
    : SfxPoolItem( RES_BOXATR_FORMULA ),
    SwTableFormula( rFormula ),
    pDefinedIn( 0 )
{
}

int SwTblBoxFormula::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return GetFormula() == ( (SwTblBoxFormula&) rAttr ).GetFormula() &&
           pDefinedIn == ( (SwTblBoxFormula&) rAttr ).pDefinedIn;
}

SfxPoolItem* SwTblBoxFormula::Clone( SfxItemPool* ) const
{
    // switch to external rendering
    SwTblBoxFormula* pNew = new SwTblBoxFormula( GetFormula() );
    pNew->SwTableFormula::operator=( *this );
    return pNew;
}

/** Get node type of the node containing this formula

    E.g. TextFeld -> TextNode, or
     BoxAttribute -> BoxStartNode

    Caution: Has to be overloaded when inheriting.
*/
const SwNode* SwTblBoxFormula::GetNodeOfFormula() const
{
    const SwNode* pRet = 0;
    if( pDefinedIn )
    {
        SwTableBox* pBox = SwIterator<SwTableBox,SwModify>::FirstElement( *pDefinedIn );
        if( pBox )
            pRet = pBox->GetSttNd();
    }
    return pRet;
}

SwTableBox* SwTblBoxFormula::GetTableBox()
{
    SwTableBox* pBox = 0;
    if( pDefinedIn )
        pBox = SwIterator<SwTableBox,SwModify>::FirstElement( *pDefinedIn );
    return pBox;
}

void SwTblBoxFormula::ChangeState( const SfxPoolItem* pItem )
{
    if( !pDefinedIn )
        return ;

    SwTableFmlUpdate* pUpdtFld;
    if( !pItem || RES_TABLEFML_UPDATE != pItem->Which() )
    {
        // reset value flag
        ChgValid( sal_False );
        return ;
    }

    pUpdtFld = (SwTableFmlUpdate*)pItem;

    // detect table that contains this attribute
    const SwTableNode* pTblNd;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && &pNd->GetNodes() == &pNd->GetDoc()->GetNodes() &&
        0 != ( pTblNd = pNd->FindTableNode() ))
    {
        switch( pUpdtFld->eFlags )
        {
        case TBL_CALC:
            // reset value flag
            ChgValid( sal_False );
            break;
        case TBL_BOXNAME:
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                // use external rendering
                PtrToBoxNm( pUpdtFld->pTbl );
            break;
        case TBL_BOXPTR:
            // internal rendering
            BoxNmToPtr( &pTblNd->GetTable() );
            break;
        case TBL_RELBOXNAME:
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                // relative rendering
                ToRelBoxNm( pUpdtFld->pTbl );
            break;

        case TBL_SPLITTBL:
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
            {
                sal_uInt16 nLnPos = SwTableFormula::GetLnPosInTbl(
                                        pTblNd->GetTable(), GetTableBox() );
                pUpdtFld->bBehindSplitLine = USHRT_MAX != nLnPos &&
                                            pUpdtFld->nSplitLine <= nLnPos;
            }
            else
                pUpdtFld->bBehindSplitLine = sal_False;
            // no break
        case TBL_MERGETBL:
            if( pUpdtFld->pHistory )
            {
                // for a history record the unchanged formula is needed
                SwTblBoxFormula aCopy( *this );
                pUpdtFld->bModified = sal_False;
                ToSplitMergeBoxNm( *pUpdtFld );

                if( pUpdtFld->bModified )
                {
                    // external rendering
                    aCopy.PtrToBoxNm( &pTblNd->GetTable() );
                    pUpdtFld->pHistory->Add( &aCopy, &aCopy,
                                pNd->FindTableBoxStartNode()->GetIndex() );
                }
            }
            else
                ToSplitMergeBoxNm( *pUpdtFld );
            break;
        }
    }
}

void SwTblBoxFormula::Calc( SwTblCalcPara& rCalcPara, double& rValue )
{
    if( !rCalcPara.rCalc.IsCalcError() )
    {
        // create pointers from box names
        BoxNmToPtr( rCalcPara.pTbl );
        String sFml( MakeFormel( rCalcPara ));
        if( !rCalcPara.rCalc.IsCalcError() )
            rValue = rCalcPara.rCalc.Calculate( sFml ).GetDouble();
        else
            rValue = DBL_MAX;
        ChgValid( !rCalcPara.IsStackOverFlow() ); // value is now valid again
    }
}

// -------------
// SwTblBoxValue
// -------------

SwTblBoxValue::SwTblBoxValue()
    : SfxPoolItem( RES_BOXATR_VALUE ), nValue( 0 )
{
}

SwTblBoxValue::SwTblBoxValue( const double nVal )
    : SfxPoolItem( RES_BOXATR_VALUE ), nValue( nVal )
{
}

int SwTblBoxValue::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "SwTblBoxValue: item not equal" );
    SwTblBoxValue const& rOther( static_cast<SwTblBoxValue const&>(rAttr) );
    // items with NaN should be equal to enable pooling
    return ::rtl::math::isNan( nValue )
        ?   ::rtl::math::isNan( rOther.nValue )
        :   ( nValue == rOther.nValue );
}

SfxPoolItem* SwTblBoxValue::Clone( SfxItemPool* ) const
{
    return new SwTblBoxValue( nValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
