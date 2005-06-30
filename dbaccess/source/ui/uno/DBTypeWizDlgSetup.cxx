/*************************************************************************
 *
 *  $RCSfile: DBTypeWizDlgSetup.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-06-30 16:35:32 $
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

#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef DBAUI_DBTYPEWIZDLGSETUP_HXX
#include "DBTypeWizDlgSetup.hxx"
#endif
#ifndef DBAUI_DBWIZSETUP_HXX
#include "dbwizsetup.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XOFFICEDATABASEDOCUMENT_HPP_
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace dbaui;
namespace css = ::com::sun::star;

extern "C" void SAL_CALL createRegistryInfo_ODBTypeWizDialogSetup()
{
    static OMultiInstanceAutoRegistration< ODBTypeWizDialogSetup > aAutoRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;

//=========================================================================
//-------------------------------------------------------------------------
ODBTypeWizDialogSetup::ODBTypeWizDialogSetup(const Reference< XMultiServiceFactory >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
    ,m_bOpenDatabase(sal_True)
    ,m_bStartTableWizard(sal_False)
{
    registerProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OpenDatabase")), 3, PropertyAttribute::TRANSIENT,
        &m_bOpenDatabase, getBooleanCppuType());

    registerProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartTableWizard")), 4, PropertyAttribute::TRANSIENT,
        &m_bStartTableWizard, getBooleanCppuType());
}
//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL ODBTypeWizDialogSetup::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODBTypeWizDialogSetup::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    Reference < XInterface > xDBWizard = *(new ODBTypeWizDialogSetup(_rxFactory));
    return xDBWizard;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODBTypeWizDialogSetup::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString ODBTypeWizDialogSetup::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.ODBTypeWizDialogSetup"));
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL ODBTypeWizDialogSetup::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence ODBTypeWizDialogSetup::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatabaseWizardDialog"));
    return aSupported;
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL ODBTypeWizDialogSetup::getPropertySetInfo() throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODBTypeWizDialogSetup::getInfoHelper()
{
    return *const_cast<ODBTypeWizDialogSetup*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODBTypeWizDialogSetup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
//------------------------------------------------------------------------------
Dialog* ODBTypeWizDialogSetup::createDialog(Window* _pParent)
{
    return new ODbTypeWizDialogSetup(_pParent, m_pDatasourceItems, m_xORB, m_aInitialSelection);
}
// -----------------------------------------------------------------------------
void ODBTypeWizDialogSetup::executedDialog(sal_Int16 _nExecutionResult)
{
    if ( _nExecutionResult == RET_OK )
    {
        const ODbTypeWizDialogSetup* pDialog = static_cast< ODbTypeWizDialogSetup* >( m_pDialog );
        m_bOpenDatabase = pDialog->IsDatabaseDocumentToBeOpened();
        m_bStartTableWizard = pDialog->IsTableWizardToBeStarted();
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

