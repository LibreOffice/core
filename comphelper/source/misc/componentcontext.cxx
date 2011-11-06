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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/componentcontext.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::lang::ServiceNotRegisteredException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    /** === end UNO using === **/

    //====================================================================
    //= ComponentContext
    //====================================================================
    //--------------------------------------------------------------------
    ComponentContext::ComponentContext( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
        if ( m_xContext.is() )
            m_xORB = m_xContext->getServiceManager();
        if ( !m_xORB.is() )
            throw NullPointerException();
    }

    //------------------------------------------------------------------------
    ComponentContext::ComponentContext( const Reference< XMultiServiceFactory >& _rxLegacyFactory )
    {
        if ( !_rxLegacyFactory.is() )
            throw NullPointerException();

        try
        {
            Reference< XPropertySet > xFactoryProperties( _rxLegacyFactory, UNO_QUERY_THROW );
            m_xContext = Reference< XComponentContext >(
                xFactoryProperties->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
                UNO_QUERY );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            throw RuntimeException();
        }

        if ( m_xContext.is() )
            m_xORB = m_xContext->getServiceManager();
        if ( !m_xORB.is() )
            throw NullPointerException();
    }

    //------------------------------------------------------------------------
    Any ComponentContext::getContextValueByName( const ::rtl::OUString& _rName ) const
    {
        Any aReturn;
        try
        {
            aReturn = m_xContext->getValueByName( _rName );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ComponentContext::getContextValueByName: caught an exception!" );
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::createComponent( const ::rtl::OUString& _rServiceName ) const
    {
        Reference< XInterface > xComponent(
            m_xORB->createInstanceWithContext( _rServiceName, m_xContext )
        );
        if ( !xComponent.is() )
            throw ServiceNotRegisteredException( _rServiceName, NULL );
        return xComponent;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::createComponentWithArguments( const ::rtl::OUString& _rServiceName, const Sequence< Any >& _rArguments ) const
    {
        Reference< XInterface > xComponent(
            m_xORB->createInstanceWithArgumentsAndContext( _rServiceName, _rArguments, m_xContext )
        );
        if ( !xComponent.is() )
            throw ServiceNotRegisteredException( _rServiceName, NULL );
        return xComponent;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > ComponentContext::getSingleton( const ::rtl::OUString& _rInstanceName ) const
    {
        ::rtl::OUString sKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/" ) ) );
        sKey += _rInstanceName;
        return Reference< XInterface >( getContextValueByName( sKey ), UNO_QUERY );
    }

    //------------------------------------------------------------------------
    Reference< XMultiServiceFactory > ComponentContext::getLegacyServiceFactory() const
    {
        return Reference< XMultiServiceFactory >( m_xORB, UNO_QUERY_THROW );
    }

//........................................................................
} // namespace comphelper
//........................................................................

