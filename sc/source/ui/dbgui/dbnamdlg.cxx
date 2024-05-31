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

#include <memory>
#include <sal/config.h>

#include <cassert>

#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <o3tl/string_view.hxx>

#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <rangenam.hxx>
#include <globalnames.hxx>
#include <dbnamdlg.hxx>
#include <dbdocfun.hxx>

namespace
{
    void ERRORBOX(weld::Window* pParent, const OUString& rString)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  rString));
        xBox->run();
    }


class DBSaveData
{
public:
    DBSaveData( formula::RefEdit& rEd, weld::CheckButton& rHdr, weld::CheckButton& rTot, weld::CheckButton& rSize, weld::CheckButton& rFmt,
                            weld::CheckButton& rStrip, ScRange& rArea )
        : rEdAssign(rEd)
        , rBtnHeader(rHdr)
        , rBtnTotals(rTot)
        , rBtnSize(rSize)
        , rBtnFormat(rFmt)
        , rBtnStrip(rStrip)
        , rCurArea(rArea)
        , bHeader(false)
        , bTotals(false)
        , bSize(false)
        , bFormat(false)
        , bStrip(false)
        , bDirty(false)
    {
    }
    void Save();
    void Restore();

private:
    formula::RefEdit& rEdAssign;
    weld::CheckButton& rBtnHeader;
    weld::CheckButton& rBtnTotals;
    weld::CheckButton& rBtnSize;
    weld::CheckButton& rBtnFormat;
    weld::CheckButton& rBtnStrip;
    ScRange&    rCurArea;
    OUString    aStr;
    ScRange     aArea;
    bool        bHeader:1;
    bool        bTotals:1;
    bool        bSize:1;
    bool        bFormat:1;
    bool        bStrip:1;
    bool        bDirty:1;
};

}

void DBSaveData::Save()
{
    aArea   = rCurArea;
    aStr    = rEdAssign.GetText();
    bHeader = rBtnHeader.get_active();
    bTotals = rBtnTotals.get_active();
    bSize   = rBtnSize.get_active();
    bFormat = rBtnFormat.get_active();
    bStrip  = rBtnStrip.get_active();
    bDirty  = true;
}

void DBSaveData::Restore()
{
    if ( bDirty )
    {
        rCurArea = aArea;
        rEdAssign.SetText( aStr );
        rBtnHeader.set_active ( bHeader );
        rBtnTotals.set_active ( bTotals );
        rBtnSize.set_active   ( bSize );
        rBtnFormat.set_active ( bFormat );
        rBtnStrip.set_active  ( bStrip );
        bDirty = false;
    }
}

static std::unique_ptr<DBSaveData> xSaveObj;

ScDbNameDlg::ScDbNameDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
    ScViewData& rViewData)
    : ScAnyRefDlgController(pB, pCW, pParent,
        u"modules/scalc/ui/definedatabaserangedialog.ui"_ustr, u"DefineDatabaseRangeDialog"_ustr)
    , m_rViewData(rViewData)
    , rDoc(rViewData.GetDocument())
    , bRefInputMode(false)
    , aAddrDetails(rDoc.GetAddressConvention(), 0, 0)
    , aLocalDbCol(*(rDoc.GetDBCollection()))
    , m_xEdName(m_xBuilder->weld_entry_tree_view(u"entrygrid"_ustr, u"entry"_ustr, u"entry-list"_ustr))
    , m_xAssignFrame(m_xBuilder->weld_frame(u"RangeFrame"_ustr))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry(u"assign"_ustr)))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button(u"assignrb"_ustr)))
    , m_xOptions(m_xBuilder->weld_widget(u"Options"_ustr))
    , m_xBtnHeader(m_xBuilder->weld_check_button(u"ContainsColumnLabels"_ustr))
    , m_xBtnTotals(m_xBuilder->weld_check_button(u"ContainsTotalsRow"_ustr))
    , m_xBtnDoSize(m_xBuilder->weld_check_button(u"InsertOrDeleteCells"_ustr))
    , m_xBtnKeepFmt(m_xBuilder->weld_check_button(u"KeepFormatting"_ustr))
    , m_xBtnStripData(m_xBuilder->weld_check_button(u"DontSaveImportedData"_ustr))
    , m_xFTSource(m_xBuilder->weld_label(u"Source"_ustr))
    , m_xFTOperations(m_xBuilder->weld_label(u"Operations"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xBtnRemove(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xModifyPB(m_xBuilder->weld_button(u"modify"_ustr))
    , m_xInvalidFT(m_xBuilder->weld_label(u"invalid"_ustr))
    , m_xFrameLabel(m_xAssignFrame->weld_label_widget())
    , m_xExpander(m_xBuilder->weld_expander("more"))
{
    m_xEdName->set_height_request_by_rows(4);
    m_xEdAssign->SetReferences(this, m_xFrameLabel.get());
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());
    aStrAdd = m_xBtnAdd->get_label();
    aStrModify = m_xModifyPB->get_label();
    aStrInvalid = m_xInvalidFT->get_label();

    //  so that the strings in the resource can stay with fixed texts:
    aStrSource      = m_xFTSource->get_label();
    aStrOperations  = m_xFTOperations->get_label();

    xSaveObj.reset(new DBSaveData( *m_xEdAssign, *m_xBtnHeader, *m_xBtnTotals,
                        *m_xBtnDoSize, *m_xBtnKeepFmt, *m_xBtnStripData, theCurArea ));
    Init();
}

ScDbNameDlg::~ScDbNameDlg()
{
    xSaveObj.reset();
}

void ScDbNameDlg::Init()
{
    m_xBtnHeader->set_active(true);          // Default: with column headers
    m_xBtnTotals->set_active( false );   // Default: without totals row
    m_xBtnDoSize->set_active(true);
    m_xBtnKeepFmt->set_active(true);

    m_xBtnOk->connect_clicked      ( LINK( this, ScDbNameDlg, OkBtnHdl ) );
    m_xBtnCancel->connect_clicked  ( LINK( this, ScDbNameDlg, CancelBtnHdl ) );
    m_xBtnAdd->connect_clicked     ( LINK( this, ScDbNameDlg, AddBtnHdl ) );
    m_xBtnRemove->connect_clicked  ( LINK( this, ScDbNameDlg, RemoveBtnHdl ) );
    m_xEdName->connect_changed( LINK( this, ScDbNameDlg, NameModifyHdl ) );
    m_xEdAssign->SetModifyHdl  ( LINK( this, ScDbNameDlg, AssModifyHdl ) );
    UpdateNames();

    OUString  theAreaStr;

    SCCOL   nStartCol   = 0;
    SCROW   nStartRow   = 0;
    SCTAB   nStartTab   = 0;
    SCCOL   nEndCol     = 0;
    SCROW   nEndRow     = 0;
    SCTAB   nEndTab     = 0;

    ScDBCollection* pDBColl = rDoc.GetDBCollection();

    m_rViewData.GetSimpleArea( nStartCol, nStartRow, nStartTab,
                              nEndCol,   nEndRow,  nEndTab );

    theCurArea = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);

    theAreaStr = theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aAddrDetails);

    if ( pDBColl )
    {
        // determine if the defined DB area has been marked:
        ScDBData* pDBData = pDBColl->GetDBAtCursor( nStartCol, nStartRow, nStartTab, ScDBDataPortion::TOP_LEFT );
        if ( pDBData )
        {
            ScAddress&  rStart = theCurArea.aStart;
            ScAddress&  rEnd   = theCurArea.aEnd;
            SCCOL nCol1;
            SCCOL  nCol2;
            SCROW  nRow1;
            SCROW  nRow2;
            SCTAB  nTab;

            pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

            if (   (rStart.Tab() == nTab)
                && (rStart.Col() == nCol1) && (rStart.Row() == nRow1)
                && (rEnd.Col()   == nCol2) && (rEnd.Row()   == nRow2 ) )
            {
                OUString aDBName = pDBData->GetName();
                if ( aDBName != STR_DB_LOCAL_NONAME )
                    m_xEdName->set_entry_text(aDBName);

                m_xBtnHeader->set_active( pDBData->HasHeader() );
                m_xBtnTotals->set_active( pDBData->HasTotals() );
                m_xBtnDoSize->set_active( pDBData->IsDoSize() );
                m_xBtnKeepFmt->set_active( pDBData->IsKeepFmt() );
                m_xBtnStripData->set_active( pDBData->IsStripData() );
                SetInfoStrings( pDBData );
            }
        }
    }

    m_xEdAssign->SetText( theAreaStr );
    m_xEdName->grab_focus();
    bSaved = true;
    xSaveObj->Save();
    NameModifyHdl( *m_xEdName );
    bInvalid = false;
}

void ScDbNameDlg::SetInfoStrings( const ScDBData* pDBData )
{
    OUStringBuffer aBuf(aStrSource);
    if (pDBData)
    {
        aBuf.append(" " + pDBData->GetSourceString());
    }
    m_xFTSource->set_label(aBuf.makeStringAndClear());

    aBuf.append(aStrOperations);
    if (pDBData)
    {
        aBuf.append(" " + pDBData->GetOperations());
    }
    m_xFTOperations->set_label(aBuf.makeStringAndClear());
}

// Transfer of a table area selected with the mouse, which is then displayed
// as a new selection in the reference window.

void ScDbNameDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (!m_xEdAssign->GetWidget()->get_sensitive())
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(m_xEdAssign.get());

    theCurArea = rRef;

    OUString aRefStr(theCurArea.Format(rDocP, ScRefFlags::RANGE_ABS_3D, aAddrDetails));
    m_xEdAssign->SetRefString( aRefStr );
    m_xOptions->set_sensitive(true);
    m_xBtnAdd->set_sensitive(true);
    bSaved = true;
    xSaveObj->Save();
}

void ScDbNameDlg::Close()
{
    DoClose( ScDbNameDlgWrapper::GetChildWindowId() );
}

void ScDbNameDlg::SetActive()
{
    m_xEdAssign->GrabFocus();

    //  No NameModifyHdl, because otherwise areas can not be changed
    //  (the old content would be displayed again after the reference selection is pulled)
    //  (the selected DB name has not changed either)

    RefInputDone();
}

void ScDbNameDlg::UpdateNames()
{
    typedef ScDBCollection::NamedDBs DBsType;

    const DBsType& rDBs = aLocalDbCol.getNamedDBs();

    m_xEdName->freeze();

    m_xEdName->clear();
    m_xEdAssign->SetText( OUString() );

    if (!rDBs.empty())
    {
        for (const auto& rxDB : rDBs)
            m_xEdName->append_text(rxDB->GetName());
    }
    else
    {
        m_xBtnAdd->set_label( aStrAdd );
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(false);
    }

    m_xEdName->thaw();
}

void ScDbNameDlg::UpdateDBData( const OUString& rStrName )
{

    const ScDBData* pData = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rStrName));

    if ( pData )
    {
        SCCOL nColStart = 0;
        SCROW nRowStart = 0;
        SCCOL nColEnd    = 0;
        SCROW nRowEnd    = 0;
        SCTAB nTab       = 0;

        pData->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
        theCurArea = ScRange( ScAddress( nColStart, nRowStart, nTab ),
                              ScAddress( nColEnd,   nRowEnd,   nTab ) );
        OUString theArea(theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aAddrDetails));
        m_xEdAssign->SetText( theArea );
        m_xBtnAdd->set_label( aStrModify );
        m_xBtnHeader->set_active( pData->HasHeader() );
        m_xBtnTotals->set_active( pData->HasTotals() );
        m_xBtnDoSize->set_active( pData->IsDoSize() );
        m_xBtnKeepFmt->set_active( pData->IsKeepFmt() );
        m_xBtnStripData->set_active( pData->IsStripData() );
        SetInfoStrings( pData );
    }

    m_xBtnAdd->set_label( aStrModify );
    m_xBtnAdd->set_sensitive(true);
    m_xBtnRemove->set_sensitive(true);
    m_xOptions->set_sensitive(true);
}

bool ScDbNameDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

// Handler:

IMPL_LINK_NOARG(ScDbNameDlg, OkBtnHdl, weld::Button&, void)
{
    bInvalid = false;
    AddBtnHdl(*m_xBtnAdd);

    // Pass the changes and the remove list to the view: both are
    // transferred as a reference only, so that no dead memory can
    // be created at this point:
    if (!bInvalid)
    {
        ScDBDocFunc aFunc(*m_rViewData.GetDocShell());
        aFunc.ModifyAllDBData(aLocalDbCol, aRemoveList);
        response(RET_OK);
    }
}

IMPL_LINK_NOARG(ScDbNameDlg, CancelBtnHdl, weld::Button&, void)
{
    response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScDbNameDlg, AddBtnHdl, weld::Button&, void)
{
    OUString aNewName = comphelper::string::strip(m_xEdName->get_active_text(), ' ');
    OUString aNewArea = m_xEdAssign->GetText();

    if ( aNewName.isEmpty() || aNewArea.isEmpty() )
        return;

    if (ScRangeData::IsNameValid(aNewName, rDoc) == ScRangeData::IsNameValidType::NAME_VALID
        && aNewName != STR_DB_LOCAL_NONAME)
    {
        //  because editing can be done now, parsing is needed first
        ScRange aTmpRange;
        OUString aText = m_xEdAssign->GetText();
        if ( aTmpRange.ParseAny( aText, rDoc, aAddrDetails ) & ScRefFlags::VALID )
        {
            theCurArea = aTmpRange;
            ScAddress aStart = theCurArea.aStart;
            ScAddress aEnd   = theCurArea.aEnd;

            ScDBData* pOldEntry = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(aNewName));
            if (pOldEntry)
            {
                //  modify area

                pOldEntry->MoveTo( aStart.Tab(), aStart.Col(), aStart.Row(),
                                                    aEnd.Col(), aEnd.Row() );
                pOldEntry->SetByRow( true );
                pOldEntry->SetHeader( m_xBtnHeader->get_active() );
                pOldEntry->SetTotals( m_xBtnTotals->get_active() );
                pOldEntry->SetDoSize( m_xBtnDoSize->get_active() );
                pOldEntry->SetKeepFmt( m_xBtnKeepFmt->get_active() );
                pOldEntry->SetStripData( m_xBtnStripData->get_active() );
            }
            else
            {
                //  insert new area

                std::unique_ptr<ScDBData> pNewEntry(new ScDBData( aNewName, aStart.Tab(),
                                                    aStart.Col(), aStart.Row(),
                                                    aEnd.Col(), aEnd.Row(),
                                                    true, m_xBtnHeader->get_active(),
                                                    m_xBtnTotals->get_active() ));
                pNewEntry->SetDoSize( m_xBtnDoSize->get_active() );
                pNewEntry->SetKeepFmt( m_xBtnKeepFmt->get_active() );
                pNewEntry->SetStripData( m_xBtnStripData->get_active() );

                bool ins = aLocalDbCol.getNamedDBs().insert(std::move(pNewEntry));
                assert(ins); (void)ins;
            }

            UpdateNames();

            m_xEdName->set_entry_text( OUString() );
            m_xEdName->grab_focus();
            m_xBtnAdd->set_label( aStrAdd );
            m_xBtnAdd->set_sensitive(false);
            m_xBtnRemove->set_sensitive(false);
            m_xEdAssign->SetText( OUString() );
            m_xBtnHeader->set_active(true);             // Default: with column headers
            m_xBtnTotals->set_active( false );      // Default: without totals row
            m_xBtnDoSize->set_active( false );
            m_xBtnKeepFmt->set_active( false );
            m_xBtnStripData->set_active( false );
            SetInfoStrings( nullptr );     // empty
            theCurArea = ScRange();
            bSaved = true;
            xSaveObj->Save();
            NameModifyHdl( *m_xEdName );
        }
        else
        {
            ERRORBOX(m_xDialog.get(), aStrInvalid);
            m_xEdAssign->SelectAll();
            m_xEdAssign->GrabFocus();
            bInvalid = true;
        }
    }
    else
    {
        ERRORBOX(m_xDialog.get(), ScResId(STR_INVALIDNAME));
        m_xEdName->select_entry_region(0, -1);
        m_xEdName->grab_focus();
        bInvalid = true;
    }
}

namespace {

class FindByName
{
    const OUString& mrName;
public:
    explicit FindByName(const OUString& rName) : mrName(rName) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetName() == mrName;
    }
};

}

IMPL_LINK_NOARG(ScDbNameDlg, RemoveBtnHdl, weld::Button&, void)
{
    OUString aStrEntry = m_xEdName->get_active_text();
    ScDBCollection::NamedDBs& rDBs = aLocalDbCol.getNamedDBs();
    ScDBCollection::NamedDBs::iterator itr =
        ::std::find_if(rDBs.begin(), rDBs.end(), FindByName(aStrEntry));

    if (itr == rDBs.end())
        return;

    OUString aStrDelMsg = ScResId( STR_QUERY_DELENTRY );
    OUString sMsg{ o3tl::getToken(aStrDelMsg, 0, '#') + aStrEntry + o3tl::getToken(aStrDelMsg, 1, '#') };
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   sMsg));
    xQueryBox->set_default_response(RET_YES);
    if (RET_YES != xQueryBox->run())
        return;

    SCTAB nTab;
    SCCOL nColStart, nColEnd;
    SCROW nRowStart, nRowEnd;
    (*itr)->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
    aRemoveList.emplace_back( ScAddress( nColStart, nRowStart, nTab ),
                 ScAddress( nColEnd,   nRowEnd,   nTab ) );

    rDBs.erase(itr);

    UpdateNames();

    m_xEdName->set_entry_text( OUString() );
    m_xEdName->grab_focus();
    m_xBtnAdd->set_label( aStrAdd );
    m_xBtnAdd->set_sensitive(false);
    m_xBtnRemove->set_sensitive(false);
    m_xEdAssign->SetText( OUString() );
    theCurArea = ScRange();
    m_xBtnHeader->set_active(true);             // Default: with column headers
    m_xBtnTotals->set_active( false );      // Default: without totals row
    m_xBtnDoSize->set_active( false );
    m_xBtnKeepFmt->set_active( false );
    m_xBtnStripData->set_active( false );
    SetInfoStrings( nullptr );     // empty
    bSaved=false;
    xSaveObj->Restore();
    NameModifyHdl( *m_xEdName );
}

IMPL_LINK_NOARG(ScDbNameDlg, NameModifyHdl, weld::ComboBox&, void)
{
    OUString  theName     = m_xEdName->get_active_text();
    bool    bNameFound  = m_xEdName->find_text(theName) != -1;

    if ( theName.isEmpty() )
    {
        if (m_xBtnAdd->get_label() != aStrAdd)
            m_xBtnAdd->set_label( aStrAdd );
        m_xBtnAdd->set_sensitive(false);
        m_xBtnRemove->set_sensitive(false);
        m_xAssignFrame->set_sensitive(false);
        m_xOptions->set_sensitive(false);
        //bSaved=sal_False;
        //xSaveObj->Restore();
        //@BugID 54702 enable/disable in the base class only
        //SFX_APPWINDOW->Disable(sal_False);        //! general method in ScAnyRefDlg
        bRefInputMode = false;
    }
    else
    {
        if ( bNameFound )
        {
            if (m_xBtnAdd->get_label() != aStrModify)
                m_xBtnAdd->set_label( aStrModify );

            if(!bSaved)
            {
                bSaved = true;
                xSaveObj->Save();
            }
            UpdateDBData( theName );
        }
        else
        {
            if (m_xBtnAdd->get_label() != aStrAdd)
                m_xBtnAdd->set_label( aStrAdd );

            bSaved=false;
            xSaveObj->Restore();

            if ( !m_xEdAssign->GetText().isEmpty() )
            {
                m_xBtnAdd->set_sensitive(true);
                m_xOptions->set_sensitive(true);
            }
            else
            {
                m_xBtnAdd->set_sensitive(false);
                m_xOptions->set_sensitive(false);
            }
            m_xBtnRemove->set_sensitive(false);
        }

        m_xAssignFrame->set_sensitive(true);

        //@BugID 54702 enable/disable in the base class only
        //SFX_APPWINDOW->set_sensitive(true);
        bRefInputMode = true;
    }
}

IMPL_LINK_NOARG(ScDbNameDlg, AssModifyHdl, formula::RefEdit&, void)
{
    //  parse here for Save(), etc.

    ScRange aTmpRange;
    OUString aText = m_xEdAssign->GetText();
    if ( aTmpRange.ParseAny( aText, rDoc, aAddrDetails ) & ScRefFlags::VALID )
        theCurArea = aTmpRange;

    if (!aText.isEmpty() && !m_xEdName->get_active_text().isEmpty())
    {
        m_xBtnAdd->set_sensitive(true);
        m_xBtnHeader->set_sensitive(true);
        m_xBtnTotals->set_sensitive(true);
        m_xBtnDoSize->set_sensitive(true);
        m_xBtnKeepFmt->set_sensitive(true);
        m_xBtnStripData->set_sensitive(true);
        m_xFTSource->set_sensitive(true);
        m_xFTOperations->set_sensitive(true);
    }
    else
    {
        m_xBtnAdd->set_sensitive(false);
        m_xBtnHeader->set_sensitive(false);
        m_xBtnTotals->set_sensitive(false);
        m_xBtnDoSize->set_sensitive(false);
        m_xBtnKeepFmt->set_sensitive(false);
        m_xBtnStripData->set_sensitive(false);
        m_xFTSource->set_sensitive(false);
        m_xFTOperations->set_sensitive(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
