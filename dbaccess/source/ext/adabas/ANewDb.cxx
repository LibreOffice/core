/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "ANewDb.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include "AdabasNewDb.hxx"
#include "adabasuistrings.hrc"
#include <vcl/msgbox.hxx>

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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.adabasui.AdabasCreateDialog"));
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
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.AdabasCreationDialog"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
