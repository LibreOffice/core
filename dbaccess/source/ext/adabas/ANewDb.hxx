/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ANewDb.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-05 08:44:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


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
