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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <toolkit/controls/roadmapentry.hxx>

#include <rtl/ustring.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>


ORoadmapEntry::ORoadmapEntry() : ORoadmapEntry_Base( )
                                ,OPropertyContainer( GetBroadcastHelper() )
{
    // registerProperty or registerMayBeVoidProperty or registerPropertyNoMember

    registerProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")), RM_PROPERTY_ID_LABEL,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_sLabel, ::getCppuType( &m_sLabel ) );
    m_nID = -1;
    registerProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ID")), RM_PROPERTY_ID_ID,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_nID, ::getCppuType( &m_nID ) );
    m_bEnabled = sal_True;
    registerProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Enabled")), RM_PROPERTY_ID_ENABLED,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bEnabled, ::getCppuType( &m_bEnabled ) );

    registerProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Interactive")), RM_PROPERTY_ID_INTERACTIVE,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bInteractive, ::getCppuType( &m_bInteractive ) );


    // ...

    // Note that the list of registered properties has to be fixed: Different
    // instances of this class have to register the same set of properties with
    // the same attributes.
    //
    // This is because all instances of the class share the same PropertySetInfo
    // which has been built from the registered property of _one_ instance.
}

//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
    // order matters:
    //  the first is the class name
    //  the second is the class which implements the ref-counting
    //  the third up to n-th (when using IMPLEMENT_FORWARD_*3 and so on) are other base classes
    //  whose XInterface and XTypeProvider implementations should be merged

//--------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star:: beans::XPropertySetInfo > SAL_CALL
    ORoadmapEntry::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >(
        createPropertySetInfo( getInfoHelper() ) );
}

::rtl::OUString SAL_CALL ORoadmapEntry::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.toolkit.RoadmapItem"));
    return aStr;
}

sal_Bool SAL_CALL ORoadmapEntry::supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException)
{
    return ServiceName.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.RoadmapItem")) );
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ORoadmapEntry::getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.RoadmapItem"));
    return aRet;
}
//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ORoadmapEntry::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ORoadmapEntry::createArrayHelper() const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
