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
#include <strings.hrc>
#include <bitmaps.hlst>
#include <indexfieldscontrol.hxx>
#include <indexcollection.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
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

    // DbaIndexDialog
    DbaIndexDialog::DbaIndexDialog(weld::Window* pParent, const Sequence< OUString >& _rFieldNames,
                                   const Reference< XNameAccess >& _rxIndexes,
                                   const Reference< XConnection >& _rxConnection,
                                   const Reference< XComponentContext >& _rxContext)
        : GenericDialogController(pParent, u"dbaccess/ui/indexdesigndialog.ui"_ustr, u"IndexDesignDialog"_ustr)
        , m_xConnection(_rxConnection)
        , m_bEditingActive(false)
        , m_bEditAgain(false)
        , m_bNoHandlerCall(false)
        , m_xContext(_rxContext)
        , m_xActions(m_xBuilder->weld_toolbar(u"ACTIONS"_ustr))
        , m_xIndexList(m_xBuilder->weld_tree_view(u"INDEX_LIST"_ustr))
        , m_xIndexDetails(m_xBuilder->weld_label(u"INDEX_DETAILS"_ustr))
        , m_xDescriptionLabel(m_xBuilder->weld_label(u"DESC_LABEL"_ustr))
        , m_xDescription(m_xBuilder->weld_label(u"DESCRIPTION"_ustr))
        , m_xUnique(m_xBuilder->weld_check_button(u"UNIQUE"_ustr))
        , m_xFieldsLabel(m_xBuilder->weld_label(u"FIELDS_LABEL"_ustr))
        , m_xClose(m_xBuilder->weld_button(u"close"_ustr))
        , m_xTable(m_xBuilder->weld_container(u"FIELDS"_ustr))
        , m_xTableCtrlParent(m_xTable->CreateChildFrame())
        , m_xFields(VclPtr<IndexFieldsControl>::Create(m_xTableCtrlParent))
    {
        m_xIndexList->set_size_request(m_xIndexList->get_approximate_digit_width() * 17,
                                       m_xIndexList->get_height_rows(12));

        int nWidth = m_xIndexList->get_approximate_digit_width() * 60;
        int nHeight = m_xIndexList->get_height_rows(8);
        m_xTable->set_size_request(nWidth, nHeight);

        m_xActions->connect_clicked(LINK(this, DbaIndexDialog, OnIndexAction));

        m_xIndexList->connect_changed(LINK(this, DbaIndexDialog, OnIndexSelected));
        m_xIndexList->connect_editing(LINK(this, DbaIndexDialog, OnEntryEditing),
                                      LINK(this, DbaIndexDialog, OnEntryEdited));

        m_xFields->SetSizePixel(Size(nWidth, 100));
        m_xFields->Init(_rFieldNames, ::dbtools::getBooleanDataSourceSetting( m_xConnection, "AddIndexAppendix" ));
        m_xFields->Show();

        m_xIndexes.reset(new OIndexCollection());
        try
        {
            m_xIndexes->attach(_rxIndexes);
        }
        catch(SQLException& e)
        {
            ::dbtools::showError(SQLExceptionInfo(e), pParent->GetXWindow(), _rxContext);
        }
        catch(Exception&)
        {
            OSL_FAIL("DbaIndexDialog::DbaIndexDialog: could not retrieve basic information from the UNO collection!");
        }

        fillIndexList();

        m_xUnique->connect_toggled(LINK(this, DbaIndexDialog, OnModifiedClick));
        m_xFields->SetModifyHdl(LINK(this, DbaIndexDialog, OnModified));

        m_xClose->connect_clicked(LINK(this, DbaIndexDialog, OnCloseDialog));

        // if all of the indexes have an empty description, we're not interested in displaying it
        bool bFound = false;
        for (auto const& check : *m_xIndexes)
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
            m_xDescription->hide();
            m_xDescriptionLabel->hide();
        }
    }

    void DbaIndexDialog::updateToolbox()
    {
        m_xActions->set_item_sensitive(u"ID_INDEX_NEW"_ustr, !m_bEditingActive);

        int nSelected = m_xIndexList->get_selected_index();
        bool bSelectedAnything = nSelected != -1;
        if (bSelectedAnything)
        {
            // is the current entry modified?
            Indexes::const_iterator aSelectedPos = m_xIndexes->begin() + m_xIndexList->get_id(nSelected).toUInt32();
            m_xActions->set_item_sensitive(u"ID_INDEX_SAVE"_ustr, aSelectedPos->isModified() || aSelectedPos->isNew());
            m_xActions->set_item_sensitive(u"ID_INDEX_RESET"_ustr, aSelectedPos->isModified() || aSelectedPos->isNew());
            bSelectedAnything = !aSelectedPos->bPrimaryKey;
        }
        else
        {
            m_xActions->set_item_sensitive(u"ID_INDEX_SAVE"_ustr, false);
            m_xActions->set_item_sensitive(u"ID_INDEX_RESET"_ustr, false);
        }
        m_xActions->set_item_sensitive(u"ID_INDEX_DROP"_ustr, bSelectedAnything);
        m_xActions->set_item_sensitive(u"ID_INDEX_RENAME"_ustr, bSelectedAnything);
    }

    void DbaIndexDialog::fillIndexList()
    {
        OUString aPKeyIcon(BMP_PKEYICON);
        // fill the list with the index names
        m_xIndexList->clear();
        sal_uInt32 nPos = 0;
        for (auto const& indexLoop : *m_xIndexes)
        {
            m_xIndexList->append(OUString::number(nPos), indexLoop.sName);
            if (indexLoop.bPrimaryKey)
                m_xIndexList->set_image(nPos, aPKeyIcon);
            ++nPos;
        }

        if (nPos)
            m_xIndexList->select(0);

        IndexSelected();
    }

    DbaIndexDialog::~DbaIndexDialog( )
    {
        m_xIndexes.reset();
        m_xFields.disposeAndClear();
        m_xTableCtrlParent->dispose();
        m_xTableCtrlParent.clear();
    }

    bool DbaIndexDialog::implCommit(const weld::TreeIter* pEntry)
    {
        assert(pEntry && "DbaIndexDialog::implCommit: invalid entry!");

        Indexes::iterator aCommitPos = m_xIndexes->begin() + m_xIndexList->get_id(*pEntry).toUInt32();

        // if it's not a new index, remove it
        // (we can't modify indexes, only drop'n'insert)
        if (!aCommitPos->isNew())
            if (!implDropIndex(pEntry, false))
                return false;

        // create the new index
        SQLExceptionInfo aExceptionInfo;
        try
        {
            m_xIndexes->commitNewIndex(aCommitPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        // reflect the new selection in the toolbox
        updateToolbox();

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, m_xDialog->GetXWindow(), m_xContext);
        else
        {
            m_xUnique->save_state();
            m_xFields->SaveValue();
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
            if (m_xIndexes->end() == m_xIndexes->find(sNewIndexName))
                break;
        }
        if (i == 0x7FFFFFFF)
        {
            OSL_FAIL("DbaIndexDialog::OnNewIndex: no free index name found!");
            // can't do anything ... of course we try another base, but this could end with the same result ...
            return;
        }

        std::unique_ptr<weld::TreeIter> xNewEntry(m_xIndexList->make_iterator());
        m_xIndexList->insert(nullptr, -1, &sNewIndexName, nullptr, nullptr, nullptr, false, xNewEntry.get());
        m_xIndexes->insert(sNewIndexName);

        // update the user data on the entries in the list box:
        // they're iterators of the index collection, and thus they have changed when removing the index
        m_xIndexList->all_foreach([this](weld::TreeIter& rEntry){
            Indexes::const_iterator aAfterInsertPos = m_xIndexes->find(m_xIndexList->get_text(rEntry));
            OSL_ENSURE(aAfterInsertPos != m_xIndexes->end(), "DbaIndexDialog::OnNewIndex: problems with one of the entries!");
            m_xIndexList->set_id(rEntry, OUString::number(aAfterInsertPos - m_xIndexes->begin()));
            return false;
        });

        // select the entry and start in-place editing
        m_bNoHandlerCall = true;
        m_xIndexList->select(*xNewEntry);
        m_bNoHandlerCall = false;
        IndexSelected();
        m_xIndexList->grab_focus();
        m_xIndexList->start_editing(*xNewEntry);
        updateToolbox();
    }

    void DbaIndexDialog::OnDropIndex(bool _bConfirm)
    {
        std::unique_ptr<weld::TreeIter> xSelected(m_xIndexList->make_iterator());
        // the selected index
        if (!m_xIndexList->get_selected(xSelected.get()))
            return;

        // let the user confirm the drop
        if (_bConfirm)
        {
            OUString sConfirm(DBA_RES(STR_CONFIRM_DROP_INDEX));
            sConfirm = sConfirm.replaceFirst("$name$", m_xIndexList->get_text(*xSelected));
            std::unique_ptr<weld::MessageDialog> xConfirm(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Question, VclButtonsType::YesNo,
                                                          sConfirm));
            if (RET_YES != xConfirm->run())
                return;
        }

        // do the drop
        implDropIndex(xSelected.get(), true);

        // reflect the new selection in the toolbox
        updateToolbox();
    }

    bool DbaIndexDialog::implDropIndex(const weld::TreeIter* pEntry, bool _bRemoveFromCollection)
    {
        // do the drop
        Indexes::iterator aDropPos = m_xIndexes->begin() + m_xIndexList->get_id(*pEntry).toUInt32();
        OSL_ENSURE(aDropPos != m_xIndexes->end(), "DbaIndexDialog::OnDropIndex: did not find the index in my collection!");

        SQLExceptionInfo aExceptionInfo;
        bool bSuccess = false;
        try
        {
            if (_bRemoveFromCollection)
                bSuccess = m_xIndexes->drop(aDropPos);
            else
                bSuccess = m_xIndexes->dropNoRemove(aDropPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, m_xDialog->GetXWindow(), m_xContext);
        else if (bSuccess && _bRemoveFromCollection)
        {
            m_bNoHandlerCall = true;

            // if the entry to remove is the selected on...
            if (m_xPreviousSelection && m_xPreviousSelection->equal(*pEntry))
                m_xPreviousSelection.reset();
            m_xIndexList->remove(*pEntry);

            m_bNoHandlerCall = false;

            // update the user data on the entries in the list box:
            // they're iterators of the index collection, and thus they have changed when removing the index
            m_xIndexList->all_foreach([this](weld::TreeIter& rEntry){
                Indexes::const_iterator aAfterDropPos = m_xIndexes->find(m_xIndexList->get_text(rEntry));
                OSL_ENSURE(aAfterDropPos != m_xIndexes->end(), "DbaIndexDialog::OnDropIndex: problems with one of the remaining entries!");
                m_xIndexList->set_id(rEntry, OUString::number(aAfterDropPos - m_xIndexes->begin()));
                return false;
            });

            // the Remove automatically selected another entry (if possible), but we disabled the calling of the handler
            // to prevent that we missed something... call the handler directly
            IndexSelected();
        }

        return !aExceptionInfo.isValid();
    }

    void DbaIndexDialog::OnRenameIndex()
    {
        // the selected iterator
        std::unique_ptr<weld::TreeIter> xSelected(m_xIndexList->make_iterator());
        if (!m_xIndexList->get_selected(xSelected.get()))
            return;

        // save the changes made 'til here
        // Upon leaving the edit mode, the control will be re-initialized with the
        // settings from the current entry
        implSaveModified(false);

        m_xIndexList->grab_focus();
        m_xIndexList->start_editing(*xSelected);
        updateToolbox();
    }

    void DbaIndexDialog::OnSaveIndex()
    {
        // the selected index
        implCommitPreviouslySelected();
        updateToolbox();
    }

    void DbaIndexDialog::OnResetIndex()
    {
        // the selected index
        std::unique_ptr<weld::TreeIter> xSelected(m_xIndexList->make_iterator());
        // the selected index
        if (!m_xIndexList->get_selected(xSelected.get()))
            xSelected.reset();
        OSL_ENSURE(xSelected, "DbaIndexDialog::OnResetIndex: invalid call!");
        if (!xSelected)
            return;

        Indexes::iterator aResetPos = m_xIndexes->begin() + m_xIndexList->get_id(*xSelected).toUInt32();

        if (aResetPos->isNew())
        {
            OnDropIndex(false);
            return;
        }

        SQLExceptionInfo aExceptionInfo;
        try
        {
            m_xIndexes->resetIndex(aResetPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, m_xDialog->GetXWindow(), m_xContext);
        else
            m_xIndexList->set_text(*xSelected, aResetPos->sName);

        updateControls(xSelected.get());
        updateToolbox();
    }

    IMPL_LINK(DbaIndexDialog, OnIndexAction, const OUString&, rClicked, void)
    {
        if (rClicked == "ID_INDEX_NEW")
            OnNewIndex();
        else if (rClicked == "ID_INDEX_DROP")
            OnDropIndex();
        else if (rClicked == "ID_INDEX_RENAME")
            OnRenameIndex();
        else if (rClicked == "ID_INDEX_SAVE")
            OnSaveIndex();
        else if (rClicked == "ID_INDEX_RESET")
            OnResetIndex();
    }

    IMPL_LINK_NOARG(DbaIndexDialog, OnCloseDialog, weld::Button&, void)
    {
        if (m_bEditingActive)
        {
            OSL_ENSURE(!m_bEditAgain, "DbaIndexDialog::OnCloseDialog: somebody was faster than hell!");
                // this means somebody entered a new name, which was invalid, which cause us to posted us an event,
                // and before the event arrived the user clicked onto "close". VERY fast, this user...
            m_xIndexList->end_editing();
            if (m_bEditAgain)
                // could not commit the new name (started a new - asynchronous - edit trial)
                return;
        }

        // the currently selected entry
        std::unique_ptr<weld::TreeIter> xSelected(m_xIndexList->make_iterator());
        // the selected index
        if (!m_xIndexList->get_selected(xSelected.get()))
            xSelected.reset();

        OSL_ENSURE(xSelected && m_xPreviousSelection && xSelected->equal(*m_xPreviousSelection), "DbaIndexDialog::OnCloseDialog: inconsistence!");

        sal_Int32 nResponse = RET_NO;
        if (xSelected)
        {
            // the descriptor
            Indexes::const_iterator aSelected = m_xIndexes->begin() + m_xIndexList->get_id(*xSelected).toUInt32();
            if (aSelected->isModified() || aSelected->isNew())
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"dbaccess/ui/saveindexdialog.ui"_ustr));
                std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog(u"SaveIndexDialog"_ustr));
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

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK(DbaIndexDialog, OnEditIndexAgain, void*, p, void)
    {
        weld::TreeIter* pEntry = static_cast<weld::TreeIter*>(p);
        m_bEditAgain = false;
        m_xIndexList->grab_focus();
        m_xIndexList->start_editing(*pEntry);
        delete pEntry;
    }

    IMPL_LINK_NOARG(DbaIndexDialog, OnEntryEditing, const weld::TreeIter&, bool)
    {
        m_bEditingActive = true;
        return true;
    }

    IMPL_LINK(DbaIndexDialog, OnEntryEdited, const IterString&, rIterString, bool)
    {
        m_bEditingActive = false;

        const weld::TreeIter& rEntry = rIterString.first;
        OUString sNewName = rIterString.second;

        Indexes::iterator aPosition = m_xIndexes->begin() + m_xIndexList->get_id(rEntry).toUInt32();

        OSL_ENSURE(aPosition >= m_xIndexes->begin() && aPosition < m_xIndexes->end(),
            "DbaIndexDialog::OnEntryEdited: invalid entry!");

        Indexes::const_iterator aSameName = m_xIndexes->find(sNewName);
        if (aSameName != aPosition && m_xIndexes->end() != aSameName)
        {
            OUString sError(DBA_RES(STR_INDEX_NAME_ALREADY_USED));
            sError = sError.replaceFirst("$name$", sNewName);
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                        VclMessageType::Warning, VclButtonsType::Ok,
                                                        sError));
            xError->run();

            updateToolbox();
            m_bEditAgain = true;
            std::unique_ptr<weld::TreeIter> xEntry(m_xIndexList->make_iterator(&rEntry));
            Application::PostUserEvent(LINK(this, DbaIndexDialog, OnEditIndexAgain), xEntry.release());
            return false;
        }

        aPosition->sName = sNewName;

        // rename can be done by a drop/insert combination only
        if (aPosition->isNew())
        {
            updateToolbox();
            // no commitment needed here...
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
        if (!m_xPreviousSelection)
            return true;

        // try to commit the previously selected index
        if (m_xFields->IsModified() && !m_xFields->SaveModified())
            return false;

        Indexes::iterator aPreviouslySelected = m_xIndexes->begin() + m_xIndexList->get_id(*m_xPreviousSelection).toUInt32();

        // the unique flag
        aPreviouslySelected->bUnique = m_xUnique->get_active();
        if (m_xUnique->get_state_changed_from_saved())
            aPreviouslySelected->setModified(true);

        // the fields
        m_xFields->commitTo(aPreviouslySelected->aFields);
        if (m_xFields->GetSavedValue() != aPreviouslySelected->aFields)
            aPreviouslySelected->setModified(true);

        // plausibility checks
        if (_bPlausibility && !implCheckPlausibility(aPreviouslySelected))
            return false;

        return true;
    }

    bool DbaIndexDialog::implCheckPlausibility(const Indexes::const_iterator& _rPos)
    {
        // need at least one field
        if (_rPos->aFields.empty())
        {
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                        VclMessageType::Warning, VclButtonsType::Ok,
                                                        DBA_RES(STR_NEED_INDEX_FIELDS)));
            xError->run();
            m_xFields->GrabFocus();
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
                std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                            VclMessageType::Warning, VclButtonsType::Ok,
                                                            sMessage));
                xError->run();
                m_xFields->GrabFocus();
                return false;
            }
            aExistentFields.insert(fieldCheck.sFieldName);
        }

        return true;
    }

    bool DbaIndexDialog::implCommitPreviouslySelected()
    {
        if (m_xPreviousSelection)
        {
            Indexes::const_iterator aPreviouslySelected = m_xIndexes->begin() + m_xIndexList->get_id(*m_xPreviousSelection).toUInt32();

            if (!implSaveModified())
                return false;

            // commit the index (if necessary)
            if (aPreviouslySelected->isModified() && !implCommit(m_xPreviousSelection.get()))
                return false;
        }

        return true;
    }

    IMPL_LINK_NOARG(DbaIndexDialog, OnModifiedClick, weld::Toggleable&, void)
    {
        OnModified(*m_xFields);
    }

    IMPL_LINK_NOARG( DbaIndexDialog, OnModified, IndexFieldsControl&, void )
    {
        assert(m_xPreviousSelection && "DbaIndexDialog, OnModified: invalid call!");
        Indexes::iterator aPosition = m_xIndexes->begin() + m_xIndexList->get_id(*m_xPreviousSelection).toUInt32();

        aPosition->setModified(true);
        updateToolbox();
    }

    void DbaIndexDialog::updateControls(const weld::TreeIter* pEntry)
    {
        if (pEntry)
        {
            // the descriptor of the selected index
            Indexes::const_iterator aSelectedIndex = m_xIndexes->begin() + m_xIndexList->get_id(*pEntry).toUInt32();

            // fill the controls
            m_xUnique->set_active(aSelectedIndex->bUnique);
            m_xUnique->set_sensitive(!aSelectedIndex->bPrimaryKey);
            m_xUnique->save_state();

            m_xFields->initializeFrom(std::vector(aSelectedIndex->aFields));
            m_xFields->Enable(!aSelectedIndex->bPrimaryKey);
            m_xFields->SaveValue();

            m_xDescription->set_label(aSelectedIndex->sDescription);
            m_xDescription->set_sensitive(!aSelectedIndex->bPrimaryKey);

            m_xDescriptionLabel->set_sensitive(!aSelectedIndex->bPrimaryKey);
        }
        else
        {
            m_xUnique->set_active(false);
            m_xFields->initializeFrom(IndexFields());
            m_xDescription->set_label(OUString());
        }
    }

    void DbaIndexDialog::IndexSelected()
    {
        if (m_bEditingActive)
            m_xIndexList->end_editing();

        std::unique_ptr<weld::TreeIter> xSelected(m_xIndexList->make_iterator());
        if (!m_xIndexList->get_selected(xSelected.get()))
            xSelected.reset();

        // commit the old data
        if (m_xPreviousSelection && (!xSelected || !m_xPreviousSelection->equal(*xSelected)))
        {
            // (this call may happen in case somebody ended an in-place edit with 'return', so we need to check this before committing)
            if (!implCommitPreviouslySelected())
            {
                m_bNoHandlerCall = true;
                m_xIndexList->select(*m_xPreviousSelection);
                m_bNoHandlerCall = false;
                return;
            }
        }

        // disable/enable the detail controls
        m_xIndexDetails->set_sensitive(xSelected != nullptr);
        m_xUnique->set_sensitive(xSelected != nullptr);
        m_xDescriptionLabel->set_sensitive(xSelected != nullptr);
        m_xFieldsLabel->set_sensitive(xSelected != nullptr);
        m_xFields->Enable(xSelected != nullptr);

        updateControls(xSelected.get());
        if (xSelected)
            m_xIndexList->grab_focus();

        m_xPreviousSelection = std::move(xSelected);

        updateToolbox();
    }

    IMPL_LINK_NOARG(DbaIndexDialog, OnIndexSelected, weld::TreeView&, void)
    {
        if (m_bNoHandlerCall)
            return;
        IndexSelected();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
