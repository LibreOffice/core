/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

