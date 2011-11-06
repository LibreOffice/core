/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
