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

#include "comphelper/legacysingletonfactory.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>

#include <algorithm>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::lang::XSingleComponentFactory;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::lang::XInitialization;
    /** === end UNO using === **/

    //====================================================================
    //= LegacySingletonFactory
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   XServiceInfo
                                    ,   XSingleComponentFactory
                                    >   LegacySingletonFactory_Base;

    class COMPHELPER_DLLPRIVATE LegacySingletonFactory : public LegacySingletonFactory_Base
    {
    public:
        LegacySingletonFactory(
            ::cppu::ComponentFactoryFunc _componentFactoryFunc, const ::rtl::OUString& _rImplementationName,
            const Sequence< ::rtl::OUString >& _rServiceNames, rtl_ModuleCount* _pModCount
        );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

        // XSingleComponentFactory
        virtual Reference< XInterface > SAL_CALL createInstanceWithContext( const Reference< XComponentContext >& Context ) throw (Exception, RuntimeException);
        virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const Sequence< Any >& Arguments, const Reference< XComponentContext >& Context ) throw (Exception, RuntimeException);

    protected:
        ~LegacySingletonFactory();

    private:
        /** creates m_xInstance, returns whether it actually was created (<TRUE/>) or existed before (<FALSE/>
        */
        bool    impl_nts_ensureInstance( const Reference< XComponentContext >& _rxContext );

    private:
        ::osl::Mutex                    m_aMutex;
        ::cppu::ComponentFactoryFunc    m_componentFactoryFunc;
        ::rtl::OUString                 m_sImplementationName;
        Sequence< ::rtl::OUString >     m_aServiceNames;
        rtl_ModuleCount*                m_pModuleCount;
        Reference< XInterface >         m_xTheInstance;
    };

    //--------------------------------------------------------------------
    LegacySingletonFactory::LegacySingletonFactory( ::cppu::ComponentFactoryFunc _componentFactoryFunc, const ::rtl::OUString& _rImplementationName,
            const Sequence< ::rtl::OUString >& _rServiceNames, rtl_ModuleCount* _pModCount )
        :m_componentFactoryFunc ( _componentFactoryFunc )
        ,m_sImplementationName  ( _rImplementationName )
        ,m_aServiceNames        ( _rServiceNames )
        ,m_pModuleCount         ( _pModCount )
        ,m_xTheInstance         ( )
    {
        if ( m_pModuleCount )
            m_pModuleCount->acquire( m_pModuleCount );
    }

    //--------------------------------------------------------------------
    LegacySingletonFactory::~LegacySingletonFactory()
    {
        if ( m_pModuleCount )
            m_pModuleCount->release( m_pModuleCount );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL LegacySingletonFactory::getImplementationName(  ) throw (RuntimeException)
    {
        return m_sImplementationName;
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL LegacySingletonFactory::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames() );
        const ::rtl::OUString* pStart = aServices.getConstArray();
        const ::rtl::OUString* pEnd = aServices.getConstArray() + aServices.getLength();
        return ::std::find( pStart, pEnd, _rServiceName ) != pEnd;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL LegacySingletonFactory::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return m_aServiceNames;
    }

    //--------------------------------------------------------------------
    bool LegacySingletonFactory::impl_nts_ensureInstance( const Reference< XComponentContext >& _rxContext )
    {
        if ( m_xTheInstance.is() )
            return false;

        m_xTheInstance = (*m_componentFactoryFunc)( _rxContext );
        if ( !m_xTheInstance.is() )
            throw RuntimeException();

        return true;    // true -> "was newly created"
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL LegacySingletonFactory::createInstanceWithContext( const Reference< XComponentContext >& _rxContext ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_nts_ensureInstance( _rxContext );

        return m_xTheInstance;
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL LegacySingletonFactory::createInstanceWithArgumentsAndContext( const Sequence< Any >& _rArguments, const Reference< XComponentContext >& _rxContext ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !impl_nts_ensureInstance( _rxContext ) )
            throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Instance already created before, unable to initialize it." ) ),
                *this
            );

        Reference< XInitialization > xInit( m_xTheInstance, UNO_QUERY_THROW );
        xInit->initialize( _rArguments );

        return m_xTheInstance;
    }

    //====================================================================
    //= createLegacySingletonFactory
    //====================================================================
    Reference< XSingleComponentFactory > createLegacySingletonFactory(
        ::cppu::ComponentFactoryFunc _componentFactoryFunc, const ::rtl::OUString& _rImplementationName,
        const Sequence< ::rtl::OUString >& _rServiceNames, rtl_ModuleCount* _pModCount )
    {
        return new LegacySingletonFactory( _componentFactoryFunc, _rImplementationName, _rServiceNames, _pModCount );
    }


//........................................................................
} // namespace comphelper
//........................................................................
