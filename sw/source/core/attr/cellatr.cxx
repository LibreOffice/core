/*************************************************************************
 *
 *  $RCSfile: cellatr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <float.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>          // fuer RES_..
#endif

#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif



//TYPEINIT1( SwFmt, SwClient ); //rtti fuer SwFmt

/*************************************************************************
|*
*************************************************************************/


SwTblBoxNumFormat::SwTblBoxNumFormat( UINT32 nFormat, BOOL bFlag )
    : SfxUInt32Item( RES_BOXATR_FORMAT, nFormat ), bAuto( bFlag )
{
}


int SwTblBoxNumFormat::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return GetValue() == ((SwTblBoxNumFormat&)rAttr).GetValue() &&
            bAuto == ((SwTblBoxNumFormat&)rAttr).bAuto;
}


SfxPoolItem* SwTblBoxNumFormat::Clone( SfxItemPool* ) const
{
    return new SwTblBoxNumFormat( GetValue(), bAuto );
}


/*************************************************************************
|*
*************************************************************************/



SwTblBoxFormula::SwTblBoxFormula( const String& rFormula )
    : SfxPoolItem( RES_BOXATR_FORMULA ),
    SwTableFormula( rFormula ),
    pDefinedIn( 0 )
{
}


int SwTblBoxFormula::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return GetFormula() == ((SwTblBoxFormula&)rAttr).GetFormula() &&
            pDefinedIn == ((SwTblBoxFormula&)rAttr).pDefinedIn;
}


SfxPoolItem* SwTblBoxFormula::Clone( SfxItemPool* ) const
{
// auf externe Darstellung umschalten!!
    SwTblBoxFormula* pNew = new SwTblBoxFormula( GetFormula() );
    pNew->SwTableFormula::operator=( *this );
    return pNew;
}



    // suche den Node, in dem die Formel steht:
    //  TextFeld    -> TextNode,
    //  BoxAttribut -> BoxStartNode
    // !!! MUSS VON JEDER ABLEITUNG UEBERLADEN WERDEN !!!
const SwNode* SwTblBoxFormula::GetNodeOfFormula() const
{
    const SwNode* pRet = 0;
    if( pDefinedIn )
    {
        SwClient* pBox = SwClientIter( *pDefinedIn ).First( TYPE( SwTableBox ));
        if( pBox )
            pRet = ((SwTableBox*)pBox)->GetSttNd();
    }
    return pRet;
}


SwTableBox* SwTblBoxFormula::GetTableBox()
{
    SwTableBox* pBox = 0;
    if( pDefinedIn )
        pBox = (SwTableBox*)SwClientIter( *pDefinedIn ).
                            First( TYPE( SwTableBox ));
    return pBox;
}


void SwTblBoxFormula::ChangeState( const SfxPoolItem* pItem )
{
    if( !pDefinedIn )
        return ;

    SwTableFmlUpdate* pUpdtFld;
    if( !pItem || RES_TABLEFML_UPDATE != pItem->Which() )
    {
        // setze bei allen das Value-Flag zurueck
        ChgValid( FALSE );
        return ;
    }

    pUpdtFld = (SwTableFmlUpdate*)pItem;

    // bestimme die Tabelle, in der das Attribut steht
    const SwTableNode* pTblNd;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && &pNd->GetNodes() == &pNd->GetDoc()->GetNodes() &&
        0 != ( pTblNd = pNd->FindTableNode() ))
    {
        switch( pUpdtFld->eFlags )
        {
        case TBL_CALC:
            // setze das Value-Flag zurueck
            // JP 17.06.96: interne Darstellung auf alle Formeln
            //              (Referenzen auf andere Tabellen!!!)
//          if( VF_CMD & pFld->GetFormat() )
//              pFld->PtrToBoxNm( pUpdtFld->pTbl );
//          else
                ChgValid( FALSE );
            break;
        case TBL_BOXNAME:
            // ist es die gesuchte Tabelle ??
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                // zur externen Darstellung
                PtrToBoxNm( pUpdtFld->pTbl );
            break;
        case TBL_BOXPTR:
            // zur internen Darstellung
            // JP 17.06.96: interne Darstellung auf alle Formeln
            //              (Referenzen auf andere Tabellen!!!)
            BoxNmToPtr( &pTblNd->GetTable() );
            break;
        case TBL_RELBOXNAME:
            // ist es die gesuchte Tabelle ??
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                // zur relativen Darstellung
                ToRelBoxNm( pUpdtFld->pTbl );
            break;

        case TBL_SPLITTBL:
            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
            {
                USHORT nLnPos = SwTableFormula::GetLnPosInTbl(
                                        pTblNd->GetTable(), GetTableBox() );
                pUpdtFld->bBehindSplitLine = USHRT_MAX != nLnPos &&
                                            pUpdtFld->nSplitLine <= nLnPos;
            }
            else
                pUpdtFld->bBehindSplitLine = FALSE;
            // kein break
        case TBL_MERGETBL:
            if( pUpdtFld->pHistory )
            {
                // fuer die History brauche ich aber die unveraenderte Formel
                SwTblBoxFormula aCopy( *this );
                pUpdtFld->bModified = FALSE;
                ToSplitMergeBoxNm( *pUpdtFld );

                if( pUpdtFld->bModified )
                {
                    // und dann in der externen Darstellung
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
    if( !rCalcPara.rCalc.IsCalcError() )        // ist schon Fehler gesetzt ?
    {
        // erzeuge aus den BoxNamen die Pointer
        BoxNmToPtr( rCalcPara.pTbl );
        String sFml( MakeFormel( rCalcPara ));
        if( !rCalcPara.rCalc.IsCalcError() )
            rValue = rCalcPara.rCalc.Calculate( sFml ).GetDouble();
        else
            rValue = DBL_MAX;
        ChgValid( !rCalcPara.IsStackOverFlow() );       // der Wert ist wieder gueltig
    }
}

/*************************************************************************
|*
*************************************************************************/


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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return nValue == ((SwTblBoxValue&)rAttr).nValue;
}


SfxPoolItem* SwTblBoxValue::Clone( SfxItemPool* ) const
{
    return new SwTblBoxValue( nValue );
}




