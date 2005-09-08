/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DBTypeWizDlgSetup.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:48:47 $
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

#ifndef DBAUI_DBTYPEWIZDLGSETUP_HXX
#define DBAUI_DBTYPEWIZDLGSETUP_HXX

#ifndef _DBAUI_UNOADMIN_
#include "unoadmin.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
//=========================================================================
//= ODBTypeWizDialogSetup
//=========================================================================
class ODBTypeWizDialogSetup
        :public ODatabaseAdministrationDialog
        ,public ::comphelper::OPropertyArrayUsageHelper< ODBTypeWizDialogSetup >
{
    ::rtl::OUString m_sExistingDocToOpen;
    sal_Bool        m_bOpenDatabase;
    sal_Bool        m_bStartTableWizard;

protected:
    ODBTypeWizDialogSetup(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

public:
    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
protected:
// OGenericUnoDialog overridables
    virtual Dialog* createDialog(Window* _pParent);
    virtual void executedDialog(sal_Int16 _nExecutionResult);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DBTYPEWIZDLG_HXX

