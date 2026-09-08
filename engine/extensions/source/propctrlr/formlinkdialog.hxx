/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <vcl/weld.hxx>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cpo/uno/XComponentContext.hpp>

#include <memory>


namespace pcr
{
    class FieldLinkRow;

    //= FormLinkDialog

    class FormLinkDialog : public weld::GenericDialogController
    {
    private:
        cpo::uno::Reference< cpo::uno::XComponentContext >
                                        m_xContext;
        cpo::uno::Reference< css::beans::XPropertySet >
                                        m_xDetailForm;
        cpo::uno::Reference< css::beans::XPropertySet >
                                        m_xMasterForm;

        std::vector< OUString >         m_aRelationDetailColumns;
        std::vector< OUString >         m_aRelationMasterColumns;

        OUString                 m_sDetailLabel;
        OUString                 m_sMasterLabel;

        std::unique_ptr<weld::Label> m_xExplanation;
        std::unique_ptr<weld::Label> m_xDetailLabel;
        std::unique_ptr<weld::Label> m_xMasterLabel;
        std::unique_ptr<FieldLinkRow> m_xRow1;
        std::unique_ptr<FieldLinkRow> m_xRow2;
        std::unique_ptr<FieldLinkRow> m_xRow3;
        std::unique_ptr<FieldLinkRow> m_xRow4;
        std::unique_ptr<weld::Button> m_xOK;
        std::unique_ptr<weld::Button> m_xSuggest;

    public:
        FormLinkDialog(
            weld::Window* _pParent,
            const cpo::uno::Reference< css::beans::XPropertySet >& _rxDetailForm,
            const cpo::uno::Reference< css::beans::XPropertySet >& _rxMasterForm,
            const cpo::uno::Reference< cpo::uno::XComponentContext >& _rxContext,
            const OUString& _sExplanation = OUString(),
            OUString _sDetailLabel = OUString(),
            OUString _sMasterLabel = OUString()
        );

        virtual ~FormLinkDialog() override;

        // Dialog overridables
        virtual short run() override;

    private:
        DECL_LINK( OnSuggest,    weld::Button&, void );
        DECL_LINK( OnFieldChanged, FieldLinkRow&, void );
        DECL_LINK( OnInitialize, void*, void);

        void        updateOkButton();
        void        initializeFieldLists();
        void        initializeColumnLabels();
        void        initializeLinks();
        void        initializeSuggest();
        void        commitLinkPairs();

        void        initializeFieldRowsFrom(
                        std::vector< OUString >& _rDetailFields,
                        std::vector< OUString >& _rMasterFields
                    );

        static OUString getFormDataSourceType(
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxForm
                    );

        void        getFormFields(
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxForm,
                            cpo::uno::Sequence< OUString >& /* [out] */ _rNames
                    ) const;

        void        ensureFormConnection(
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxFormProps,
                            cpo::uno::Reference< css::sdbc::XConnection >& /* [out] */ _rxConnection
                    ) const;

        static void getConnectionMetaData(
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxFormProps,
                            cpo::uno::Reference< css::sdbc::XDatabaseMetaData >& /* [out] */ _rxMeta
                    );

        cpo::uno::Reference< css::beans::XPropertySet >
                    getCanonicUnderlyingTable( const cpo::uno::Reference< css::beans::XPropertySet >& _rxFormProps ) const;
        static bool getExistingRelation(
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxLHS,
                        const cpo::uno::Reference< css::beans::XPropertySet >& _rxRHS,
                        std::vector< OUString >& /* [out] */ _rLeftFields,
                        std::vector< OUString >& /* [out] */ _rRightFields
                    );
    };

}   // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
