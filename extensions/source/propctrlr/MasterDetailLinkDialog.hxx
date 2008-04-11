/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterDetailLinkDialog.hxx,v $
 * $Revision: 1.3 $
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
#ifndef PROPCTRLR_MASTERDETAILLINKDIALOG_HXX
#define PROPCTRLR_MASTERDETAILLINKDIALOG_HXX
#include <svtools/genericunodialog.hxx>
#include "modulepcr.hxx"
//........................................................................
namespace pcr
{
//........................................................................

    class MasterDetailLinkDialog;
    typedef ::svt::OGenericUnoDialog                                            MasterDetailLinkDialog_DBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< MasterDetailLinkDialog >   MasterDetailLinkDialog_PBase;

    //========================================================================
    //= MasterDetailLinkDialog
    //========================================================================
    class MasterDetailLinkDialog :   public MasterDetailLinkDialog_DBase
                                    ,public MasterDetailLinkDialog_PBase
                                    ,public PcrClient
    {
    public:
        MasterDetailLinkDialog(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& _rxContext);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >&);
    private:
        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void implInitialize(const com::sun::star::uno::Any& _rValue);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xDetail;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xMaster;
        ::rtl::OUString m_sExplanation;
        ::rtl::OUString m_sDetailLabel;
        ::rtl::OUString m_sMasterLabel;
    };

//........................................................................
}   // namespace pcr
//........................................................................
#endif // PROPCTRLR_MASTERDETAILLINKDIALOG_HXX
