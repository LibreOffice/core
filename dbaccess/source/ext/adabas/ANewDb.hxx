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




#ifndef ADABASUI_NEWDB_HXX
#define ADABASUI_NEWDB_HXX

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "Acomponentmodule.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCREATECATALOG_HPP_
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#endif



//.........................................................................
namespace adabasui
{
//.........................................................................
    class OAdabasNewDbDlg;
    typedef ::svt::OGenericUnoDialog OAdabasCreateDialogBase;
    class OAdabasCreateDialog
            :public OAdabasCreateDialogBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdabasCreateDialog >
            ,public OModuleResourceClient
    {
    protected:
        OAdabasNewDbDlg* m_pDialog; // just to avoid a cast
        // <properties>
        ::rtl::OUString m_sDatabaseName;
        ::rtl::OUString m_sControlUser;
        ::rtl::OUString m_sControlPassword;
        ::rtl::OUString m_sUser;
        ::rtl::OUString m_sUserPassword;
        sal_Int32       m_nCacheSize;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XCreateCatalog> m_xCreateCatalog;
        // </properties>
    protected:
        OAdabasCreateDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    public:
        // XTypeProvider
        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );
        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
                SAL_CALL Create(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };
//.........................................................................
}   // namespace adabasui
//.........................................................................

#endif // ADABASUI_NEWDB_HXX
