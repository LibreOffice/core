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

#include <float.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <frmfmt.hxx>
#include <layfrm.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <cellfml.hxx>
#include <calc.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <flddat.hxx>
#include <cellatr.hxx>
#include <ndindex.hxx>
#include <comphelper/string.hxx>

namespace
{

const sal_Unicode cRelSeparator = ',';
const sal_Unicode cRelIdentifier = '';     // CTRL-R

enum
{
    cMAXSTACKSIZE = 50
};

}

static const SwFrm* lcl_GetBoxFrm( const SwTableBox& rBox );
static sal_Int32 lcl_GetLongBoxNum( OUString& rStr );
static const SwTableBox* lcl_RelToBox( const SwTable& rTbl,
                                       const SwTableBox* pRefBox,
                                       OUString sGetName);
static OUString lcl_BoxNmToRel( const SwTable& rTbl,
                                const SwTableNode& rTblNd,
                                OUString sRefBoxNm,
                                OUString sGetStr,
                                bool bExtrnlNm);


/** Get value of this box.
 *
 * The value is comes from the first TextNode. If it starts with a number/
 * formula then calculate it, if it starts with a field then get the value.
 * All other conditions return 0 (and an error?).
 */
double SwTableBox::GetValue( SwTblCalcPara& rCalcPara ) const
{
    double nRet = 0;

    if( rCalcPara.rCalc.IsCalcError() )
        return nRet;            // stop if there is already an error set

    rCalcPara.rCalc.SetCalcError( CALC_SYNTAX );    // default: error

    // no content box?
    if( !pSttNd  )
        return nRet;

    if( rCalcPara.IncStackCnt() )
        return nRet;

    rCalcPara.SetLastTblBox( this );

    // Does it create a recursion?
    SwTableBox* pBox = (SwTableBox*)this;
    if( rCalcPara.pBoxStk->find( pBox ) != rCalcPara.pBoxStk->end() )
        return nRet;            // already on the stack: error

    // re-start with this box
    rCalcPara.SetLastTblBox( this );

    rCalcPara.pBoxStk->insert( pBox );      // add
    do {        // Middle-Check-Loop, so that we can jump from here. Used so that the box pointer
                // will be removed from stack at the end.
        SwDoc* pDoc = GetFrmFmt()->GetDoc();

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == GetFrmFmt()->GetItemState(
                                RES_BOXATR_FORMULA, sal_False, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status
            if( !((SwTblBoxFormula*)pItem)->IsValid() )
            {
                // calculate
                const SwTable* pTmp = rCalcPara.pTbl;
                rCalcPara.pTbl = &pBox->GetSttNd()->FindTableNode()->GetTable();
                ((SwTblBoxFormula*)pItem)->Calc( rCalcPara, nRet );

                if( !rCalcPara.IsStackOverFlow() )
                {
                    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
                    SfxItemSet aTmp( pDoc->GetAttrPool(),
                                        RES_BOXATR_BEGIN,RES_BOXATR_END-1 );
                    aTmp.Put( SwTblBoxValue( nRet ) );
                    if( SFX_ITEM_SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetFmtAttr( aTmp );
                }
                rCalcPara.pTbl = pTmp;
            }
            else
                nRet = GetFrmFmt()->GetTblBoxValue().GetValue();
            break;
        }
        else if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                                RES_BOXATR_VALUE, sal_False, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status
            nRet = ((SwTblBoxValue*)pItem)->GetValue();
            break;
        }

        SwTxtNode* pTxtNd = pDoc->GetNodes()[ pSttNd->GetIndex() + 1 ]->GetTxtNode();
        if( !pTxtNd )
            break;

        sal_Int32 nSttPos = 0;
        OUString sTxt = pTxtNd->GetTxt();
        while ( nSttPos < sTxt.getLength() && ( sTxt[nSttPos]==' ' || sTxt[nSttPos]=='\t' ) )
            ++nSttPos;

        // if there is a calculation field at position 1, get the value of it
        const bool bOK = nSttPos<sTxt.getLength();
        const sal_Unicode Char = bOK ? sTxt[nSttPos] : 0;
        if ( bOK && (Char==CH_TXTATR_BREAKWORD || Char==CH_TXTATR_INWORD) )
        {
            SwIndex aIdx( pTxtNd, static_cast<xub_StrLen>(nSttPos) );
            SwTxtFld * const pTxtFld = static_cast<SwTxtFld*>(
                pTxtNd->GetTxtAttrForCharAt(aIdx.GetIndex(), RES_TXTATR_FIELD));
            if( !pTxtFld )
                break;

            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status

            const SwField* pFld = pTxtFld->GetFld().GetFld();
            switch( pFld->GetTyp()->Which()  )
            {
            case RES_SETEXPFLD:
                nRet = ((SwSetExpField*)pFld)->GetValue();
                break;
            case RES_USERFLD:
                nRet = ((SwUserFieldType*)pFld)->GetValue();
                break;
            case RES_TABLEFLD:
                {
                    SwTblField* pTblFld = (SwTblField*)pFld;
                    if( !pTblFld->IsValid() )
                    {
                        // use the right table!
                        const SwTable* pTmp = rCalcPara.pTbl;
                        rCalcPara.pTbl = &pTxtNd->FindTableNode()->GetTable();
                        pTblFld->CalcField( rCalcPara );
                        rCalcPara.pTbl = pTmp;
                    }
                    nRet = pTblFld->GetValue();
                }
                break;

            case RES_DATETIMEFLD:
                nRet = ((SwDateTimeField*)pFld)->GetValue();
                break;

            case RES_JUMPEDITFLD:
                //JP 14.09.98: Bug 56112 - placeholder never have the right content!
                nRet = 0;
                break;

            default:
                nRet = rCalcPara.rCalc.Calculate( pFld->ExpandField(true) ).GetDouble();
            }
        }
        else
        {
            // result is 0 but no error!
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status

            double aNum = 0.0;
            sTxt = bOK ? sTxt.copy( nSttPos ) : OUString();
            sal_uInt32 nFmtIndex = GetFrmFmt()->GetTblBoxNumFmt().GetValue();

            SvNumberFormatter* pNumFmtr = pDoc->GetNumberFormatter();

            if( NUMBERFORMAT_TEXT == nFmtIndex )
                nFmtIndex = 0;
            // JP 22.04.98: Bug 49659 - special treatment for percentages
            else if( !sTxt.isEmpty() &&
                    NUMBERFORMAT_PERCENT == pNumFmtr->GetType( nFmtIndex ))
            {
                sal_uInt32 nTmpFmt = 0;
                if( pNumFmtr->IsNumberFormat( sTxt, nTmpFmt, aNum ) &&
                    NUMBERFORMAT_NUMBER == pNumFmtr->GetType( nTmpFmt ))
                    sTxt += OUString('%');
            }

            if( pNumFmtr->IsNumberFormat( sTxt, nFmtIndex, aNum ))
                nRet = aNum;
        }
        // ?? otherwise it is an error
    } while( false );

    if( !rCalcPara.IsStackOverFlow() )
    {
        rCalcPara.pBoxStk->erase( pBox );      // remove from stack
        rCalcPara.DecStackCnt();
    }

    //JP 12.01.99: error detection, Bug 60794
    if( DBL_MAX == nRet )
        rCalcPara.rCalc.SetCalcError( CALC_SYNTAX ); // set error

    return nRet;
}

// structure needed for calculation of tables

SwTblCalcPara::SwTblCalcPara( SwCalc& rCalculator, const SwTable& rTable )
    : pLastTblBox( 0 ), nStackCnt( 0 ), nMaxSize( cMAXSTACKSIZE ),
    rCalc( rCalculator ), pTbl( &rTable )
{
    pBoxStk = new SwTableSortBoxes;
}

SwTblCalcPara::~SwTblCalcPara()
{
    delete pBoxStk;
}

sal_Bool SwTblCalcPara::CalcWithStackOverflow()
{
    // If a stack overflow was detected, redo with last box.
    sal_uInt16 nSaveMaxSize = nMaxSize;

    nMaxSize = cMAXSTACKSIZE - 5;
    sal_uInt16 nCnt = 0;
    SwTableBoxes aStackOverFlows;
    do {
        SwTableBox* pBox = (SwTableBox*)pLastTblBox;
        nStackCnt = 0;
        rCalc.SetCalcError( CALC_NOERR );
        aStackOverFlows.insert( aStackOverFlows.begin() + nCnt++, pBox );

        pBoxStk->erase( pBox );
        pBox->GetValue( *this );
    } while( IsStackOverFlow() );

    nMaxSize = cMAXSTACKSIZE - 3; // decrease at least one level

    // if recursion was detected
    nStackCnt = 0;
    rCalc.SetCalcError( CALC_NOERR );
    pBoxStk->clear();

    while( !rCalc.IsCalcError() && nCnt )
    {
        aStackOverFlows[ --nCnt ]->GetValue( *this );
        if( IsStackOverFlow() && !CalcWithStackOverflow() )
            break;
    }

    nMaxSize = nSaveMaxSize;
    aStackOverFlows.clear();
    return !rCalc.IsCalcError();
}

SwTableFormula::SwTableFormula( const OUString& rFormula )
: m_sFormula( rFormula )
, m_eNmType( EXTRNL_NAME )
, m_bValidValue( false )
{
}

SwTableFormula::~SwTableFormula()
{
}

void SwTableFormula::_MakeFormula( const SwTable& rTbl, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwTblCalcPara* pCalcPara = (SwTblCalcPara*)pPara;
    if( pCalcPara->rCalc.IsCalcError() )        // stop if there is already an error set
        return;

    SwTableBox *pEndBox = 0;

    rFirstBox = rFirstBox.copy(1); // erase label of this box
    // a region in this area?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));

        // Is it actually a valid pointer?
        if( rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() )
            pEndBox = 0;
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }
    SwTableBox* pSttBox = reinterpret_cast<SwTableBox*>(
                            sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() )
        pSttBox = 0;

    rNewStr += " ";
    if( pEndBox && pSttBox )    // area?
    {
        // get all selected boxes via layout and calculate their values
        SwSelBoxes aBoxes;
        GetBoxes( *pSttBox, *pEndBox, aBoxes );

        rNewStr += "(";
        bool bDelim = false;
        for (size_t n = 0; n < aBoxes.size() &&
                           !pCalcPara->rCalc.IsCalcError(); ++n)
        {
            const SwTableBox* pTblBox = aBoxes[n];
            if ( pTblBox->getRowSpan() >= 1 )
            {
                if( bDelim )
                    rNewStr += OUString(cListDelim);
                bDelim = true;
                rNewStr += pCalcPara->rCalc.GetStrResult(
                            pTblBox->GetValue( *pCalcPara ), sal_False );
            }
        }
        rNewStr += ")";
    }
    else if( pSttBox && !pLastBox ) // only the StartBox?
    {
        // JP 12.01.99: and no EndBox in the formula!
        // calculate the value of the box
        if ( pSttBox->getRowSpan() >= 1 )
        {
            rNewStr += pCalcPara->rCalc.GetStrResult(
                            pSttBox->GetValue( *pCalcPara ), sal_False );
        }
    }
    else
        pCalcPara->rCalc.SetCalcError( CALC_SYNTAX );   // set error
    rNewStr += " ";
}

void SwTableFormula::RelNmsToBoxNms( const SwTable& rTbl, OUString& rNewStr,
            OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // relative name w.r.t. box name (external presentation)
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Feld steht in keinem TextNode" );
    const SwTableBox *pBox = (SwTableBox *)rTbl.GetTblBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        const SwTableBox *pRelLastBox = lcl_RelToBox( rTbl, pBox, *pLastBox );
        if ( pRelLastBox )
            rNewStr += pRelLastBox->GetName();
        else
            rNewStr += "A1";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    const SwTableBox *pRelFirstBox = lcl_RelToBox( rTbl, pBox, rFirstBox );

    if (pRelFirstBox)
        rNewStr += pRelFirstBox->GetName();
    else
        rNewStr += "A1";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::RelBoxNmsToPtr( const SwTable& rTbl, OUString& rNewStr,
            OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // relative name w.r.t. box name (internal presentation)
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Field not placed in any Node" );
    const SwTableBox *pBox = (SwTableBox*)rTbl.GetTblBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        const SwTableBox *pRelLastBox = lcl_RelToBox( rTbl, pBox, *pLastBox );
        if ( pRelLastBox )
            rNewStr += OUString::number((sal_PtrDiff)pRelLastBox);
        else
            rNewStr += "0";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    const SwTableBox *pRelFirstBox = lcl_RelToBox( rTbl, pBox, rFirstBox );
    if ( pRelFirstBox )
        rNewStr += OUString::number((sal_PtrDiff)pRelFirstBox);
    else
        rNewStr += "0";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::BoxNmsToRelNm( const SwTable& rTbl, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // box name (external presentation) w.r.t. relative name
    SwNode* pNd = (SwNode*)pPara;
    OSL_ENSURE( pNd, "Field not placed in any Node" );
    const SwTableNode* pTblNd = pNd->FindTableNode();

    OUString sRefBoxNm;
    if( &pTblNd->GetTable() == &rTbl )
    {
        const SwTableBox *pBox = rTbl.GetTblBox(
                pNd->FindTableBoxStartNode()->GetIndex() );
        OSL_ENSURE( pBox, "Field not placed in any Table" );
        sRefBoxNm = pBox->GetName();
    }

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        rNewStr += lcl_BoxNmToRel( rTbl, *pTblNd, sRefBoxNm, *pLastBox,
                                m_eNmType == EXTRNL_NAME );
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    rNewStr += lcl_BoxNmToRel( rTbl, *pTblNd, sRefBoxNm, rFirstBox,
                            m_eNmType == EXTRNL_NAME );

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}


void SwTableFormula::PtrToBoxNms( const SwTable& rTbl, OUString& rNewStr,
                        OUString& rFirstBox, OUString* pLastBox, void* ) const
{
    // area in these parentheses?
    SwTableBox* pBox;

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));

        // Is it actually a valid pointer?
        if( rTbl.GetTabSortBoxes().find( pBox ) != rTbl.GetTabSortBoxes().end() )
            rNewStr += pBox->GetName();
        else
            rNewStr += "?";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( rTbl.GetTabSortBoxes().find( pBox ) != rTbl.GetTabSortBoxes().end() )
        rNewStr += pBox->GetName();
    else
        rNewStr += "?";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::BoxNmsToPtr( const SwTable& rTbl, OUString& rNewStr,
                        OUString& rFirstBox, OUString* pLastBox, void* ) const
{
    // area in these parentheses?
    const SwTableBox* pBox;

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        pBox = rTbl.GetTblBox( *pLastBox );
        rNewStr += OUString::number((sal_PtrDiff)pBox)
                +  ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    pBox = rTbl.GetTblBox( rFirstBox );
    rNewStr += OUString::number((sal_PtrDiff)pBox)
            +  OUString(rFirstBox[ rFirstBox.getLength()-1 ]); // get label for the box
}

/// create external formula (for UI)
void SwTableFormula::PtrToBoxNm( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormula fnFormula = 0;
    switch (m_eNmType)
    {
    case INTRNL_NAME:
        if( pTbl )
            fnFormula = &SwTableFormula::PtrToBoxNms;
        break;
    case REL_NAME:
        if( pTbl )
        {
            fnFormula = &SwTableFormula::RelNmsToBoxNms;
            pNd = GetNodeOfFormula();
        }
        break;
    case EXTRNL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTbl, (void*)pNd );
    m_eNmType = EXTRNL_NAME;
}

/// create internal formula (in CORE)
void SwTableFormula::BoxNmToPtr( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormula fnFormula = 0;
    switch (m_eNmType)
    {
    case EXTRNL_NAME:
        if( pTbl )
            fnFormula = &SwTableFormula::BoxNmsToPtr;
        break;
    case REL_NAME:
        if( pTbl )
        {
            fnFormula = &SwTableFormula::RelBoxNmsToPtr;
            pNd = GetNodeOfFormula();
        }
        break;
    case INTRNL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTbl, (void*)pNd );
    m_eNmType = INTRNL_NAME;
}

/// create relative formula (for copy)
void SwTableFormula::ToRelBoxNm( const SwTable* pTbl )
{
    const SwNode* pNd = 0;
    FnScanFormula fnFormula = 0;
    switch (m_eNmType)
    {
    case INTRNL_NAME:
    case EXTRNL_NAME:
        if( pTbl )
        {
            fnFormula = &SwTableFormula::BoxNmsToRelNm;
            pNd = GetNodeOfFormula();
        }
        break;
    case REL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTbl, (void*)pNd );
    m_eNmType = REL_NAME;
}

OUString SwTableFormula::ScanString( FnScanFormula fnFormula, const SwTable& rTbl,
                                    void* pPara ) const
{
    OUString aStr;
    sal_Int32 nFml = 0;
    sal_Int32 nEnd = 0;

    do {
        // If the formula is preceded by a name, use this table!
        const SwTable* pTbl = &rTbl;

        sal_Int32 nStt = m_sFormula.indexOf( '<', nFml );
        if ( nStt>=0 )
        {
            while ( nStt>=0 )
            {
                const sal_Int32 nNxt = nStt+1;
                if (nNxt>=m_sFormula.getLength())
                {
                    nStt = -1;
                    break;
                }
                if ( m_sFormula[nNxt]!=' ' && m_sFormula[nNxt]!='=' )
                    break;
                nStt = m_sFormula.indexOf( '<', nNxt );
            }

            if ( nStt>=0 )
                // Start searching from current position, which is valid for sure
                nEnd = m_sFormula.indexOf( '>', nStt );
        }
        if (nStt<0 || nEnd<0 )
        {
            // set the rest and finish
            aStr += m_sFormula.copy(nFml);
            break;
        }

        // write beginning
        aStr += m_sFormula.copy(nFml, nStt - nFml);

        if (fnFormula)
        {
            sal_Int32 nSeparator = 0;
            // Is a table name preceded?
            // JP 16.02.99: SplitMergeBoxNm take care of the name themself
            // JP 22.02.99: Linux compiler needs cast
            // JP 28.06.99: rel. BoxName has no preceding tablename!
            if( fnFormula != (FnScanFormula)&SwTableFormula::_SplitMergeBoxNm &&
                m_sFormula.getLength()>1 && cRelIdentifier != m_sFormula[1] &&
                (nSeparator = m_sFormula.indexOf( '.', nStt ))>=0
                && nSeparator < nEnd )
            {
                OUString sTblNm( m_sFormula.copy( nStt, nEnd - nStt ));

                // If there are dots in the name, then they appear in pairs (e.g. A1.1.1)!
                if( (comphelper::string::getTokenCount(sTblNm, '.') - 1) & 1 )
                {
                    sTblNm = sTblNm.copy( 0, nSeparator - nStt );

                    // when creating a formula the table name is unwanted
                    if( fnFormula != (FnScanFormula)&SwTableFormula::_MakeFormula )
                        aStr += sTblNm;
                    nStt = nSeparator;

                    sTblNm = sTblNm.copy( 1 );   // delete separator
                    if( sTblNm != rTbl.GetFrmFmt()->GetName() )
                    {
                        // then search for table
                        const SwTable* pFnd = FindTable(
                                                *rTbl.GetFrmFmt()->GetDoc(),
                                                sTblNm );
                        if( pFnd )
                            pTbl = pFnd;
                        // ??
                        OSL_ENSURE( pFnd, "No table found. What now?" );
                    }
                }
            }

            OUString sBox( m_sFormula.copy( nStt, nEnd - nStt + 1 ));
            // area in these parentheses?
            nSeparator = m_sFormula.indexOf( ':', nStt );
            if ( nSeparator>=0 && nSeparator<nEnd )
            {
                // without opening parenthesis
                OUString aFirstBox( m_sFormula.copy( nStt+1, nSeparator - nStt - 1 ));
                (this->*fnFormula)( *pTbl, aStr, sBox, &aFirstBox, pPara );
            }
            else
                (this->*fnFormula)( *pTbl, aStr, sBox, 0, pPara );
        }

        nFml = nEnd+1;
    } while( true );
    return aStr;
}

const SwTable* SwTableFormula::FindTable( SwDoc& rDoc, const OUString& rNm ) const
{
    const SwFrmFmts& rTblFmts = *rDoc.GetTblFrmFmts();
    const SwTable* pTmpTbl = 0, *pRet = 0;
    for( sal_uInt16 nFmtCnt = rTblFmts.size(); nFmtCnt; )
    {
        SwFrmFmt* pFmt = rTblFmts[ --nFmtCnt ];
        // if we are called from Sw3Writer, a number is dependent on the format name
        SwTableBox* pFBox;
        if ( rNm.startsWith(pFmt->GetName().getToken(0, 0x0a)) &&
            0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
            0 != (pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            // a table in the normal NodesArr
            pRet = pTmpTbl;
            break;
        }
    }
    return pRet;
}

static const SwFrm* lcl_GetBoxFrm( const SwTableBox& rBox )
{
    SwNodeIndex aIdx( *rBox.GetSttNd() );
    SwCntntNode* pCNd = aIdx.GetNodes().GoNext( &aIdx );
    OSL_ENSURE( pCNd, "Box has no TextNode" );
    Point aPt;      // get the first frame of the layout - table headline
    return pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout(), &aPt, NULL, sal_False );
}

static sal_Int32 lcl_GetLongBoxNum( OUString& rStr )
{
    sal_Int32 nRet;
    const sal_Int32 nPos = rStr.indexOf( cRelSeparator );
    if ( nPos<0 )
    {
        nRet = rStr.toInt32();
        rStr = OUString();
    }
    else
    {
        nRet = rStr.copy( 0, nPos ).toInt32();
        rStr = rStr.copy( nPos+1 );
    }
    return nRet;
}

static const SwTableBox* lcl_RelToBox( const SwTable& rTbl,
                                    const SwTableBox* pRefBox,
                                    OUString sGetName )
{
    // get line
    const SwTableBox* pBox = 0;

    // Is it really a relative value?
    if ( cRelIdentifier == sGetName[0] ) // yes
    {
        if( !pRefBox )
            return 0;

        sGetName = sGetName.copy( 1 );

        const SwTableLines* pLines = (SwTableLines*)&rTbl.GetTabLines();
        const SwTableBoxes* pBoxes;
        const SwTableLine* pLine;

        // determine starting values of the box,...
        pBox = (SwTableBox*)pRefBox;
        pLine = pBox->GetUpper();
        while( pLine->GetUpper() )
        {
            pBox = pLine->GetUpper();
            pLine = pBox->GetUpper();
        }
        sal_uInt16 nSttBox = pLine->GetTabBoxes().GetPos( pBox );
        sal_uInt16 nSttLine = rTbl.GetTabLines().GetPos( pLine );

        const sal_Int32 nBoxOffset = lcl_GetLongBoxNum( sGetName ) + nSttBox;
        const sal_Int32 nLineOffset = lcl_GetLongBoxNum( sGetName ) + nSttLine;

        if( nBoxOffset < 0 || nBoxOffset >= USHRT_MAX ||
            nLineOffset < 0 || nLineOffset >= USHRT_MAX )
            return 0;

        if( static_cast<size_t>(nLineOffset) >= pLines->size() )
            return 0;

        pLine = (*pLines)[ nLineOffset ];

        // ... then search the box
        pBoxes = &pLine->GetTabBoxes();
        if( static_cast<size_t>(nBoxOffset) >= pBoxes->size() )
            return 0;
        pBox = (*pBoxes)[ nBoxOffset ];

        while (!sGetName.isEmpty())
        {
            nSttBox = SwTable::_GetBoxNum( sGetName );
            pLines = &pBox->GetTabLines();
            if( nSttBox )
                --nSttBox;

            nSttLine = SwTable::_GetBoxNum( sGetName );

            // determine line
            if( !nSttLine || nSttLine > pLines->size() )
                break;
            pLine = (*pLines)[ nSttLine-1 ];

            // determine box
            pBoxes = &pLine->GetTabBoxes();
            if( nSttBox >= pBoxes->size() )
                break;
            pBox = (*pBoxes)[ nSttBox ];
        }

        if( pBox )
        {
            if( !pBox->GetSttNd() )
                // "bubble up" to first box
                while( !pBox->GetTabLines().empty() )
                    pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
        }
    }
    else
    {
        // otherwise it is an absolute external presentation
        pBox = rTbl.GetTblBox( sGetName );
    }
    return pBox;
}

static OUString lcl_BoxNmToRel( const SwTable& rTbl, const SwTableNode& rTblNd,
                                OUString sRefBoxNm, OUString sTmp, bool bExtrnlNm )
{
    if( !bExtrnlNm )
    {
        // convert into external presentation
        SwTableBox* pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(sTmp.toInt64()));
        if( rTbl.GetTabSortBoxes().find( pBox ) == rTbl.GetTabSortBoxes().end() )
            return OUString('?');
        sTmp = pBox->GetName();
    }

    // If the formula is spanning over a table then keep external presentation
    if( &rTbl == &rTblNd.GetTable() )
    {
        long nBox = SwTable::_GetBoxNum( sTmp, sal_True );
        nBox -= SwTable::_GetBoxNum( sRefBoxNm, sal_True );
        long nLine = SwTable::_GetBoxNum( sTmp );
        nLine -= SwTable::_GetBoxNum( sRefBoxNm );

        const OUString sCpy = sTmp;        //JP 01.11.95: add rest from box name

        sTmp = OUString(cRelIdentifier) + OUString::number( nBox )
             + OUString(cRelSeparator) + OUString::number( nLine );

        if (!sCpy.isEmpty())
        {
            sTmp += OUString(cRelSeparator) + sCpy;
        }
    }

    if (sTmp.endsWith(">"))
        return sTmp.copy(0, sTmp.getLength()-1 );

    return sTmp;
}

void SwTableFormula::GetBoxesOfFormula( const SwTable& rTbl,
                                        SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    BoxNmToPtr( &rTbl );
    ScanString( &SwTableFormula::_GetFmlBoxes, rTbl, &rBoxes );
}

void SwTableFormula::_GetFmlBoxes( const SwTable& rTbl, OUString& ,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwSelBoxes* pBoxes = (SwSelBoxes*)pPara;
    SwTableBox* pEndBox = 0;

    rFirstBox = rFirstBox.copy(1); // delete box label
    // area in these parentheses?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));

        // Is it actually a valid pointer?
        if( rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() )
            pEndBox = 0;
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    SwTableBox *pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( !pSttBox || rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() )
        return;

    if ( pEndBox ) // area?
    {
        // get all selected boxes via layout and calculate their values
        SwSelBoxes aBoxes;
        GetBoxes( *pSttBox, *pEndBox, aBoxes );
        pBoxes->insert( aBoxes );
    }
    else          // only the StartBox?
        pBoxes->insert( pSttBox );
}

void SwTableFormula::GetBoxes( const SwTableBox& rSttBox,
                                const SwTableBox& rEndBox,
                                SwSelBoxes& rBoxes ) const
{
    // get all selected boxes via layout
    const SwLayoutFrm *pStt, *pEnd;
    const SwFrm* pFrm = lcl_GetBoxFrm( rSttBox );
    pStt = pFrm ? pFrm->GetUpper() : 0;
    pEnd = ( 0 != (pFrm = lcl_GetBoxFrm( rEndBox ))) ? pFrm->GetUpper() : 0;
    if( !pStt || !pEnd )
        return ;                        // no valid selection

    GetTblSel( pStt, pEnd, rBoxes, 0 );

    const SwTable* pTbl = pStt->FindTabFrm()->GetTable();

    // filter headline boxes
    if( pTbl->GetRowsToRepeat() > 0 )
    {
        do {    // middle-check loop
            const SwTableLine* pLine = rSttBox.GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTbl->IsHeadline( *pLine ) )
                break;      // headline in this area!

            // maybe start and end are swapped
            pLine = rEndBox.GetUpper();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTbl->IsHeadline( *pLine ) )
                break;      // headline in this area!

            const SwTabFrm *pTable = pStt->FindTabFrm();
            const SwTabFrm *pEndTable = pEnd->FindTabFrm();

            if( pTable == pEndTable ) // no split table
                break;

            // then remove table headers
            for (size_t n = 0; n < rBoxes.size(); ++n)
            {
                pLine = rBoxes[n]->GetUpper();
                while( pLine->GetUpper() )
                    pLine = pLine->GetUpper()->GetUpper();

                if( pTbl->IsHeadline( *pLine ) )
                    rBoxes.erase( rBoxes.begin() + n-- );
            }
        } while( false );
    }
}

/// Are all boxes valid that are referenced by the formula?
void SwTableFormula::_HasValidBoxes( const SwTable& rTbl, OUString& ,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    bool* pBValid = (bool*)pPara;
    if( *pBValid )      // wrong is wrong
    {
        SwTableBox* pSttBox = 0, *pEndBox = 0;
        rFirstBox = rFirstBox.copy(1);       // Kennung fuer Box loeschen

        // area in this parenthesis?
        if( pLastBox )
            rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );

        switch (m_eNmType)
        {
        case INTRNL_NAME:
            if( pLastBox )
                pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));
            pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
            break;

        case REL_NAME:
            {
                const SwNode* pNd = GetNodeOfFormula();
                const SwTableBox* pBox = !pNd ? 0
                                               : (SwTableBox *)rTbl.GetTblBox(
                                    pNd->FindTableBoxStartNode()->GetIndex() );
                if( pLastBox )
                    pEndBox = (SwTableBox*)lcl_RelToBox( rTbl, pBox, *pLastBox );
                pSttBox = (SwTableBox*)lcl_RelToBox( rTbl, pBox, rFirstBox );
            }
            break;

        case EXTRNL_NAME:
            if( pLastBox )
                pEndBox = (SwTableBox*)rTbl.GetTblBox( *pLastBox );
            pSttBox = (SwTableBox*)rTbl.GetTblBox( rFirstBox );
            break;
        }

        // Are these valid pointers?
        if( ( pLastBox &&
              ( !pEndBox || rTbl.GetTabSortBoxes().find( pEndBox ) == rTbl.GetTabSortBoxes().end() ) ) ||
            ( !pSttBox || rTbl.GetTabSortBoxes().find( pSttBox ) == rTbl.GetTabSortBoxes().end() ) )
                *pBValid = false;
    }
}

bool SwTableFormula::HasValidBoxes() const
{
    bool bRet = true;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ) )
        ScanString( &SwTableFormula::_HasValidBoxes,
                        ((SwTableNode*)pNd)->GetTable(), &bRet );
    return bRet;
}

sal_uInt16 SwTableFormula::GetLnPosInTbl( const SwTable& rTbl, const SwTableBox* pBox )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( pBox )
    {
        const SwTableLine* pLn = pBox->GetUpper();
        while( pLn->GetUpper() )
            pLn = pLn->GetUpper()->GetUpper();
        nRet = rTbl.GetTabLines().GetPos( pLn );
    }
    return nRet;
}

void SwTableFormula::_SplitMergeBoxNm( const SwTable& rTbl, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwTableFmlUpdate& rTblUpd = *(SwTableFmlUpdate*)pPara;

    rNewStr += OUString(rFirstBox[0]);     // get label for the box
    rFirstBox = rFirstBox.copy(1);

    OUString sTblNm;
    const SwTable* pTbl = &rTbl;

    OUString* pTblNmBox = pLastBox ? pLastBox : &rFirstBox;

    const sal_Int32 nLastBoxLen = pTblNmBox->getLength();
    const sal_Int32 nSeparator = pTblNmBox->indexOf('.');
    if ( nSeparator>=0 &&
        // If there are dots in the name, than these appear in pairs (e.g. A1.1.1)!
        (comphelper::string::getTokenCount(*pTblNmBox, '.') - 1) & 1 )
    {
        sTblNm = pTblNmBox->copy( 0, nSeparator );
        *pTblNmBox = pTblNmBox->copy( nSeparator + 1); // remove dot
        const SwTable* pFnd = FindTable( *rTbl.GetFrmFmt()->GetDoc(), sTblNm );
        if( pFnd )
            pTbl = pFnd;

        if( TBL_MERGETBL == rTblUpd.eFlags )
        {
            if( pFnd )
            {
                if( pFnd == rTblUpd.DATA.pDelTbl )
                {
                    if( rTblUpd.pTbl != &rTbl ) // not the current one
                        rNewStr += rTblUpd.pTbl->GetFrmFmt()->GetName() + "."; // set new table name
                    rTblUpd.bModified = sal_True;
                }
                else if( pFnd != rTblUpd.pTbl ||
                    ( rTblUpd.pTbl != &rTbl && &rTbl != rTblUpd.DATA.pDelTbl))
                    rNewStr += sTblNm + "."; // keep table name
                else
                    rTblUpd.bModified = sal_True;
            }
            else
                rNewStr += sTblNm + ".";     // keep table name
        }
    }
    if( pTblNmBox == pLastBox )
        rFirstBox = rFirstBox.copy( nLastBoxLen + 1 );

    SwTableBox* pSttBox = 0, *pEndBox = 0;
    switch (m_eNmType)
    {
    case INTRNL_NAME:
        if( pLastBox )
            pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));
        pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
        break;

    case REL_NAME:
        {
            const SwNode* pNd = GetNodeOfFormula();
            const SwTableBox* pBox = pNd ? pTbl->GetTblBox(
                            pNd->FindTableBoxStartNode()->GetIndex() ) : 0;
            if( pLastBox )
                pEndBox = (SwTableBox*)lcl_RelToBox( *pTbl, pBox, *pLastBox );
            pSttBox = (SwTableBox*)lcl_RelToBox( *pTbl, pBox, rFirstBox );
        }
        break;

    case EXTRNL_NAME:
        if( pLastBox )
            pEndBox = (SwTableBox*)pTbl->GetTblBox( *pLastBox );
        pSttBox = (SwTableBox*)pTbl->GetTblBox( rFirstBox );
        break;
    }

    if( pLastBox && pTbl->GetTabSortBoxes().find( pEndBox ) == pTbl->GetTabSortBoxes().end() )
        pEndBox = 0;
    if( pTbl->GetTabSortBoxes().find( pSttBox ) == pTbl->GetTabSortBoxes().end() )
        pSttBox = 0;

    if( TBL_SPLITTBL == rTblUpd.eFlags )
    {
        // Where are the boxes - in the old or in the new table?
        bool bInNewTbl = false;
        if( pLastBox )
        {
            // It is the "first" box in this selection. It determines if the formula is placed in
            // the new or the old table.
            sal_uInt16 nEndLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pEndBox ),
                    nSttLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pSttBox );

            if( USHRT_MAX != nSttLnPos && USHRT_MAX != nEndLnPos &&
                ((rTblUpd.nSplitLine <= nSttLnPos) ==
                (rTblUpd.nSplitLine <= nEndLnPos)) )
            {
                // stay in same table
                bInNewTbl = rTblUpd.nSplitLine <= nEndLnPos &&
                                    pTbl == rTblUpd.pTbl;
            }
            else
            {
                // this is definitely an invalid formula, also mark as modified for Undo
                rTblUpd.bModified = sal_True;
                if( pEndBox )
                    bInNewTbl = USHRT_MAX != nEndLnPos &&
                                    rTblUpd.nSplitLine <= nEndLnPos &&
                                    pTbl == rTblUpd.pTbl;
            }
        }
        else
        {
            sal_uInt16 nSttLnPos = SwTableFormula::GetLnPosInTbl( *pTbl, pSttBox );
            // Put it in the new table?
            bInNewTbl = USHRT_MAX != nSttLnPos &&
                            rTblUpd.nSplitLine <= nSttLnPos &&
                            pTbl == rTblUpd.pTbl;
        }

        // formula goes into new table
        if( rTblUpd.bBehindSplitLine )
        {
            if( !bInNewTbl )
            {
                rTblUpd.bModified = sal_True;
                rNewStr += rTblUpd.pTbl->GetFrmFmt()->GetName() + ".";
            }
            else if( !sTblNm.isEmpty() )
                rNewStr += sTblNm + ".";
        }
        else if( bInNewTbl )
        {
            rTblUpd.bModified = sal_True;
            rNewStr += *rTblUpd.DATA.pNewTblNm + ".";
        }
        else if( !sTblNm.isEmpty() )
            rNewStr += sTblNm + ".";
    }

    if( pLastBox )
        rNewStr += OUString::number((sal_PtrDiff)pEndBox) + ":";

    rNewStr += OUString::number((sal_PtrDiff)pSttBox)
            +  OUString(rFirstBox[ rFirstBox.getLength()-1] );
}

/// Create external formula but remember that the formula is placed in a split/merged table
void SwTableFormula::ToSplitMergeBoxNm( SwTableFmlUpdate& rTblUpd )
{
    const SwTable* pTbl;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && 0 != ( pNd = pNd->FindTableNode() ))
        pTbl = &((SwTableNode*)pNd)->GetTable();
    else
        pTbl = rTblUpd.pTbl;

    m_sFormula = ScanString( &SwTableFormula::_SplitMergeBoxNm, *pTbl, (void*)&rTblUpd );
    m_eNmType = INTRNL_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
