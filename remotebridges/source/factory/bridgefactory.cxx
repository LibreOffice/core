/*************************************************************************
 *
 *  $RCSfile: bridgefactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-27 09:08:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <hash_map>

#include <rtl/alloc.h>

#include <uno/mapping.hxx>
#include <osl/mutex.hxx>

#include <bridges/remote/context.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>


#include "bridgeimpl.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;

#define SERVICE_NAME "com.sun.star.bridge.BridgeFactory"
#define IMPLEMENTATION_NAME "com.sun.star.comp.remotebridges.BridgeFactory"

namespace remotebridges_factory
{
    struct hashOUString
    {
        size_t operator()(const OUString & s) const
            { return s.hashCode(); }
    };

    struct equalOUString
    {
        sal_Bool operator()(const OUString & s1 , const OUString &s2 ) const
            {
                return s1 == s2;
            }
    };

    typedef ::std::hash_map
    <
    OUString,
    WeakReference< XBridge >,
    hashOUString,
    equalOUString
    > BridgeHashMap;


    typedef ::std::hash_map
    <
    OUString,
    OUString,
    hashOUString,
    equalOUString
    > ServiceHashMap;

    class OBridgeFactory :
        public MyMutex,
        public OComponentHelper,
        public XBridgeFactory,
        public XServiceInfo
    {
    public:
        OBridgeFactory( const Reference < XMultiServiceFactory > &rSMgr );

    public: // XInterface
        ::com::sun::star::uno::Any      SAL_CALL
            queryInterface( const ::com::sun::star::uno::Type & aType );

        void        SAL_CALL acquire()
            { OComponentHelper::acquire(); }
        void        SAL_CALL release()
            { OComponentHelper::release(); }

    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > SAL_CALL
                  createBridge(
                      const ::rtl::OUString& sName,
                      const ::rtl::OUString& sProtocol,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::connection::XConnection >& aConnection,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XInstanceProvider >& anInstanceProvider )
            throw(::com::sun::star::bridge::BridgeExistsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > SAL_CALL
            getBridge( const ::rtl::OUString& sName )
              throw(::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > > SAL_CALL
              getExistingBridges(  ) throw(::com::sun::star::uno::RuntimeException);

    public: //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
                     getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
                     getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    private: //XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    private:
        void init();
        OUString getServiceNameForProtocol( const OUString &sProtocol );

    private:
        Reference < XMultiServiceFactory > m_rSMgr;
        BridgeHashMap m_mapBridge;
        ServiceHashMap m_mapProtocolToService;
        sal_Bool m_bInitialized;
        ::osl::Mutex m_mutexInit;
    };

    OBridgeFactory::OBridgeFactory( const Reference < XMultiServiceFactory > &rSMgr ) :
        OComponentHelper( m_mutex ),
        m_rSMgr( rSMgr ),
        m_bInitialized( sal_False )
    {

    }

    void OBridgeFactory::init()
    {
        MutexGuard guard( m_mutexInit );
        if( ! m_bInitialized )
        {
            Reference< XContentEnumerationAccess > rContent( m_rSMgr , UNO_QUERY );
            if( rContent.is() )
            {
                OUString sMetaService = OUString::createFromAscii( "com.sun.star.bridge.Bridge" );
                Reference < XEnumeration > rEnum =
                    rContent->createContentEnumeration( sMetaService );
                while( rEnum->hasMoreElements() )
                {
                    Any a = rEnum->nextElement();
                    Reference <XServiceInfo> rInfo;
                    if( a >>= rInfo )
                    {
                        Sequence< OUString > seq = rInfo->getSupportedServiceNames();
                        sal_Int32 i;
                        for( i = 0 ; i < seq.getLength() ; i ++ )
                        {
                            if( seq.getConstArray()[i] != sMetaService )
                            {
                                sal_Int32 nIndex = seq.getConstArray()[i].lastIndexOf( '.' );
                                OUString sProtocol = seq.getConstArray()[i].copy( nIndex+1).toLowerCase();
                                m_mapProtocolToService[ sProtocol ] = seq.getConstArray()[i];
                            }
                        }
                    }
                }
            }
            m_bInitialized = sal_True;
        }
    }

    OUString OBridgeFactory::getServiceNameForProtocol( const OUString & sProtocol )
    {
        init();
        OUString sService;
        OUString sProtocolName = sProtocol.toLowerCase();
        ServiceHashMap::iterator ii = m_mapProtocolToService.find( sProtocolName );
        if( ii != m_mapProtocolToService.end() )
        {
            sService = (*ii).second;
        }
        else
        {
            // fallback to the old solution, deprecated, should be removed !
            OUString sService = OUString::createFromAscii( "com.sun.star.bridge.Bridge." );
            OUString sProtocolName = sProtocol;
            sal_Int32 nIndex = sProtocol.indexOf( (sal_Unicode)',' );
            if( nIndex > 0 )
            {
                sProtocolName = sProtocol.copy( 0 , nIndex );
            }
            sService += sProtocolName;
        }
        return sService;
    }

    Any OBridgeFactory::queryInterface( const Type &aType )
    {
        Any a = ::cppu::queryInterface(
            aType ,
            ( XBridgeFactory * ) this );
        if( a.hasValue() )
        {
            return a;
        }

        return OComponentHelper::queryInterface( aType );

    }

    Reference< XBridge > OBridgeFactory::createBridge(
        const OUString& sName,
        const OUString& sProtocol,
        const Reference< XConnection >& aConnection,
        const Reference< XInstanceProvider >& anInstanceProvider )
        throw(::com::sun::star::bridge::BridgeExistsException,
              ::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)
    {
        OUString sService = getServiceNameForProtocol( sProtocol );

        Reference < XInterface > rXInterface = m_rSMgr->createInstance ( sService );
        Reference < XInitialization > rInit(rXInterface, UNO_QUERY );
        Reference < XBridge > rBridge( rInit , UNO_QUERY );

        if( rInit.is() && rBridge.is() )
        {
            Sequence < Any > seqAny( 4 );
            seqAny.getArray()[0] <<= sName;
            seqAny.getArray()[1] <<= sProtocol;
            seqAny.getArray()[2] <<= aConnection;
            seqAny.getArray()[3] <<= anInstanceProvider;

            // let the Exceptions fly ....
            rInit->initialize( seqAny );
        }
        else
        {
            throw IllegalArgumentException();
        }

        if( sName.getLength() )
        {
            MutexGuard guard( m_mutex );
            // put the bridge into the hashmap
            m_mapBridge[ sName ] = rBridge;
        }
        return rBridge;
    }

    Reference< XBridge > OBridgeFactory::getBridge( const ::rtl::OUString& sName )
        throw(::com::sun::star::uno::RuntimeException )

    {
        MutexGuard guard( m_mutex );
        BridgeHashMap::iterator ii = m_mapBridge.find( sName );

        Reference < XBridge > rBridge;

        if( ii != m_mapBridge.end() )
        {
            rBridge = (*ii).second;
            if( ! rBridge.is() )
            {
                m_mapBridge.erase( ii );
            }
        }

        if( ! rBridge.is() )
        {
            // try to get it via the C-Context
            remote_Context * pRemoteC = remote_getContext( sName.pData );

            if( pRemoteC )
            {
                rBridge = Reference < XBridge > ((XBridge *) new OBridge( pRemoteC ) );
                pRemoteC->aBase.release( (uno_Context * )pRemoteC );
                m_mapBridge[ sName ] = rBridge;
            }
        }
        return rBridge;
    }

    Sequence< Reference< XBridge > > OBridgeFactory::getExistingBridges(  )
        throw(::com::sun::star::uno::RuntimeException)
    {
        MutexGuard guard( m_mutex );

        sal_Int32 nCount;

        rtl_uString **ppName = remote_getContextList(
            &nCount,
            rtl_allocateMemory );

        Sequence < Reference < XBridge > > seq( nCount );
        if( nCount )
        {

            for( sal_Int32 i = 0;
                 i < nCount ;
                 i ++ )
            {
                seq.getArray()[i] = getBridge( OUString( ppName[i]) );
                rtl_uString_release( ppName[i] );
            }
            rtl_freeMemory( ppName );
        }

        return seq;
    }

    // XTypeProvider
    Sequence< Type > SAL_CALL OBridgeFactory::getTypes(void) throw( RuntimeException )
    {
        static OTypeCollection *pCollection = 0;
        if( ! pCollection )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pCollection )
            {
                static OTypeCollection collection(
                    getCppuType( (Reference< XBridgeFactory > * ) 0 ),
                    OComponentHelper::getTypes() );
                pCollection = &collection;
            }
        }
        return (*pCollection).getTypes();
    }

    Sequence< sal_Int8 > SAL_CALL OBridgeFactory::getImplementationId(  ) throw( RuntimeException)
    {
        static OImplementationId *pId = 0;
        if( ! pId )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pId )
            {
                static OImplementationId id( sal_False );
                pId = &id;
            }
        }
        return (*pId).getImplementationId();
    }

    Sequence< OUString > getSupportedServiceNames()
    {
        static Sequence < OUString > *pNames = 0;
        if( ! pNames )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( !pNames )
            {
                static Sequence< OUString > seqNames(1);
                seqNames.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
                pNames = &seqNames;
            }
        }
        return *pNames;
    }

    OUString OBridgeFactory::getImplementationName(  ) throw(RuntimeException)
    {
        return OUString::createFromAscii( IMPLEMENTATION_NAME );
    }

    sal_Bool SAL_CALL OBridgeFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException)
    {
        Sequence< OUString > seq = remotebridges_factory::getSupportedServiceNames();
        sal_Bool bReturn = sal_False;
        for( sal_Int32 i = 0 ; i < seq.getLength() ; i ++ )
        {
            if( seq.getConstArray()[i] == ServiceName )
            {
                bReturn = sal_True;
                break;
            }
        }
        return bReturn;
    }

    Sequence< OUString > SAL_CALL OBridgeFactory::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return remotebridges_factory::getSupportedServiceNames();
    }

    Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &r)
    {
        return Reference < XInterface > (( OWeakObject * ) new OBridgeFactory( r ) );
    }

}

using namespace remotebridges_factory;

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ), CreateInstance, getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}




