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

// no include protection. This is included from within geometrycontrolmodel.hxx only

//====================================================================
//= OGeometryControlModel
//====================================================================
//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
    :OGeometryControlModel_Base(new CONTROLMODEL( i_factory ) )
{
}

//template <class CONTROLMODEL>
//OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & i_xCompContext)
//  :OGeometryControlModel_Base(new CONTROLMODEL(i_xCompContext))
//{
//}
//
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
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OGeometryControlModel<CONTROLMODEL>::getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException)
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
