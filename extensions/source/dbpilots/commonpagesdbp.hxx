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

#include "controlwizard.hxx"
#include <vcl/weld.hxx>
#include <com/sun/star/sdb/XDatabaseContext.hpp>

namespace dbp
{
    class OTableSelectionPage final : public OControlWizardPage
    {
        std::unique_ptr<weld::TreeView> m_xTable;
        std::unique_ptr<weld::TreeView> m_xDatasource;
        std::unique_ptr<weld::Label> m_xDatasourceLabel;
        std::unique_ptr<weld::Button> m_xSearchDatabase;
        std::unique_ptr<weld::Container> m_xSourceBox;

        css::uno::Reference< css::sdb::XDatabaseContext >
                                m_xDSContext;

    public:
        explicit OTableSelectionPage(weld::Container* pPage, OControlWizard* pParent);
        virtual ~OTableSelectionPage() override;

    private:
        // BuilderPage overridables
        void Activate() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        DECL_LINK( OnListboxSelection, weld::TreeView&, void );
        DECL_LINK( OnListboxDoubleClicked, weld::TreeView&, bool );
        DECL_LINK( OnSearchClicked, weld::Button&, void );

        void implFillTables(const css::uno::Reference< css::sdbc::XConnection >&
                        _rxConn = css::uno::Reference< css::sdbc::XConnection >());

        // OControlWizardPage overridables
        virtual bool    canAdvance() const override;
    };

    class OMaybeListSelectionPage : public OControlWizardPage
    {
        weld::RadioButton* m_pYes;
        weld::RadioButton* m_pNo;
        weld::ComboBox* m_pList;

    public:
        OMaybeListSelectionPage(weld::Container* pPage, OControlWizard* pWizard, const OUString& rUIXMLDescription, const OString& rID);
        virtual ~OMaybeListSelectionPage() override;

    protected:
        DECL_LINK( OnRadioSelected, weld::Button&, void );

        // BuilderPage overridables
        void Activate() override;

        // own helper
        void    announceControls(
            weld::RadioButton& _rYesButton,
            weld::RadioButton& _rNoButton,
            weld::ComboBox& _rSelection);

        void implEnableWindows();

        void implInitialize(const OUString& _rSelection);
        void implCommit(OUString& _rSelection);
    };

    class ODBFieldPage : public OMaybeListSelectionPage
    {
    protected:
        std::unique_ptr<weld::Label> m_xDescription;
        std::unique_ptr<weld::RadioButton> m_xStoreYes;
        std::unique_ptr<weld::RadioButton> m_xStoreNo;
        std::unique_ptr<weld::ComboBox> m_xStoreWhere;

    public:
        explicit ODBFieldPage(weld::Container* pPage, OControlWizard* pWizard);
        virtual ~ODBFieldPage() override;

    protected:
        void setDescriptionText(const OUString& rDesc)
        {
            m_xDescription->set_label(rDesc);
        }

        // OWizardPage overridables
        virtual void initializePage() override;
        virtual bool commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        // own overridables
        virtual OUString& getDBFieldSetting() = 0;
    };


}   // namespace dbp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
