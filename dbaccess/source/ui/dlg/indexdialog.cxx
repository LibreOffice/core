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

#include <sal/config.h>

#include <set>

#include <core_resource.hxx>
#include <indexdialog.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <indexfieldscontrol.hxx>
#include <indexcollection.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <UITools.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/treelistentry.hxx>
#include <browserids.hxx>
#include <connectivity/dbtools.hxx>
#include <osl/diagnose.h>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::dbtools;

    // helper
    static bool operator ==(const OIndexField& _rLHS, const OIndexField& _rRHS)
    {
        return  (_rLHS.sFieldName == _rRHS.sFieldName)
            &&  (_rLHS.bSortAscending == _rRHS.bSortAscending);
    }

    static bool operator ==(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        return std::equal(_rLHS.begin(), _rLHS.end(), _rRHS.begin(), _rRHS.end());
    }

    static bool operator !=(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        return !(_rLHS == _rRHS);
    }

    // DbaIndexList
    DbaIndexList::DbaIndexList(vcl::Window* _pParent, WinBits nWinBits)
        :SvTreeListBox(_pParent, nWinBits)
        ,m_bSuspendSelectHdl(false)
    {
    }

    bool DbaIndexList::EditedEntry( SvTreeListEntry* _pEntry, const OUString& _rNewText )
    {
        // first check if this is valid SQL92 name
        if ( isSQL92CheckEnabled(m_xConnection) )
        {
            Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
            if ( xMeta.is() )
            {
                OUString sAlias = ::dbtools::convertName2SQLName(_rNewText, xMeta->getExtraNameCharacters());
                if ( ( xMeta->supportsMixedCaseQuotedIdentifiers() )
                        ?
                        sAlias != _rNewText
                        :
                !_rNewText.equalsIgnoreAsciiCase(sAlias))
                    return false;
            }
        }

        if (!SvTreeListBox::EditedEntry(_pEntry, _rNewText))
            return false;

        OUString sOldText = GetEntryText(_pEntry);
        SvTreeListBox::SetEntryText(_pEntry, _rNewText);

        bool bValid = true;
        if (m_aEndEditHdl.IsSet())
            bValid = m_aEndEditHdl.Call(_pEntry);

        if (bValid)
            return true;

        SvTreeListBox::SetEntryText(_pEntry, sOldText);

        return false;
    }

    void DbaIndexList::enableSelectHandler()
    {
        OSL_ENSURE(m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = false;
    }

    void DbaIndexList::disableSelectHandler()
    {
        OSL_ENSURE(!m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = true;
    }

    void DbaIndexList::SelectNoHandlerCall( SvTreeListEntry* _pEntry )
    {
        disableSelectHandler();
        Select(_pEntry );
        enableSelectHandler();
    }

    bool DbaIndexList::Select(SvTreeListEntry* pEntry, bool _bSelect)
    {
        bool bReturn = SvTreeListBox::Select(pEntry, _bSelect);

        if (!m_bSuspendSelectHdl && _bSelect)
            m_aSelectHdl.Call(*this);

        return bReturn;
    }

    VCL_BUILDER_FACTORY_ARGS(DbaIndexList, WB_BORDER)

    // DbaIndexDialog
    DbaIndexDialog::DbaIndexDialog( vcl::Window* _pParent, const Sequence< OUString >& _rFieldNames,
                                    const Reference< XNameAccess >& _rxIndexes,
                                    const Reference< XConnection >& _rxConnection,
                                    const Reference< XComponentContext >& _rxContext)
        :ModalDialog( _pParent, "IndexDesignDialog", "dbaccess/ui/indexdesigndialog.ui")
        ,m_xConnection(_rxConnection)
        ,m_pPreviousSelection(nullptr)
        ,m_bEditAgain(false)
        ,m_xContext(_rxContext)
    {
        get(m_pActions, "ACTIONS");

        mnNewCmdId = m_pActions->GetItemId(".index:createNew");
        mnDropCmdId = m_pActions->GetItemId(".index:dropCurrent");
        mnRenameCmdId = m_pActions->GetItemId(".index:renameCurrent");
        mnSaveCmdId = m_pActions->GetItemId(".index:saveCurrent");
        mnResetCmdId = m_pActions->GetItemId(".index:resetCurrent");

        maScNewCmdImg = m_pActions->GetItemImage(mnNewCmdId);
        maScDropCmdImg = m_pActions->GetItemImage(mnDropCmdId);
        maScRenameCmdImg = m_pActions->GetItemImage(mnRenameCmdId);
        maScSaveCmdImg = m_pActions->GetItemImage(mnSaveCmdId);
        maScResetCmdImg = m_pActions->GetItemImage(mnResetCmdId);
        maLcNewCmdImg = get<FixedImage>("image1")->GetImage();
        maLcDropCmdImg = get<FixedImage>("image2")->GetImage();
        maLcRenameCmdImg = get<FixedImage>("image3")->GetImage();
        maLcSaveCmdImg = get<FixedImage>("image4")->GetImage();
        maLcResetCmdImg = get<FixedImage>("image5")->GetImage();

        get(m_pIndexList, "INDEX_LIST");
        Size aSize(LogicToPixel(Size(70, 97), MapMode(MapUnit::MapAppFont)));
        m_pIndexList->set_width_request(aSize.Width());
        m_pIndexList->set_height_request(aSize.Height());
        get(m_pIndexDetails, "INDEX_DETAILS");
        get(m_pDescriptionLabel, "DESC_LABEL");
        get(m_pDescription, "DESCRIPTION");
        get(m_pUnique, "UNIQUE");
        get(m_pFieldsLabel, "FIELDS_LABEL");
        get(m_pFields, "FIELDS");
        aSize = LogicToPixel(Size(128, 61), MapMode(MapUnit::MapAppFont));
        m_pFields->set_width_request(aSize.Width());
        m_pFields->set_height_request(aSize.Height());
        get(m_pClose, "close");

        m_pActions->SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexAction));

        m_pIndexList->SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexSelected));
        m_pIndexList->SetEndEditHdl(LINK(this, DbaIndexDialog, OnEntryEdited));
        m_pIndexList->SetSelectionMode(SelectionMode::Single);
        m_pIndexList->SetHighlightRange();
        m_pIndexList->setConnection(m_xConnection);

        m_pFields->SetSizePixel(Size(300, 100));
        m_pFields->Init(_rFieldNames, ::dbtools::getBooleanDataSourceSetting( m_xConnection, "AddIndexAppendix" ));

        setToolBox(m_pActions);

        m_pIndexes.reset(new OIndexCollection());
        try
        {
            m_pIndexes->attach(_rxIndexes);
        }
        catch(SQLException& e)
        {
            ::dbtools::showError(SQLExceptionInfo(e),VCLUnoHelper::GetInterface(_pParent),_rxContext);
        }
        catch(Exception&)
        {
            OSL_FAIL("DbaIndexDialog::DbaIndexDialog: could not retrieve basic information from the UNO collection!");
        }

        fillIndexList();

        m_pUnique->SetClickHdl(LINK(this, DbaIndexDialog, OnModifiedClick));
        m_pFields->SetModifyHdl(LINK(this, DbaIndexDialog, OnModified));

        m_pClose->SetClickHdl(LINK(this, DbaIndexDialog, OnCloseDialog));

        // if all of the indexes have an empty description, we're not interested in displaying it
        bool bFound = false;
        for (auto const& check : *m_pIndexes)
        {
            if (!check.sDescription.isEmpty())
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            // hide the controls which are necessary for the description
            m_pDescription->Hide();
            m_pDescriptionLabel->Hide();
        }
    }

    void DbaIndexDialog::updateToolbox()
    {
        m_pActions->EnableItem(mnNewCmdId, !m_pIndexList->IsEditingActive());

        SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        bool bSelectedAnything = nullptr != pSelected;

        if (pSelected)
        {
            // is the current entry modified?
            Indexes::const_iterator aSelectedPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());
            m_pActions->EnableItem(mnSaveCmdId, aSelectedPos->isModified() || aSelectedPos->isNew());
            m_pActions->EnableItem(mnResetCmdId, aSelectedPos->isModified() || aSelectedPos->isNew());
            bSelectedAnything = bSelectedAnything && !aSelectedPos->bPrimaryKey;
        }
        else
        {
            m_pActions->EnableItem(mnSaveCmdId, false);
            m_pActions->EnableItem(mnResetCmdId, false);
        }
        m_pActions->EnableItem(mnDropCmdId, bSelectedAnything);
        m_pActions->EnableItem(mnRenameCmdId, bSelectedAnything);
    }

    void DbaIndexDialog::fillIndexList()
    {
        Image aPKeyIcon(StockImage::Yes, BMP_PKEYICON);
        // fill the list with the index names
        m_pIndexList->Clear();
        sal_Int32 nPos = 0;
        for (auto const& indexLoop : *m_pIndexes)
        {
            SvTreeListEntry* pNewEntry = nullptr;
            if (indexLoop.bPrimaryKey)
                pNewEntry = m_pIndexList->InsertEntry(indexLoop.sName, aPKeyIcon, aPKeyIcon);
            else
                pNewEntry = m_pIndexList->InsertEntry(indexLoop.sName);

            pNewEntry->SetUserData(reinterpret_cast< void* >(nPos));
            ++nPos;
        }

        OnIndexSelected(*m_pIndexList);
    }

    DbaIndexDialog::~DbaIndexDialog( )
    {
        disposeOnce();
    }

    void DbaIndexDialog::dispose()
    {
        setToolBox(nullptr);
        m_pIndexes.reset();
        m_pActions.clear();
        m_pIndexList.clear();
        m_pIndexDetails.clear();
        m_pDescriptionLabel.clear();
        m_pDescription.clear();
        m_pUnique.clear();
        m_pFieldsLabel.clear();
        m_pFields.clear();
        m_pClose.clear();
        ModalDialog::dispose();
    }

    bool DbaIndexDialog::implCommit(SvTreeListEntry const * _pEntry)
    {
        OSL_ENSURE(_pEntry, "DbaIndexDialog::implCommit: invalid entry!");

        Indexes::iterator aCommitPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

        // if it's not a new index, remove it
        // (we can't modify indexes, only drop'n'insert)
        if (!aCommitPos->isNew())
            if (!implDropIndex(_pEntry, false))
                return false;

        // create the new index
        SQLExceptionInfo aExceptionInfo;
        try
        {
            m_pIndexes->commitNewIndex(aCommitPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        // reflect the new selection in the toolbox
        updateToolbox();

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, VCLUnoHelper::GetInterface(this), m_xContext);
        else
        {
            m_pUnique->SaveValue();
            m_pFields->SaveValue();
        }

        return !aExceptionInfo.isValid();
    }

    void DbaIndexDialog::OnNewIndex()
    {
        // commit the current entry, if necessary
        if (!implCommitPreviouslySelected())
            return;

        // get a new unique name for the new index
        OUString sNewIndexName;
        const OUString sNewIndexNameBase(DBA_RES(STR_LOGICAL_INDEX_NAME));
        sal_Int32 i;

        for ( i = 1; i < 0x7FFFFFFF; ++i )
        {
            sNewIndexName = sNewIndexNameBase + OUString::number(i);
            if (m_pIndexes->end() == m_pIndexes->find(sNewIndexName))
                break;
        }
        if (i == 0x7FFFFFFF)
        {
            OSL_FAIL("DbaIndexDialog::OnNewIndex: no free index name found!");
            // can't do anything ... of course we try another base, but this could end with the same result ...
            return;
        }

        SvTreeListEntry* pNewEntry = m_pIndexList->InsertEntry(sNewIndexName);
        m_pIndexes->insert(sNewIndexName);

        // update the user data on the entries in the list box:
        // they're iterators of the index collection, and thus they have changed when removing the index
        for (SvTreeListEntry* pAdjust = m_pIndexList->First(); pAdjust; pAdjust = m_pIndexList->Next(pAdjust))
        {
            Indexes::const_iterator aAfterInsertPos = m_pIndexes->find(m_pIndexList->GetEntryText(pAdjust));
            OSL_ENSURE(aAfterInsertPos != m_pIndexes->end(), "DbaIndexDialog::OnNewIndex: problems with one of the entries!");
            pAdjust->SetUserData(reinterpret_cast< void* >(sal_Int32(aAfterInsertPos - m_pIndexes->begin())));
        }

        // select the entry and start in-place editing
        m_pIndexList->SelectNoHandlerCall(pNewEntry);
        OnIndexSelected(*m_pIndexList);
        m_pIndexList->EditEntry(pNewEntry);
        updateToolbox();
    }

    void DbaIndexDialog::OnDropIndex(bool _bConfirm)
    {
        // the selected index
        SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnDropIndex: invalid call!");
        if (pSelected)
        {
            // let the user confirm the drop
            if (_bConfirm)
            {
                OUString sConfirm(DBA_RES(STR_CONFIRM_DROP_INDEX));
                sConfirm = sConfirm.replaceFirst("$name$", m_pIndexList->GetEntryText(pSelected));
                std::unique_ptr<weld::MessageDialog> xConfirm(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Question, VclButtonsType::YesNo,
                                                              sConfirm));
                if (RET_YES != xConfirm->run())
                    return;
            }

            // do the drop
            implDropIndex(pSelected, true);

            // reflect the new selection in the toolbox
            updateToolbox();
        }
    }

    bool DbaIndexDialog::implDropIndex(SvTreeListEntry const * _pEntry, bool _bRemoveFromCollection)
    {
        // do the drop
        Indexes::iterator aDropPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());
        OSL_ENSURE(aDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: did not find the index in my collection!");

        SQLExceptionInfo aExceptionInfo;
        bool bSuccess = false;
        try
        {
            if (_bRemoveFromCollection)
                bSuccess = m_pIndexes->drop(aDropPos);
            else
                bSuccess = m_pIndexes->dropNoRemove(aDropPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, VCLUnoHelper::GetInterface(this), m_xContext);
        else if (bSuccess && _bRemoveFromCollection)
        {
            SvTreeList* pModel = m_pIndexList->GetModel();

            m_pIndexList->disableSelectHandler();
            pModel->Remove(_pEntry);
            m_pIndexList->enableSelectHandler();

            // update the user data on the entries in the list box:
            // they're iterators of the index collection, and thus they have changed when removing the index
            for (SvTreeListEntry* pAdjust = m_pIndexList->First(); pAdjust; pAdjust = m_pIndexList->Next(pAdjust))
            {
                Indexes::const_iterator aAfterDropPos = m_pIndexes->find(m_pIndexList->GetEntryText(pAdjust));
                OSL_ENSURE(aAfterDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: problems with one of the remaining entries!");
                pAdjust->SetUserData(reinterpret_cast< void* >(sal_Int32(aAfterDropPos - m_pIndexes->begin())));
            }

            // if the removed entry was the selected on...
            if (m_pPreviousSelection == _pEntry)
                m_pPreviousSelection = nullptr;

            // the Remove automatically selected another entry (if possible), but we disabled the calling of the handler
            // to prevent that we missed something... call the handler directly
            OnIndexSelected(*m_pIndexList);
        }

        return !aExceptionInfo.isValid();
    }

    void DbaIndexDialog::OnRenameIndex()
    {
        // the selected index
        SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnRenameIndex: invalid call!");

        // save the changes made 'til here
        // Upon leaving the edit mode, the control will be re-initialized with the
        // settings from the current entry
        implSaveModified(false);

        m_pIndexList->EditEntry(pSelected);
        updateToolbox();
    }

    void DbaIndexDialog::OnSaveIndex()
    {
        // the selected index
#if OSL_DEBUG_LEVEL > 0
        SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        OSL_ENSURE( pSelected, "DbaIndexDialog::OnSaveIndex: invalid call!" );
#endif

        implCommitPreviouslySelected();
        updateToolbox();
    }

    void DbaIndexDialog::OnResetIndex()
    {
        // the selected index
        SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnResetIndex: invalid call!");

        Indexes::iterator aResetPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());

        if (aResetPos->isNew())
        {
            OnDropIndex(false);
            return;
        }

        SQLExceptionInfo aExceptionInfo;
        try
        {
            m_pIndexes->resetIndex(aResetPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, VCLUnoHelper::GetInterface(this), m_xContext);
        else
            m_pIndexList->SetEntryText(pSelected, aResetPos->sName);

        updateControls(pSelected);
        updateToolbox();
    }

    IMPL_LINK_NOARG( DbaIndexDialog, OnIndexAction, ToolBox*, void )
    {
        sal_uInt16 nClicked = m_pActions->GetCurItemId();
        if (nClicked == mnNewCmdId)
            OnNewIndex();
        else if (nClicked == mnDropCmdId)
            OnDropIndex();
        else if (nClicked == mnRenameCmdId)
            OnRenameIndex();
        else if (nClicked == mnSaveCmdId)
            OnSaveIndex();
        else if (nClicked == mnResetCmdId)
            OnResetIndex();
    }

    IMPL_LINK_NOARG( DbaIndexDialog, OnCloseDialog, Button*, void )
    {
        if (m_pIndexList->IsEditingActive())
        {
            OSL_ENSURE(!m_bEditAgain, "DbaIndexDialog::OnCloseDialog: somebody was faster than hell!");
                // this means somebody entered a new name, which was invalid, which cause us to posted us an event,
                // and before the event arrived the user clicked onto "close". VERY fast, this user ....
            m_pIndexList->EndEditing();
            if (m_bEditAgain)
                // could not commit the new name (started a new - asynchronous - edit trial)
                return;
        }

        // the currently selected entry
        const SvTreeListEntry* pSelected = m_pIndexList->FirstSelected();
        OSL_ENSURE(pSelected == m_pPreviousSelection, "DbaIndexDialog::OnCloseDialog: inconsistence!");

        sal_Int32 nResponse = RET_NO;
        if (pSelected)
        {
            // the descriptor
            Indexes::const_iterator aSelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());

            if (aSelected->isModified() || aSelected->isNew())
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "dbaccess/ui/saveindexdialog.ui"));
                std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("SaveIndexDialog"));
                nResponse = xQuery->run();
            }
        }

        switch (nResponse)
        {
            case RET_YES:
                if (!implCommitPreviouslySelected())
                    return;
                break;
            case RET_NO:
                break;
            default:
                return;
        }

        EndDialog(RET_OK);
    }

    IMPL_LINK( DbaIndexDialog, OnEditIndexAgain, void*, p, void )
    {
        SvTreeListEntry* _pEntry = static_cast<SvTreeListEntry*>(p);
        m_bEditAgain = false;
        m_pIndexList->EditEntry(_pEntry);
    }

    IMPL_LINK( DbaIndexDialog, OnEntryEdited, SvTreeListEntry*, _pEntry, bool )
    {
        Indexes::iterator aPosition = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

        OSL_ENSURE(aPosition >= m_pIndexes->begin() && aPosition < m_pIndexes->end(),
            "DbaIndexDialog::OnEntryEdited: invalid entry!");

        OUString sNewName = m_pIndexList->GetEntryText(_pEntry);

        Indexes::const_iterator aSameName = m_pIndexes->find(sNewName);
        if ((aSameName != aPosition) && (m_pIndexes->end() != aSameName))
        {
            OUString sError(DBA_RES(STR_INDEX_NAME_ALREADY_USED));
            sError = sError.replaceFirst("$name$", sNewName);
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(GetFrameWeld(),
                                                        VclMessageType::Warning, VclButtonsType::Ok,
                                                        sError));
            xError->run();

            updateToolbox();
            m_bEditAgain = true;
            PostUserEvent(LINK(this, DbaIndexDialog, OnEditIndexAgain), _pEntry, true);
            return false;
        }

        aPosition->sName = sNewName;

        // rename can be done by a drop/insert combination only
        if (aPosition->isNew())
        {
            updateToolbox();
            // no commitment needed here ....
            return true;
        }

        if (aPosition->sName != aPosition->getOriginalName())
        {
            aPosition->setModified(true);
            updateToolbox();
        }

        return true;
    }

    bool DbaIndexDialog::implSaveModified(bool _bPlausibility)
    {
        if (m_pPreviousSelection)
        {
            // try to commit the previously selected index
            if (m_pFields->IsModified() && !m_pFields->SaveModified())
                return false;

            Indexes::iterator aPreviouslySelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

            // the unique flag
            aPreviouslySelected->bUnique = m_pUnique->IsChecked();
            if (m_pUnique->GetSavedValue() != m_pUnique->GetState())
                aPreviouslySelected->setModified(true);

            // the fields
            m_pFields->commitTo(aPreviouslySelected->aFields);
            if (m_pFields->GetSavedValue() != aPreviouslySelected->aFields)
                aPreviouslySelected->setModified(true);

            // plausibility checks
            if (_bPlausibility && !implCheckPlausibility(aPreviouslySelected))
                return false;
        }

        return true;
    }

    bool DbaIndexDialog::implCheckPlausibility(const Indexes::const_iterator& _rPos)
    {
        // need at least one field
        if (_rPos->aFields.empty())
        {
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(GetFrameWeld(),
                                                        VclMessageType::Warning, VclButtonsType::Ok,
                                                        DBA_RES(STR_NEED_INDEX_FIELDS)));
            xError->run();
            m_pFields->GrabFocus();
            return false;
        }

        // no double fields
        std::set< OUString > aExistentFields;
        for (auto const& fieldCheck : _rPos->aFields)
        {
            if (aExistentFields.end() != aExistentFields.find(fieldCheck.sFieldName))
            {
                // a column is specified twice ... won't work anyway, so prevent this here and now
                OUString sMessage(DBA_RES(STR_INDEXDESIGN_DOUBLE_COLUMN_NAME));
                sMessage = sMessage.replaceFirst("$name$", fieldCheck.sFieldName);
                std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(GetFrameWeld(),
                                                            VclMessageType::Warning, VclButtonsType::Ok,
                                                            sMessage));
                xError->run();
                m_pFields->GrabFocus();
                return false;
            }
            aExistentFields.insert(fieldCheck.sFieldName);
        }

        return true;
    }

    bool DbaIndexDialog::implCommitPreviouslySelected()
    {
        if (m_pPreviousSelection)
        {
            Indexes::const_iterator aPreviouslySelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

            if (!implSaveModified())
                return false;

            // commit the index (if necessary)
            if (aPreviouslySelected->isModified() && !implCommit(m_pPreviousSelection))
                return false;
        }

        return true;
    }

    IMPL_LINK_NOARG( DbaIndexDialog, OnModifiedClick, Button*, void )
    {
        OnModified(*m_pFields);
    }
    IMPL_LINK_NOARG( DbaIndexDialog, OnModified, IndexFieldsControl&, void )
    {
        OSL_ENSURE(m_pPreviousSelection, "DbaIndexDialog, OnModified: invalid call!");
        Indexes::iterator aPosition = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

        aPosition->setModified(true);
        updateToolbox();
    }

    void DbaIndexDialog::updateControls(const SvTreeListEntry* _pEntry)
    {
        if (_pEntry)
        {
            // the descriptor of the selected index
            Indexes::const_iterator aSelectedIndex = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

            // fill the controls
            m_pUnique->Check(aSelectedIndex->bUnique);
            m_pUnique->Enable(!aSelectedIndex->bPrimaryKey);
            m_pUnique->SaveValue();

            m_pFields->initializeFrom(aSelectedIndex->aFields);
            m_pFields->Enable(!aSelectedIndex->bPrimaryKey);
            m_pFields->SaveValue();

            m_pDescription->SetText(aSelectedIndex->sDescription);
            m_pDescription->Enable(!aSelectedIndex->bPrimaryKey);

            m_pDescriptionLabel->Enable(!aSelectedIndex->bPrimaryKey);
        }
        else
        {
            m_pUnique->Check(false);
            m_pFields->initializeFrom(IndexFields());
            m_pDescription->SetText(OUString());
        }
    }

    IMPL_LINK_NOARG( DbaIndexDialog, OnIndexSelected, DbaIndexList&, void )
    {
        m_pIndexList->EndSelection();

        if (m_pIndexList->IsEditingActive())
            m_pIndexList->EndEditing();

        // commit the old data
        if (m_pIndexList->FirstSelected() != m_pPreviousSelection)
        {   // (this call may happen in case somebody ended an in-place edit with 'return', so we need to check this before committing)
            if (!implCommitPreviouslySelected())
            {
                m_pIndexList->SelectNoHandlerCall(m_pPreviousSelection);
                return;
            }
        }

        bool bHaveSelection = (nullptr != m_pIndexList->FirstSelected());

        // disable/enable the detail controls
        m_pIndexDetails->Enable(bHaveSelection);
        m_pUnique->Enable(bHaveSelection);
        m_pDescriptionLabel->Enable(bHaveSelection);
        m_pFieldsLabel->Enable(bHaveSelection);
        m_pFields->Enable(bHaveSelection);

        SvTreeListEntry* pNewSelection = m_pIndexList->FirstSelected();
        updateControls(pNewSelection);
        if (bHaveSelection)
            m_pIndexList->GrabFocus();

        m_pPreviousSelection = pNewSelection;

        updateToolbox();
    }
    void DbaIndexDialog::StateChanged( StateChangedType nType )
    {
        ModalDialog::StateChanged( nType );

        if ( nType == StateChangedType::ControlBackground )
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
        }
        else if ( nType == StateChangedType::Text )
        {
            // The physical toolbar changed its outlook and shows another logical toolbar!
            // We have to set the correct high contrast mode on the new tbx manager.
            //  pMgr->SetHiContrast( IsHiContrastMode() );
            checkImageList();
        }
    }
    void DbaIndexDialog::DataChanged( const DataChangedEvent& rDCEvt )
    {
        ModalDialog::DataChanged( rDCEvt );

        if ((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
            ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        ))
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
        }
    }

    void DbaIndexDialog::setImageList(sal_Int16 _eBitmapSet)
    {
        if ( _eBitmapSet == SFX_SYMBOLS_SIZE_LARGE )
        {
            m_pActions->SetItemImage(mnNewCmdId, maLcNewCmdImg);
            m_pActions->SetItemImage(mnDropCmdId, maLcDropCmdImg);
            m_pActions->SetItemImage(mnRenameCmdId, maLcRenameCmdImg);
            m_pActions->SetItemImage(mnSaveCmdId, maLcSaveCmdImg);
            m_pActions->SetItemImage(mnResetCmdId, maLcResetCmdImg);
        }
        else
        {
            m_pActions->SetItemImage(mnNewCmdId, maScNewCmdImg);
            m_pActions->SetItemImage(mnDropCmdId, maScDropCmdImg);
            m_pActions->SetItemImage(mnRenameCmdId, maScRenameCmdImg);
            m_pActions->SetItemImage(mnSaveCmdId, maScSaveCmdImg);
            m_pActions->SetItemImage(mnResetCmdId, maScResetCmdImg);
        }
    }

    void DbaIndexDialog::resizeControls(const Size&)
    {
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
