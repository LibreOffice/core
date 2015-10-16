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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>


namespace pcr
{


    class FieldLinkRow;

    //= FormLinkDialog

    class FormLinkDialog : public ModalDialog
    {
    private:
        VclPtr<FixedText>                      m_pExplanation;
        VclPtr<FixedText>                      m_pDetailLabel;
        VclPtr<FixedText>                      m_pMasterLabel;
        VclPtr<FieldLinkRow>                   m_aRow1;
        VclPtr<FieldLinkRow>                   m_aRow2;
        VclPtr<FieldLinkRow>                   m_aRow3;
        VclPtr<FieldLinkRow>                   m_aRow4;
        VclPtr<OKButton>                       m_pOK;
        VclPtr<PushButton>                     m_pSuggest;

        css::uno::Reference< css::uno::XComponentContext >
                                        m_xContext;
        css::uno::Reference< css::beans::XPropertySet >
                                        m_xDetailForm;
        css::uno::Reference< css::beans::XPropertySet >
                                        m_xMasterForm;

        css::uno::Sequence< OUString >
                                        m_aRelationDetailColumns;
        css::uno::Sequence< OUString >
                                        m_aRelationMasterColumns;

        OUString                 m_sDetailLabel;
        OUString                 m_sMasterLabel;

    public:
        FormLinkDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxDetailForm,
            const css::uno::Reference< css::beans::XPropertySet >& _rxMasterForm,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const OUString& _sExplanation = OUString(),
            const OUString& _sDetailLabel = OUString(),
            const OUString& _sMasterLabel = OUString()
        );
        virtual ~FormLinkDialog( );
        virtual void dispose() override;

        // Dialog overridables
        virtual short   Execute() override;

    private:
        DECL_LINK_TYPED( OnSuggest,    Button*, void );
        DECL_LINK_TYPED( OnFieldChanged, FieldLinkRow&, void );
        DECL_LINK_TYPED( OnInitialize, void*, void);

        void        updateOkButton();
        void        initializeFieldLists();
        void        initializeColumnLabels();
        void        initializeLinks();
        void        initializeSuggest();
        void        commitLinkPairs();

        void        initializeFieldRowsFrom(
                        css::uno::Sequence< OUString >& _rDetailFields,
                        css::uno::Sequence< OUString >& _rMasterFields
                    );

        static OUString getFormDataSourceType(
                        const css::uno::Reference< css::beans::XPropertySet >& _rxForm
                    );

        void        getFormFields(
                        const css::uno::Reference< css::beans::XPropertySet >& _rxForm,
                            css::uno::Sequence< OUString >& /* [out] */ _rNames
                    ) const;

        void        ensureFormConnection(
                        const css::uno::Reference< css::beans::XPropertySet >& _rxFormProps,
                            css::uno::Reference< css::sdbc::XConnection >& /* [out] */ _rxConnection
                    ) const;

        static void getConnectionMetaData(
                        const css::uno::Reference< css::beans::XPropertySet >& _rxFormProps,
                            css::uno::Reference< css::sdbc::XDatabaseMetaData >& /* [out] */ _rxMeta
                    );

        css::uno::Reference< css::beans::XPropertySet >
                    getCanonicUnderlyingTable( const css::uno::Reference< css::beans::XPropertySet >& _rxFormProps ) const;
        static bool getExistingRelation(
                        const css::uno::Reference< css::beans::XPropertySet >& _rxLHS,
                        const css::uno::Reference< css::beans::XPropertySet >& _rxRHS,
                        css::uno::Sequence< OUString >& /* [out] */ _rLeftFields,
                        css::uno::Sequence< OUString >& /* [out] */ _rRightFields
                    );
    };


}   // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
