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

// no include protection. This is included from within geometrycontrolmodel.hxx only

//====================================================================
//= OGeometryControlModel
//====================================================================
//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory )
    :OGeometryControlModel_Base(new CONTROLMODEL( i_factory ) )
{
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance)
    :OGeometryControlModel_Base(_rxAggregateInstance)
{
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
::cppu::IPropertyArrayHelper& SAL_CALL OGeometryControlModel<CONTROLMODEL>::getInfoHelper()
{
    return *this->getArrayHelper();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
void OGeometryControlModel<CONTROLMODEL>::fillProperties(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rProps, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rAggregateProps) const
{
    // our own properties
    OPropertyContainer::describeProperties(_rProps);
    // the aggregate properties
    if (m_xAggregateSet.is())
        _rAggregateProps = m_xAggregateSet->getPropertySetInfo()->getProperties();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OGeometryControlModel<CONTROLMODEL>::getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    static ::cppu::OImplementationId * pId = NULL;
    if ( !pId )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static ::cppu::OImplementationId s_aId;
            pId = &s_aId;
        }
    }
    return pId->getImplementationId();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel_Base* OGeometryControlModel<CONTROLMODEL>::createClone_Impl(
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance)
{
    return new OGeometryControlModel<CONTROLMODEL>(_rxAggregateInstance);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
