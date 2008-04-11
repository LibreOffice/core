/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: legacysingletonfactory.cxx,v $
 * $Revision: 1.3 $
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
