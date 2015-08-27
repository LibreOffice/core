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

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        m_xDetailForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        m_xMasterForm;

        ::com::sun::star::uno::Sequence< OUString >
                                        m_aRelationDetailColumns;
        ::com::sun::star::uno::Sequence< OUString >
                                        m_aRelationMasterColumns;

        OUString                 m_sDetailLabel;
        OUString                 m_sMasterLabel;

    public:
        FormLinkDialog(
            vcl::Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDetailForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxMasterForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const OUString& _sExplanation = OUString(),
            const OUString& _sDetailLabel = OUString(),
            const OUString& _sMasterLabel = OUString()
        );
        virtual ~FormLinkDialog( );
        virtual void dispose() SAL_OVERRIDE;

        // Dialog overridables
        virtual short   Execute() SAL_OVERRIDE;

    private:
        DECL_LINK_TYPED( OnSuggest,    Button*, void );
        DECL_LINK( OnFieldChanged,     FieldLinkRow* );
        DECL_LINK_TYPED( OnInitialize, void*, void);

        void        updateOkButton();
        void        initializeFieldLists();
        void        initializeColumnLabels();
        void        initializeLinks();
        void        initializeSuggest();
        void        commitLinkPairs();

        void        initializeFieldRowsFrom(
                        ::com::sun::star::uno::Sequence< OUString >& _rDetailFields,
                        ::com::sun::star::uno::Sequence< OUString >& _rMasterFields
                    );

        static OUString getFormDataSourceType(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm
                    );

        void        getFormFields(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm,
                            ::com::sun::star::uno::Sequence< OUString >& /* [out] */ _rNames
                    ) const;

        void        ensureFormConnection(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& /* [out] */ _rxConnection
                    ) const;

        static void getConnectionMetaData(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& /* [out] */ _rxMeta
                    );

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    getCanonicUnderlyingTable( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps ) const;
        static bool getExistingRelation(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxLHS,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxRHS,
                        ::com::sun::star::uno::Sequence< OUString >& /* [out] */ _rLeftFields,
                        ::com::sun::star::uno::Sequence< OUString >& /* [out] */ _rRightFields
                    );
    };


}   // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
