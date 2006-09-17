/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: admindlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:33:40 $
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
#ifndef _DBAUI_ADMINDLG_HXX
#include "admindlg.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif


using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_ODataSourcePropertyDialog()
{
    static OMultiInstanceAutoRegistration< ODataSourcePropertyDialog > aAutoRegistration;
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
ODataSourcePropertyDialog::ODataSourcePropertyDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
{
}
//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL ODataSourcePropertyDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODataSourcePropertyDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODataSourcePropertyDialog(_rxFactory));
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODataSourcePropertyDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString ODataSourcePropertyDialog::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.ODatasourceAdministrationDialog"));
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL ODataSourcePropertyDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence ODataSourcePropertyDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatasourceAdministrationDialog"));
    return aSupported;
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL ODataSourcePropertyDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODataSourcePropertyDialog::getInfoHelper()
{
    return *const_cast<ODataSourcePropertyDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODataSourcePropertyDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
//------------------------------------------------------------------------------
Dialog* ODataSourcePropertyDialog::createDialog(Window* _pParent)
{

    ODbAdminDialog* pDialog = new ODbAdminDialog(_pParent, m_pDatasourceItems, m_xORB);

    // the initial selection
    if ( m_aInitialSelection.hasValue() )
        pDialog->selectDataSource(m_aInitialSelection);

    return pDialog;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

