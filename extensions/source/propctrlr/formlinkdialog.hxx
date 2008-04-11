/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formlinkdialog.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

/** === begin UNO includes === **/
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#include <memory>

//............................................................................
namespace pcr
{
//............................................................................

    class FieldLinkRow;
    //========================================================================
    //= FormLinkDialog
    //========================================================================
    class FormLinkDialog : public ModalDialog
    {
    private:
        FixedText                       m_aExplanation;
        FixedText                       m_aDetailLabel;
        FixedText                       m_aMasterLabel;
        ::std::auto_ptr< FieldLinkRow > m_aRow1;
        ::std::auto_ptr< FieldLinkRow > m_aRow2;
        ::std::auto_ptr< FieldLinkRow > m_aRow3;
        ::std::auto_ptr< FieldLinkRow > m_aRow4;
        OKButton                        m_aOK;
        CancelButton                    m_aCancel;
        HelpButton                      m_aHelp;
        PushButton                      m_aSuggest;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        m_xDetailForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        m_xMasterForm;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                        m_aRelationDetailColumns;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                        m_aRelationMasterColumns;

        ::rtl::OUString                 m_sDetailLabel;
        ::rtl::OUString                 m_sMasterLabel;

    public:
        FormLinkDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDetailForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxMasterForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _sExplanation = ::rtl::OUString(),
            const ::rtl::OUString& _sDetailLabel = ::rtl::OUString(),
            const ::rtl::OUString& _sMasterLabel = ::rtl::OUString()
        );
        ~FormLinkDialog( );

        // Dialog overridables
        virtual short   Execute();

    private:
        DECL_LINK( OnSuggest,          void*         );
        DECL_LINK( OnFieldChanged,     FieldLinkRow* );
        DECL_LINK( OnInitialize, void*         );

        void        updateOkButton();
        void        initializeFieldLists();
        void        initializeColumnLabels();
        void        initializeLinks();
        void        initializeSuggest();
        void        commitLinkPairs();

        void        initializeFieldRowsFrom(
                        ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rDetailFields,
                        ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rMasterFields
                    );

        String      getFormDataSourceType(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm
                    ) const SAL_THROW(());

        void        getFormFields(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm,
                            ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* [out] */ _rNames
                    ) const SAL_THROW(());

        void        ensureFormConnection(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& /* [out] */ _rxConnection
                    ) const SAL_THROW(( ::com::sun::star::uno::Exception ));

        void        getConnectionMetaData(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& /* [out] */ _rxMeta
                    ) const SAL_THROW(( ::com::sun::star::uno::Exception ));

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    getCanonicUnderlyingTable( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxFormProps ) const;
        sal_Bool    getExistingRelation(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxLHS,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxRHS,
                        ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* [out] */ _rLeftFields,
                        ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* [out] */ _rRightFields
                    ) const;
    };

//............................................................................
}   // namespace pcr
//............................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX
