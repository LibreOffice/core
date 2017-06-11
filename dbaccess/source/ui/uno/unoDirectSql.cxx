/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include "uiservices.hxx"
#include "unoDirectSql.hxx"
#include "dbu_reghelper.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "directsql.hxx"
#include "stringconstants.hxx"
#include "datasourceconnector.hxx"
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>

extern "C" void SAL_CALL createRegistryInfo_ODirectSQLDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::ODirectSQLDialog > aAutoRegistration;
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    // ODirectSQLDialog
    ODirectSQLDialog::ODirectSQLDialog(const Reference< XComponentContext >& _rxORB)
        :ODirectSQLDialog_BASE( _rxORB )
    {

    }

    ODirectSQLDialog::~ODirectSQLDialog()
    {

    }

    css::uno::Sequence<sal_Int8> ODirectSQLDialog::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(ODirectSQLDialog, "com.sun.star.comp.sdb.DirectSQLDialog")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(ODirectSQLDialog)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(ODirectSQLDialog, SERVICE_SDB_DIRECTSQLDIALOG)

    css::uno::Reference< css::uno::XInterface >
        SAL_CALL ODirectSQLDialog::Create(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new ODirectSQLDialog( comphelper::getComponentContext(_rxORB)));
    }

    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( ODirectSQLDialog )

    VclPtr<Dialog> ODirectSQLDialog::createDialog(vcl::Window* _pParent)
    {
        // obtain all the objects needed for the dialog
        Reference< XConnection > xConnection = m_xActiveConnection;
        if ( !xConnection.is() )
        {
            try
            {
                // the connection the row set is working with
                ODatasourceConnector aDSConnector(m_aContext, _pParent);
                xConnection = aDSConnector.connect( m_sInitialSelection, nullptr );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        if ( !xConnection.is() )
            // can't create the dialog if I have improper settings
            return nullptr;

        return VclPtr<DirectSQLDialog>::Create( _pParent, xConnection );
    }
    void ODirectSQLDialog::implInitialize(const Any& _rValue)
    {
        PropertyValue aProperty;
        if (_rValue >>= aProperty)
        {
            if (aProperty.Name == "InitialSelection")
            {
                OSL_VERIFY( aProperty.Value >>= m_sInitialSelection );
                return;
            }
            else if (aProperty.Name == "ActiveConnection")
            {
                m_xActiveConnection.set( aProperty.Value, UNO_QUERY );
                OSL_ENSURE( m_xActiveConnection.is(), "ODirectSQLDialog::implInitialize: invalid connection!" );
                return;
            }
        }
        ODirectSQLDialog_BASE::implInitialize(_rValue);
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
