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
        css::uno::Reference< css::sdbc::XConnection > m_xConnection;
        Link<DbaIndexList&,void>                      m_aSelectHdl;
        Link<SvTreeListEntry*,bool>                   m_aEndEditHdl;
        bool                                          m_bSuspendSelectHdl;

    public:
        DbaIndexList(vcl::Window* _pParent, WinBits nWinBits);

        void SetSelectHdl(const Link<DbaIndexList&,void>& _rHdl) { m_aSelectHdl = _rHdl; }

        void SetEndEditHdl(const Link<SvTreeListEntry*,bool>& _rHdl) { m_aEndEditHdl = _rHdl; }

        virtual bool Select(SvTreeListEntry* pEntry, bool bSelect = true) override;

        void enableSelectHandler();
        void disableSelectHandler();

        void SelectNoHandlerCall( SvTreeListEntry* pEntry );

        inline void setConnection(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection)
        {
             m_xConnection = _rxConnection;
        }

    protected:
        virtual bool EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

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
        css::uno::Reference< css::sdbc::XConnection > m_xConnection;

        VclPtr<ToolBox>                 m_pActions;
        VclPtr<DbaIndexList>            m_pIndexList;
        VclPtr<FixedText>               m_pIndexDetails;
        VclPtr<FixedText>               m_pDescriptionLabel;
        VclPtr<FixedText>               m_pDescription;
        VclPtr<CheckBox>                m_pUnique;
        VclPtr<FixedText>               m_pFieldsLabel;
        VclPtr<IndexFieldsControl>      m_pFields;
        VclPtr<PushButton>              m_pClose;

        OIndexCollection*               m_pIndexes;
        SvTreeListEntry*                m_pPreviousSelection;
        bool                            m_bEditAgain;

        css::uno::Reference< css::uno::XComponentContext >
                                        m_xContext;
    public:
        DbaIndexDialog(
            vcl::Window* _pParent,
            const css::uno::Sequence< OUString >& _rFieldNames,
            const css::uno::Reference< css::container::XNameAccess >& _rxIndexes,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            sal_Int32 _nMaxColumnsInIndex
            );
        virtual ~DbaIndexDialog();
        virtual void dispose() override;

        virtual void StateChanged( StateChangedType nStateChange ) override;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

        //TO-DO, remove when all other OToolBoxHelper are converted to .ui
        virtual void resizeControls(const Size&) override;

        /** will be called when the id of the image list needs to change
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
        */
        virtual void setImageList(sal_Int16 _eBitmapSet) override;
    protected:
        void fillIndexList();
        void updateToolbox();
        void updateControls(const SvTreeListEntry* _pEntry);

    protected:
        DECL_LINK_TYPED( OnIndexSelected, DbaIndexList&, void );
        DECL_LINK_TYPED( OnIndexAction, ToolBox*, void );
        DECL_LINK_TYPED( OnEntryEdited, SvTreeListEntry*, bool );
        DECL_LINK_TYPED( OnModifiedClick, Button*, void );
        DECL_LINK_TYPED( OnModified, IndexFieldsControl&, void );
        DECL_LINK_TYPED( OnCloseDialog, Button*, void );

        DECL_LINK_TYPED( OnEditIndexAgain, void*, void );

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
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
