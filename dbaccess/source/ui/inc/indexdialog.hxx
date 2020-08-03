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

#pragma once

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/weld.hxx>
#include "indexes.hxx"

namespace dbaui
{
    // DbaIndexDialog
    class IndexFieldsControl;
    class OIndexCollection;
    class DbaIndexDialog final : public weld::GenericDialogController
    {
        css::uno::Reference< css::sdbc::XConnection > m_xConnection;

        std::unique_ptr<OIndexCollection> m_xIndexes;
        std::unique_ptr<weld::TreeIter> m_xPreviousSelection;
        bool                            m_bEditingActive;
        bool                            m_bEditAgain;
        bool                            m_bNoHandlerCall;

        css::uno::Reference< css::uno::XComponentContext >
                                        m_xContext;

        std::unique_ptr<weld::Toolbar> m_xActions;
        std::unique_ptr<weld::TreeView> m_xIndexList;
        std::unique_ptr<weld::Label> m_xIndexDetails;
        std::unique_ptr<weld::Label> m_xDescriptionLabel;
        std::unique_ptr<weld::Label> m_xDescription;
        std::unique_ptr<weld::CheckButton> m_xUnique;
        std::unique_ptr<weld::Label> m_xFieldsLabel;
        std::unique_ptr<weld::Button> m_xClose;
        std::unique_ptr<weld::Container> m_xTable;
        css::uno::Reference<css::awt::XWindow> m_xTableCtrlParent;
        VclPtr<IndexFieldsControl> m_xFields;

    public:
        DbaIndexDialog(
            weld::Window* _pParent,
            const css::uno::Sequence< OUString >& _rFieldNames,
            const css::uno::Reference< css::container::XNameAccess >& _rxIndexes,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
        virtual ~DbaIndexDialog() override;

        typedef std::pair<const weld::TreeIter&, OUString> IterString;
    private:
        void fillIndexList();
        void updateToolbox();
        void updateControls(const weld::TreeIter* pEntry);

        void IndexSelected();

        DECL_LINK( OnIndexSelected, weld::TreeView&, void );
        DECL_LINK( OnIndexAction, const OString&, void );
        DECL_LINK( OnEntryEditing, const weld::TreeIter&, bool );
        DECL_LINK( OnEntryEdited, const IterString&, bool );
        DECL_LINK( OnModifiedClick, weld::Button&, void );
        DECL_LINK( OnModified, IndexFieldsControl&, void );
        DECL_LINK( OnCloseDialog, weld::Button&, void );

        DECL_LINK( OnEditIndexAgain, void*, void );

        void OnNewIndex();
        void OnDropIndex(bool _bConfirm = true);
        void OnRenameIndex();
        void OnSaveIndex();
        void OnResetIndex();

        bool implCommit(const weld::TreeIter* pEntry);
        bool implSaveModified(bool _bPlausibility = true);
        bool implCommitPreviouslySelected();

        bool implDropIndex(const weld::TreeIter* pEntry, bool _bRemoveFromCollection);

        bool implCheckPlausibility(const Indexes::const_iterator& _rPos);
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
