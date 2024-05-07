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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/svapp.hxx>
#include "pcrunodialogs.hxx"
#include "formstrings.hxx"
#include "pcrstrings.hxx"
#include "taborder.hxx"
#include "pcrcommon.hxx"


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;


    //= OTabOrderDialog


    OTabOrderDialog::OTabOrderDialog( const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
        registerProperty( PROPERTY_CONTROLCONTEXT, static_cast<sal_Int32>(OwnPropertyId::CONTROLCONTEXT),
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlContext, cppu::UnoType<decltype(m_xControlContext)>::get() );

        registerProperty( PROPERTY_TABBINGMODEL, static_cast<sal_Int32>(OwnPropertyId::TABBINGMODEL),
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xTabbingModel, cppu::UnoType<decltype(m_xTabbingModel)>::get() );
    }

    OTabOrderDialog::~OTabOrderDialog()
    {
        if (m_xDialog)
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (m_xDialog)
                destroyDialog();
        }
    }

    Sequence<sal_Int8> SAL_CALL OTabOrderDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }

    OUString SAL_CALL OTabOrderDialog::getImplementationName()
    {
        return u"org.openoffice.comp.form.ui.OTabOrderDialog"_ustr;
    }


    css::uno::Sequence<OUString> SAL_CALL OTabOrderDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.form.ui.TabOrderDialog"_ustr, u"com.sun.star.form.TabOrderDialog"_ustr };
    }


    Reference<XPropertySetInfo>  SAL_CALL OTabOrderDialog::getPropertySetInfo()
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    ::cppu::IPropertyArrayHelper& OTabOrderDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OTabOrderDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    std::unique_ptr<weld::DialogController> OTabOrderDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return std::make_unique<TabOrderDialog>(Application::GetFrameWeld(rParent), m_xTabbingModel, m_xControlContext, m_aContext);
    }

    void OTabOrderDialog::initialize( const Sequence< Any >& aArguments )
    {
        Reference<css::awt::XTabControllerModel> xTabbingModel;
        Reference<css::awt::XControlContainer> xControlContext;
        Reference<css::awt::XWindow> xParentWindow;
        if (aArguments.getLength() == 3 && (aArguments[0] >>= xTabbingModel) && (aArguments[1] >>= xControlContext) && (aArguments[2] >>= xParentWindow))
        {
            Sequence< Any > aNewArguments{
                Any(NamedValue(
                    u"TabbingModel"_ustr,
                    Any( xTabbingModel )
                )),
                Any(NamedValue(
                    u"ControlContext"_ustr,
                    Any( xControlContext )
                )),
                Any(NamedValue(
                    u"ParentWindow"_ustr,
                    Any( xParentWindow )
                ))
            };
            OTabOrderDialog_DBase::initialize(aNewArguments);
        }
        else
            OTabOrderDialog_DBase::initialize(aArguments);
    }

}   // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propcrltr_OTabOrderDialog_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::OTabOrderDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
