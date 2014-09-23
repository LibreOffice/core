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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXDIALOG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/toolbox.hxx>
#include <svtools/treelistbox.hxx>
#include <unotools/viewoptions.hxx>
#include "indexes.hxx"
#include <dbaccess/ToolBoxHelper.hxx>

namespace dbaui
{

    // DbaIndexList
    class DbaIndexList : public SvTreeListBox
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        Link        m_aSelectHdl;
        Link        m_aEndEditHdl;
        bool    m_bSuspendSelectHdl;

    public:
        DbaIndexList(vcl::Window* _pParent, WinBits nWinBits);

        void SetSelectHdl(const Link& _rHdl) { m_aSelectHdl = _rHdl; }
        Link GetSelectHdl() const { return m_aSelectHdl; }

        void SetEndEditHdl(const Link& _rHdl) { m_aEndEditHdl = _rHdl; }
        Link GetEndEditHdl() const { return m_aEndEditHdl; }

        virtual bool Select(SvTreeListEntry* pEntry, bool bSelect) SAL_OVERRIDE;

        void enableSelectHandler();
        void disableSelectHandler();

        void SelectNoHandlerCall( SvTreeListEntry* pEntry );

        inline void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection)
        {
             m_xConnection = _rxConnection;
        }

    protected:
        virtual bool EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    private:
        using SvTreeListBox::Select;
    };

    // DbaIndexDialog
    class IndexFieldsControl;
    class OIndexCollection;
    class DbaIndexDialog :  public ModalDialog,
                            public OToolBoxHelper
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        SvtViewOptions          m_aGeometrySettings;

        ToolBox                 *m_pActions;
        DbaIndexList            *m_pIndexList;
        FixedText               *m_pIndexDetails;
        FixedText               *m_pDescriptionLabel;
        FixedText               *m_pDescription;
        CheckBox                *m_pUnique;
        FixedText               *m_pFieldsLabel;
        IndexFieldsControl      *m_pFields;
        PushButton              *m_pClose;

        OIndexCollection*       m_pIndexes;
        SvTreeListEntry*            m_pPreviousSelection;
        bool                m_bEditAgain;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                m_xContext;
    public:
        DbaIndexDialog(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Sequence< OUString >& _rFieldNames,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            sal_Int32 _nMaxColumnsInIndex
            );
        virtual ~DbaIndexDialog();

        virtual void StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

        //TO-DO, remove when all other OToolBoxHelper are converted to .ui
        virtual void resizeControls(const Size&) SAL_OVERRIDE;

        /** will be called when the id of the image list needs to change
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
        */
        virtual void setImageList(sal_Int16 _eBitmapSet) SAL_OVERRIDE;
    protected:
        void fillIndexList();
        void updateToolbox();
        void updateControls(const SvTreeListEntry* _pEntry);

    protected:
        DECL_LINK( OnIndexSelected, DbaIndexList* );
        DECL_LINK( OnIndexAction, ToolBox* );
        DECL_LINK( OnEntryEdited, SvTreeListEntry* );
        DECL_LINK( OnModified, void* );
        DECL_LINK( OnCloseDialog, void* );

        DECL_LINK( OnEditIndexAgain, SvTreeListEntry* );

    private:
        sal_uInt16 mnNewCmdId;
        sal_uInt16 mnDropCmdId;
        sal_uInt16 mnRenameCmdId;
        sal_uInt16 mnSaveCmdId;
        sal_uInt16 mnResetCmdId;

        Image maScNewCmdImg;
        Image maScDropCmdImg;
        Image maScRenameCmdImg;
        Image maScSaveCmdImg;
        Image maScResetCmdImg;
        Image maLcNewCmdImg;
        Image maLcDropCmdImg;
        Image maLcRenameCmdImg;
        Image maLcSaveCmdImg;
        Image maLcResetCmdImg;

        void OnNewIndex();
        void OnDropIndex(bool _bConfirm = true);
        void OnRenameIndex();
        void OnSaveIndex();
        void OnResetIndex();

        bool implCommit(SvTreeListEntry* _pEntry);
        bool implSaveModified(bool _bPlausibility = true);
        bool implCommitPreviouslySelected();

        bool implDropIndex(SvTreeListEntry* _pEntry, bool _bRemoveFromCollection);

        bool implCheckPlausibility(const Indexes::const_iterator& _rPos);

        /** checks if the controls have to be replaced and moved.
        */
        void checkControls();
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
