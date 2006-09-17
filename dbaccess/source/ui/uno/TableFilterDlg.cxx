/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableFilterDlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:33:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBAUI_TABLEFILTERDLG_HXX
#include "TableFilterDlg.hxx"
#endif
#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#include "TablesSingleDlg.hxx"
#endif


using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_OTableFilterDialog()
{
    static OMultiInstanceAutoRegistration< OTableFilterDialog > aAutoRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

//=========================================================================
//-------------------------------------------------------------------------
OTableFilterDialog::OTableFilterDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
{
}
//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OTableFilterDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OTableFilterDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OTableFilterDialog(_rxFactory));
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTableFilterDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString OTableFilterDialog::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.OTableFilterDialog"));
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL OTableFilterDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence OTableFilterDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.TableFilterDialog"));
    return aSupported;
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL OTableFilterDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OTableFilterDialog::getInfoHelper()
{
    return *const_cast<OTableFilterDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OTableFilterDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
//------------------------------------------------------------------------------
Dialog* OTableFilterDialog::createDialog(Window* _pParent)
{
    OTableSubscriptionDialog* pDlg = new OTableSubscriptionDialog(_pParent, m_pDatasourceItems, m_xORB,m_aInitialSelection);
    return pDlg;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

