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

#include <rangelst.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/waitobj.hxx>

#include "uiitems.hxx"
#include "dbdata.hxx"
#include "reffact.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "queryentry.hxx"

#include "foptmgr.hxx"

#include "globstr.hrc"
#include "filter.hrc"

#define _FILTDLG_CXX
#include "filtdlg.hxx"
#undef _FILTDLG_CXX
#include <vcl/msgbox.hxx>

#include <limits>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

#define ERRORBOX(rid)   ErrorBox( this, WinBits( WB_OK|WB_DEF_OK), \
                                   ScGlobal::GetRscString(rid) ).Execute()

#define QUERY_ENTRY_COUNT 4
#define INVALID_HEADER_POS std::numeric_limits<size_t>::max()

ScFilterDlg::EntryList::EntryList() :
    mnHeaderPos(INVALID_HEADER_POS) {}

ScFilterDlg::ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          const SfxItemSet& rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_FILTER ),
        //
        aFlCriteria     ( this, ScResId( FL_CRITERIA ) ),
        aLbConnect1     ( this, ScResId( LB_OP1 ) ),
        aLbField1       ( this, ScResId( LB_FIELD1 ) ),
        aLbCond1        ( this, ScResId( LB_COND1 ) ),
        aEdVal1         ( this, ScResId( ED_VAL1 ) ),
        aLbConnect2     ( this, ScResId( LB_OP2 ) ),
        aLbField2       ( this, ScResId( LB_FIELD2 ) ),
        aLbCond2        ( this, ScResId( LB_COND2 ) ),
        aEdVal2         ( this, ScResId( ED_VAL2 ) ),
        aLbConnect3     ( this, ScResId( LB_OP3 ) ),
        aLbField3       ( this, ScResId( LB_FIELD3 ) ),
        aLbCond3        ( this, ScResId( LB_COND3 ) ),
        aEdVal3         ( this, ScResId( ED_VAL3 ) ),
        aLbConnect4     ( this, ScResId( LB_OP4 ) ),
        aLbField4       ( this, ScResId( LB_FIELD4 ) ),
        aLbCond4        ( this, ScResId( LB_COND4 ) ),
        aEdVal4         ( this, ScResId( ED_VAL4 ) ),
        aFtConnect      ( this, ScResId( FT_OP ) ),
        aFtField        ( this, ScResId( FT_FIELD ) ),
        aFtCond         ( this, ScResId( FT_COND ) ),
        aFtVal          ( this, ScResId( FT_VAL ) ),
        aFlSeparator    ( this, ScResId( FL_SEPARATOR ) ),
        aScrollBar      ( this, ScResId( LB_SCROLL ) ),
        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ),
        aBtnHeader      ( this, ScResId( BTN_HEADER ) ),
        aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ),
        aBtnCopyResult  ( this, ScResId( BTN_COPY_RESULT ) ),
        aLbCopyArea     ( this, ScResId( LB_COPY_AREA ) ),
        aEdCopyArea     ( this, this, ScResId( ED_COPY_AREA ) ),
        aRbCopyArea     ( this, ScResId( RB_COPY_AREA ), &aEdCopyArea, this ),
        aBtnDestPers    ( this, ScResId( BTN_DEST_PERS ) ),
        aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ),
        aFtDbArea       ( this, ScResId( FT_DBAREA ) ),
        aStrUndefined   ( SC_RESSTR(SCSTR_UNDEFINED) ),
        aStrNone        ( SC_RESSTR(SCSTR_NONE) ),
        aStrEmpty       ( SC_RESSTR(SCSTR_FILTER_EMPTY) ),
        aStrNotEmpty    ( SC_RESSTR(SCSTR_FILTER_NOTEMPTY) ),
        aStrRow         ( SC_RESSTR(SCSTR_ROW) ),
        aStrColumn      ( SC_RESSTR(SCSTR_COLUMN) ),
        //
        pOptionsMgr     ( NULL ),
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( ((const ScQueryItem&)
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pOutItem        ( NULL ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        nSrcTab         ( 0 ),
        bRefInputMode   ( false ),
        pTimer          ( NULL )
{
    aBtnMore.SetMoreText( String(ScResId( SCSTR_MOREBTN_MOREOPTIONS )) );
    aBtnMore.SetLessText( String(ScResId( SCSTR_MOREBTN_FEWEROPTIONS )) );
    Init( rArgSet );
    FreeResource();

    // Hack: RefInput control
    pTimer = new Timer;
    pTimer->SetTimeout( 50 ); // Wait 50ms
    pTimer->SetTimeoutHdl( LINK( this, ScFilterDlg, TimeOutHdl ) );

    OUStringBuffer aBuf;

    OUString sAccName = SC_RESSTR(RID_FILTER_OPERATOR);
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 1"));
    aLbConnect1.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 2"));
    aLbConnect2.SetAccessibleName(aBuf.makeStringAndClear());

    sAccName = SC_RESSTR(RID_FILTER_FIELDNAME);
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 1"));
    aLbField1.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 2"));
    aLbField2.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 3"));
    aLbField3.SetAccessibleName(aBuf.makeStringAndClear());

    sAccName = SC_RESSTR(RID_FILTER_CONDITION);
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 1"));
    aLbCond1.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 2"));
    aLbCond2.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 3"));
    aLbCond3.SetAccessibleName(aBuf.makeStringAndClear());

    sAccName = SC_RESSTR(RID_FILTER_VALUE);
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 1"));
    aEdVal1.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 2"));
    aEdVal2.SetAccessibleName(aBuf.makeStringAndClear());
    aBuf.append(sAccName);
    aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM (" 3"));
    aEdVal3.SetAccessibleName(aBuf.makeStringAndClear());

    aLbCopyArea.SetAccessibleName(ScResId(STR_COPY_AREA_TO));
    aEdCopyArea.SetAccessibleName(ScResId(STR_COPY_AREA_TO));
    aLbCopyArea.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aEdCopyArea.SetAccessibleRelationLabeledBy(&aBtnCopyResult);

    aLbConnect1.SetAccessibleRelationLabeledBy(&aFtConnect);
    aLbConnect2.SetAccessibleRelationLabeledBy(&aFtConnect);
    aLbField1.SetAccessibleRelationLabeledBy(&aFtField);
    aLbField2.SetAccessibleRelationLabeledBy(&aFtField);
    aLbField3.SetAccessibleRelationLabeledBy(&aFtField);
    aLbCond1.SetAccessibleRelationLabeledBy(&aFtCond);
    aLbCond2.SetAccessibleRelationLabeledBy(&aFtCond);
    aLbCond3.SetAccessibleRelationLabeledBy(&aFtCond);
    aEdVal1.SetAccessibleRelationLabeledBy(&aFtVal);
    aEdVal2.SetAccessibleRelationLabeledBy(&aFtVal);
    aEdVal3.SetAccessibleRelationLabeledBy(&aFtVal);
}


//----------------------------------------------------------------------------

ScFilterDlg::~ScFilterDlg()
{
    delete pOptionsMgr;
    delete pOutItem;

    // Hack: RefInput control
    pTimer->Stop();
    delete pTimer;
}


//----------------------------------------------------------------------------

void ScFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = (const ScQueryItem&)
                                    rArgSet.Get( nWhichQuery );

    aBtnOk.SetClickHdl      ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    aBtnMore.SetClickHdl    ( LINK( this, ScFilterDlg, MoreClickHdl ) );
    aBtnHeader.SetClickHdl  ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    aBtnCase.SetClickHdl    ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    //
    aLbField1.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbField2.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbField3.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbField4.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect1.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect2.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect3.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect4.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );

    aLbCond1.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbCond2.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbCond3.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbCond4.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : NULL;
    nSrcTab     = pViewData ? pViewData->GetTabNo() : static_cast<SCTAB>(0);

    // for easier access:
    maFieldLbArr.reserve(QUERY_ENTRY_COUNT);
    maFieldLbArr.push_back(&aLbField1);
    maFieldLbArr.push_back(&aLbField2);
    maFieldLbArr.push_back(&aLbField3);
    maFieldLbArr.push_back(&aLbField4);
    maValueEdArr.reserve(QUERY_ENTRY_COUNT);
    maValueEdArr.push_back(&aEdVal1);
    maValueEdArr.push_back(&aEdVal2);
    maValueEdArr.push_back(&aEdVal3);
    maValueEdArr.push_back(&aEdVal4);
    maCondLbArr.reserve(QUERY_ENTRY_COUNT);
    maCondLbArr.push_back(&aLbCond1);
    maCondLbArr.push_back(&aLbCond2);
    maCondLbArr.push_back(&aLbCond3);
    maCondLbArr.push_back(&aLbCond4);
    maConnLbArr.reserve(QUERY_ENTRY_COUNT);
    maConnLbArr.push_back(&aLbConnect1);
    maConnLbArr.push_back(&aLbConnect2);
    maConnLbArr.push_back(&aLbConnect3);
    maConnLbArr.push_back(&aLbConnect4);

    // Option initialization:

    pOptionsMgr  = new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            aBtnMore,
                            aBtnCase,
                            aBtnRegExp,
                            aBtnHeader,
                            aBtnUnique,
                            aBtnCopyResult,
                            aBtnDestPers,
                            aLbCopyArea,
                            aEdCopyArea,
                            aRbCopyArea,
                            aFtDbAreaLabel,
                            aFtDbArea,
                            aFlOptions,
                            aStrUndefined );

    // Read in field lists and select entries

    FillFieldLists();

    for (size_t i = 0; i < QUERY_ENTRY_COUNT; ++i)
    {
        rtl::OUString aValStr;
        size_t nCondPos = 0;
        size_t nFieldSelPos = 0;

        ScQueryEntry& rEntry = theQueryData.GetEntry(i);
        if ( rEntry.bDoQuery )
        {
            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            nCondPos = static_cast<size_t>(rEntry.eOp);
            nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );
            if (rEntry.IsQueryByEmpty())
            {
                aValStr = aStrEmpty;
                maCondLbArr[i]->Disable();
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->Disable();
            }
            else
            {
                if (rItem.maString.isEmpty())
                {
                    if (rItem.meType == ScQueryEntry::ByValue)
                        pDoc->GetFormatTable()->GetInputLineString( rItem.mfVal, 0, aValStr);
                    else if (rItem.meType == ScQueryEntry::ByDate)
                    {
                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        pFormatter->GetInputLineString( rItem.mfVal,
                                pFormatter->GetStandardFormat( NUMBERFORMAT_DATE), aValStr);
                    }
                    else
                    {
                        SAL_WARN( "sc", "ScFilterDlg::Init: empty query string, really?");
                        aValStr = rItem.maString;
                    }
                }
                else
                {
                    // XXX NOTE: if not ByString we just assume this has been
                    // set to a proper string corresponding to the numeric
                    // value earlier!
                    aValStr = rItem.maString;
                }
            }
        }
        else if ( i == 0 )
        {
            nFieldSelPos = GetFieldSelPos( pViewData->GetCurX() );
            rEntry.nField = nFieldSelPos ? (theQueryData.nCol1 +
                static_cast<SCCOL>(nFieldSelPos) - 1) : static_cast<SCCOL>(0);
            rEntry.bDoQuery=true;
            if (maRefreshExceptQuery.size() < i + 1)
                maRefreshExceptQuery.resize(i + 1, false);
            maRefreshExceptQuery[i] = true;

        }
        maFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
        maCondLbArr [i]->SelectEntryPos( nCondPos );
        maValueEdArr[i]->SetText( aValStr );
        maValueEdArr[i]->EnableAutocomplete( false );
        maValueEdArr[i]->SetModifyHdl( LINK( this, ScFilterDlg, ValModifyHdl ) );
        UpdateValueList(i+1);
    }

    aScrollBar.SetEndScrollHdl( LINK( this, ScFilterDlg, ScrollHdl ) );
    aScrollBar.SetScrollHdl( LINK( this, ScFilterDlg, ScrollHdl ) );

    aScrollBar.SetRange( Range( 0, 4 ) );
    aScrollBar.SetLineSize( 1 );
    aLbConnect1.Hide();
    // Disable/Enable Logic:

       (aLbField1.GetSelectEntryPos() != 0)
    && (aLbField2.GetSelectEntryPos() != 0)
        ? aLbConnect2.SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(1).eConnect )
        : aLbConnect2.SetNoSelection();

       (aLbField2.GetSelectEntryPos() != 0)
    && (aLbField3.GetSelectEntryPos() != 0)
        ? aLbConnect3.SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(2).eConnect )
        : aLbConnect3.SetNoSelection();

       (aLbField3.GetSelectEntryPos() != 0)
    && (aLbField4.GetSelectEntryPos() != 0)
        ? aLbConnect4.SelectEntryPos( (sal_uInt16)theQueryData.GetEntry(3).eConnect )
        : aLbConnect4.SetNoSelection();
    if ( aLbField1.GetSelectEntryPos() == 0 )
    {
        aLbConnect2.Disable();
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }
    else if ( aLbConnect2.GetSelectEntryCount() == 0 )
    {
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }

    if ( aLbField2.GetSelectEntryPos() == 0 )
    {
        aLbConnect3.Disable();
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }
    else if ( aLbConnect3.GetSelectEntryCount() == 0 )
    {
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }
    if ( aLbField3.GetSelectEntryPos() == 0 )
    {
        aLbConnect4.Disable();
        aLbField4.Disable();
        aLbCond4.Disable();
        aEdVal4.Disable();
    }
    else if ( aLbConnect4.GetSelectEntryCount() == 0 )
    {
        aLbField4.Disable();
        aLbCond4.Disable();
        aEdVal4.Disable();
    }

    if(pDoc!=NULL &&
        pDoc->GetChangeTrack()!=NULL) aBtnCopyResult.Disable();
    // Switch on modal mode
//  SetDispatcherLock( true );
    //@BugID 54702 Enable/disable only in Basic class
//  SFX_APPWINDOW->Disable(false);      //! general method in ScAnyRefDlg
}


//----------------------------------------------------------------------------

sal_Bool ScFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    return DoClose( ScFilterDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Mouse-selected cell area becomes the new selection and is shown in the
// reference text box

void ScFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( bRefInputMode )    // Only possible if in reference edit mode
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdCopyArea );
        rtl::OUString aRefStr;
        rRef.aStart.Format( aRefStr, SCA_ABS_3D, pDocP, pDocP->GetAddressConvention() );
        aEdCopyArea.SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        aEdCopyArea.GrabFocus();
        if ( aEdCopyArea.GetModifyHdl().IsSet() )
            ((Link&)aEdCopyArea.GetModifyHdl()).Call( &aEdCopyArea );
    }
    else
        GrabFocus();

    RefInputDone();
}

//----------------------------------------------------------------------------

void ScFilterDlg::FillFieldLists()
{
    aLbField1.Clear();
    aLbField2.Clear();
    aLbField3.Clear();
    aLbField4.Clear();
    aLbField1.InsertEntry( aStrNone, 0 );
    aLbField2.InsertEntry( aStrNone, 0 );
    aLbField3.InsertEntry( aStrNone, 0 );
    aLbField4.InsertEntry( aStrNone, 0 );

    if ( pDoc )
    {
        rtl::OUString aFieldName;
        SCTAB   nTab        = nSrcTab;
        SCCOL   nFirstCol   = theQueryData.nCol1;
        SCROW   nFirstRow   = theQueryData.nRow1;
        SCCOL   nMaxCol     = theQueryData.nCol2;
        SCCOL   col = 0;
        sal_uInt16  i=1;

        for ( col=nFirstCol; col<=nMaxCol; col++ )
        {
            pDoc->GetString( col, nFirstRow, nTab, aFieldName );
            if (!aBtnHeader.IsChecked() || aFieldName.isEmpty())
            {
                rtl::OUStringBuffer aBuf;
                aBuf.append(aStrColumn);
                aBuf.append(sal_Unicode(' '));
                aBuf.append(ScColToAlpha(col));
                aFieldName = aBuf.makeStringAndClear();
            }
            aLbField1.InsertEntry( aFieldName, i );
            aLbField2.InsertEntry( aFieldName, i );
            aLbField3.InsertEntry( aFieldName, i );
            aLbField4.InsertEntry( aFieldName, i );
            i++;
        }
    }
}

void ScFilterDlg::UpdateValueList( size_t nList )
{
    bool bCaseSens = aBtnCase.IsChecked();

    if (pDoc && nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        ComboBox*   pValList        = maValueEdArr[nList-1];
        sal_uInt16      nFieldSelPos    = maFieldLbArr[nList-1]->GetSelectEntryPos();
        sal_uInt16      nListPos        = 0;
        rtl::OUString aCurValue = pValList->GetText();

        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        nListPos = 2;

        if ( nFieldSelPos )
        {
            WaitObject aWaiter( this );     // even if only the list box has content

            SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
            EntryList* pList = NULL;
            if (!maEntryLists.count(nColumn))
            {
                size_t nOffset = GetSliderPos();
                SCTAB nTab       = nSrcTab;
                SCROW nFirstRow = theQueryData.nRow1;
                SCROW nLastRow   = theQueryData.nRow2;
                if (maHasDates.size() < nOffset+nList)
                    maHasDates.resize(nOffset+nList, false);
                maHasDates[nOffset+nList-1] = false;

                // first without the first line
                std::pair<EntryListsMap::iterator, bool> r =
                    maEntryLists.insert(nColumn, new EntryList);
                if (!r.second)
                    // insertion failed.
                    return;

                pList = r.first->second;
                pDoc->GetFilterEntriesArea(
                    nColumn, nFirstRow+1, nLastRow,
                    nTab, bCaseSens, pList->maList, maHasDates[nOffset+nList-1] );

                // Entry for the first line
                //! Entry (pHdrEntry) doesn't generate collection?

                pList->mnHeaderPos = INVALID_HEADER_POS;
                std::vector<ScTypedStrData> aHdrColl;
                bool bDummy = false;
                pDoc->GetFilterEntriesArea(
                    nColumn, nFirstRow, nFirstRow, nTab, true, aHdrColl, bDummy );
                if (!aHdrColl.empty())
                {
                    // See if the header value is already in the list.
                    std::vector<ScTypedStrData>::iterator itBeg = pList->maList.begin(), itEnd = pList->maList.end();
                    std::vector<ScTypedStrData>::iterator it = std::find_if(
                        itBeg, itEnd, FindTypedStrData(aHdrColl.front(), bCaseSens));
                    if (it == itEnd)
                    {
                        // Not in the list. Insert it.
                        pList->maList.push_back(aHdrColl.front());
                        if (bCaseSens)
                            std::sort(pList->maList.begin(), pList->maList.end(), ScTypedStrData::LessCaseSensitive());
                        else
                            std::sort(pList->maList.begin(), pList->maList.end(), ScTypedStrData::LessCaseInsensitive());

                        // Record its position.
                        itBeg = pList->maList.begin();
                        itEnd = pList->maList.end();
                        it = std::find_if(itBeg, itEnd, FindTypedStrData(aHdrColl.front(), bCaseSens));
                        pList->mnHeaderPos = std::distance(itBeg, it);
                    }
                }
            }
            else
                pList = &maEntryLists[nColumn];

            OSL_ASSERT(pList);

            std::vector<ScTypedStrData>::const_iterator it = pList->maList.begin(), itEnd = pList->maList.end();
            for (; it != itEnd; ++it)
            {
                pValList->InsertEntry(it->GetString(), nListPos);
                nListPos++;
            }
        }
        pValList->SetText( aCurValue );
        pValList->EnableDDAutoWidth(false);
    }

    UpdateHdrInValueList( nList );
}

void ScFilterDlg::UpdateHdrInValueList( size_t nList )
{
    //! GetText / SetText ??

    if (!pDoc)
        return;

    if (nList == 0 || nList > QUERY_ENTRY_COUNT)
        return;

    size_t nFieldSelPos = maFieldLbArr[nList-1]->GetSelectEntryPos();
    if (!nFieldSelPos)
        return;

    SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
    if (!maEntryLists.count(nColumn))
    {
        OSL_FAIL("Spalte noch nicht initialisiert");
        return;
    }

    size_t nPos = maEntryLists[nColumn].mnHeaderPos;
    if (nPos == INVALID_HEADER_POS)
        return;

    ComboBox* pValList = maValueEdArr[nList-1];
    size_t nListPos = nPos + 2;                 // for "empty" and "non-empty"

    const ScTypedStrData& rHdrEntry = maEntryLists[nColumn].maList[nPos];

    const rtl::OUString& aHdrStr = rHdrEntry.GetString();
    bool bWasThere = aHdrStr.equals(pValList->GetEntry(nListPos));
    bool bInclude = !aBtnHeader.IsChecked();

    if (bInclude)           // Include entry
    {
        if (!bWasThere)
            pValList->InsertEntry(aHdrStr, nListPos);
    }
    else                    // Omit entry
    {
        if (bWasThere)
            pValList->RemoveEntry(nListPos);
    }
}

//----------------------------------------------------------------------------

void ScFilterDlg::ClearValueList( size_t nList )
{
    if (nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        ComboBox* pValList = maValueEdArr[nList-1];
        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        pValList->SetText( EMPTY_STRING );
    }
}


//----------------------------------------------------------------------------

size_t ScFilterDlg::GetFieldSelPos( SCCOL nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return static_cast<size_t>(nField - theQueryData.nCol1 + 1);
    else
        return 0;
}

//----------------------------------------------------------------------------

ScQueryItem* ScFilterDlg::GetOutputItem()
{
    ScAddress       theCopyPos;
    ScQueryParam    theParam( theQueryData );
    bool            bCopyPosOk = false;

    if ( aBtnCopyResult.IsChecked() )
    {
        sal_uInt16 nResult = theCopyPos.Parse(
            aEdCopyArea.GetText(), pDoc, pDoc->GetAddressConvention());
        bCopyPosOk = ( SCA_VALID == (nResult & SCA_VALID) );
    }

    if ( aBtnCopyResult.IsChecked() && bCopyPosOk )
    {
        theParam.bInplace   = false;
        theParam.nDestTab   = theCopyPos.Tab();
        theParam.nDestCol   = theCopyPos.Col();
        theParam.nDestRow   = theCopyPos.Row();
    }
    else
    {
        theParam.bInplace   = true;
        theParam.nDestTab   = 0;
        theParam.nDestCol   = 0;
        theParam.nDestRow   = 0;
    }

    theParam.bHasHeader     = aBtnHeader.IsChecked();
    theParam.bByRow         = true;
    theParam.bDuplicate     = !aBtnUnique.IsChecked();
    theParam.bCaseSens      = aBtnCase.IsChecked();
    theParam.bRegExp        = aBtnRegExp.IsChecked();
    theParam.bDestPers      = aBtnDestPers.IsChecked();

    // only set the three - reset everything else

    DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &theParam );

    return pOutItem;
}


//----------------------------------------------------------------------------

sal_Bool ScFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScFilterDlg, EndDlgHdl, Button*, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        bool bAreaInputOk = true;

        if ( aBtnCopyResult.IsChecked() )
        {
            if ( !pOptionsMgr->VerifyPosStr( aEdCopyArea.GetText() ) )
            {
                if ( !aBtnMore.GetState() )
                    aBtnMore.SetState( true );

                ERRORBOX( STR_INVALID_TABREF );
                aEdCopyArea.GrabFocus();
                bAreaInputOk = false;
            }
        }

        if ( bAreaInputOk )
        {
            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->Execute( FID_FILTER_OK,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      GetOutputItem(), 0L, 0L );
            Close();
        }
    }
    else if ( pBtn == &aBtnCancel )
    {
        Close();
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScFilterDlg, MoreClickHdl)
{
    if ( aBtnMore.GetState() )
        pTimer->Start();
    else
    {
        pTimer->Stop();
        bRefInputMode = false;
        //@BugID 54702 Enable/disable only in Basic class
        //SFX_APPWINDOW->Disable(FALSE);        //! general method in ScAnyRefDlg
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, TimeOutHdl, Timer*, _pTimer )
{
    // Check if RefInputMode is still true every 50ms

    if( _pTimer == pTimer && IsActive() )
        bRefInputMode = (aEdCopyArea.HasFocus() || aRbCopyArea.HasFocus());

    if ( aBtnMore.GetState() )
        pTimer->Start();

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, LbSelectHdl, ListBox*, pLb )
{
    /*
     * Handle enable/disable logic depending on which ListBox was selected
     */
    sal_uInt16 nOffset = GetSliderPos();

    if ( pLb == &aLbConnect1 )
    {
        aLbField1.Enable();
        aLbCond1.Enable();
        aEdVal1.Enable();

        sal_uInt16  nConnect1 = aLbConnect1.GetSelectEntryPos();
        size_t nQE = nOffset;
        theQueryData.GetEntry(nQE).eConnect =(ScQueryConnect)nConnect1;
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }

    else if ( pLb == &aLbConnect2 )
    {
        aLbField2.Enable();
        aLbCond2.Enable();
        aEdVal2.Enable();

        sal_uInt16  nConnect2 = aLbConnect2.GetSelectEntryPos();
        size_t nQE = 1+nOffset;
        theQueryData.GetEntry(nQE).eConnect =(ScQueryConnect)nConnect2;
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE]=true;
    }
    else if ( pLb == &aLbConnect3 )
    {
        aLbField3.Enable();
        aLbCond3.Enable();
        aEdVal3.Enable();

        sal_uInt16  nConnect3 = aLbConnect3.GetSelectEntryPos();
        size_t nQE = 2 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = (ScQueryConnect)nConnect3;
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;

    }
    else if ( pLb == &aLbConnect4 )
    {
        aLbField4.Enable();
        aLbCond4.Enable();
        aEdVal4.Enable();

        sal_uInt16  nConnect4 = aLbConnect4.GetSelectEntryPos();
        size_t nQE = 3 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = (ScQueryConnect)nConnect4;
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }
    else if ( pLb == &aLbField1 )
    {
        if ( aLbField1.GetSelectEntryPos() == 0 )
        {
            aLbConnect2.SetNoSelection();
            aLbConnect3.SetNoSelection();
            aLbConnect4.SetNoSelection();
            aLbField2.SelectEntryPos( 0 );
            aLbField3.SelectEntryPos( 0 );
            aLbField4.SelectEntryPos( 0 );
            aLbCond2.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            aLbCond4.SelectEntryPos( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            aLbConnect2.Disable();
            aLbConnect3.Disable();
            aLbConnect4.Disable();
            aLbField2.Disable();
            aLbField3.Disable();
            aLbField4.Disable();
            aLbCond2.Disable();
            aLbCond3.Disable();
            aLbCond4.Disable();
            aEdVal2.Disable();
            aEdVal3.Disable();
            aEdVal4.Disable();
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount + 1)
                maRefreshExceptQuery.resize(nCount + 1, false);
            for (sal_uInt16 i = nOffset; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nOffset] = true;
        }
        else
        {
            UpdateValueList( 1 );
            if ( !aLbConnect2.IsEnabled() )
            {
                aLbConnect2.Enable();
            }
            theQueryData.GetEntry(nOffset).bDoQuery = true;
            sal_uInt16  nField  = pLb->GetSelectEntryPos();
            theQueryData.GetEntry(nOffset).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( pLb == &aLbField2 )
    {
        if ( aLbField2.GetSelectEntryPos() == 0 )
        {
            aLbConnect3.SetNoSelection();
            aLbConnect4.SetNoSelection();
            aLbField3.SelectEntryPos( 0 );
            aLbField4.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            aLbCond4.SelectEntryPos( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            aLbConnect3.Disable();
            aLbConnect4.Disable();
            aLbField3.Disable();
            aLbField4.Disable();
            aLbCond3.Disable();
            aLbCond4.Disable();
            aEdVal3.Disable();
            aEdVal4.Disable();

            sal_uInt16 nTemp=nOffset+1;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (sal_uInt16 i= nTemp; i< nCount; i++)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 2 );
            if ( !aLbConnect3.IsEnabled() )
            {
                aLbConnect3.Enable();
            }
            sal_uInt16  nField  = pLb->GetSelectEntryPos();
            sal_uInt16 nQ=1+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( pLb == &aLbField3 )
    {
        if ( aLbField3.GetSelectEntryPos() == 0 )
        {
            aLbConnect4.SetNoSelection();
            aLbField4.SelectEntryPos( 0 );
            aLbCond4.SelectEntryPos( 0 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            aLbConnect4.Disable();
            aLbField4.Disable();
            aLbCond4.Disable();
            aEdVal4.Disable();

            sal_uInt16 nTemp=nOffset+2;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (sal_uInt16 i = nTemp; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 3 );
            if ( !aLbConnect4.IsEnabled() )
            {
                aLbConnect4.Enable();
            }

            sal_uInt16  nField  = pLb->GetSelectEntryPos();
            sal_uInt16 nQ=2+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;

        }
    }
    else if ( pLb == &aLbField4 )
    {
        if ( aLbField4.GetSelectEntryPos() == 0 )
        {
            ClearValueList( 4 );
            sal_uInt16 nTemp=nOffset+3;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (sal_uInt16 i = nTemp; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 4 );
            sal_uInt16  nField  = pLb->GetSelectEntryPos();
            sal_uInt16 nQ=3+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }

    }
    else if ( pLb == &aLbCond1)
    {
        theQueryData.GetEntry(nOffset).eOp=(ScQueryOp)pLb->GetSelectEntryPos();
    }
    else if ( pLb == &aLbCond2)
    {
        sal_uInt16 nQ=1+nOffset;
        theQueryData.GetEntry(nQ).eOp=(ScQueryOp)pLb->GetSelectEntryPos();
    }
    else if ( pLb == &aLbCond3)
    {
        sal_uInt16 nQ=2+nOffset;
        theQueryData.GetEntry(nQ).eOp=(ScQueryOp)pLb->GetSelectEntryPos();
    }
    else
    {
        sal_uInt16 nQ=3+nOffset;
        theQueryData.GetEntry(nQ).eOp=(ScQueryOp)pLb->GetSelectEntryPos();
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, CheckBoxHdl, CheckBox*, pBox )
{
    //  Column headers:
    //      Field list: Columnxx <-> column header string
    //      Value list: Column header value not applicable.
    //  Upper/lower case:
    //      Value list: completely new

    if ( pBox == &aBtnHeader )              // Field list and value list
    {
        sal_uInt16 nCurSel1 = aLbField1.GetSelectEntryPos();
        sal_uInt16 nCurSel2 = aLbField2.GetSelectEntryPos();
        sal_uInt16 nCurSel3 = aLbField3.GetSelectEntryPos();
        sal_uInt16 nCurSel4 = aLbField4.GetSelectEntryPos();
        FillFieldLists();
        aLbField1.SelectEntryPos( nCurSel1 );
        aLbField2.SelectEntryPos( nCurSel2 );
        aLbField3.SelectEntryPos( nCurSel3 );
        aLbField4.SelectEntryPos( nCurSel4 );

        UpdateHdrInValueList( 1 );
        UpdateHdrInValueList( 2 );
        UpdateHdrInValueList( 3 );
        UpdateHdrInValueList( 4 );
    }

    if ( pBox == &aBtnCase )            // Complete value list
    {
        maEntryLists.clear();
        UpdateValueList( 1 );       // current text is recorded
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        UpdateValueList( 4 );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, ValModifyHdl, ComboBox*, pEd )
{
    size_t nOffset = GetSliderPos();
    size_t i = 0;
    size_t nQE = i + nOffset;
    if ( pEd )
    {
        rtl::OUString aStrVal = pEd->GetText();
        ListBox*  pLbCond   = &aLbCond1;
        ListBox*  pLbField  = &aLbField1;
        if ( pEd == &aEdVal2 )
        {
            pLbCond  = &aLbCond2;
            pLbField = &aLbField2;
            i=1;
            nQE=i+nOffset;
        }
        if ( pEd == &aEdVal3 )
        {
            pLbCond = &aLbCond3;
            pLbField = &aLbField3;
            i=2;
            nQE=i+nOffset;
        }
        if ( pEd == &aEdVal4 )
        {
            pLbCond = &aLbCond4;
            pLbField = &aLbField4;
            i=3;
            nQE=i+nOffset;
        }

        if ( aStrEmpty.equals(aStrVal) || aStrNotEmpty.equals(aStrVal) )
        {
            pLbCond->SelectEntry(rtl::OUString('='));
            pLbCond->Disable();
        }
        else
            pLbCond->Enable();

        if (maHasDates.size() < nQE + 1)
            maHasDates.resize(nQE + 1, false);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);

        ScQueryEntry& rEntry = theQueryData.GetEntry( nQE );
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        bool bDoThis = (pLbField->GetSelectEntryPos() != 0);
        rEntry.bDoQuery = bDoThis;

        if ( rEntry.bDoQuery || maRefreshExceptQuery[nQE] )
        {
            if ( aStrEmpty.equals(aStrVal) )
            {
                rEntry.SetQueryByEmpty();
            }
            else if ( aStrNotEmpty.equals(aStrVal) )
            {
                rEntry.SetQueryByNonEmpty();
            }
            else
            {
                rItem.maString = aStrVal;
                rItem.mfVal = 0.0;

                sal_uInt32 nIndex;
                bool bNumber = pDoc->GetFormatTable()->IsNumberFormat(
                    rItem.maString, nIndex, rItem.mfVal);
                rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
            }

            sal_uInt16  nField  = pLbField->GetSelectEntryPos();
            rEntry.nField = nField ? (theQueryData.nCol1 +
                static_cast<SCCOL>(nField) - 1) : static_cast<SCCOL>(0);

            ScQueryOp eOp  = (ScQueryOp)pLbCond->GetSelectEntryPos();
            rEntry.eOp     = eOp;
            if (maHasDates[nQE])
                rItem.meType = ScQueryEntry::ByDate;
        }
    }
    return 0;
}

//----------------------------------------------------------------------------
IMPL_LINK_NOARG(ScFilterDlg, ScrollHdl)
{
    SliderMoved();
    return 0;
}

void ScFilterDlg::SliderMoved()
{
    size_t nOffset = GetSliderPos();
    RefreshEditRow( nOffset);
}

size_t ScFilterDlg::GetSliderPos()
{
    return static_cast<size_t>(aScrollBar.GetThumbPos());
}

void ScFilterDlg::RefreshEditRow( size_t nOffset )
{
    if (nOffset==0)
        maConnLbArr[0]->Hide();
    else
        maConnLbArr[0]->Show();

    for (size_t i = 0; i < QUERY_ENTRY_COUNT; ++i)
    {
        rtl::OUString aValStr;
        size_t nCondPos = 0;
        size_t nFieldSelPos = 0;
        size_t nQE = i + nOffset;

        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);

        ScQueryEntry& rEntry = theQueryData.GetEntry( nQE);
        if ( rEntry.bDoQuery || maRefreshExceptQuery[nQE] )
        {
            nCondPos = static_cast<size_t>(rEntry.eOp);
            if(rEntry.bDoQuery)
               nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );

            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            const rtl::OUString& rQueryStr = rItem.maString;
            if (rEntry.IsQueryByEmpty())
            {
                aValStr = aStrEmpty;
                maCondLbArr[i]->Disable();
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->Disable();
            }
            else
            {
                aValStr = rQueryStr;
                maCondLbArr[i]->Enable();
            }
            maFieldLbArr[i]->Enable();
            maValueEdArr[i]->Enable();

            if (nOffset==0)
            {
                if (i<3)
                {
                    if(rEntry.bDoQuery)
                        maConnLbArr[i+1]->Enable();
                    else
                        maConnLbArr[i+1]->Disable();
                    size_t nQENext = nQE + 1;
                    if (maRefreshExceptQuery.size() < nQENext + 1)
                        maRefreshExceptQuery.resize(nQENext + 1, false);
                    if (theQueryData.GetEntry(nQENext).bDoQuery || maRefreshExceptQuery[nQENext])
                        maConnLbArr[i+1]->SelectEntryPos( (sal_uInt16) theQueryData.GetEntry(nQENext).eConnect );
                    else
                        maConnLbArr[i+1]->SetNoSelection();
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->Enable();
                else
                    maConnLbArr[i]->Disable();

                if (maRefreshExceptQuery.size() < nQE + 1)
                    maRefreshExceptQuery.resize(nQE + 1, false);
                if(rEntry.bDoQuery || maRefreshExceptQuery[nQE])
                    maConnLbArr[i]->SelectEntryPos( (sal_uInt16) rEntry.eConnect );
                else
                    maConnLbArr[i]->SetNoSelection();
            }

        }
        else
        {
            if (nOffset==0)
            {
                if(i<3)
                {
                    maConnLbArr[i+1]->SetNoSelection();
                    maConnLbArr[i+1]->Disable();
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->Enable();
                else
                    maConnLbArr[i]->Disable();
                maConnLbArr[i]->SetNoSelection();
            }
            maFieldLbArr[i]->Disable();
            maCondLbArr[i]->Disable();
            maValueEdArr[i]->Disable();
        }
        maFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
        maCondLbArr [i]->SelectEntryPos( nCondPos );
        maValueEdArr[i]->SetText( aValStr );
        UpdateValueList(i+1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
