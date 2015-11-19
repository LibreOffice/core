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
#include <IDocumentLayoutAccess.hxx>
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
const sal_Unicode cRelIdentifier = '\x12'; // CTRL-R

enum
{
    cMAXSTACKSIZE = 50
};

}

static const SwFrame* lcl_GetBoxFrame( const SwTableBox& rBox );
static sal_Int32 lcl_GetLongBoxNum( OUString& rStr );
static const SwTableBox* lcl_RelToBox( const SwTable& rTable,
                                       const SwTableBox* pRefBox,
                                       const OUString& sGetName);
static OUString lcl_BoxNmToRel( const SwTable& rTable,
                                const SwTableNode& rTableNd,
                                const OUString& sRefBoxNm,
                                const OUString& sGetStr,
                                bool bExtrnlNm);

/** Get value of this box.
 *
 * The value is comes from the first TextNode. If it starts with a number/
 * formula then calculate it, if it starts with a field then get the value.
 * All other conditions return 0 (and an error?).
 */
double SwTableBox::GetValue( SwTableCalcPara& rCalcPara ) const
{
    double nRet = 0;

    if( rCalcPara.rCalc.IsCalcError() )
        return nRet;            // stop if there is already an error set

    rCalcPara.rCalc.SetCalcError( CALC_SYNTAX );    // default: error

    // no content box?
    if( !m_pStartNode  )
        return nRet;

    if( rCalcPara.IncStackCnt() )
        return nRet;

    rCalcPara.SetLastTableBox( this );

    // Does it create a recursion?
    SwTableBox* pBox = const_cast<SwTableBox*>(this);
    if( rCalcPara.pBoxStack->find( pBox ) != rCalcPara.pBoxStack->end() )
        return nRet;            // already on the stack: error

    // re-start with this box
    rCalcPara.SetLastTableBox( this );

    rCalcPara.pBoxStack->insert( pBox );      // add
    do {        // Middle-Check-Loop, so that we can jump from here. Used so that the box pointer
                // will be removed from stack at the end.
        SwDoc* pDoc = GetFrameFormat()->GetDoc();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == GetFrameFormat()->GetItemState(
                                RES_BOXATR_FORMULA, false, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status
            if( !static_cast<const SwTableBoxFormula*>(pItem)->IsValid() )
            {
                // calculate
                const SwTable* pTmp = rCalcPara.pTable;
                rCalcPara.pTable = &pBox->GetSttNd()->FindTableNode()->GetTable();
                const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem))->Calc( rCalcPara, nRet );

                if( !rCalcPara.IsStackOverflow() )
                {
                    SwFrameFormat* pFormat = pBox->ClaimFrameFormat();
                    SfxItemSet aTmp( pDoc->GetAttrPool(),
                                        RES_BOXATR_BEGIN,RES_BOXATR_END-1 );
                    aTmp.Put( SwTableBoxValue( nRet ) );
                    if( SfxItemState::SET != pFormat->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTableBoxNumFormat( 0 ));
                    pFormat->SetFormatAttr( aTmp );
                }
                rCalcPara.pTable = pTmp;
            }
            else
                nRet = GetFrameFormat()->GetTableBoxValue().GetValue();
            break;
        }
        else if( SfxItemState::SET == pBox->GetFrameFormat()->GetItemState(
                                RES_BOXATR_VALUE, false, &pItem ) )
        {
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status
            nRet = static_cast<const SwTableBoxValue*>(pItem)->GetValue();
            break;
        }

        SwTextNode* pTextNd = pDoc->GetNodes()[ m_pStartNode->GetIndex() + 1 ]->GetTextNode();
        if( !pTextNd )
            break;

        sal_Int32 nSttPos = 0;
        OUString sText = pTextNd->GetText();
        while ( nSttPos < sText.getLength() && ( sText[nSttPos]==' ' || sText[nSttPos]=='\t' ) )
            ++nSttPos;

        // if there is a calculation field at position 1, get the value of it
        const bool bOK = nSttPos<sText.getLength();
        const sal_Unicode Char = bOK ? sText[nSttPos] : 0;
        if ( bOK && (Char==CH_TXTATR_BREAKWORD || Char==CH_TXTATR_INWORD) )
        {
            SwTextField * const pTextField =
                static_txtattr_cast<SwTextField*>(pTextNd->GetTextAttrForCharAt(nSttPos, RES_TXTATR_FIELD));
            if ( pTextField == nullptr )
                break;

            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status

            const SwField* pField = pTextField->GetFormatField().GetField();
            switch ( pField->GetTyp()->Which() )
            {
            case RES_SETEXPFLD:
                nRet = static_cast<const SwSetExpField*>(pField)->GetValue();
                break;
            case RES_USERFLD:
                nRet = static_cast<const SwUserField*>(pField)->GetValue();
                break;
            case RES_TABLEFLD:
                {
                    SwTableField* pTableField = const_cast<SwTableField*>(static_cast<const SwTableField*>(pField));
                    if( !pTableField->IsValid() )
                    {
                        // use the right table!
                        const SwTable* pTmp = rCalcPara.pTable;
                        rCalcPara.pTable = &pTextNd->FindTableNode()->GetTable();
                        pTableField->CalcField( rCalcPara );
                        rCalcPara.pTable = pTmp;
                    }
                    nRet = pTableField->GetValue();
                }
                break;

            case RES_DATETIMEFLD:
                nRet = static_cast<const SwDateTimeField*>( pField )->GetValue();
                break;

            case RES_JUMPEDITFLD:
                //JP 14.09.98: Bug 56112 - placeholder never have the right content!
                nRet = 0;
                break;

            default:
                nRet = rCalcPara.rCalc.Calculate( pField->ExpandField(true) ).GetDouble();
            }
        }
        else if ( nSttPos < sText.getLength()
                  && Char == CH_TXT_ATR_INPUTFIELDSTART )
        {
            const SwTextInputField * pTextInputField =
                dynamic_cast< const SwTextInputField* >(
                    pTextNd->GetTextAttrAt( nSttPos, RES_TXTATR_INPUTFIELD ) );
            if ( pTextInputField == nullptr )
                break;
            nRet = rCalcPara.rCalc.Calculate( pTextInputField->GetFieldContent() ).GetDouble();
        }
        else
        {
            // result is 0 but no error!
            rCalcPara.rCalc.SetCalcError( CALC_NOERR ); // reset status

            double aNum = 0.0;
            sText = bOK ? sText.copy( nSttPos ) : OUString();
            sal_uInt32 nFormatIndex = GetFrameFormat()->GetTableBoxNumFormat().GetValue();

            SvNumberFormatter* pNumFormatr = pDoc->GetNumberFormatter();

            if( static_cast<sal_uInt32>(css::util::NumberFormat::TEXT) == nFormatIndex )
                nFormatIndex = 0;
            // JP 22.04.98: Bug 49659 - special treatment for percentages
            else if( !sText.isEmpty() &&
                    css::util::NumberFormat::PERCENT == pNumFormatr->GetType( nFormatIndex ))
            {
                sal_uInt32 nTmpFormat = 0;
                if( pNumFormatr->IsNumberFormat( sText, nTmpFormat, aNum ) &&
                    css::util::NumberFormat::NUMBER == pNumFormatr->GetType( nTmpFormat ))
                    sText += "%";
            }

            if( pNumFormatr->IsNumberFormat( sText, nFormatIndex, aNum ))
                nRet = aNum;
        }
        // ?? otherwise it is an error
    } while( false );

    if( !rCalcPara.IsStackOverflow() )
    {
        rCalcPara.pBoxStack->erase( pBox );      // remove from stack
        rCalcPara.DecStackCnt();
    }

    //JP 12.01.99: error detection, Bug 60794
    if( DBL_MAX == nRet )
        rCalcPara.rCalc.SetCalcError( CALC_SYNTAX ); // set error

    return nRet;
}

// structure needed for calculation of tables

SwTableCalcPara::SwTableCalcPara( SwCalc& rCalculator, const SwTable& rTable )
    : pLastTableBox( nullptr ), nStackCnt( 0 ), nMaxSize( cMAXSTACKSIZE ),
    rCalc( rCalculator ), pTable( &rTable )
{
    pBoxStack = new SwTableSortBoxes;
}

SwTableCalcPara::~SwTableCalcPara()
{
    delete pBoxStack;
}

bool SwTableCalcPara::CalcWithStackOverflow()
{
    // If a stack overflow was detected, redo with last box.
    sal_uInt16 nSaveMaxSize = nMaxSize;

    nMaxSize = cMAXSTACKSIZE - 5;
    sal_uInt16 nCnt = 0;
    SwTableBoxes aStackOverflows;
    do {
        SwTableBox* pBox = const_cast<SwTableBox*>(pLastTableBox);
        nStackCnt = 0;
        rCalc.SetCalcError( CALC_NOERR );
        aStackOverflows.insert( aStackOverflows.begin() + nCnt++, pBox );

        pBoxStack->erase( pBox );
        pBox->GetValue( *this );
    } while( IsStackOverflow() );

    nMaxSize = cMAXSTACKSIZE - 3; // decrease at least one level

    // if recursion was detected
    nStackCnt = 0;
    rCalc.SetCalcError( CALC_NOERR );
    pBoxStack->clear();

    while( !rCalc.IsCalcError() && nCnt )
    {
        aStackOverflows[ --nCnt ]->GetValue( *this );
        if( IsStackOverflow() && !CalcWithStackOverflow() )
            break;
    }

    nMaxSize = nSaveMaxSize;
    aStackOverflows.clear();
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

void SwTableFormula::_MakeFormula( const SwTable& rTable, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwTableCalcPara* pCalcPara = static_cast<SwTableCalcPara*>(pPara);
    if( pCalcPara->rCalc.IsCalcError() )        // stop if there is already an error set
        return;

    SwTableBox *pEndBox = nullptr;

    rFirstBox = rFirstBox.copy(1); // erase label of this box
    // a region in this area?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));

        // Is it actually a valid pointer?
        if( rTable.GetTabSortBoxes().find( pEndBox ) == rTable.GetTabSortBoxes().end() )
            pEndBox = nullptr;
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }
    SwTableBox* pSttBox = reinterpret_cast<SwTableBox*>(
                            sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( rTable.GetTabSortBoxes().find( pSttBox ) == rTable.GetTabSortBoxes().end() )
        pSttBox = nullptr;

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
            const SwTableBox* pTableBox = aBoxes[n];
            if ( pTableBox->getRowSpan() >= 1 )
            {
                if( bDelim )
                    rNewStr += OUStringLiteral1<cListDelim>();
                bDelim = true;
                rNewStr += pCalcPara->rCalc.GetStrResult(
                            pTableBox->GetValue( *pCalcPara ), false );
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
            rNewStr += "(";
            rNewStr += pCalcPara->rCalc.GetStrResult(
                            pSttBox->GetValue( *pCalcPara ), false );
            rNewStr += ")";
        }
    }
    else
        pCalcPara->rCalc.SetCalcError( CALC_SYNTAX );   // set error
    rNewStr += " ";
}

void SwTableFormula::RelNmsToBoxNms( const SwTable& rTable, OUString& rNewStr,
            OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // relative name w.r.t. box name (external presentation)
    SwNode* pNd = static_cast<SwNode*>(pPara);
    OSL_ENSURE( pNd, "Feld steht in keinem TextNode" );
    const SwTableBox *pBox = rTable.GetTableBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        const SwTableBox *pRelLastBox = lcl_RelToBox( rTable, pBox, *pLastBox );
        if ( pRelLastBox )
            rNewStr += pRelLastBox->GetName();
        else
            rNewStr += "A1";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    const SwTableBox *pRelFirstBox = lcl_RelToBox( rTable, pBox, rFirstBox );

    if (pRelFirstBox)
        rNewStr += pRelFirstBox->GetName();
    else
        rNewStr += "A1";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::RelBoxNmsToPtr( const SwTable& rTable, OUString& rNewStr,
            OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // relative name w.r.t. box name (internal presentation)
    SwNode* pNd = static_cast<SwNode*>(pPara);
    OSL_ENSURE( pNd, "Field not placed in any Node" );
    const SwTableBox *pBox = rTable.GetTableBox(
                    pNd->FindTableBoxStartNode()->GetIndex() );

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        const SwTableBox *pRelLastBox = lcl_RelToBox( rTable, pBox, *pLastBox );
        if ( pRelLastBox )
            rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pRelLastBox));
        else
            rNewStr += "0";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    const SwTableBox *pRelFirstBox = lcl_RelToBox( rTable, pBox, rFirstBox );
    if ( pRelFirstBox )
        rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pRelFirstBox));
    else
        rNewStr += "0";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::BoxNmsToRelNm( const SwTable& rTable, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    // box name (external presentation) w.r.t. relative name
    SwNode* pNd = static_cast<SwNode*>(pPara);
    OSL_ENSURE( pNd, "Field not placed in any Node" );
    const SwTableNode* pTableNd = pNd->FindTableNode();

    OUString sRefBoxNm;
    if( &pTableNd->GetTable() == &rTable )
    {
        const SwTableBox *pBox = rTable.GetTableBox(
                pNd->FindTableBoxStartNode()->GetIndex() );
        OSL_ENSURE( pBox, "Field not placed in any Table" );
        sRefBoxNm = pBox->GetName();
    }

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        rNewStr += lcl_BoxNmToRel( rTable, *pTableNd, sRefBoxNm, *pLastBox,
                                m_eNmType == EXTRNL_NAME );
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    rNewStr += lcl_BoxNmToRel( rTable, *pTableNd, sRefBoxNm, rFirstBox,
                            m_eNmType == EXTRNL_NAME );

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::PtrToBoxNms( const SwTable& rTable, OUString& rNewStr,
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
        if( rTable.GetTabSortBoxes().find( pBox ) != rTable.GetTabSortBoxes().end() )
            rNewStr += pBox->GetName();
        else
            rNewStr += "?";
        rNewStr += ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( rTable.GetTabSortBoxes().find( pBox ) != rTable.GetTabSortBoxes().end() )
        rNewStr += pBox->GetName();
    else
        rNewStr += "?";

    // get label for the box
    rNewStr += OUString(rFirstBox[ rFirstBox.getLength()-1 ]);
}

void SwTableFormula::BoxNmsToPtr( const SwTable& rTable, OUString& rNewStr,
                        OUString& rFirstBox, OUString* pLastBox, void* ) const
{
    // area in these parentheses?
    const SwTableBox* pBox;

    rNewStr += OUString(rFirstBox[0]); // get label for the box
    rFirstBox = rFirstBox.copy(1);
    if( pLastBox )
    {
        pBox = rTable.GetTableBox( *pLastBox );
        rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pBox))
                +  ":";
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    pBox = rTable.GetTableBox( rFirstBox );
    rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pBox))
            +  OUString(rFirstBox[ rFirstBox.getLength()-1 ]); // get label for the box
}

/// create external formula (for UI)
void SwTableFormula::PtrToBoxNm( const SwTable* pTable )
{
    const SwNode* pNd = nullptr;
    FnScanFormula fnFormula = nullptr;
    switch (m_eNmType)
    {
    case INTRNL_NAME:
        if( pTable )
            fnFormula = &SwTableFormula::PtrToBoxNms;
        break;
    case REL_NAME:
        if( pTable )
        {
            fnFormula = &SwTableFormula::RelNmsToBoxNms;
            pNd = GetNodeOfFormula();
        }
        break;
    case EXTRNL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTable, const_cast<void*>(static_cast<void const *>(pNd)) );
    m_eNmType = EXTRNL_NAME;
}

/// create internal formula (in CORE)
void SwTableFormula::BoxNmToPtr( const SwTable* pTable )
{
    const SwNode* pNd = nullptr;
    FnScanFormula fnFormula = nullptr;
    switch (m_eNmType)
    {
    case EXTRNL_NAME:
        if( pTable )
            fnFormula = &SwTableFormula::BoxNmsToPtr;
        break;
    case REL_NAME:
        if( pTable )
        {
            fnFormula = &SwTableFormula::RelBoxNmsToPtr;
            pNd = GetNodeOfFormula();
        }
        break;
    case INTRNL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTable, const_cast<void*>(static_cast<void const *>(pNd)) );
    m_eNmType = INTRNL_NAME;
}

/// create relative formula (for copy)
void SwTableFormula::ToRelBoxNm( const SwTable* pTable )
{
    const SwNode* pNd = nullptr;
    FnScanFormula fnFormula = nullptr;
    switch (m_eNmType)
    {
    case INTRNL_NAME:
    case EXTRNL_NAME:
        if( pTable )
        {
            fnFormula = &SwTableFormula::BoxNmsToRelNm;
            pNd = GetNodeOfFormula();
        }
        break;
    case REL_NAME:
        return;
    }
    m_sFormula = ScanString( fnFormula, *pTable, const_cast<void*>(static_cast<void const *>(pNd)) );
    m_eNmType = REL_NAME;
}

OUString SwTableFormula::ScanString( FnScanFormula fnFormula, const SwTable& rTable,
                                    void* pPara ) const
{
    OUString aStr;
    sal_Int32 nFormula = 0;
    sal_Int32 nEnd = 0;

    do {
        // If the formula is preceded by a name, use this table!
        const SwTable* pTable = &rTable;

        sal_Int32 nStt = m_sFormula.indexOf( '<', nFormula );
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
            aStr += m_sFormula.copy(nFormula);
            break;
        }

        // write beginning
        aStr += m_sFormula.copy(nFormula, nStt - nFormula);

        if (fnFormula)
        {
            sal_Int32 nSeparator = 0;
            // Is a table name preceded?
            // JP 16.02.99: SplitMergeBoxNm take care of the name themself
            // JP 22.02.99: Linux compiler needs cast
            // JP 28.06.99: rel. BoxName has no preceding tablename!
            if( fnFormula != (FnScanFormula)&SwTableFormula::_SplitMergeBoxNm &&
                m_sFormula.getLength()>(nStt+1) && cRelIdentifier != m_sFormula[nStt+1] &&
                (nSeparator = m_sFormula.indexOf( '.', nStt ))>=0
                && nSeparator < nEnd )
            {
                OUString sTableNm( m_sFormula.copy( nStt, nEnd - nStt ));

                // If there are dots in the name, then they appear in pairs (e.g. A1.1.1)!
                if( (comphelper::string::getTokenCount(sTableNm, '.') - 1) & 1 )
                {
                    sTableNm = sTableNm.copy( 0, nSeparator - nStt );

                    // when creating a formula the table name is unwanted
                    if( fnFormula != (FnScanFormula)&SwTableFormula::_MakeFormula )
                        aStr += sTableNm;
                    nStt = nSeparator;

                    sTableNm = sTableNm.copy( 1 );   // delete separator
                    if( sTableNm != rTable.GetFrameFormat()->GetName() )
                    {
                        // then search for table
                        const SwTable* pFnd = FindTable(
                                                *rTable.GetFrameFormat()->GetDoc(),
                                                sTableNm );
                        if( pFnd )
                            pTable = pFnd;
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
                (this->*fnFormula)( *pTable, aStr, sBox, &aFirstBox, pPara );
            }
            else
                (this->*fnFormula)( *pTable, aStr, sBox, nullptr, pPara );
        }

        nFormula = nEnd+1;
    } while( true );
    return aStr;
}

const SwTable* SwTableFormula::FindTable( SwDoc& rDoc, const OUString& rNm )
{
    const SwFrameFormats& rTableFormats = *rDoc.GetTableFrameFormats();
    const SwTable* pTmpTable = nullptr, *pRet = nullptr;
    for( auto nFormatCnt = rTableFormats.size(); nFormatCnt; )
    {
        SwFrameFormat* pFormat = rTableFormats[ --nFormatCnt ];
        // if we are called from Sw3Writer, a number is dependent on the format name
        SwTableBox* pFBox;
        if ( rNm.equals(pFormat->GetName().getToken(0, 0x0a)) &&
            nullptr != ( pTmpTable = SwTable::FindTable( pFormat ) ) &&
            nullptr != (pFBox = pTmpTable->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            pFBox->GetSttNd()->GetNodes().IsDocNodes() )
        {
            // a table in the normal NodesArr
            pRet = pTmpTable;
            break;
        }
    }
    return pRet;
}

static const SwFrame* lcl_GetBoxFrame( const SwTableBox& rBox )
{
    SwNodeIndex aIdx( *rBox.GetSttNd() );
    SwContentNode* pCNd = aIdx.GetNodes().GoNext( &aIdx );
    OSL_ENSURE( pCNd, "Box has no TextNode" );
    Point aPt;      // get the first frame of the layout - table headline
    return pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false );
}

static sal_Int32 lcl_GetLongBoxNum( OUString& rStr )
{
    sal_Int32 nRet;
    const sal_Int32 nPos = rStr.indexOf( cRelSeparator );
    if ( nPos<0 )
    {
        nRet = rStr.toInt32();
        rStr.clear();
    }
    else
    {
        nRet = rStr.copy( 0, nPos ).toInt32();
        rStr = rStr.copy( nPos+1 );
    }
    return nRet;
}

static const SwTableBox* lcl_RelToBox( const SwTable& rTable,
                                    const SwTableBox* pRefBox,
                                    const OUString& _sGetName )
{
    // get line
    const SwTableBox* pBox = nullptr;
    OUString sGetName = _sGetName;

    // Is it really a relative value?
    if ( cRelIdentifier == sGetName[0] ) // yes
    {
        if( !pRefBox )
            return nullptr;

        sGetName = sGetName.copy( 1 );

        const SwTableLines* pLines = &rTable.GetTabLines();
        const SwTableBoxes* pBoxes;
        const SwTableLine* pLine;

        // determine starting values of the box,...
        pBox = pRefBox;
        pLine = pBox->GetUpper();
        while( pLine->GetUpper() )
        {
            pBox = pLine->GetUpper();
            pLine = pBox->GetUpper();
        }
        sal_uInt16 nSttBox = pLine->GetTabBoxes().GetPos( pBox );
        sal_uInt16 nSttLine = rTable.GetTabLines().GetPos( pLine );

        const sal_Int32 nBoxOffset = lcl_GetLongBoxNum( sGetName ) + nSttBox;
        const sal_Int32 nLineOffset = lcl_GetLongBoxNum( sGetName ) + nSttLine;

        if( nBoxOffset < 0 || nBoxOffset >= USHRT_MAX ||
            nLineOffset < 0 || nLineOffset >= USHRT_MAX )
            return nullptr;

        if( static_cast<size_t>(nLineOffset) >= pLines->size() )
            return nullptr;

        pLine = (*pLines)[ nLineOffset ];

        // ... then search the box
        pBoxes = &pLine->GetTabBoxes();
        if( static_cast<size_t>(nBoxOffset) >= pBoxes->size() )
            return nullptr;
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
        pBox = rTable.GetTableBox( sGetName );
    }
    return pBox;
}

static OUString lcl_BoxNmToRel( const SwTable& rTable, const SwTableNode& rTableNd,
                                const OUString& _sRefBoxNm, const OUString& _sTmp, bool bExtrnlNm )
{
    OUString sTmp = _sTmp;
    OUString sRefBoxNm = _sRefBoxNm;
    if( !bExtrnlNm )
    {
        // convert into external presentation
        SwTableBox* pBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(sTmp.toInt64()));
        if( rTable.GetTabSortBoxes().find( pBox ) == rTable.GetTabSortBoxes().end() )
            return OUString('?');
        sTmp = pBox->GetName();
    }

    // If the formula is spanning over a table then keep external presentation
    if( &rTable == &rTableNd.GetTable() )
    {
        long nBox = SwTable::_GetBoxNum( sTmp, true );
        nBox -= SwTable::_GetBoxNum( sRefBoxNm, true );
        long nLine = SwTable::_GetBoxNum( sTmp );
        nLine -= SwTable::_GetBoxNum( sRefBoxNm );

        const OUString sCpy = sTmp;        //JP 01.11.95: add rest from box name

        sTmp = OUStringLiteral1<cRelIdentifier>() + OUString::number( nBox )
             + OUStringLiteral1<cRelSeparator>() + OUString::number( nLine );

        if (!sCpy.isEmpty())
        {
            sTmp += OUStringLiteral1<cRelSeparator>() + sCpy;
        }
    }

    if (sTmp.endsWith(">"))
        return sTmp.copy(0, sTmp.getLength()-1 );

    return sTmp;
}

void SwTableFormula::GetBoxesOfFormula( const SwTable& rTable,
                                        SwSelBoxes& rBoxes )
{
    rBoxes.clear();

    BoxNmToPtr( &rTable );
    ScanString( &SwTableFormula::_GetFormulaBoxes, rTable, &rBoxes );
}

void SwTableFormula::_GetFormulaBoxes( const SwTable& rTable, OUString& ,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwSelBoxes* pBoxes = static_cast<SwSelBoxes*>(pPara);
    SwTableBox* pEndBox = nullptr;

    rFirstBox = rFirstBox.copy(1); // delete box label
    // area in these parentheses?
    if( pLastBox )
    {
        pEndBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(pLastBox->toInt64()));

        // Is it actually a valid pointer?
        if( rTable.GetTabSortBoxes().find( pEndBox ) == rTable.GetTabSortBoxes().end() )
            pEndBox = nullptr;
        rFirstBox = rFirstBox.copy( pLastBox->getLength()+1 );
    }

    SwTableBox *pSttBox = reinterpret_cast<SwTableBox*>(sal::static_int_cast<sal_IntPtr>(rFirstBox.toInt64()));
    // Is it actually a valid pointer?
    if( !pSttBox || rTable.GetTabSortBoxes().find( pSttBox ) == rTable.GetTabSortBoxes().end() )
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
                                SwSelBoxes& rBoxes )
{
    // get all selected boxes via layout
    const SwLayoutFrame *pStt, *pEnd;
    const SwFrame* pFrame = lcl_GetBoxFrame( rSttBox );
    pStt = pFrame ? pFrame->GetUpper() : nullptr;
    pEnd = ( nullptr != (pFrame = lcl_GetBoxFrame( rEndBox ))) ? pFrame->GetUpper() : nullptr;
    if( !pStt || !pEnd )
        return ;                        // no valid selection

    GetTableSel( pStt, pEnd, rBoxes, nullptr );

    const SwTable* pTable = pStt->FindTabFrame()->GetTable();

    // filter headline boxes
    if( pTable->GetRowsToRepeat() > 0 )
    {
        do {    // middle-check loop
            const SwTableLine* pLine = rSttBox.GetUpper();
            while( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTable->IsHeadline( *pLine ) )
                break;      // headline in this area!

            // maybe start and end are swapped
            pLine = rEndBox.GetUpper();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTable->IsHeadline( *pLine ) )
                break;      // headline in this area!

            const SwTabFrame *pStartTable = pStt->FindTabFrame();
            const SwTabFrame *pEndTable = pEnd->FindTabFrame();

            if (pStartTable == pEndTable) // no split table
                break;

            // then remove table headers
            for (size_t n = 0; n < rBoxes.size(); ++n)
            {
                pLine = rBoxes[n]->GetUpper();
                while( pLine->GetUpper() )
                    pLine = pLine->GetUpper()->GetUpper();

                if( pTable->IsHeadline( *pLine ) )
                    rBoxes.erase( rBoxes.begin() + n-- );
            }
        } while( false );
    }
}

/// Are all boxes valid that are referenced by the formula?
void SwTableFormula::_HasValidBoxes( const SwTable& rTable, OUString& ,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    bool* pBValid = static_cast<bool*>(pPara);
    if( *pBValid )      // wrong is wrong
    {
        SwTableBox* pSttBox = nullptr, *pEndBox = nullptr;
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
                const SwTableBox* pBox = !pNd ? nullptr
                                               : const_cast<SwTableBox *>(rTable.GetTableBox(
                                    pNd->FindTableBoxStartNode()->GetIndex() ));
                if( pLastBox )
                    pEndBox = const_cast<SwTableBox*>(lcl_RelToBox( rTable, pBox, *pLastBox ));
                pSttBox = const_cast<SwTableBox*>(lcl_RelToBox( rTable, pBox, rFirstBox ));
            }
            break;

        case EXTRNL_NAME:
            if( pLastBox )
                pEndBox = const_cast<SwTableBox*>(rTable.GetTableBox( *pLastBox ));
            pSttBox = const_cast<SwTableBox*>(rTable.GetTableBox( rFirstBox ));
            break;
        }

        // Are these valid pointers?
        if( ( pLastBox &&
              ( !pEndBox || rTable.GetTabSortBoxes().find( pEndBox ) == rTable.GetTabSortBoxes().end() ) ) ||
            ( !pSttBox || rTable.GetTabSortBoxes().find( pSttBox ) == rTable.GetTabSortBoxes().end() ) )
                *pBValid = false;
    }
}

bool SwTableFormula::HasValidBoxes() const
{
    bool bRet = true;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && nullptr != ( pNd = pNd->FindTableNode() ) )
        ScanString( &SwTableFormula::_HasValidBoxes,
                        static_cast<const SwTableNode*>(pNd)->GetTable(), &bRet );
    return bRet;
}

sal_uInt16 SwTableFormula::GetLnPosInTable( const SwTable& rTable, const SwTableBox* pBox )
{
    sal_uInt16 nRet = USHRT_MAX;
    if( pBox )
    {
        const SwTableLine* pLn = pBox->GetUpper();
        while( pLn->GetUpper() )
            pLn = pLn->GetUpper()->GetUpper();
        nRet = rTable.GetTabLines().GetPos( pLn );
    }
    return nRet;
}

void SwTableFormula::_SplitMergeBoxNm( const SwTable& rTable, OUString& rNewStr,
                    OUString& rFirstBox, OUString* pLastBox, void* pPara ) const
{
    SwTableFormulaUpdate& rTableUpd = *static_cast<SwTableFormulaUpdate*>(pPara);

    rNewStr += OUString(rFirstBox[0]);     // get label for the box
    rFirstBox = rFirstBox.copy(1);

    OUString sTableNm;
    const SwTable* pTable = &rTable;

    OUString* pTableNmBox = pLastBox ? pLastBox : &rFirstBox;

    const sal_Int32 nLastBoxLen = pTableNmBox->getLength();
    const sal_Int32 nSeparator = pTableNmBox->indexOf('.');
    if ( nSeparator>=0 &&
        // If there are dots in the name, than these appear in pairs (e.g. A1.1.1)!
        (comphelper::string::getTokenCount(*pTableNmBox, '.') - 1) & 1 )
    {
        sTableNm = pTableNmBox->copy( 0, nSeparator );
        *pTableNmBox = pTableNmBox->copy( nSeparator + 1); // remove dot
        const SwTable* pFnd = FindTable( *rTable.GetFrameFormat()->GetDoc(), sTableNm );
        if( pFnd )
            pTable = pFnd;

        if( TBL_MERGETBL == rTableUpd.m_eFlags )
        {
            if( pFnd )
            {
                if( pFnd == rTableUpd.m_aData.pDelTable )
                {
                    if( rTableUpd.m_pTable != &rTable ) // not the current one
                        rNewStr += rTableUpd.m_pTable->GetFrameFormat()->GetName() + "."; // set new table name
                    rTableUpd.m_bModified = true;
                }
                else if( pFnd != rTableUpd.m_pTable ||
                    ( rTableUpd.m_pTable != &rTable && &rTable != rTableUpd.m_aData.pDelTable))
                    rNewStr += sTableNm + "."; // keep table name
                else
                    rTableUpd.m_bModified = true;
            }
            else
                rNewStr += sTableNm + ".";     // keep table name
        }
    }
    if( pTableNmBox == pLastBox )
        rFirstBox = rFirstBox.copy( nLastBoxLen + 1 );

    SwTableBox* pSttBox = nullptr, *pEndBox = nullptr;
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
            const SwTableBox* pBox = pNd ? pTable->GetTableBox(
                            pNd->FindTableBoxStartNode()->GetIndex() ) : nullptr;
            if( pLastBox )
                pEndBox = const_cast<SwTableBox*>(lcl_RelToBox( *pTable, pBox, *pLastBox ));
            pSttBox = const_cast<SwTableBox*>(lcl_RelToBox( *pTable, pBox, rFirstBox ));
        }
        break;

    case EXTRNL_NAME:
        if( pLastBox )
            pEndBox = const_cast<SwTableBox*>(pTable->GetTableBox( *pLastBox ));
        pSttBox = const_cast<SwTableBox*>(pTable->GetTableBox( rFirstBox ));
        break;
    }

    if( pLastBox && pTable->GetTabSortBoxes().find( pEndBox ) == pTable->GetTabSortBoxes().end() )
        pEndBox = nullptr;
    if( pTable->GetTabSortBoxes().find( pSttBox ) == pTable->GetTabSortBoxes().end() )
        pSttBox = nullptr;

    if( TBL_SPLITTBL == rTableUpd.m_eFlags )
    {
        // Where are the boxes - in the old or in the new table?
        bool bInNewTable = false;
        if( pLastBox )
        {
            // It is the "first" box in this selection. It determines if the formula is placed in
            // the new or the old table.
            sal_uInt16 nEndLnPos = SwTableFormula::GetLnPosInTable( *pTable, pEndBox ),
                    nSttLnPos = SwTableFormula::GetLnPosInTable( *pTable, pSttBox );

            if( USHRT_MAX != nSttLnPos && USHRT_MAX != nEndLnPos &&
                ((rTableUpd.m_nSplitLine <= nSttLnPos) ==
                (rTableUpd.m_nSplitLine <= nEndLnPos)) )
            {
                // stay in same table
                bInNewTable = rTableUpd.m_nSplitLine <= nEndLnPos &&
                                    pTable == rTableUpd.m_pTable;
            }
            else
            {
                // this is definitely an invalid formula, also mark as modified for Undo
                rTableUpd.m_bModified = true;
                if( pEndBox )
                    bInNewTable = USHRT_MAX != nEndLnPos &&
                                    rTableUpd.m_nSplitLine <= nEndLnPos &&
                                    pTable == rTableUpd.m_pTable;
            }
        }
        else
        {
            sal_uInt16 nSttLnPos = SwTableFormula::GetLnPosInTable( *pTable, pSttBox );
            // Put it in the new table?
            bInNewTable = USHRT_MAX != nSttLnPos &&
                            rTableUpd.m_nSplitLine <= nSttLnPos &&
                            pTable == rTableUpd.m_pTable;
        }

        // formula goes into new table
        if( rTableUpd.m_bBehindSplitLine )
        {
            if( !bInNewTable )
            {
                rTableUpd.m_bModified = true;
                rNewStr += rTableUpd.m_pTable->GetFrameFormat()->GetName() + ".";
            }
            else if( !sTableNm.isEmpty() )
                rNewStr += sTableNm + ".";
        }
        else if( bInNewTable )
        {
            rTableUpd.m_bModified = true;
            rNewStr += *rTableUpd.m_aData.pNewTableNm + ".";
        }
        else if( !sTableNm.isEmpty() )
            rNewStr += sTableNm + ".";
    }

    if( pLastBox )
        rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pEndBox)) + ":";

    rNewStr += OUString::number(reinterpret_cast<sal_PtrDiff>(pSttBox))
            +  OUString(rFirstBox[ rFirstBox.getLength()-1] );
}

/// Create external formula but remember that the formula is placed in a split/merged table
void SwTableFormula::ToSplitMergeBoxNm( SwTableFormulaUpdate& rTableUpd )
{
    const SwTable* pTable;
    const SwNode* pNd = GetNodeOfFormula();
    if( pNd && nullptr != ( pNd = pNd->FindTableNode() ))
        pTable = &static_cast<const SwTableNode*>(pNd)->GetTable();
    else
        pTable = rTableUpd.m_pTable;

    m_sFormula = ScanString( &SwTableFormula::_SplitMergeBoxNm, *pTable, static_cast<void*>(&rTableUpd) );
    m_eNmType = INTRNL_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
