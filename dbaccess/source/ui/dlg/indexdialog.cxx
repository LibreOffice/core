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

#include "sal/config.h"

#include <set>

#include "indexdialog.hxx"
#include "dbu_dlg.hrc"
#include "dbaccess_helpid.hrc"
#include "indexdialog.hrc"
#include "indexfieldscontrol.hxx"
#include "indexcollection.hxx"
#include <vcl/msgbox.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
#include "UITools.hxx"
#include <svtools/imgdef.hxx>
#include "svtools/treelistentry.hxx"
#include "browserids.hxx"
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
    sal_Bool operator ==(const OIndexField& _rLHS, const OIndexField& _rRHS)
    {
        return  (_rLHS.sFieldName == _rRHS.sFieldName)
            &&  (_rLHS.bSortAscending == _rRHS.bSortAscending);
    }

    sal_Bool operator !=(const OIndexField& _rLHS, const OIndexField& _rRHS)
    {
        return !(_rLHS == _rRHS);
    }

    sal_Bool operator ==(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        if (_rLHS.size() != _rRHS.size())
            return sal_False;

        ConstIndexFieldsIterator aLeft = _rLHS.begin();
        ConstIndexFieldsIterator aLeftEnd = _rLHS.end();
        ConstIndexFieldsIterator aRight = _rRHS.begin();
        for (; aLeft != aLeftEnd; ++aLeft, ++aRight)
        {
            if (*aLeft != *aRight)
                return sal_False;
        }

        return sal_True;
    }

    sal_Bool operator !=(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        return !(_rLHS == _rRHS);
    }

    // DbaIndexList
    DbaIndexList::DbaIndexList(Window* _pParent, const ResId& _rId)
        :SvTreeListBox(_pParent, _rId)
        ,m_bSuspendSelectHdl(sal_False)
    {
    }

    extern sal_Bool isCharOk(sal_Unicode _cChar,sal_Bool _bFirstChar,sal_Bool _bUpperCase,const OUString& _sAllowedChars);
    sal_Bool DbaIndexList::EditedEntry( SvTreeListEntry* _pEntry, const OUString& _rNewText )
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
                    return sal_False;
            }
        }

        if (!SvTreeListBox::EditedEntry(_pEntry, _rNewText))
            return sal_False;

        String sOldText = GetEntryText(_pEntry);
        SvTreeListBox::SetEntryText(_pEntry, _rNewText);

        sal_Bool bValid = sal_True;
        if (m_aEndEditHdl.IsSet())
            bValid = (0 != m_aEndEditHdl.Call(_pEntry));

        if (bValid)
            return sal_True;

        SvTreeListBox::SetEntryText(_pEntry, sOldText);

        return sal_False;
    }

    void DbaIndexList::enableSelectHandler()
    {
        OSL_ENSURE(m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = sal_False;
    }

    void DbaIndexList::disableSelectHandler()
    {
        OSL_ENSURE(!m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = sal_True;
    }

    void DbaIndexList::SelectNoHandlerCall( SvTreeListEntry* _pEntry )
    {
        disableSelectHandler();
        Select(_pEntry, sal_True);
        enableSelectHandler();
    }

    sal_Bool DbaIndexList::Select( SvTreeListEntry* pEntry, sal_Bool _bSelect )
    {
        sal_Bool bReturn = SvTreeListBox::Select(pEntry, _bSelect);

        if (m_aSelectHdl.IsSet() && !m_bSuspendSelectHdl && _bSelect)
            m_aSelectHdl.Call(this);

        return bReturn;
    }

    // DbaIndexDialog
DBG_NAME(DbaIndexDialog)
    DbaIndexDialog::DbaIndexDialog( Window* _pParent, const Sequence< OUString >& _rFieldNames,
                                    const Reference< XNameAccess >& _rxIndexes,
                                    const Reference< XConnection >& _rxConnection,
                                    const Reference< XComponentContext >& _rxContext,sal_Int32 _nMaxColumnsInIndex)
        :ModalDialog( _pParent, ModuleRes(DLG_INDEXDESIGN))
        ,m_xConnection(_rxConnection)
        ,m_aGeometrySettings(E_DIALOG, OUString("dbaccess.tabledesign.indexdialog"))
        ,m_aActions                         (this, ModuleRes(TLB_ACTIONS))
        ,m_aIndexes                         (this, ModuleRes(CTR_INDEXLIST))
        ,m_aIndexDetails                    (this, ModuleRes(FL_INDEXDETAILS))
        ,m_aDescriptionLabel                (this, ModuleRes(FT_DESC_LABEL))
        ,m_aDescription                     (this, ModuleRes(FT_DESCRIPTION))
        ,m_aUnique                          (this, ModuleRes(CB_UNIQUE))
        ,m_aFieldsLabel                     (this, ModuleRes(FT_FIELDS))
        ,m_pFields(new IndexFieldsControl   (this, ModuleRes(CTR_FIELDS),_nMaxColumnsInIndex,::dbtools::getBooleanDataSourceSetting( m_xConnection, "AddIndexAppendix" )))
        ,m_aClose                           (this, ModuleRes(PB_CLOSE))
        ,m_aHelp                            (this, ModuleRes(HB_HELP))
        ,m_pIndexes(NULL)
        ,m_pPreviousSelection(NULL)
        ,m_bEditAgain(sal_False)
        ,m_xContext(_rxContext)
    {
        DBG_CTOR(DbaIndexDialog,NULL);

        FreeResource();

        m_aActions.SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexAction));

        m_aIndexes.SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexSelected));
        m_aIndexes.SetEndEditHdl(LINK(this, DbaIndexDialog, OnEntryEdited));
        m_aIndexes.SetSelectionMode(SINGLE_SELECTION);
        m_aIndexes.SetHighlightRange();
        m_aIndexes.setConnection(m_xConnection);

        m_pFields->Init(_rFieldNames);

        setToolBox(&m_aActions);

        m_pIndexes = new OIndexCollection();
        try
        {
            m_pIndexes->attach(_rxIndexes);
        }
        catch(SQLException& e)
        {
            ::dbaui::showError(SQLExceptionInfo(e),_pParent,_rxContext);
        }
        catch(Exception&)
        {
            OSL_FAIL("DbaIndexDialog::DbaIndexDialog: could not retrieve basic information from the UNO collection!");
        }

        fillIndexList();

        m_aUnique.SetClickHdl(LINK(this, DbaIndexDialog, OnModified));
        m_pFields->SetModifyHdl(LINK(this, DbaIndexDialog, OnModified));

        m_aClose.SetClickHdl(LINK(this, DbaIndexDialog, OnCloseDialog));

        // if all of the indexes have an empty description, we're not interested in displaying it
        Indexes::const_iterator aCheck;

        for (   aCheck = m_pIndexes->begin();
                aCheck != m_pIndexes->end();
                ++aCheck
            )
        {
            if (!aCheck->sDescription.isEmpty())
                break;
        }

        if (aCheck == m_pIndexes->end())
        {
            sal_Int32 nMoveUp = m_aUnique.GetPosPixel().Y() - m_aDescriptionLabel.GetPosPixel().Y();

            // hide the controls which are necessary for the description
            m_aDescription.Hide();
            m_aDescriptionLabel.Hide();

            // move other controls up
            Point aPos = m_aUnique.GetPosPixel();
            aPos.Y() -= nMoveUp;
            m_aUnique.SetPosPixel(aPos);

            aPos = m_aFieldsLabel.GetPosPixel();
            aPos.Y() -= nMoveUp;
            m_aFieldsLabel.SetPosPixel(aPos);

            aPos = m_pFields->GetPosPixel();
            aPos.Y() -= nMoveUp;
            m_pFields->SetPosPixel(aPos);

            // and enlarge the fields list
            Size aSize = m_pFields->GetSizePixel();
            aSize.Height() += nMoveUp;
            m_pFields->SetSizePixel(aSize);
        }
    }

    void DbaIndexDialog::updateToolbox()
    {
        m_aActions.EnableItem(ID_INDEX_NEW, !m_aIndexes.IsEditingActive());

        SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        sal_Bool bSelectedAnything = NULL != pSelected;

        if (pSelected)
        {
            // is the current entry modified?
            Indexes::const_iterator aSelectedPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());
            m_aActions.EnableItem(ID_INDEX_SAVE, aSelectedPos->isModified() || aSelectedPos->isNew());
            m_aActions.EnableItem(ID_INDEX_RESET, aSelectedPos->isModified() || aSelectedPos->isNew());
            bSelectedAnything = bSelectedAnything && !aSelectedPos->bPrimaryKey;
        }
        else
        {
            m_aActions.EnableItem(ID_INDEX_SAVE, sal_False);
            m_aActions.EnableItem(ID_INDEX_RESET, sal_False);
        }
        m_aActions.EnableItem(ID_INDEX_DROP, bSelectedAnything);
        m_aActions.EnableItem(ID_INDEX_RENAME, bSelectedAnything);
    }

    void DbaIndexDialog::fillIndexList()
    {
        Image aPKeyIcon(ModuleRes( IMG_PKEYICON ));
        // fill the list with the index names
        m_aIndexes.Clear();
        Indexes::iterator aIndexLoop = m_pIndexes->begin();
        Indexes::iterator aEnd = m_pIndexes->end();
        for (; aIndexLoop != aEnd; ++aIndexLoop)
        {
            SvTreeListEntry* pNewEntry = NULL;
            if (aIndexLoop->bPrimaryKey)
                pNewEntry = m_aIndexes.InsertEntry(aIndexLoop->sName, aPKeyIcon, aPKeyIcon);
            else
                pNewEntry = m_aIndexes.InsertEntry(aIndexLoop->sName);

            pNewEntry->SetUserData(reinterpret_cast< void* >(sal_Int32(aIndexLoop - m_pIndexes->begin())));
        }

        OnIndexSelected(&m_aIndexes);
    }

    DbaIndexDialog::~DbaIndexDialog( )
    {
        setToolBox(NULL);
        delete m_pIndexes;
        delete m_pFields;

        DBG_DTOR(DbaIndexDialog,NULL);
    }

    sal_Bool DbaIndexDialog::implCommit(SvTreeListEntry* _pEntry)
    {
        OSL_ENSURE(_pEntry, "DbaIndexDialog::implCommit: invalid entry!");

        Indexes::iterator aCommitPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

        // if it's not a new index, remove it
        // (we can't modify indexes, only drop'n'insert)
        if (!aCommitPos->isNew())
            if (!implDropIndex(_pEntry, sal_False))
                return sal_False;

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
            showError(aExceptionInfo, this, m_xContext);
        else
        {
            m_aUnique.SaveValue();
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
        String sNewIndexName;
        const String sNewIndexNameBase(ModuleRes(STR_LOGICAL_INDEX_NAME));
        sal_Int32 i;

        for ( i = 1; i < 0x7FFFFFFF; ++i )
        {
            sNewIndexName = sNewIndexNameBase;
            sNewIndexName += OUString::number(i);
            if (m_pIndexes->end() == m_pIndexes->find(sNewIndexName))
                break;
        }
        if (i == 0x7FFFFFFF)
        {
            OSL_FAIL("DbaIndexDialog::OnNewIndex: no free index name found!");
            // can't do anything ... of course we try another base, but this could end with the same result ...
            return;
        }

        SvTreeListEntry* pNewEntry = m_aIndexes.InsertEntry(sNewIndexName);
        m_pIndexes->insert(sNewIndexName);

        // update the user data on the entries in the list box:
        // they're iterators of the index collection, and thus they have changed when removing the index
        for (SvTreeListEntry* pAdjust = m_aIndexes.First(); pAdjust; pAdjust = m_aIndexes.Next(pAdjust))
        {
            Indexes::iterator aAfterInsertPos = m_pIndexes->find(m_aIndexes.GetEntryText(pAdjust));
            OSL_ENSURE(aAfterInsertPos != m_pIndexes->end(), "DbaIndexDialog::OnNewIndex: problems with on of the entries!");
            pAdjust->SetUserData(reinterpret_cast< void* >(sal_Int32(aAfterInsertPos - m_pIndexes->begin())));
        }

        // select the entry and start in-place editing
        m_aIndexes.SelectNoHandlerCall(pNewEntry);
        OnIndexSelected(&m_aIndexes);
        m_aIndexes.EditEntry(pNewEntry);
        updateToolbox();
    }

    void DbaIndexDialog::OnDropIndex(sal_Bool _bConfirm)
    {
        // the selected index
        SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnDropIndex: invalid call!");
        if (pSelected)
        {
            // let the user confirm the drop
            if (_bConfirm)
            {
                String sConfirm(ModuleRes(STR_CONFIRM_DROP_INDEX));
                sConfirm.SearchAndReplaceAscii("$name$", m_aIndexes.GetEntryText(pSelected));
                QueryBox aConfirm(this, WB_YES_NO, sConfirm);
                if (RET_YES != aConfirm.Execute())
                    return;
            }

            // do the drop
            implDropIndex(pSelected, sal_True);

            // reflect the new selection in the toolbox
            updateToolbox();
        }
    }

    sal_Bool DbaIndexDialog::implDropIndex(SvTreeListEntry* _pEntry, sal_Bool _bRemoveFromCollection)
    {
        // do the drop
        Indexes::iterator aDropPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());
        OSL_ENSURE(aDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: did not find the index in my collection!");

        SQLExceptionInfo aExceptionInfo;
        sal_Bool bSuccess = sal_False;
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
            showError(aExceptionInfo, this, m_xContext);
        else if (bSuccess && _bRemoveFromCollection)
        {
            SvTreeList* pModel = m_aIndexes.GetModel();

            m_aIndexes.disableSelectHandler();
            pModel->Remove(_pEntry);
            m_aIndexes.enableSelectHandler();

            // update the user data on the entries in the list box:
            // they're iterators of the index collection, and thus they have changed when removing the index
            for (SvTreeListEntry* pAdjust = m_aIndexes.First(); pAdjust; pAdjust = m_aIndexes.Next(pAdjust))
            {
                Indexes::iterator aAfterDropPos = m_pIndexes->find(m_aIndexes.GetEntryText(pAdjust));
                OSL_ENSURE(aAfterDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: problems with on of the remaining entries!");
                pAdjust->SetUserData(reinterpret_cast< void* >(sal_Int32(aAfterDropPos - m_pIndexes->begin())));
            }

            // if the remvoved entry was the selected on ...
            if (m_pPreviousSelection == _pEntry)
                m_pPreviousSelection = NULL;

            // the Remove automatically selected another entry (if possible), but we disabled the calling of the handler
            // to prevent that we missed something ... call the handler directly
            OnIndexSelected(&m_aIndexes);
        }

        return !aExceptionInfo.isValid();
    }

    void DbaIndexDialog::OnRenameIndex()
    {
        // the selected index
        SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnRenameIndex: invalid call!");

        // save the changes made 'til here
        // Upon leaving the edit mode, the control will be re-initialized with the
        // settings from the current entry
        implSaveModified(sal_False);

        m_aIndexes.EditEntry(pSelected);
        updateToolbox();
    }

    void DbaIndexDialog::OnSaveIndex()
    {
        // the selected index
#if OSL_DEBUG_LEVEL > 0
        SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        OSL_ENSURE( pSelected, "DbaIndexDialog::OnSaveIndex: invalid call!" );
#endif

        implCommitPreviouslySelected();
        updateToolbox();
    }

    void DbaIndexDialog::OnResetIndex()
    {
        // the selected index
        SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        OSL_ENSURE(pSelected, "DbaIndexDialog::OnResetIndex: invalid call!");

        Indexes::iterator aResetPos = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());

        if (aResetPos->isNew())
        {
            OnDropIndex(sal_False);
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
            showError(aExceptionInfo, this, m_xContext);
        else
            m_aIndexes.SetEntryText(pSelected, aResetPos->sName);

        updateControls(pSelected);
        updateToolbox();
    }

    IMPL_LINK( DbaIndexDialog, OnIndexAction, ToolBox*, /*NOTINTERESTEDIN*/ )
    {
        sal_uInt16 nClicked = m_aActions.GetCurItemId();
        switch (nClicked)
        {
            case ID_INDEX_NEW:
                OnNewIndex();
                break;
            case ID_INDEX_DROP:
                OnDropIndex();
                break;
            case ID_INDEX_RENAME:
                OnRenameIndex();
                break;
            case ID_INDEX_SAVE:
                OnSaveIndex();
                break;
            case ID_INDEX_RESET:
                OnResetIndex();
                break;
        }
        return 0L;
    }

    IMPL_LINK( DbaIndexDialog, OnCloseDialog, void*, /*NOTINTERESTEDIN*/ )
    {
        if (m_aIndexes.IsEditingActive())
        {
            OSL_ENSURE(!m_bEditAgain, "DbaIndexDialog::OnCloseDialog: somebody was faster than hell!");
                // this means somebody entered a new name, which was invalid, which cause us to posted us an event,
                // and before the event arrived the user clicked onto "close". VERY fast, this user ....
            m_aIndexes.EndEditing(sal_False);
            if (m_bEditAgain)
                // could not commit the new name (started a new - asynchronous - edit trial)
                return 1L;
        }

        // the currently selected entry
        const SvTreeListEntry* pSelected = m_aIndexes.FirstSelected();
        OSL_ENSURE(pSelected == m_pPreviousSelection, "DbaIndexDialog::OnCloseDialog: inconsistence!");

        sal_Int32 nResponse = RET_NO;
        if (pSelected)
        {
            // the descriptor
            Indexes::const_iterator aSelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(pSelected->GetUserData());

            if (aSelected->isModified() || aSelected->isNew())
            {
                QueryBox aQuestion(this, ModuleRes(QUERY_SAVE_CURRENT_INDEX));
                nResponse = aQuestion.Execute();
            }
        }

        switch (nResponse)
        {
            case RET_YES:
                if (!implCommitPreviouslySelected())
                    return 1L;
                break;
            case RET_NO:
                break;
            default:
                return 1L;
        }

        EndDialog(RET_OK);

        return 0L;
    }

    IMPL_LINK( DbaIndexDialog, OnEditIndexAgain, SvTreeListEntry*, _pEntry )
    {
        m_bEditAgain = sal_False;
        m_aIndexes.EditEntry(_pEntry);
        return 0L;
    }

    IMPL_LINK( DbaIndexDialog, OnEntryEdited, SvTreeListEntry*, _pEntry )
    {
        Indexes::iterator aPosition = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

        OSL_ENSURE(aPosition >= m_pIndexes->begin() && aPosition < m_pIndexes->end(),
            "DbaIndexDialog::OnEntryEdited: invalid entry!");

        String sNewName = m_aIndexes.GetEntryText(_pEntry);

        Indexes::const_iterator aSameName = m_pIndexes->find(sNewName);
        if ((aSameName != aPosition) && (m_pIndexes->end() != aSameName))
        {
            String sError(ModuleRes(STR_INDEX_NAME_ALREADY_USED));
            sError.SearchAndReplaceAscii("$name$", sNewName);
            ErrorBox aError(this, WB_OK, sError);
            aError.Execute();

            updateToolbox();
            m_bEditAgain = sal_True;
            PostUserEvent(LINK(this, DbaIndexDialog, OnEditIndexAgain), _pEntry);
            return 0L;
        }

        aPosition->sName = sNewName;

        // rename can be done by a drop/insert combination only
        if (aPosition->isNew())
        {
            updateToolbox();
            // no commitment needed here ....
            return 1L;
        }

        if (aPosition->sName != aPosition->getOriginalName())
        {
            aPosition->setModified(sal_True);
            updateToolbox();
        }

        return 1L;
    }

    sal_Bool DbaIndexDialog::implSaveModified(sal_Bool _bPlausibility)
    {
        if (m_pPreviousSelection)
        {
            // try to commit the previously selected index
            if (m_pFields->IsModified() && !m_pFields->SaveModified())
                return sal_False;

            Indexes::iterator aPreviouslySelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

            // the unique flag
            aPreviouslySelected->bUnique = m_aUnique.IsChecked();
            if (m_aUnique.GetSavedValue() != m_aUnique.GetState())
                aPreviouslySelected->setModified(sal_True);

            // the fields
            m_pFields->commitTo(aPreviouslySelected->aFields);
            if (m_pFields->GetSavedValue() != aPreviouslySelected->aFields)
                aPreviouslySelected->setModified(sal_True);

            // plausibility checks
            if (_bPlausibility && !implCheckPlausibility(aPreviouslySelected))
                return sal_False;
        }

        return sal_True;
    }

    sal_Bool DbaIndexDialog::implCheckPlausibility(const ConstIndexesIterator& _rPos)
    {
        // need at least one field
        if (0 == _rPos->aFields.size())
        {
            ErrorBox aError(this, ModuleRes(ERR_NEED_INDEX_FIELDS));
            aError.Execute();
            m_pFields->GrabFocus();
            return sal_False;
        }

        // no double fields
        std::set< String > aExistentFields;
        for (   ConstIndexFieldsIterator aFieldCheck = _rPos->aFields.begin();
                aFieldCheck != _rPos->aFields.end();
                ++aFieldCheck
            )
        {
            if (aExistentFields.end() != aExistentFields.find(aFieldCheck->sFieldName))
            {
                // a column is specified twice ... won't work anyway, so prevent this here and now
                String sMessage(ModuleRes(STR_INDEXDESIGN_DOUBLE_COLUMN_NAME));
                sMessage.SearchAndReplaceAscii("$name$", aFieldCheck->sFieldName);
                ErrorBox aError(this, WB_OK, sMessage);
                aError.Execute();
                m_pFields->GrabFocus();
                return sal_False;
            }
            aExistentFields.insert(aFieldCheck->sFieldName);
        }

        return sal_True;
    }

    sal_Bool DbaIndexDialog::implCommitPreviouslySelected()
    {
        if (m_pPreviousSelection)
        {
            Indexes::iterator aPreviouslySelected = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

            if (!implSaveModified())
                return sal_False;

            // commit the index (if necessary)
            if (aPreviouslySelected->isModified() && !implCommit(m_pPreviousSelection))
                return sal_False;
        }

        return sal_True;
    }

    IMPL_LINK( DbaIndexDialog, OnModified, void*, /*NOTINTERESTEDIN*/ )
    {
        OSL_ENSURE(m_pPreviousSelection, "DbaIndexDialog, OnModified: invalid call!");
        Indexes::iterator aPosition = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(m_pPreviousSelection->GetUserData());

        aPosition->setModified(sal_True);
        updateToolbox();

        return 1L;
    }

    void DbaIndexDialog::updateControls(const SvTreeListEntry* _pEntry)
    {
        if (_pEntry)
        {
            // the descriptor of the selected index
            Indexes::const_iterator aSelectedIndex = m_pIndexes->begin() + reinterpret_cast<sal_IntPtr>(_pEntry->GetUserData());

            // fill the controls
            m_aUnique.Check(aSelectedIndex->bUnique);
            m_aUnique.Enable(!aSelectedIndex->bPrimaryKey);
            m_aUnique.SaveValue();

            m_pFields->initializeFrom(aSelectedIndex->aFields);
            m_pFields->Enable(!aSelectedIndex->bPrimaryKey);
            m_pFields->SaveValue();

            m_aDescription.SetText(aSelectedIndex->sDescription);
            m_aDescription.Enable(!aSelectedIndex->bPrimaryKey);

            m_aDescriptionLabel.Enable(!aSelectedIndex->bPrimaryKey);
        }
        else
        {
            m_aUnique.Check(sal_False);
            m_pFields->initializeFrom(IndexFields());
            m_aDescription.SetText(String());
        }
    }

    IMPL_LINK( DbaIndexDialog, OnIndexSelected, DbaIndexList*, /*NOTINTERESTEDIN*/ )
    {
        m_aIndexes.EndSelection();

        if (m_aIndexes.IsEditingActive())
            m_aIndexes.EndEditing(sal_False);

        // commit the old data
        if (m_aIndexes.FirstSelected() != m_pPreviousSelection)
        {   // (this call may happen in case somebody ended an in-place edit with 'return', so we need to check this before committing)
            if (!implCommitPreviouslySelected())
            {
                m_aIndexes.SelectNoHandlerCall(m_pPreviousSelection);
                return 1L;
            }
        }

        sal_Bool bHaveSelection = (NULL != m_aIndexes.FirstSelected());

        // disable/enable the detail controls
        m_aIndexDetails.Enable(bHaveSelection);
        m_aUnique.Enable(bHaveSelection);
        m_aDescriptionLabel.Enable(bHaveSelection);
        m_aFieldsLabel.Enable(bHaveSelection);
        m_pFields->Enable(bHaveSelection);

        SvTreeListEntry* pNewSelection = m_aIndexes.FirstSelected();
        updateControls(pNewSelection);
        if (bHaveSelection)
            m_aIndexes.GrabFocus();

        m_pPreviousSelection = pNewSelection;

        updateToolbox();
        return 0L;
    }
    void DbaIndexDialog::StateChanged( StateChangedType nType )
    {
        ModalDialog::StateChanged( nType );

        if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
        }
        else if ( nType == STATE_CHANGE_TEXT )
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

        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            // Check if we need to get new images for normal/high contrast mode
            checkImageList();
        }
    }
    ImageList DbaIndexDialog::getImageList(sal_Int16 _eBitmapSet) const
    {
        sal_Int16 nN = IMG_INDEX_DLG_SC;
        if ( _eBitmapSet == SFX_SYMBOLS_SIZE_LARGE )
        {
            nN = IMG_INDEX_DLG_LC;
        }
        return ImageList(ModuleRes(nN));
    }
    void DbaIndexDialog::resizeControls(const Size& _rDiff)
    {
        // we use large images so we must change them
        if ( _rDiff.Width() || _rDiff.Height() )
        {
            Size aDlgSize = GetSizePixel();
            // adjust size of dlg
            SetSizePixel(Size(aDlgSize.Width() + _rDiff.Width(),
                              aDlgSize.Height() + _rDiff.Height())
                        );
            Size aIndexSize = m_aIndexes.GetSizePixel();
            m_aIndexes.SetPosSizePixel(m_aIndexes.GetPosPixel() + Point(0,_rDiff.Height()),
                                    Size(aIndexSize.Width() + _rDiff.Width(),
                                         aIndexSize.Height()));

            //now move the rest to the left side
            Point aMove(_rDiff.Width(),_rDiff.Height());
            m_aIndexDetails.SetPosPixel(m_aIndexDetails.GetPosPixel() + aMove);
            m_aDescriptionLabel.SetPosPixel(m_aDescriptionLabel.GetPosPixel() + aMove);
            m_aDescription.SetPosPixel(m_aDescription.GetPosPixel() + aMove);
            m_aUnique.SetPosPixel(m_aUnique.GetPosPixel() + aMove);
            m_aFieldsLabel.SetPosPixel(m_aFieldsLabel.GetPosPixel() + aMove);
            OSL_ENSURE(m_pFields,"NO valid fields!");
            m_pFields->SetPosPixel(m_pFields->GetPosPixel() + aMove);
            m_aClose.SetPosPixel(m_aClose.GetPosPixel() + aMove);
            m_aHelp.SetPosPixel(m_aHelp.GetPosPixel() + aMove);

            Invalidate();
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
