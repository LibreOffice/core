/*************************************************************************
 *
 *  $RCSfile: servicemanager.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-06-18 10:32:32 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <hash_map>
#include <hash_set>
#include <list>

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/bootstrap.hxx>


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace std;



namespace stoc_smgr
{




/*****************************************************************************
    helper functions
*****************************************************************************/
OUString Point2Slash(const OUString& s)
{
    OUStringBuffer ret;

    sal_Int32 nIndex = 0;
    do
    {
        OUString token( s.getToken(0, '.', nIndex) );

        if (token.getLength())
        {
            ret.append( (sal_Unicode)'/' );
            ret.append( token );
        }
    } while( nIndex != -1 );

    return ret.makeStringAndClear();
}

/*****************************************************************************
    Enumeration by ServiceName
*****************************************************************************/
struct hashRef_Impl
{
    size_t operator()(const Reference<XInterface > & rName) const
    {
        // query to XInterface. The cast to XInterface* must be the same for the same object
        Reference<XInterface > x( Reference<XInterface >::query( rName ) );
        return (size_t)x.get();
    }
};

struct equaltoRef_Impl
{
    size_t operator()(const Reference<XInterface > & rName1, const Reference<XInterface > & rName2 ) const
        { return rName1 == rName2; }
};

typedef hash_set
<
    Reference<XInterface >,
    hashRef_Impl,
    equaltoRef_Impl
> HashSet_Ref;


class ServiceEnumeration_Impl : public WeakImplHelper1< XEnumeration >
{
public:
    ServiceEnumeration_Impl( const Sequence< Reference<XInterface > > & rFactories )
        : aFactories( rFactories )
        , nIt( 0 )
        {}
    virtual ~ServiceEnumeration_Impl() {};

    // XEnumeration
    sal_Bool SAL_CALL hasMoreElements()
        throw(::com::sun::star::uno::RuntimeException);
    Any SAL_CALL nextElement()
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
private:
    Mutex                               aMutex;
    Sequence< Reference<XInterface > >  aFactories;
    sal_Int32                           nIt;
};

// XEnumeration
sal_Bool ServiceEnumeration_Impl::hasMoreElements() throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( aMutex );
    return nIt != aFactories.getLength();
}

// XEnumeration
Any ServiceEnumeration_Impl::nextElement()
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( aMutex );
    if( nIt == aFactories.getLength() )
        throw NoSuchElementException();

    return Any( &aFactories.getConstArray()[nIt++], ::getCppuType( (const Reference<XInterface > *)0 ) );
}

/*****************************************************************************
    Enumeration by implementation
*****************************************************************************/
class ImplementationEnumeration_Impl : public WeakImplHelper1< XEnumeration >
{
public:
    ImplementationEnumeration_Impl( const HashSet_Ref & rImplementationMap )
        : aImplementationMap( rImplementationMap )
        , aIt( aImplementationMap.begin() )
        {}
    virtual ~ImplementationEnumeration_Impl();

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements()
         throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL nextElement()
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private:
    Mutex                           aMutex;
    HashSet_Ref                     aImplementationMap;
    HashSet_Ref::iterator           aIt;
    sal_Int32                       nNext;
    Reference<XInterface >          xNext;
};

ImplementationEnumeration_Impl::~ImplementationEnumeration_Impl()
{
}

// XEnumeration
sal_Bool ImplementationEnumeration_Impl::hasMoreElements()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( aMutex );
    return aIt != aImplementationMap.end();
}

// XEnumeration
Any ImplementationEnumeration_Impl::nextElement()
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( aMutex );
    if( aIt == aImplementationMap.end() )
        throw NoSuchElementException();

    Any ret( &(*aIt), ::getCppuType( (const Reference<XInterface > *)0 ) );
    ++aIt;
    return ret;
}

/*****************************************************************************
    Hash tables
*****************************************************************************/
struct equalOWString_Impl
{
  sal_Bool operator()(const OUString & s1, const OUString & s2) const
        { return s1 == s2; }
};

struct hashOWString_Impl
{
    size_t operator()(const OUString & rName) const
        { return rName.hashCode(); }
};

typedef hash_set
<
    OUString,
    hashOWString_Impl,
    equalOWString_Impl
> HashSet_OWString;

typedef hash_multimap
<
    OUString,
    Reference<XInterface >,
    hashOWString_Impl,
    equalOWString_Impl
> HashMultimap_OWString_Interface;

typedef hash_map
<
    OUString,
    Reference<XInterface >,
    hashOWString_Impl,
    equalOWString_Impl
> HashMap_OWString_Interface;

/*****************************************************************************
    class OServiceManager_Listener
*****************************************************************************/
class OServiceManager_Listener : public WeakImplHelper1< XEventListener >
{
private:
    WeakReference<XSet > xSMgr;

public:
    OServiceManager_Listener( const Reference<XSet > & rSMgr )
        : xSMgr( rSMgr )
        {}

    // XEventListener
    virtual void SAL_CALL disposing(const EventObject & rEvt ) throw(::com::sun::star::uno::RuntimeException);
};

void OServiceManager_Listener::disposing(const EventObject & rEvt )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XSet > x( xSMgr );
    if( x.is() )
    {
        try
        {
            x->remove( Any( &rEvt.Source, ::getCppuType( (const Reference<XInterface > *)0 ) ) );
        }
        catch( const IllegalArgumentException & )
        {
            OSL_ENSURE( sal_False, "IllegalArgumentException catched" );
        }
        catch( const NoSuchElementException & )
        {
            OSL_ENSURE( sal_False, "NoSuchElementException catched" );
        }
    }
}


/*****************************************************************************
    class OServiceManager
*****************************************************************************/
struct OServiceManagerMutex
{
    Mutex                           m_mutex;
};

extern "C" void SAL_CALL smgrUnloadingListener(void* id);

class OServiceManager
    : public XMultiServiceFactory
    , public XMultiComponentFactory
    , public XSet
    , public XContentEnumerationAccess
    , public XServiceInfo
    , public OServiceManagerMutex
    , public OComponentHelper
{
public:
    friend void SAL_CALL smgrUnloadingListener(void* id);

    OServiceManager( Reference< XComponentContext > const & xContext );
    ~OServiceManager();

    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw()
        { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw()
        { OComponentHelper::release(); }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    static OUString getImplementationName_Static() throw(::com::sun::star::uno::RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.OServiceManager") ); }
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static() throw(::com::sun::star::uno::RuntimeException);

    // XMultiComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        OUString const & rServiceSpecifier, Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        OUString const & rServiceSpecifier,
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);
//      virtual Sequence< OUString > SAL_CALL getAvailableServiceNames()
//          throw (RuntimeException);

    // XMultiServiceFactory
    virtual Sequence< OUString > SAL_CALL getAvailableServiceNames() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference<XInterface > SAL_CALL createInstance(const OUString &) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const OUString &, const Sequence<Any >& Arguments) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // The same as the interface method, but only uique names
    Sequence< OUString > getAvailableServiceNames( HashSet_OWString & aNameSet );

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual Reference<XEnumeration > SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & Element ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insert( const Any & Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const Any & Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    virtual Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

protected:

    sal_Bool haveFactoryWithThisImplementation(const OUString& aImplName);

    virtual Reference< XInterface > queryServiceFactory(const OUString& aServiceName);

    Reference< XComponentContext >  m_xContext;
    sal_Int32 m_nUnloadingListenerId;
private:

    Reference<XEventListener >      getFactoryListener();


    HashMultimap_OWString_Interface m_ServiceMap;
    HashSet_Ref                     m_ImplementationMap;
    HashMap_OWString_Interface      m_ImplementationNameMap;
    Reference<XEventListener >      xFactoryListener;
};

/**
 * Create a ServiceManager
 */
OServiceManager::OServiceManager( Reference< XComponentContext > const & xContext )
    : OComponentHelper( m_mutex )
    , m_xContext( xContext )
{
    m_nUnloadingListenerId= rtl_addUnloadingListener( smgrUnloadingListener, this);
}

/**
 * Destroy the ServiceManager
 */
OServiceManager::~OServiceManager()
{
    rtl_removeUnloadingListener( m_nUnloadingListenerId );
}

// OComponentHelper
void OServiceManager::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    // notify the disposing listener, the service manager is still full alive
    EventObject aEvt;
    aEvt.Source = Reference<XInterface >( *this );
    rBHelper.aLC.disposeAndClear( aEvt );

    // dispose all factories
    HashSet_Ref aImpls;
    {
        MutexGuard aGuard( m_mutex );
        aImpls = m_ImplementationMap;
    }
    HashSet_Ref::iterator aIt = aImpls.begin();
    while( aIt != aImpls.end() )
    {
        Reference<XComponent > xComp( Reference<XComponent >::query( *aIt++ ) );
        if( xComp.is() )
            xComp->dispose();
    }

    // set the service manager to disposed
    OComponentHelper::dispose();

    // dispose
    HashSet_Ref aImplMap;
    {
        MutexGuard aGuard( m_mutex );
        // erase all members
        m_ServiceMap = HashMultimap_OWString_Interface();
        aImplMap = m_ImplementationMap;
        m_ImplementationMap = HashSet_Ref();
        m_ImplementationNameMap = HashMap_OWString_Interface();
    }

    m_xContext.clear();

    // not only the Event should hold the object
    OSL_ASSERT( m_refCount != 1 );
}

// XTypeProvider
Sequence< Type > OServiceManager::getTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XMultiComponentFactory > *)0 ),
                ::getCppuType( (const Reference< XMultiServiceFactory > *)0 ),
                ::getCppuType( (const Reference< XSet > *)0 ),
                ::getCppuType( (const Reference< XContentEnumerationAccess > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                OComponentHelper::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
Sequence< sal_Int8 > OServiceManager::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// OServiceManager
Reference<XEventListener > OServiceManager::getFactoryListener()
{
    MutexGuard aGuard( m_mutex );
    if( !xFactoryListener.is() )
        xFactoryListener = new OServiceManager_Listener( this );
    return xFactoryListener;
}

// XInterface
Any OServiceManager::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XMultiComponentFactory * >( this ),
        static_cast< XMultiServiceFactory * >( this ),
        static_cast< XServiceInfo * >( this ),
        static_cast< XContentEnumerationAccess *>( this ),
        static_cast< XSet *>( this ),
        static_cast< XEnumerationAccess *>( this ),
        static_cast< XElementAccess *>( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getAvailableServiceNames( HashSet_OWString & aNameSet )
{
    MutexGuard aGuard( m_mutex );

    HashMultimap_OWString_Interface::iterator aSIt = m_ServiceMap.begin();
    while( aSIt != m_ServiceMap.end() )
        aNameSet.insert( (*aSIt++).first );

    /* do not return the implementation names
    HashMap_OWString_Interface      m_ImplementationNameMap;
    HashMap_OWString_Interface::iterator aIt = m_ImplementationNameMap.begin();
    while( aIt != m_ImplementationNameMap.end() )
        aNameSet.insert( (*aIt++).first );
    */

    Sequence< OUString > aNames( aNameSet.size() );
    OUString * pArray = aNames.getArray();
    sal_Int32 i = 0;
    HashSet_OWString::iterator next = aNameSet.begin();
    while( next != aNameSet.end() )
        pArray[i++] = (*next++);

    return aNames;
}

// XMultiComponentFactory
Reference< XInterface > OServiceManager::createInstanceWithContext(
    OUString const & rServiceSpecifier, Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
{
    Reference< XInterface > xRet;

    Reference< XInterface > xFactory( queryServiceFactory( rServiceSpecifier ) );
    if (xFactory.is())
    {
        Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
        if (xFac.is())
        {
            xRet = xFac->createInstanceWithContext( xContext );
        }
        else
        {
            Reference< XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
            if (xFac.is())
            {
#ifdef DEBUG
                OString aStr( OUStringToOString( rServiceSpecifier, RTL_TEXTENCODING_ASCII_US ) );
                OSL_TRACE( "### ignoring given context raising service %s !!!\n", aStr.getStr() );
#endif
                xRet = xFac->createInstance();
            }
        }
    }

    return xRet;
}
// XMultiComponentFactory
Reference< XInterface > OServiceManager::createInstanceWithArgumentsAndContext(
    OUString const & rServiceSpecifier,
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
{
    Reference< XInterface > xRet;

    Reference< XInterface > xFactory( queryServiceFactory( rServiceSpecifier ) );
    if (xFactory.is())
    {
        Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
        if (xFac.is())
        {
            xRet = xFac->createInstanceWithArgumentsAndContext( rArguments, xContext );
        }
        else
        {
            Reference< XSingleServiceFactory > xFac( xFactory, UNO_QUERY );
            if (xFac.is())
            {
#ifdef DEBUG
                OString aStr( OUStringToOString( rServiceSpecifier, RTL_TEXTENCODING_ASCII_US ) );
                OSL_TRACE( "### ignoring given context raising service %s !!!\n", aStr.getStr() );
#endif
                xRet = xFac->createInstanceWithArguments( rArguments );
            }
        }
    }

    return xRet;
}

// XMultiServiceFactory, XMultiComponentFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getAvailableServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    // all names
    HashSet_OWString aNameSet;
    return getAvailableServiceNames( aNameSet );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstance(
    const OUString& rServiceSpecifier )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithContext( rServiceSpecifier, m_xContext );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstanceWithArguments(
    const OUString& rServiceSpecifier,
    const Sequence<Any >& rArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithArgumentsAndContext( rServiceSpecifier, rArguments, m_xContext );
}

// XServiceInfo
OUString OServiceManager::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool OServiceManager::supportsService(const OUString& ServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > OServiceManager::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// OServiceManager_Static
Sequence< OUString > OServiceManager::getSupportedServiceNames_Static()
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.MultiServiceFactory") );
    aSNS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.ServiceManager") );
    return aSNS;
}

Reference< XInterface > OServiceManager::queryServiceFactory( const OUString& aServiceName )
{
    MutexGuard aGuard( m_mutex );

    Reference< XInterface > xRet;

    HashMultimap_OWString_Interface::iterator aMultiIt = m_ServiceMap.find( aServiceName );
    if( aMultiIt == m_ServiceMap.end() )
    {
        // no service found, look for an implementation
        HashMap_OWString_Interface::iterator aIt = m_ImplementationNameMap.find( aServiceName );
        if( aIt != m_ImplementationNameMap.end() )
            // an implementation found
            xRet = (*aIt).second;
    }
    else
    {
        xRet = (*aMultiIt).second;
    }

    return xRet;
}

// XContentEnumerationAccess
Reference<XEnumeration > OServiceManager::createContentEnumeration(const OUString& aServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_mutex );

    // get all factories
    pair<HashMultimap_OWString_Interface::iterator, HashMultimap_OWString_Interface::iterator> p =
        m_ServiceMap.equal_range( aServiceName );

    if( p.first == p.second )
        // nothing
        return Reference<XEnumeration >();

    // get the length of the sequence
    sal_Int32 nLen = 0;
    HashMultimap_OWString_Interface::iterator next = p.first;
    while( next != p.second )
    {
        ++nLen;
        ++next;
    }

    Sequence< Reference<XInterface > > aFactories( nLen );
    Reference<XInterface > * pArray = aFactories.getArray();
    sal_Int32 i = 0;
    while( p.first != p.second )
    {
        pArray[i++] = (*p.first).second;
        ++p.first;
    }
    return new ServiceEnumeration_Impl( aFactories );
}

// XEnumeration
Reference<XEnumeration > OServiceManager::createEnumeration() throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_mutex );
    return new ImplementationEnumeration_Impl( m_ImplementationMap );
}

// XElementAccess
Type OServiceManager::getElementType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType( (const Reference< XInterface > *)0 );
}

// XElementAccess
sal_Bool OServiceManager::hasElements()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_mutex );
    return !m_ImplementationMap.empty();
}

// XSet
sal_Bool OServiceManager::has( const Any & Element )
    throw(::com::sun::star::uno::RuntimeException)
{
    if( Element.getValueTypeClass() == TypeClass_INTERFACE )
    {
        MutexGuard aGuard( m_mutex );
        HashSet_Ref::iterator aIt =
            ((OServiceManager*)this)->m_ImplementationMap.find( *(Reference<XInterface >*)Element.getValue() );
        return aIt != m_ImplementationMap.end();
    }
    return sal_False;
}

// XSet
void OServiceManager::insert( const Any & Element )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException)
{
    if( Element.getValueTypeClass() != TypeClass_INTERFACE )
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no interface given!") ),
            Reference< XInterface >(), 0 );
    }
    Reference<XInterface > xEle( *(Reference<XInterface >*)Element.getValue(), UNO_QUERY );

    {
    MutexGuard aGuard( m_mutex );
    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );
    if( aIt != m_ImplementationMap.end() )
    {
        throw ElementExistException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("element already exists!") ),
            Reference< XInterface >() );
    }

    // put into the implementation hashmap
    m_ImplementationMap.insert( xEle );

    // put into the implementation name hashmap
    Reference<XServiceInfo > xInfo( Reference<XServiceInfo >::query( xEle ) );
    if( xInfo.is() )
    {
        OUString aImplName = xInfo->getImplementationName();
        if( aImplName.getLength() )
            m_ImplementationNameMap[ aImplName ] = xEle;
    }

    //put into the service map
    Reference<XServiceInfo > xSF( Reference<XServiceInfo >::query( xEle ) );
    if( xSF.is() )
    {
        Sequence< OUString > aServiceNames = xSF->getSupportedServiceNames();
        const OUString * pArray = aServiceNames.getConstArray();
        for( sal_Int32 i = 0; i < aServiceNames.getLength(); i++ )
        {
            m_ServiceMap.insert( HashMultimap_OWString_Interface::value_type(
                pArray[i], *(Reference<XInterface > *)Element.getValue() ) );
        }
    }
    }
    // add the disposing listener to the factory
    Reference<XComponent > xComp( Reference<XComponent >::query( xEle ) );
    if( xComp.is() )
        xComp->addEventListener( getFactoryListener() );
}

// helper function
sal_Bool OServiceManager::haveFactoryWithThisImplementation(const OUString& aImplName)
{
    return ( m_ImplementationNameMap.find(aImplName) != m_ImplementationNameMap.end());
}

// XSet
void OServiceManager::remove( const Any & Element )
     throw(::com::sun::star::lang::IllegalArgumentException,
           ::com::sun::star::container::NoSuchElementException,
           ::com::sun::star::uno::RuntimeException)
{
    if( Element.getValueTypeClass() != TypeClass_INTERFACE )
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no interface given!") ),
            Reference< XInterface >(), 0 );
    }
    Reference<XInterface > xEle( *(Reference<XInterface >*)Element.getValue(), UNO_QUERY );

    // remove the disposing listener from the factory
    Reference<XComponent > xComp( Reference<XComponent >::query( xEle ) );
    if( xComp.is() )
        xComp->removeEventListener( getFactoryListener() );

    MutexGuard aGuard( m_mutex );
    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );
    if( aIt == m_ImplementationMap.end() )
    {
        throw NoSuchElementException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("element is not in!") ),
            Reference< XInterface >() );
    }

    // remove from the implementation map
    m_ImplementationMap.erase( aIt );
    // remove from the implementation name hashmap
    Reference<XServiceInfo > xInfo( Reference<XServiceInfo >::query( xEle ) );
    if( xInfo.is() )
    {
        OUString aImplName = xInfo->getImplementationName();
        if( aImplName.getLength() )
            m_ImplementationNameMap.erase( aImplName );
    }

    //remove from the service map
    Reference<XServiceInfo > xSF( Reference<XServiceInfo >::query( xEle ) );
    if( xSF.is() )
    {
        Sequence< OUString > aServiceNames = xSF->getSupportedServiceNames();
        const OUString * pArray = aServiceNames.getConstArray();
        for( sal_Int32 i = 0; i < aServiceNames.getLength(); i++ )
        {
            pair<HashMultimap_OWString_Interface::iterator, HashMultimap_OWString_Interface::iterator> p =
                m_ServiceMap.equal_range( pArray[i] );

            while( p.first != p.second )
            {
                if( xEle == (*p.first).second )
                {
                    m_ServiceMap.erase( p.first );
                    break;
                }
                ++p.first;
            }
        }
    }
}

/*****************************************************************************
    class ORegistryServiceManager
*****************************************************************************/
class ORegistryServiceManager
    : public XInitialization
    , public XPropertySet
    , public OServiceManager
{
public:
    ORegistryServiceManager( Reference< XComponentContext > const & xContext );
    ~ORegistryServiceManager();

    Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL acquire() throw()
        { OServiceManager::acquire(); }
    void SAL_CALL release() throw()
        { OServiceManager::release(); }

    // XInitialization
    void SAL_CALL initialize(const Sequence< Any >& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    static OUString getImplementationName_Static() throw(::com::sun::star::uno::RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.ORegistryServiceManager") ); }

    //sal_Bool                      supportsService(const OUString& ServiceName) throw( () );
    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static() throw(::com::sun::star::uno::RuntimeException);

    // XMultiServiceFactory
    Sequence< OUString > SAL_CALL getAvailableServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    Sequence< Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPropertyValue(const OUString& PropertyName, const Any& aValue)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    Any SAL_CALL getPropertyValue(const OUString& PropertyName)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL addPropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL removePropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected:
    //OServiceManager
    Reference< XInterface > queryServiceFactory(const OUString& aServiceName);
private:
    Reference<XRegistryKey >        getRootKey();
    Reference<XInterface > loadWithImplementationName( const OUString & rImplName );
    Sequence<OUString>          getFromServiceName(const OUString& serviceName);
    Reference<XInterface > loadWithServiceName( const OUString & rImplName );
    void                        fillAllNamesFromRegistry( HashSet_OWString & );

    sal_Bool                    m_searchedRegistry;
    Reference<XSimpleRegistry > m_xRegistry;    // readonly property Registry
    Reference<XRegistryKey >    m_xRootKey;

#ifdef _DEBUG
    bool m_init;
#endif
};

/**
 * Create a ServiceManager
 */
ORegistryServiceManager::ORegistryServiceManager( Reference< XComponentContext > const & xContext )
    : OServiceManager( xContext )
    , m_searchedRegistry(sal_False)
#ifdef _DEBUG
    , m_init( false )
#endif
{
    m_nUnloadingListenerId= rtl_addUnloadingListener( smgrUnloadingListener, this);
}

/**
 * Destroy the ServiceManager
 */
ORegistryServiceManager::~ORegistryServiceManager()
{
    rtl_removeUnloadingListener( m_nUnloadingListenerId);
}

// OComponentHelper
void ORegistryServiceManager::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    OServiceManager::dispose();
    // dispose
    MutexGuard aGuard( m_mutex );
    // erase all members
    m_xRegistry = Reference<XSimpleRegistry >();
    m_xRootKey = Reference<XRegistryKey >();
}

// XTypeProvider
Sequence< Type > ORegistryServiceManager::getTypes()
    throw(::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * pTypes = 0;
    if (! pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pTypes)
        {
            static OTypeCollection aTypes(
                ::getCppuType( (const Reference< XPropertySet > *)0 ),
                ::getCppuType( (const Reference< XInitialization > *)0 ),
                OServiceManager::getTypes() );
            pTypes = &aTypes;
        }
    }
    return pTypes->getTypes();
}
Sequence< sal_Int8 > ORegistryServiceManager::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

/**
 * Return the root key of the registry. The Default registry service is ordered
 * if no registry is set.
 */
//Reference<XServiceProvider > create_DefaultRegistry_ServiceProvider();

Reference<XRegistryKey > ORegistryServiceManager::getRootKey()
{
    if( !m_xRootKey.is() )
    {
        MutexGuard aGuard( m_mutex );
        //  DefaultRegistry suchen !!!!
        if( !m_xRegistry.is() && !m_searchedRegistry )
        {
            // merken, es wird nur einmal gesucht
            m_searchedRegistry = sal_True;

            m_xRegistry.set(
                createInstanceWithContext(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.DefaultRegistry") ),
                    m_xContext ),
                UNO_QUERY );
        }
        if( m_xRegistry.is() && !m_xRootKey.is() )
            m_xRootKey = m_xRegistry->getRootKey();
    }

    return m_xRootKey;
}

/**
 * Create a service provider from the registry with an implementation name
 */
Reference<XInterface > ORegistryServiceManager::loadWithImplementationName(
    const OUString& name )
{
    Reference<XInterface > ret;

    Reference<XRegistryKey > xRootKey = getRootKey();
    if( !xRootKey.is() )
        return ret;

    try
    {
        OUString implementationName = OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS/") ) + name;
        Reference<XRegistryKey > xImpKey = m_xRootKey->openKey(implementationName);

        if( xImpKey.is() )
        {
            ret = createSingleRegistryFactory( this, name, xImpKey );
            insert( makeAny( ret ) );
        }
    }
    catch (InvalidRegistryException &)
    {
    }

    return ret;
}

/**
 * Return all implementation out of the registry.
 */
Sequence<OUString> ORegistryServiceManager::getFromServiceName(
    const OUString& serviceName )
{
    Reference<XRegistryKey > xRootKey = getRootKey();
    if( !xRootKey.is() )
        return Sequence<OUString>();

    try
    {
        OUString regName = OUString( RTL_CONSTASCII_USTRINGPARAM("/SERVICES/") ) + serviceName;
        Reference<XRegistryKey > xServKey = m_xRootKey->openKey(regName);

        if( xServKey.is() && xServKey->getValueType() == RegistryValueType_ASCIILIST )
        {
            return xServKey->getAsciiListValue();
        }
    }
    catch (InvalidRegistryException &)
    {
    }

    return Sequence<OUString>();
}

/**
 * Create a service provider from the registry
 */
Reference<XInterface > ORegistryServiceManager::loadWithServiceName(
    const OUString& serviceName )
{
    Sequence<OUString> implEntries = getFromServiceName( serviceName );
    for (sal_Int32 i = 0; i < implEntries.getLength(); i++)
    {
        Reference< XInterface > x( loadWithImplementationName( implEntries.getConstArray()[i] ) );
        if (x.is())
            return x;
    }

    return Reference<XSingleServiceFactory >();
}

/**
 * Return a sequence of all service names from the registry.
 */
void ORegistryServiceManager::fillAllNamesFromRegistry( HashSet_OWString & rSet )
{
    Reference<XRegistryKey > xRootKey = getRootKey();
    if( !xRootKey.is() )
        return;

    try
    {
        Reference<XRegistryKey > xServicesKey = xRootKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("SERVICES") ) );
        // root + /Services + /
        if( xServicesKey.is() )
        {
            sal_Int32 nPrefix = xServicesKey->getKeyName().getLength() +1;
            Sequence<Reference<XRegistryKey > > aKeys = xServicesKey->openKeys();
            for( sal_Int32 i = 0; i < aKeys.getLength(); i++ )
                rSet.insert( aKeys.getConstArray()[i]->getKeyName().copy( nPrefix ) );
        }
    }
    catch (InvalidRegistryException &)
    {
    }
}

// XInterface
Any ORegistryServiceManager::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( cppu::queryInterface(
        rType,
        static_cast< XInitialization * >( this ),
        static_cast< XPropertySet * >( this ) ) );

    return (aRet.hasValue() ? aRet : OServiceManager::queryInterface( rType ));
}

// XInitialization
void ORegistryServiceManager::initialize(const Sequence< Any >& Arguments)
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // read out default context to be used for components
    // => bootstrap problem: first service manager has to be instanced without context,
    // default context for createInstance() (!= createInstanceWithContext()) calls
    // gets this context

    MutexGuard aGuard( m_mutex );
    if (Arguments.getLength() > 0)
    {
        m_xRootKey.clear();
        Arguments[ 0 ] >>= m_xRegistry;
    }

    if (!m_xContext.is() && (Arguments.getLength() > 1))
    {
        Arguments[ 1 ] >>= m_xContext;
    }

#ifdef _DEBUG
    // to find all bootstrapping processes to be fixed...
    OSL_ENSURE( !m_init, "### second init of service manager instance!" );
    m_init = true;
#endif
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > ORegistryServiceManager::getAvailableServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_mutex );
    // all names
    HashSet_OWString aNameSet;

    // all names from the registry
    fillAllNamesFromRegistry( aNameSet );

    return OServiceManager::getAvailableServiceNames( aNameSet );
}

// XServiceInfo
OUString ORegistryServiceManager::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_Static();
}

// XServiceInfo
Sequence< OUString > ORegistryServiceManager::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > ORegistryServiceManager::getSupportedServiceNames_Static()
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.MultiServiceFactory") );
    aSNS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.RegistryServiceManager") );
    return aSNS;
}

// OServiceManager
Reference< XInterface > ORegistryServiceManager::queryServiceFactory(
    const OUString& aServiceName )
{
    Reference< XInterface > xRet = OServiceManager::queryServiceFactory( aServiceName );
    if( !xRet.is() )
    {
        MutexGuard aGuard( m_mutex );
        xRet = loadWithServiceName( aServiceName );
        if( !xRet.is() )
            xRet = loadWithImplementationName( aServiceName );
    }

    return xRet;
}

// XContentEnumerationAccess
Reference<XEnumeration > ORegistryServiceManager::createContentEnumeration(const OUString& aServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( ((ORegistryServiceManager *)this)->m_mutex );
    // get all implementation names registered under this service name from the registry
    Sequence<OUString> aImpls = ((ORegistryServiceManager *)this)->getFromServiceName( aServiceName );
    // load and insert all factories specified by the registry
    sal_Int32 i;
    OUString aImplName;
    for( i = 0; i < aImpls.getLength(); i++ )
    {
        aImplName = aImpls.getConstArray()[i];
        if ( !haveFactoryWithThisImplementation(aImplName) )
        {
            loadWithImplementationName( aImplName );
        }
    }
    // call the superclass to enumerate all contents
    return OServiceManager::createContentEnumeration( aServiceName );
}

// XPropertySet
Reference<XPropertySetInfo > ORegistryServiceManager::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference<XPropertySetInfo >();
}

void ORegistryServiceManager::setPropertyValue(const OUString& PropertyName,
                                               const Any& aValue)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    throw UnknownPropertyException();
}

Any ORegistryServiceManager::getPropertyValue(const OUString& PropertyName)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    Any ret;

    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Registry") ))
    {
        MutexGuard aGuard( m_mutex );
        if( m_xRegistry.is() )
        {
            ret = makeAny( m_xRegistry );
        }
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("DefaultContext") ))
    {
        MutexGuard aGuard( m_mutex );
        if( m_xContext.is() )
        {
            ret = makeAny( m_xContext );
        }
    }
    else
    {
        UnknownPropertyException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager : unknown property " ) );
        except.Message += PropertyName;
        throw except;
    }

    return ret;
}

void ORegistryServiceManager::addPropertyChangeListener(const OUString& PropertyName,
                                                        const Reference<XPropertyChangeListener >& aListener)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    throw UnknownPropertyException();
}

void ORegistryServiceManager::removePropertyChangeListener(const OUString& PropertyName,
                                                           const Reference<XPropertyChangeListener >& aListener)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    throw UnknownPropertyException();
}

void ORegistryServiceManager::addVetoableChangeListener(const OUString& PropertyName,
                                                        const Reference<XVetoableChangeListener >& aListener)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    throw UnknownPropertyException();
}

void ORegistryServiceManager::removeVetoableChangeListener(const OUString& PropertyName,
                                                           const Reference<XVetoableChangeListener >& aListener)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    throw UnknownPropertyException();
}


/**
 * Create the ServiceManager
 */
static Reference<XInterface > SAL_CALL OServiceManager_CreateInstance(
    const Reference< XComponentContext > & xContext )
{
    return Reference<XInterface >( SAL_STATIC_CAST( XInterface *, SAL_STATIC_CAST( OWeakObject *, new OServiceManager( xContext ) ) ) );
}

/**
 * Create the ServiceManager
 */
static Reference<XInterface > SAL_CALL ORegistryServiceManager_CreateInstance(
    const Reference< XComponentContext > & xContext )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return Reference<XInterface >( SAL_STATIC_CAST( XInterface *, SAL_STATIC_CAST( OWeakObject *, new ORegistryServiceManager( xContext ) ) ) );
}
/* This is the listener function used by the service manager in order
to implement the unloading mechanism, id is the this pointer of the
service manager instances. On notification, that is the function is being called
by rtl_unloadUnusedModules, the cached factroies are being removed from the
service manager.
*/
extern "C" void SAL_CALL smgrUnloadingListener(void* id)
{
      stoc_smgr::OServiceManager* pMgr= reinterpret_cast<stoc_smgr::OServiceManager*>( id);
    Reference<XInterface> xInt( static_cast<OWeakObject*>(pMgr) );
    Reference<XEnumerationAccess>xEnumAcc( xInt, UNO_QUERY);
    Reference<XSet>xSet( xInt, UNO_QUERY);

    if(xEnumAcc.is() && xSet.is())
    {
        Reference<XEnumeration> xEnum= xEnumAcc->createEnumeration();
        if( xEnum.is())
        {
            try
            {
                while( xEnum->hasMoreElements())
                {
                    Any val= xEnum->nextElement();

                    Reference<XInterface> xIntFactory;
                    if( val >>= xIntFactory)
                    {
                        Reference<XUnloadingPreference>xUnload( xIntFactory, UNO_QUERY);
                        if( xUnload.is())
                            if( xUnload->releaseOnNotification())
                                xSet->remove( val);
                        else
                            xSet->remove( val);
                    }
                }
            }
            catch( NoSuchElementException)
            {
            }
            catch( WrappedTargetException)
            {
            }
        }
    }
}

} // namespace





//##################################################################################################
//##################################################################################################
//##################################################################################################


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
            OUStringBuffer buf( 32 );
            // OServiceManager
            buf.append( (sal_Unicode)'/' );
            buf.append( stoc_smgr::OServiceManager::getImplementationName_Static() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/UNO/SERVICES") );
            Reference<XRegistryKey> xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear() ) );
            const Sequence<OUString > & rSMnames =
                stoc_smgr::OServiceManager::getSupportedServiceNames_Static();
            const OUString * pNames = rSMnames.getConstArray();
            sal_Int32 nPos;
            for ( nPos = rSMnames.getLength(); nPos--; )
                xNewKey->createKey( pNames[nPos] );

            // ORegistryServiceManager
            buf.append( (sal_Unicode)'/' );
            buf.append( stoc_smgr::ORegistryServiceManager::getImplementationName_Static() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/UNO/SERVICES") );
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                buf.makeStringAndClear() );
            const Sequence<OUString > & rRSMnames =
                stoc_smgr::ORegistryServiceManager::getSupportedServiceNames_Static();
            pNames = rRSMnames.getConstArray();
            for ( nPos = rRSMnames.getLength(); nPos--; )
                xNewKey->createKey( pNames[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    Reference< XInterface > xFactory;

    if (stoc_smgr::OServiceManager::getImplementationName_Static().compareToAscii( pImplName ) == 0)
    {
        xFactory = createSingleComponentFactory(
            stoc_smgr::OServiceManager_CreateInstance,
            stoc_smgr::OServiceManager::getImplementationName_Static(),
            stoc_smgr::OServiceManager::getSupportedServiceNames_Static() );
    }
    else if (stoc_smgr::ORegistryServiceManager::getImplementationName_Static().compareToAscii( pImplName ) == 0)
    {
        xFactory = createSingleComponentFactory(
            stoc_smgr::ORegistryServiceManager_CreateInstance,
            stoc_smgr::ORegistryServiceManager::getImplementationName_Static(),
            stoc_smgr::ORegistryServiceManager::getSupportedServiceNames_Static() );
    }

    if (xFactory.is())
    {
        xFactory->acquire();
        return xFactory.get();
    }
    return 0;
}
}


