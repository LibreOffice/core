/*************************************************************************
 *
 *  $RCSfile: formlinkdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-03-19 12:01:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_FORMLINKDIALOG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
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
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                                        m_xDetailForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                                        m_xMasterForm;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                        m_aRelationDetailColumns;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                        m_aRelationMasterColumns;

    public:
        FormLinkDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxDetailForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxMasterForm,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
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
                        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm
                    ) const SAL_THROW(());

        void        getFormFields(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
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
