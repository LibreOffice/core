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

#include "unoDirectSql.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <directsql.hxx>
#include <datasourceconnector.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sdb_DirectSQLDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::ODirectSQLDialog(context));
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;

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

    OUString SAL_CALL ODirectSQLDialog::getImplementationName()
    {
        return "com.sun.star.comp.sdb.DirectSQLDialog";
    }
    IMPLEMENT_SERVICE_INFO_SUPPORTS(ODirectSQLDialog)
    css::uno::Sequence< OUString > SAL_CALL ODirectSQLDialog::getSupportedServiceNames(  )
    {
        return { SERVICE_SDB_DIRECTSQLDIALOG };
    }

    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( ODirectSQLDialog )

    std::unique_ptr<weld::DialogController> ODirectSQLDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        // obtain all the objects needed for the dialog
        Reference< XConnection > xConnection = m_xActiveConnection;
        weld::Window* pParent = Application::GetFrameWeld(rParent);
        if ( !xConnection.is() )
        {
            try
            {
                // the connection the row set is working with
                ODatasourceConnector aDSConnector(m_aContext, pParent);
                xConnection = aDSConnector.connect( m_sInitialSelection, nullptr );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
        if (!xConnection.is())
        {
            // can't create the dialog if I have improper settings
            return nullptr;
        }

        return std::make_unique<DirectSQLDialog>(pParent, xConnection);
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
