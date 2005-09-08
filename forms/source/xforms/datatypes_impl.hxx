/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datatypes_impl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:16:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    return createPropertySetInfo( getInfoHelper() );
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

