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

#include "FormsCollection.hxx"
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <com/sun/star/form/XForm.hpp>
#include <rtl/ref.hxx>

using namespace frm;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

OUString SAL_CALL OFormsCollection::getServiceName()
{
    return "com.sun.star.form.Forms";
}

Sequence< sal_Int8 > SAL_CALL OFormsCollection::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

Sequence<Type> SAL_CALL OFormsCollection::getTypes()
{
    return concatSequences(OInterfaceContainer::getTypes(), ::cppu::OComponentHelper::getTypes(), OFormsCollection_BASE::getTypes());
}

OFormsCollection::OFormsCollection(const Reference<XComponentContext>& _rxFactory)
    : ::cppu::OComponentHelper( m_aMutex )
    ,OInterfaceContainer( _rxFactory, m_aMutex, cppu::UnoType<XForm>::get() )
    ,OFormsCollection_BASE()
{
}

OFormsCollection::OFormsCollection( const OFormsCollection& _cloneSource )
    : ::cppu::OComponentHelper( m_aMutex )
    ,OInterfaceContainer( m_aMutex, _cloneSource )
    ,OFormsCollection_BASE()
{
}

OFormsCollection::~OFormsCollection()
{
    if (!::cppu::OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

Any SAL_CALL OFormsCollection::queryAggregation(const Type& _rType)
{
    Any aReturn = OFormsCollection_BASE::queryInterface(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OInterfaceContainer::queryInterface(_rType);

        if (!aReturn.hasValue())
            aReturn = ::cppu::OComponentHelper::queryAggregation(_rType);
    }

    return aReturn;
}

OUString SAL_CALL OFormsCollection::getImplementationName()
{
    return "com.sun.star.form.OFormsCollection";
}

sal_Bool SAL_CALL OFormsCollection::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL OFormsCollection::getSupportedServiceNames()
{
    return { "com.sun.star.form.Forms", "com.sun.star.form.FormComponents" };
}

// XCloneable
Reference< XCloneable > SAL_CALL OFormsCollection::createClone(  )
{
    rtl::Reference<OFormsCollection> pClone = new OFormsCollection( *this );
    pClone->clonedFrom( *this );
    return static_cast<OInterfaceContainer*>(pClone.get());
}

// OComponentHelper

void OFormsCollection::disposing()
{
    {
        SAL_INFO( "forms.component", "forms::OFormsCollection::disposing" );
        OInterfaceContainer::disposing();
    }
    ::cppu::OComponentHelper::disposing();
    m_xParent = nullptr;
}

//XChild

void OFormsCollection::setParent(const css::uno::Reference<css::uno::XInterface>& Parent)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

css::uno::Reference<css::uno::XInterface>  OFormsCollection::getParent()
{
    return m_xParent;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OFormsCollection_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OFormsCollection(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
