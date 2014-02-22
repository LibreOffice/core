/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <toolkit/controls/roadmapentry.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>


ORoadmapEntry::ORoadmapEntry() : ORoadmapEntry_Base( )
                                ,OPropertyContainer( GetBroadcastHelper() )
{
    

    registerProperty( OUString("Label"), RM_PROPERTY_ID_LABEL,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_sLabel, ::getCppuType( &m_sLabel ) );
    m_nID = -1;
    registerProperty( OUString("ID"), RM_PROPERTY_ID_ID,
                      ::com::sun::star::beans::PropertyAttribute::BOUND |
                      ::com::sun::star::beans::PropertyAttribute::CONSTRAINED,
                      & m_nID, ::getCppuType( &m_nID ) );
    m_bEnabled = sal_True;
    registerProperty( OUString("Enabled"), RM_PROPERTY_ID_ENABLED,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bEnabled, ::getCppuType( &m_bEnabled ) );

    registerProperty( OUString("Interactive"), RM_PROPERTY_ID_INTERACTIVE,
                    ::com::sun::star::beans::PropertyAttribute::BOUND |
                    ::com::sun::star::beans::PropertyAttribute::MAYBEDEFAULT,
                    & m_bInteractive, ::getCppuType( &m_bInteractive ) );


    

    
    
    
    //
    
    
}


IMPLEMENT_FORWARD_XINTERFACE2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORoadmapEntry, ORoadmapEntry_Base, ::comphelper::OPropertyContainer );
    
    
    
    
    


::com::sun::star::uno::Reference< ::com::sun::star:: beans::XPropertySetInfo > SAL_CALL
    ORoadmapEntry::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >(
        createPropertySetInfo( getInfoHelper() ) );
}

OUString SAL_CALL ORoadmapEntry::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OUString aStr("com.sun.star.comp.toolkit.RoadmapItem");
    return aStr;
}

sal_Bool SAL_CALL ORoadmapEntry::supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL ORoadmapEntry::getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.awt.RoadmapItem";
    return aRet;
}

::cppu::IPropertyArrayHelper& ORoadmapEntry::getInfoHelper()
{
    return *getArrayHelper();
}


::cppu::IPropertyArrayHelper* ORoadmapEntry::createArrayHelper() const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
