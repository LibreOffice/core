/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datatypes_impl.hxx,v $
 * $Revision: 1.6 $
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

#ifndef DATATYPES_INCLUDED_BY_MASTER_HEADER
    #error "not to be included directly!"
#endif

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::ODerivedDataType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
    :SUPERCLASS( _rName, _nTypeClass )
    ,m_bPropertiesRegistered( false )
{
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper* ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::createArrayHelper( ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
        return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper& SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getInfoHelper()
{
    if ( !m_bPropertiesRegistered )
    {
        const_cast< ODerivedDataType* >( this )->registerProperties();
        const_cast< ODerivedDataType* >( this )->m_bPropertiesRegistered = true;
    }

    return *ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getArrayHelper();
}


//--------------------------------------------------------------------
template< typename VALUE_TYPE >
OValueLimitedType< VALUE_TYPE >::OValueLimitedType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
    :OValueLimitedType_Base( _rName, _nTypeClass )
{
}

