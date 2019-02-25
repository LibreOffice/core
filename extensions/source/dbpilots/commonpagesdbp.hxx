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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_COMMONPAGESDBP_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_COMMONPAGESDBP_HXX

#include "controlwizard.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/sdb/XDatabaseContext.hpp>


namespace dbp
{

    class OTableSelectionPage final : public OControlWizardPage
    {
        VclPtr<FixedText>       m_pDatasourceLabel;
        VclPtr<ListBox>         m_pDatasource;
        VclPtr<PushButton>      m_pSearchDatabase;
        VclPtr<ListBox>         m_pTable;

        css::uno::Reference< css::sdb::XDatabaseContext >
                                m_xDSContext;

    public:
        explicit OTableSelectionPage(OControlWizard* _pParent);
        virtual ~OTableSelectionPage() override;
        virtual void dispose() override;

    private:
        // TabPage overridables
        void ActivatePage() override;

        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

        DECL_LINK( OnListboxSelection, ListBox&, void );
        DECL_LINK( OnListboxDoubleClicked, ListBox&, void );
        DECL_LINK( OnSearchClicked, Button*, void );

        void implFillTables(const css::uno::Reference< css::sdbc::XConnection >&
                        _rxConn = css::uno::Reference< css::sdbc::XConnection >());

        // OControlWizardPage overridables
        virtual bool    canAdvance() const override;
    };

    class OMaybeListSelectionPage : public OControlWizardPage
    {
        VclPtr<RadioButton>    m_pYes;
        VclPtr<RadioButton>    m_pNo;
        VclPtr<ListBox>        m_pList;

    public:
        OMaybeListSelectionPage( OControlWizard* _pParent, const OString& _rID, const OUString& _rUIXMLDescription );
        virtual ~OMaybeListSelectionPage() override;
        virtual void dispose() override;

    protected:
        DECL_LINK( OnRadioSelected, Button*, void );

        // TabPage overridables
        void ActivatePage() override;

        // own helper
        void    announceControls(
            RadioButton& _rYesButton,
            RadioButton& _rNoButton,
            ListBox& _rSelection);

        void implEnableWindows();

        void implInitialize(const OUString& _rSelection);
        void implCommit(OUString& _rSelection);
    };

    class ODBFieldPage : public OMaybeListSelectionPage
    {
    protected:
        VclPtr<FixedText>      m_pDescription;
        VclPtr<RadioButton>    m_pStoreYes;
        VclPtr<RadioButton>    m_pStoreNo;
        VclPtr<ListBox>        m_pStoreWhere;

    public:
        explicit ODBFieldPage( OControlWizard* _pParent );
        virtual ~ODBFieldPage() override;
        virtual void dispose() override;

    protected:
        void setDescriptionText(const OUString& rDesc)
        {
            m_pDescription->set_width_request(-1);
            m_pDescription->set_height_request(-1);

            auto nWidthAvail = GetParent()->GetSizePixel().Width();

            m_pDescription->SetText(rDesc);

            //tdf#122307 wrap based on current wizard width
            Size aPrefSize(m_pDescription->get_preferred_size());
            Size aSize(m_pDescription->CalcMinimumSize(nWidthAvail));
            if (aSize.Height() > aPrefSize.Height())
            {
                m_pDescription->set_width_request(aSize.Width());
                m_pDescription->set_height_request(aSize.Height());
            }
        }

        // OWizardPage overridables
        virtual void initializePage() override;
        virtual bool commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

        // own overridables
        virtual OUString& getDBFieldSetting() = 0;
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_COMMONPAGESDBP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
