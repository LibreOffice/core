/*************************************************************************
 *
 *  $RCSfile: indexdialog.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-29 10:04:47 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXDIALOG_HXX_
#include "indexdialog.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_INDEXDIALOG_HRC_
#include "indexdialog.hrc"
#endif
#ifndef _DBAUI_INDEXFIELDSCONTROL_HXX_
#include "indexfieldscontrol.hxx"
#endif
#ifndef _DBAUI_INDEXCOLLECTION_HXX_
#include "indexcollection.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

//......................................................................
namespace dbaui
{
//......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::dbtools;

    //==================================================================
    //= helper
    //==================================================================
    //------------------------------------------------------------------
    sal_Bool operator ==(const OIndexField& _rLHS, const OIndexField& _rRHS)
    {
        return  (_rLHS.sFieldName == _rRHS.sFieldName)
            &&  (_rLHS.bSortAscending == _rRHS.bSortAscending);
    }

    //------------------------------------------------------------------
    sal_Bool operator !=(const OIndexField& _rLHS, const OIndexField& _rRHS)
    {
        return !(_rLHS == _rRHS);
    }

    //------------------------------------------------------------------
    sal_Bool operator ==(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        if (_rLHS.size() != _rRHS.size())
            return sal_False;

        ConstIndexFieldsIterator aLeft = _rLHS.begin();
        ConstIndexFieldsIterator aRight = _rRHS.begin();
        for (; aLeft != _rLHS.end(); ++aLeft, ++aRight)
        {
            if (*aLeft != *aRight)
                return sal_False;
        }

        return sal_True;
    }

    //------------------------------------------------------------------
    sal_Bool operator !=(const IndexFields& _rLHS, const IndexFields& _rRHS)
    {
        return !(_rLHS == _rRHS);
    }

    //==================================================================
    //= DbaIndexList
    //==================================================================
    //------------------------------------------------------------------
    DbaIndexList::DbaIndexList(Window* _pParent, const ResId& _rId)
        :SvTreeListBox(_pParent, _rId)
        ,m_bSuspendSelectHdl(sal_False)
    {
    }

    //------------------------------------------------------------------
    sal_Bool DbaIndexList::EditedEntry( SvLBoxEntry* _pEntry, const String& _rNewText )
    {
        sal_Bool bReturn = SvTreeListBox::EditedEntry(_pEntry, _rNewText);
        SvTreeListBox::SetEntryText(_pEntry, _rNewText);

        if (m_aEndEditHdl.IsSet())
            m_aEndEditHdl.Call(_pEntry);

        return bReturn;
    }

    //------------------------------------------------------------------
    void DbaIndexList::enableSelectHandler()
    {
        DBG_ASSERT(m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = sal_False;
    }

    //------------------------------------------------------------------
    void DbaIndexList::disableSelectHandler()
    {
        DBG_ASSERT(!m_bSuspendSelectHdl, "DbaIndexList::enableSelectHandler: invalid call (this is not cumulative)!");
        m_bSuspendSelectHdl = sal_True;
    }

    //------------------------------------------------------------------
    void DbaIndexList::SelectNoHandlerCall( SvLBoxEntry* _pEntry )
    {
        disableSelectHandler();
        Select(_pEntry, sal_True);
        enableSelectHandler();
    }

    //------------------------------------------------------------------
    sal_Bool DbaIndexList::Select( SvLBoxEntry* pEntry, sal_Bool _bSelect )
    {
        sal_Bool bReturn = SvTreeListBox::Select(pEntry, _bSelect);

        if (m_aSelectHdl.IsSet() && !m_bSuspendSelectHdl && _bSelect)
            m_aSelectHdl.Call(this);

        return bReturn;
    }

    //==================================================================
    //= DbaIndexDialog
    //==================================================================
    //------------------------------------------------------------------
    DbaIndexDialog::DbaIndexDialog(Window* _pParent, const Sequence< ::rtl::OUString >& _rFieldNames,
        const Reference< XNameAccess >& _rxIndexes, const Reference< XMultiServiceFactory >& _rxORB)
        :ModalDialog( _pParent, ModuleRes(DLG_INDEXDESIGN))
        ,m_aGeometrySettings(E_DIALOG, ::rtl::OUString::createFromAscii("dbaccess.tabledesign.indexdialog"))
        ,m_aActions                         (this, ResId(TLB_ACTIONS))
        ,m_aIndexes                         (this, ResId(CTR_INDEXLIST))
        ,m_aIndexDetails                    (this, ResId(FL_INDEXDETAILS))
        ,m_aDescriptionLabel                (this, ResId(FT_DESC_LABEL))
        ,m_aDescription                     (this, ResId(FT_DESCRIPTION))
        ,m_aUnique                          (this, ResId(CB_UNIQUE))
        ,m_aFieldsLabel                     (this, ResId(FT_FIELDS))
        ,m_pFields(new IndexFieldsControl   (this, ResId(CTR_FIELDS)))
        ,m_aClose                           (this, ResId(PB_CLOSE))
        ,m_pIndexes(NULL)
        ,m_pPreviousSelection(NULL)
        ,m_xORB(_rxORB)
    {
        FreeResource();

        m_aActions.SetOutStyle(TOOLBOX_STYLE_FLAT);
            // TODO: need the global application style for this ....
        m_aActions.SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexAction));

        m_aIndexes.SetSelectHdl(LINK(this, DbaIndexDialog, OnIndexSelected));
        m_aIndexes.SetEndEditHdl(LINK(this, DbaIndexDialog, OnEntryEdited));
        m_aIndexes.SetSelectionMode(SINGLE_SELECTION);
        m_aIndexes.SetHighlightRange();
        m_pFields->Init(_rFieldNames);

        m_pIndexes = new OIndexCollection(_rxIndexes);
        fillIndexList();

        m_aUnique.SetClickHdl(LINK(this, DbaIndexDialog, OnModified));
        m_pFields->SetModifyHdl(LINK(this, DbaIndexDialog, OnModified));

        m_aClose.SetClickHdl(LINK(this, DbaIndexDialog, OnCloseDialog));

        // get our most recent geometry settings
//      if (m_aGeometrySettings.Exists())
//      {
//          Point aPos;
//          m_aGeometrySettings.GetPosition(aPos.X(), aPos.Y());
//          SetPosPixel(aPos);
//      }

        // if all of the indexes have an empty description, we're not interested in displaying it
        for (   OIndexCollection::const_iterator aCheck = m_pIndexes->begin();
                aCheck != m_pIndexes->end();
                ++aCheck
            )
        {
            if (aCheck->sDescription.getLength())
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

    //------------------------------------------------------------------
    void DbaIndexDialog::updateToolbox()
    {
        m_aActions.EnableItem(ID_INDEX_NEW, !m_aIndexes.IsEditingActive());

        SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        sal_Bool bSelectedAnything = NULL != pSelected;
        m_aActions.EnableItem(ID_INDEX_DROP, bSelectedAnything);
        m_aActions.EnableItem(ID_INDEX_RENAME, bSelectedAnything);

        if (pSelected)
        {
            // is the current entry modified?
            OIndexCollection::const_iterator aSelectedPos = reinterpret_cast<OIndexCollection::const_iterator>(pSelected->GetUserData());
            m_aActions.EnableItem(ID_INDEX_SAVE, aSelectedPos->isModified() || aSelectedPos->isNew());
            m_aActions.EnableItem(ID_INDEX_RESET, aSelectedPos->isModified() && !aSelectedPos->isNew());
        }
        else
        {
            m_aActions.EnableItem(ID_INDEX_SAVE, sal_False);
            m_aActions.EnableItem(ID_INDEX_RESET, sal_False);
        }
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::fillIndexList()
    {
        Image aPKeyIcon(ModuleRes(IMG_PKEYICON));
        // fill the list with the index names
        m_aIndexes.Clear();
        OIndexCollection::iterator aIndexLoop = m_pIndexes->begin();
        OIndexCollection::iterator aEnd = m_pIndexes->end();
        for (; aIndexLoop != aEnd; ++aIndexLoop)
        {
            SvLBoxEntry* pNewEntry = NULL;
            if (aIndexLoop->bPrimaryKey)
                pNewEntry = m_aIndexes.InsertEntry(aIndexLoop->sName, aPKeyIcon, aPKeyIcon);
            else
                pNewEntry = m_aIndexes.InsertEntry(aIndexLoop->sName);

            pNewEntry->SetUserData(aIndexLoop);
        }

        OnIndexSelected(&m_aIndexes);
    }

    //------------------------------------------------------------------
    DbaIndexDialog::~DbaIndexDialog( )
    {
        delete m_pIndexes;
        delete m_pFields;

        // save our geometry settings
//      Point aPos = GetPosPixel();
//      m_aGeometrySettings.SetPosition(aPos.X(), aPos.Y());
    }

    //------------------------------------------------------------------
    sal_Bool DbaIndexDialog::implCommit(SvLBoxEntry* _pEntry)
    {
        DBG_ASSERT(_pEntry, "DbaIndexDialog::implCommit: invalid entry!");

        OIndexCollection::iterator aCommitPos = static_cast< OIndexCollection::iterator >(_pEntry->GetUserData());

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
            showError(aExceptionInfo, this, m_xORB);
        else
        {
            m_aUnique.SaveValue();
            m_pFields->SaveValue();
        }

        return !aExceptionInfo.isValid();
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::OnNewIndex()
    {
        // commit the current entry, if necessary
        if (!implCommitPreviouslySelected())
            return;

        // get a new unique name for the new index
        String sNewIndexName;
        const String sNewIndexNameBase(ModuleRes(STR_LOGICAL_INDEX_NAME));
        for (sal_Int32 i=1; i<0x7FFFFFFF; ++i)
        {
            sNewIndexName = sNewIndexNameBase;
            sNewIndexName += String::CreateFromInt32(i);
            if (m_pIndexes->end() == m_pIndexes->find(sNewIndexName))
                break;
        }
        if ((i>0x7FFFFFFF) || (i<0))
        {
            DBG_ERROR("DbaIndexDialog::OnNewIndex: no free index name found!");
            // can't do anything ... of course we try another base, but this could end with the same result ...
            return;
        }

        SvLBoxEntry* pNewEntry = m_aIndexes.InsertEntry(sNewIndexName);
        OIndexCollection::iterator aIndexDescriptor = m_pIndexes->insert(sNewIndexName);
//      pNewEntry->SetUserData(aIndexDescriptor);

        // update the user data on the entries in the list box:
        // they're iterators of the index collection, and thus they have changed when removing the index
        for (SvLBoxEntry* pAdjust = m_aIndexes.First(); pAdjust; pAdjust = m_aIndexes.Next(pAdjust))
        {
            OIndexCollection::iterator aAfterInsertPos = m_pIndexes->find(m_aIndexes.GetEntryText(pAdjust));
            DBG_ASSERT(aAfterInsertPos != m_pIndexes->end(), "DbaIndexDialog::OnNewIndex: problems with on of the entries!");
            pAdjust->SetUserData(aAfterInsertPos);
        }

        // select the entry and start in-place editing
        m_aIndexes.SelectNoHandlerCall(pNewEntry);
        OnIndexSelected(&m_aIndexes);
        m_aIndexes.EditEntry(pNewEntry);
        updateToolbox();
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::OnDropIndex()
    {
        // the selected index
        SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        DBG_ASSERT(pSelected, "DbaIndexDialog::OnDropIndex: invalid call!");
        if (pSelected)
        {
            // let the user confirm the drop
            String sConfirm(ModuleRes(STR_CONFIRM_DROP_INDEX));
            sConfirm.SearchAndReplaceAscii("$name$", m_aIndexes.GetEntryText(pSelected));
            QueryBox aConfirm(this, WB_YES_NO, sConfirm);
            if (RET_YES != aConfirm.Execute())
                return;

            // do the drop
            implDropIndex(pSelected, sal_True);

            // reflect the new selection in the toolbox
            updateToolbox();
        }
    }

    //------------------------------------------------------------------
    sal_Bool DbaIndexDialog::implDropIndex(SvLBoxEntry* _pEntry, sal_Bool _bRemoveFromCollection)
    {
        // do the drop
        OIndexCollection::iterator aDropPos = static_cast< OIndexCollection::iterator >(_pEntry->GetUserData());
        DBG_ASSERT(aDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: did not find the index in my collection!");

        SQLExceptionInfo aExceptionInfo;
        sal_Bool bNewIndex = aDropPos->isNew();
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
            showError(aExceptionInfo, this, m_xORB);
        else if (bSuccess && _bRemoveFromCollection)
        {
            SvLBoxTreeList* pModel = m_aIndexes.GetModel();

            m_aIndexes.disableSelectHandler();
            pModel->Remove(_pEntry);
            m_aIndexes.enableSelectHandler();

            // update the user data on the entries in the list box:
            // they're iterators of the index collection, and thus they have changed when removing the index
            for (SvLBoxEntry* pAdjust = m_aIndexes.First(); pAdjust; pAdjust = m_aIndexes.Next(pAdjust))
            {
                OIndexCollection::iterator aAfterDropPos = m_pIndexes->find(m_aIndexes.GetEntryText(pAdjust));
                DBG_ASSERT(aAfterDropPos != m_pIndexes->end(), "DbaIndexDialog::OnDropIndex: problems with on of the remaining entries!");
                pAdjust->SetUserData(aAfterDropPos);
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

    //------------------------------------------------------------------
    void DbaIndexDialog::OnRenameIndex()
    {
        // the selected index
        SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        DBG_ASSERT(pSelected, "DbaIndexDialog::OnRenameIndex: invalid call!");

        // save the changes made 'til here
        // Upon leaving the edit mode, the control will be re-initialized with the
        // settings from the current entry
        implSaveModified(sal_False);

        m_aIndexes.EditEntry(pSelected);
        updateToolbox();
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::OnSaveIndex()
    {
        // the selected index
        SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        DBG_ASSERT(pSelected, "DbaIndexDialog::OnSaveIndex: invalid call!");

        implCommitPreviouslySelected();
        updateToolbox();
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::OnResetIndex()
    {
        // the selected index
        SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        DBG_ASSERT(pSelected, "DbaIndexDialog::OnResetIndex: invalid call!");

        OIndexCollection::iterator aResetPos = static_cast< OIndexCollection::iterator >(pSelected->GetUserData());

        SQLExceptionInfo aExceptionInfo;
        try
        {
            m_pIndexes->resetIndex(aResetPos);
        }
        catch(SQLContext& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aExceptionInfo = SQLExceptionInfo(e); }
        catch(SQLException& e) { aExceptionInfo = SQLExceptionInfo(e); }

        if (aExceptionInfo.isValid())
            showError(aExceptionInfo, this, m_xORB);
        else
            m_aIndexes.SetEntryText(pSelected, aResetPos->sName);

        updateControls(pSelected);
        updateToolbox();
    }

    //------------------------------------------------------------------
    IMPL_LINK( DbaIndexDialog, OnIndexAction, ToolBox*, NOTINTERESTEDIN )
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

    //------------------------------------------------------------------
    IMPL_LINK( DbaIndexDialog, OnCloseDialog, void*, NOTINTERESTEDIN )
    {
        // the currently selected entry
        const SvLBoxEntry* pSelected = m_aIndexes.FirstSelected();
        DBG_ASSERT(pSelected == m_pPreviousSelection, "DbaIndexDialog::OnCloseDialog: inconsistence!");

        sal_Int32 nResponse = RET_NO;
        if (pSelected)
        {
            // the descriptor
            OIndexCollection::const_iterator aSelected = static_cast<OIndexCollection::iterator>(pSelected->GetUserData());
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

    //------------------------------------------------------------------
    IMPL_LINK( DbaIndexDialog, OnEntryEdited, SvLBoxEntry*, _pEntry )
    {
        OIndexCollection::iterator aPosition = static_cast< OIndexCollection::iterator >(_pEntry->GetUserData());
        DBG_ASSERT(aPosition >= m_pIndexes->begin() && aPosition < m_pIndexes->end(),
            "DbaIndexDialog::OnEntryEdited: invalid entry!");

        aPosition->sName = m_aIndexes.GetEntryText(_pEntry);

        // rename can be done by a drop/insert combination only
        if (aPosition->isNew())
        {
            updateToolbox();
            // no commitment needed here ....
            return 0L;
        }

        if (aPosition->sName != aPosition->getOriginalName())
        {
            aPosition->setModified(sal_True);
            updateToolbox();
        }

        return 1L;
    }

    //------------------------------------------------------------------
    sal_Bool DbaIndexDialog::implSaveModified(sal_Bool _bPlausibility)
    {
        if (m_pPreviousSelection)
        {
            // try to commit the previously selected index
            if (!m_pFields->SaveModified())
                return sal_False;

            OIndexCollection::iterator aPreviouslySelected = static_cast<OIndexCollection::iterator>(m_pPreviousSelection->GetUserData());

            // the unique flag
            if (m_aUnique.GetSavedValue() != m_aUnique.GetState())
            {
                aPreviouslySelected->setModified(sal_True);
                aPreviouslySelected->bUnique = m_aUnique.IsChecked();
            }

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

    //------------------------------------------------------------------
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
        DECLARE_STL_STDKEY_SET( String, StringBag );
        StringBag aExistentFields;
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

    //------------------------------------------------------------------
    sal_Bool DbaIndexDialog::implCommitPreviouslySelected()
    {
        if (m_pPreviousSelection)
        {
            OIndexCollection::iterator aPreviouslySelected = static_cast<OIndexCollection::iterator>(m_pPreviousSelection->GetUserData());

            if (!implSaveModified())
                return sal_False;

            // commit the index (if necessary)
            if (aPreviouslySelected->isModified() && !implCommit(m_pPreviousSelection))
                return sal_False;
        }

        return sal_True;
    }

    //------------------------------------------------------------------
    IMPL_LINK( DbaIndexDialog, OnModified, void*, NOTINTERESTEDIN )
    {
        DBG_ASSERT(m_pPreviousSelection, "DbaIndexDialog, OnModified: invalid call!");
        OIndexCollection::iterator aPosition = static_cast< OIndexCollection::iterator >(m_pPreviousSelection->GetUserData());

        aPosition->setModified(sal_True);
        updateToolbox();

        return 1L;
    }

    //------------------------------------------------------------------
    void DbaIndexDialog::updateControls(const SvLBoxEntry* _pEntry)
    {
        if (_pEntry)
        {
            // the descriptor of the selected index
            OIndexCollection::const_iterator aSelectedIndex = static_cast<OIndexCollection::iterator>(_pEntry->GetUserData());

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

    //------------------------------------------------------------------
    IMPL_LINK( DbaIndexDialog, OnIndexSelected, DbaIndexList*, NOTINTERESTEDIN )
    {
        m_aIndexes.EndSelection();

        if (m_aIndexes.IsEditingActive())
            m_aIndexes.EndEditing(sal_True);

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

        SvLBoxEntry* pNewSelection = m_aIndexes.FirstSelected();
        updateControls(pNewSelection);
        if (bHaveSelection)
            m_aIndexes.GrabFocus();

        m_pPreviousSelection = pNewSelection;

        updateToolbox();
        return 0L;
    }

//......................................................................
}   // namespace dbaui
//......................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/03/22 07:44:54  avy
 *  Some error checked for linux
 *
 *  Revision 1.2  2001/03/19 05:59:23  fs
 *  check plausibility before saving
 *
 *  Revision 1.1  2001/03/16 16:23:02  fs
 *  initial checkin - index design dialog and friends
 *
 *
 *  Revision 1.0 07.03.01 12:16:06  fs
 ************************************************************************/

