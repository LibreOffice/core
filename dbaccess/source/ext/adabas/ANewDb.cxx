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
#include "ANewDb.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef adabasui_ADABAS_CREATEDB_HXX
#include "AdabasNewDb.hxx"
#endif
#ifndef adabasui_SHARED_DBUSTRINGS_HRC
#include "adabasuistrings.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace adabasui;
using namespace dbtools;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;

extern "C" void SAL_CALL createRegistryInfo_OAdabasCreateDialog()
{
    static OMultiInstanceAutoRegistration< OAdabasCreateDialog > aAutoRegistration;
}


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//=========================================================================
//-------------------------------------------------------------------------
OAdabasCreateDialog::OAdabasCreateDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :OAdabasCreateDialogBase(_rxORB)
    ,m_pDialog(NULL)
{
    registerProperty(PROPERTY_CREATECATALOG,    PROPERTY_ID_CREATECATALOG,      PropertyAttribute::TRANSIENT,&m_xCreateCatalog,     ::getCppuType(&m_xCreateCatalog));
    registerProperty(PROPERTY_DATABASENAME,     PROPERTY_ID_DATABASENAME,       PropertyAttribute::TRANSIENT,&m_sDatabaseName,      ::getCppuType(&m_sDatabaseName));
    registerProperty(PROPERTY_CONTROL_USER,     PROPERTY_ID_CONTROL_USER,       PropertyAttribute::TRANSIENT,&m_sControlUser,       ::getCppuType(&m_sControlUser));
    registerProperty(PROPERTY_CONTROL_PASSWORD, PROPERTY_ID_CONTROL_PASSWORD,   PropertyAttribute::TRANSIENT,&m_sControlPassword,   ::getCppuType(&m_sControlPassword));
    registerProperty(PROPERTY_USER,             PROPERTY_ID_USER,               PropertyAttribute::TRANSIENT,&m_sUser,              ::getCppuType(&m_sUser));
    registerProperty(PROPERTY_PASSWORD,         PROPERTY_ID_PASSWORD,           PropertyAttribute::TRANSIENT,&m_sUserPassword,      ::getCppuType(&m_sUserPassword));
    registerProperty(PROPERTY_CACHESIZE,        PROPERTY_ID_CACHESIZE,          PropertyAttribute::TRANSIENT,&m_nCacheSize,         ::getCppuType(&m_nCacheSize));
}

//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OAdabasCreateDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OAdabasCreateDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OAdabasCreateDialog(_rxFactory));
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OAdabasCreateDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString OAdabasCreateDialog::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.adabasui.AdabasCreateDialog");
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL OAdabasCreateDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence OAdabasCreateDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.AdabasCreationDialog");
    return aSupported;
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL OAdabasCreateDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OAdabasCreateDialog::getInfoHelper()
{
    return *const_cast<OAdabasCreateDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OAdabasCreateDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//------------------------------------------------------------------------------
Dialog* OAdabasCreateDialog::createDialog(Window* _pParent)
{
    if(!m_xCreateCatalog.is())
        throw SQLException();
    m_pDialog = new OAdabasNewDbDlg(_pParent,m_xCreateCatalog,m_aContext.getLegacyServiceFactory());
    return m_pDialog;
}
// -----------------------------------------------------------------------------
void OAdabasCreateDialog::executedDialog(sal_Int16 _nExecutionResult)
{
    if(m_pDialog && _nExecutionResult == RET_OK)
    { // fill the variables
        m_sDatabaseName     = m_pDialog->GetDatabaseName();
        m_sControlUser      = m_pDialog->GetControlUser();
        m_sControlPassword  = m_pDialog->GetControlPassword();
        m_sUser             = m_pDialog->GetUser();
        m_sUserPassword     = m_pDialog->GetUserPassword();
        m_nCacheSize        = m_pDialog->GetCacheSize();
    }
}
// -----------------------------------------------------------------------------

