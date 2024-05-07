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

#include "composerdialogs.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <queryfilter.hxx>
#include <queryorder.hxx>
#include <strings.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_uno_comp_sdb_RowsetOrderDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::RowsetOrderDialog(context));
}
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_uno_comp_sdb_RowsetFilterDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::RowsetFilterDialog(context));
}

namespace dbaui
{

#define PROPERTY_ID_QUERYCOMPOSER       100
#define PROPERTY_ID_ROWSET              101

constexpr OUStringLiteral PROPERTY_QUERYCOMPOSER = u"QueryComposer";
constexpr OUStringLiteral PROPERTY_ROWSET = u"RowSet";

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    // ComposerDialog
    ComposerDialog::ComposerDialog(const Reference< XComponentContext >& _rxORB)
        :OGenericUnoDialog( _rxORB )
    {

        registerProperty( PROPERTY_QUERYCOMPOSER, PROPERTY_ID_QUERYCOMPOSER, PropertyAttribute::TRANSIENT,
            &m_xComposer, cppu::UnoType<decltype(m_xComposer)>::get() );
        registerProperty( PROPERTY_ROWSET, PROPERTY_ID_ROWSET, PropertyAttribute::TRANSIENT,
            &m_xRowSet, cppu::UnoType<decltype(m_xRowSet)>::get() );
    }

    ComposerDialog::~ComposerDialog()
    {

    }

    css::uno::Sequence<sal_Int8> ComposerDialog::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL ComposerDialog::getPropertySetInfo()
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }
    ::cppu::IPropertyArrayHelper& ComposerDialog::getInfoHelper()
    {
        return *ComposerDialog::getArrayHelper();
    }
    ::cppu::IPropertyArrayHelper* ComposerDialog::createArrayHelper( ) const
    {
        css::uno::Sequence< css::beans::Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    std::unique_ptr<weld::DialogController> ComposerDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        // obtain all the objects needed for the dialog
        Reference< XConnection > xConnection;
        Reference< XNameAccess > xColumns;
        try
        {
            // the connection the row set is working with
            if ( !::dbtools::isEmbeddedInDatabase( m_xRowSet, xConnection ) )
            {
                Reference< XPropertySet > xRowsetProps( m_xRowSet, UNO_QUERY );
                if ( xRowsetProps.is() )
                    xRowsetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            }

            // fallback: if there is a connection and thus a row set, but no composer, create one
            if ( xConnection.is() && !m_xComposer.is() )
                m_xComposer = ::dbtools::getCurrentSettingsComposer( Reference< XPropertySet >( m_xRowSet, UNO_QUERY ), m_aContext, rParent );

            // the columns of the row set
            Reference< XColumnsSupplier > xSuppColumns( m_xRowSet, UNO_QUERY );
            if ( xSuppColumns.is() )
                xColumns = xSuppColumns->getColumns();

            if ( !xColumns.is() || !xColumns->hasElements() )
            {   // perhaps the composer can supply us with columns? This is necessary for cases
                // where the dialog is invoked for a rowset which is not yet loaded
                // #i22878#
                xSuppColumns.set(m_xComposer, css::uno::UNO_QUERY);
                if ( xSuppColumns.is() )
                    xColumns = xSuppColumns->getColumns();
            }

            OSL_ENSURE( xColumns.is() && xColumns->hasElements(), "ComposerDialog::createDialog: not much fun without any columns!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        if ( !xConnection.is() || !xColumns.is() || !m_xComposer.is() )
        {
            // can't create the dialog if I have improper settings
            return nullptr;
        }

        return createComposerDialog(Application::GetFrameWeld(rParent), xConnection, xColumns);
    }

    // RowsetFilterDialog
    RowsetFilterDialog::RowsetFilterDialog( const Reference< XComponentContext >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    OUString SAL_CALL RowsetFilterDialog::getImplementationName()
    {
        return u"com.sun.star.uno.comp.sdb.RowsetFilterDialog"_ustr;
    }
    sal_Bool SAL_CALL RowsetFilterDialog::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
    css::uno::Sequence< OUString > SAL_CALL RowsetFilterDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.sdb.FilterDialog"_ustr };
    }

    std::unique_ptr<weld::GenericDialogController> RowsetFilterDialog::createComposerDialog(weld::Window* _pParent, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxColumns )
    {
        return std::make_unique<DlgFilterCrit>(_pParent, m_aContext, _rxConnection, m_xComposer, _rxColumns);
    }

    void SAL_CALL RowsetFilterDialog::initialize( const Sequence< Any >& aArguments )
    {
        if( aArguments.getLength() == 3 )
        {
            // this is the FilterDialog::createWithQuery method
            Reference<css::sdb::XSingleSelectQueryComposer> xQueryComposer;
            aArguments[0] >>= xQueryComposer;
            Reference<css::sdbc::XRowSet> xRowSet;
            aArguments[1] >>= xRowSet;
            Reference<css::awt::XWindow> xParentWindow;
            aArguments[2] >>= xParentWindow;
            setPropertyValue( u"QueryComposer"_ustr, Any( xQueryComposer ) );
            setPropertyValue( u"RowSet"_ustr,        Any( xRowSet ) );
            setPropertyValue( u"ParentWindow"_ustr,  Any( xParentWindow ) );
        }
        else
            ComposerDialog::initialize(aArguments);
    }

    void RowsetFilterDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        ComposerDialog::executedDialog( _nExecutionResult );

        if ( _nExecutionResult && m_xDialog )
            static_cast<DlgFilterCrit*>(m_xDialog.get())->BuildWherePart();
    }

    // RowsetOrderDialog
    RowsetOrderDialog::RowsetOrderDialog( const Reference< XComponentContext >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    OUString SAL_CALL RowsetOrderDialog::getImplementationName()
    {
        return u"com.sun.star.uno.comp.sdb.RowsetOrderDialog"_ustr;
    }
    sal_Bool SAL_CALL RowsetOrderDialog::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
    css::uno::Sequence< OUString > SAL_CALL RowsetOrderDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.sdb.OrderDialog"_ustr };
    }

    std::unique_ptr<weld::GenericDialogController> RowsetOrderDialog::createComposerDialog(weld::Window* pParent, const Reference< XConnection >& rxConnection, const Reference< XNameAccess >& rxColumns)
    {
        return std::make_unique<DlgOrderCrit>(pParent, rxConnection, m_xComposer, rxColumns);
    }

    void SAL_CALL RowsetOrderDialog::initialize( const Sequence< Any >& aArguments )
    {
        if (aArguments.getLength() == 2 || aArguments.getLength() == 3)
        {
            Reference<css::sdb::XSingleSelectQueryComposer> xQueryComposer;
            aArguments[0] >>= xQueryComposer;
            Reference<css::beans::XPropertySet> xRowSet;
            aArguments[1] >>= xRowSet;
            setPropertyValue( u"QueryComposer"_ustr, Any( xQueryComposer ) );
            setPropertyValue( u"RowSet"_ustr,        Any( xRowSet ) );
            if (aArguments.getLength() == 3)
            {
                Reference<css::awt::XWindow> xParentWindow;
                aArguments[2] >>= xParentWindow;
                setPropertyValue(u"ParentWindow"_ustr,  Any(xParentWindow));
            }
        }
        else
            ComposerDialog::initialize(aArguments);
    }

    void RowsetOrderDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        ComposerDialog::executedDialog( _nExecutionResult );

        if ( !m_xDialog )
            return;

        if ( _nExecutionResult )
            static_cast<DlgOrderCrit*>(m_xDialog.get())->BuildOrderPart();
        else if ( m_xComposer.is() )
            m_xComposer->setOrder(static_cast<DlgOrderCrit*>(m_xDialog.get())->GetOriginalOrder());
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
