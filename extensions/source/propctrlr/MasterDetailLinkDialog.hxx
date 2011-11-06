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
