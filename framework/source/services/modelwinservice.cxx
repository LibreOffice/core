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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "services.h"
#include "services/modelwinservice.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/awt/XControlModel.hpp>

using namespace ::com::sun::star;

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

class Impl_ModelWinService
{
    public:
        ~Impl_ModelWinService();

        static Impl_ModelWinService* getSingleInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

        uno::Any getByName( const ::rtl::OUString& sName )
            throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException );

        uno::Sequence< ::rtl::OUString > getElementNames()
            throw( uno::RuntimeException );

        sal_Bool hasByName( const ::rtl::OUString& sName )
            throw( uno::RuntimeException );

        uno::Type getElementType()
            throw( css::uno::RuntimeException );

        sal_Bool hasElements()
            throw( css::uno::RuntimeException );

        void registerModelForXWindow( const uno::Reference< awt::XWindow >& rWindow, const uno::Reference< awt::XControlModel >& rModel );

        void deregisterModelForXWindow( const uno::Reference< awt::XWindow >& rWindow );

    private:
        typedef BaseHash< uno::WeakReference< awt::XControlModel > > ModelWinMap;

        Impl_ModelWinService();
        Impl_ModelWinService( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

        static Impl_ModelWinService* m_pModelWinService;

        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
        ModelWinMap m_aModelMap;
};

Impl_ModelWinService* Impl_ModelWinService::m_pModelWinService = 0;

Impl_ModelWinService* Impl_ModelWinService::getSingleInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager )
{
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
    if ( !m_pModelWinService )
        m_pModelWinService = new Impl_ModelWinService( rServiceManager );
    return m_pModelWinService;
}

Impl_ModelWinService::Impl_ModelWinService( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager ) :
    m_xServiceManager( rServiceManager )
{
}

Impl_ModelWinService::Impl_ModelWinService()
{
}

Impl_ModelWinService::~Impl_ModelWinService()
{
}

void Impl_ModelWinService::registerModelForXWindow( const uno::Reference< awt::XWindow >& rWindow, const uno::Reference< awt::XControlModel >& rModel )
{
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

    ::rtl::OUString sName = rtl::OUString::valueOf( reinterpret_cast< sal_Int64 >((void*)rWindow.get()));
    ModelWinMap::iterator pIter = m_aModelMap.find( sName );
    if ( pIter != m_aModelMap.end() )
        pIter->second = rModel;
    else
        m_aModelMap[sName] = rModel;
}

void Impl_ModelWinService::deregisterModelForXWindow( const uno::Reference< awt::XWindow >& /*rWindow*/ )
{
}

uno::Any Impl_ModelWinService::getByName( const ::rtl::OUString& sName )
throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aAny;

    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
    ModelWinMap::iterator pIter = m_aModelMap.find( sName );
    if ( pIter != m_aModelMap.end())
    {
        uno::Reference< awt::XControlModel > xModel( pIter->second );
        aAny = uno::makeAny(xModel);
    }

    return aAny;
}

uno::Sequence< ::rtl::OUString > Impl_ModelWinService::getElementNames()
throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
    uno::Sequence< ::rtl::OUString > aResult( m_aModelMap.size() );

    sal_Int32 i = 0;
    ModelWinMap::const_iterator pIter = m_aModelMap.begin();
    while ( pIter != m_aModelMap.end())
        aResult[i++] = pIter->first;

    return aResult;
}

sal_Bool Impl_ModelWinService::hasByName( const ::rtl::OUString& sName )
throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
    ModelWinMap::iterator pIter = m_aModelMap.find( sName );
    if ( pIter != m_aModelMap.end())
        return true;
    else
        return false;
}

uno::Type Impl_ModelWinService::getElementType()
throw( css::uno::RuntimeException )
{
    return ::getCppuType(( const uno::Reference< awt::XControlModel >*)NULL );
}

sal_Bool Impl_ModelWinService::hasElements()
throw( css::uno::RuntimeException )
{
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
    return (m_aModelMap.size() > 0);
}

//*****************************************************************************************************************
//  css::uno::XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_4                 (   ModelWinService                                 ,
                                        OWeakObject                                     ,
                                        DIRECT_INTERFACE(css::lang::XTypeProvider       ),
                                        DIRECT_INTERFACE(css::lang::XServiceInfo        ),
                                        DIRECT_INTERFACE(css::container::XNameAccess    ),
                                        DIRECT_INTERFACE(css::container::XElementAccess )
                                    )

DEFINE_XTYPEPROVIDER_4              (   ModelWinService                ,
                                        css::lang::XTypeProvider       ,
                                        css::lang::XServiceInfo        ,
                                        css::container::XNameAccess    ,
                                        css::container::XElementAccess
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   ModelWinService                    ,
                                        OWeakObject                        ,
                                        SERVICENAME_MODELWINSERVICE        ,
                                        IMPLEMENTATIONNAME_MODELWINSERVICE
                                    )

DEFINE_INIT_SERVICE                 (   ModelWinService,
                                        {
                                        }
                                    )

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
ModelWinService::ModelWinService(const uno::Reference< lang::XMultiServiceFactory >& rServiceManager ) :
    m_xServiceManager( rServiceManager )
{
}

ModelWinService::~ModelWinService()
{
}

void ModelWinService::registerModelForXWindow( const uno::Reference< awt::XWindow >& rWindow, const uno::Reference< awt::XControlModel >& rModel )
{
    Impl_ModelWinService::getSingleInstance(m_xServiceManager)->registerModelForXWindow( rWindow, rModel );
}

void ModelWinService::deregisterModelForXWindow( const uno::Reference< awt::XWindow >& rWindow )
{
    Impl_ModelWinService::getSingleInstance(m_xServiceManager)->deregisterModelForXWindow( rWindow );
}

uno::Any SAL_CALL ModelWinService::getByName( const ::rtl::OUString& sName )
throw(  container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    return Impl_ModelWinService::getSingleInstance(m_xServiceManager)->getByName( sName );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ModelWinService::getElementNames()
throw( uno::RuntimeException )
{
    return Impl_ModelWinService::getSingleInstance(m_xServiceManager)->getElementNames( );
}

sal_Bool SAL_CALL ModelWinService::hasByName( const ::rtl::OUString& sName )
throw( uno::RuntimeException )
{
    return Impl_ModelWinService::getSingleInstance(m_xServiceManager)->hasByName( sName );
}

//---------------------------------------------------------------------------------------------------------
//  XElementAccess
//---------------------------------------------------------------------------------------------------------
uno::Type SAL_CALL ModelWinService::getElementType()
throw( uno::RuntimeException )
{
    return ::getCppuType( (const uno::Reference< awt::XControlModel > *)NULL );
}

sal_Bool SAL_CALL ModelWinService::hasElements()
throw( uno::RuntimeException )
{
    return Impl_ModelWinService::getSingleInstance(m_xServiceManager)->hasElements();
}

}
