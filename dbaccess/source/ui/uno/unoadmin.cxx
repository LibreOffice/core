/*************************************************************************
 *
 *  $RCSfile: unoadmin.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:27:03 $
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

#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBAUI_UNOADMIN_
#include "unoadmin.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// --- needed because of the solar mutex
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// ---

#define THISREF()   static_cast< XServiceInfo* >(this)

using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_ODatabaseAdministrationDialog()
{
    static OMultiInstanceAutoRegistration< ODatabaseAdministrationDialog > aAutoRegistration;
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
ODatabaseAdministrationDialog::ODatabaseAdministrationDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :OGenericUnoDialog(_rxORB)
    ,m_pDatasourceItems(NULL)
    ,m_pItemPool(NULL)
    ,m_pItemPoolDefaults(NULL)
{
}

//-------------------------------------------------------------------------
ODatabaseAdministrationDialog::~ODatabaseAdministrationDialog()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
    // so this virtual-method-call the base class does does not work, we're already dead then ...
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
            destroyDialog();
    }
}

//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL ODatabaseAdministrationDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseAdministrationDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODatabaseAdministrationDialog(_rxFactory));
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseAdministrationDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString ODatabaseAdministrationDialog::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.ODatasourceAdministrationDialog");
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL ODatabaseAdministrationDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence ODatabaseAdministrationDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatasourceAdministrationDialog");
    return aSupported;
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL ODatabaseAdministrationDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODatabaseAdministrationDialog::getInfoHelper()
{
    return *const_cast<ODatabaseAdministrationDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseAdministrationDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//-------------------------------------------------------------------------
void ODatabaseAdministrationDialog::destroyDialog()
{
    OGenericUnoDialog::destroyDialog();
    ODbAdminDialog::destroyItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults);
}

//------------------------------------------------------------------------------
Dialog* ODatabaseAdministrationDialog::createDialog(Window* _pParent)
{
    ODbAdminDialog::createItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults, &m_aCollection);
    ODbAdminDialog* pDialog = new ODbAdminDialog(_pParent, m_pDatasourceItems, m_xORB);
    pDialog->selectDataSource(m_sInitialSelection);
    return pDialog;
}

//------------------------------------------------------------------------------
void ODatabaseAdministrationDialog::implInitialize(const Any& _rValue)
{
    PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (aProperty.Name.equalsAsciiL("InitialSelection", sizeof("InitialSelection") - 1))
        {
            aProperty.Value >>= m_sInitialSelection;
            if (m_pDialog)
                static_cast<ODbAdminDialog*>(m_pDialog)->selectDataSource(m_sInitialSelection);
            return;
        }
    }
    OGenericUnoDialog::implInitialize(_rValue);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2001/01/05 12:16:07  fs
 *  adjusted the implementation name
 *
 *  Revision 1.5  2000/12/15 15:48:13  fs
 *  #82151# two instances of the registration helper module - one for every library in this project
 *
 *  Revision 1.4  2000/11/01 16:34:16  fs
 *  migrated from awt::XDialog to ui::XExecutableDialog / removed the star* namespace shortcuts
 *
 *  Revision 1.3  2000/10/31 08:08:48  fs
 *  support an initial selection
 *
 *  Revision 1.2  2000/10/27 08:08:47  fs
 *  don't include stringconstants.hrc directly anymore
 *
 *  Revision 1.1  2000/10/25 12:56:26  fs
 *  moved herein from ..\dlg
 *
 *  Revision 1.3  2000/10/16 10:46:59  fs
 *  service name changed
 *
 *  Revision 1.2  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.1  2000/10/05 10:07:23  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 12:12:44  fs
 ************************************************************************/

