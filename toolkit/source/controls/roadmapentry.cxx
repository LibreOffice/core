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

#include <toolkit/controls/roadmapentry.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>


ORoadmapEntry::ORoadmapEntry() : ORoadmapEntry_Base( )
                                ,OPropertyContainer( GetBroadcastHelper() )
{
    // registerProperty or registerMayBeVoidProperty or registerPropertyNoMember

    registerProperty( "Label", RM_PROPERTY_ID_LABEL,
                      css::beans::PropertyAttribute::BOUND |
                      css::beans::PropertyAttribute::CONSTRAINED,
                      & m_sLabel, cppu::UnoType<decltype(m_sLabel)>::get() );
    m_nID = -1;
    registerProperty( "ID", RM_PROPERTY_ID_ID,
                      css::beans::PropertyAttribute::BOUND |
                      css::beans::PropertyAttribute::CONSTRAINED,
                      & m_nID, cppu::UnoType<decltype(m_nID)>::get() );
    m_bEnabled = true;
    registerProperty( "Enabled", RM_PROPERTY_ID_ENABLED,
                    css::beans::PropertyAttribute::BOUND |
                    css::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bEnabled, cppu::UnoType<decltype(m_bEnabled)>::get() );

    registerProperty( "Interactive", RM_PROPERTY_ID_INTERACTIVE,
                    css::beans::PropertyAttribute::BOUND |
                    css::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bInteractive, cppu::UnoType<decltype(m_bInteractive)>::get() );




    // Note that the list of registered properties has to be fixed: Different
    // instances of this class have to register the same set of properties with
    // the same attributes.

    // This is because all instances of the class share the same PropertySetInfo
    // which has been built from the registered property of _one_ instance.
}


IMPLEMENT_FORWARD_XINTERFACE2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
    // order matters:
    //  the first is the class name
    //  the second is the class which implements the ref-counting
    //  the third up to n-th (when using IMPLEMENT_FORWARD_*3 and so on) are other base classes
    //  whose XInterface and XTypeProvider implementations should be merged


css::uno::Reference< css:: beans::XPropertySetInfo > SAL_CALL
    ORoadmapEntry::getPropertySetInfo()
    throw(css::uno::RuntimeException, std::exception)
{
    return css::uno::Reference< css::beans::XPropertySetInfo >(
        createPropertySetInfo( getInfoHelper() ) );
}

OUString SAL_CALL ORoadmapEntry::getImplementationName(  ) throw (css::uno::RuntimeException, std::exception)
{
    OUString aStr("com.sun.star.comp.toolkit.RoadmapItem");
    return aStr;
}

sal_Bool SAL_CALL ORoadmapEntry::supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ORoadmapEntry::getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.awt.RoadmapItem" };
    return aRet;
}

::cppu::IPropertyArrayHelper& ORoadmapEntry::getInfoHelper()
{
    return *getArrayHelper();
}


::cppu::IPropertyArrayHelper* ORoadmapEntry::createArrayHelper() const
{
    css::uno::Sequence< css::beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
