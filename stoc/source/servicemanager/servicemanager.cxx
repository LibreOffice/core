/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <list>
#include <uno/mapping.hxx>
#include <uno/dispatcher.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <rtl/unload.h>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/compbase7.hxx>


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>

#include <bootstrapservices.hxx>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace cppu;
using namespace osl;
using namespace std;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

namespace stoc_bootstrap
{
Sequence< OUString > smgr_wrapper_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.MultiServiceFactory") );
    return seqNames;
}

OUString smgr_wrapper_getImplementationName()
{
    return OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.OServiceManagerWrapper"));
}

Sequence< OUString > smgr_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames.getArray()[0] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.MultiServiceFactory") );
    seqNames.getArray()[1] = OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.ServiceManager") );
    return seqNames;
}

OUString smgr_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.OServiceManager"));
}

Sequence< OUString > regsmgr_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.MultiServiceFactory"));
    seqNames.getArray()[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.RegistryServiceManager"));
    return seqNames;
}

OUString regsmgr_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.stoc.ORegistryServiceManager" ) );
}
}

namespace stoc_smgr
{

static Sequence< OUString > retrieveAsciiValueList(
    const Reference< XSimpleRegistry > &xReg, const OUString &keyName )
{
    Reference< XEnumerationAccess > xAccess( xReg, UNO_QUERY );
    Sequence< OUString > seq;
    if( xAccess.is() )
    {
        Reference< XEnumeration > xEnum = xAccess->createEnumeration();
        while( xEnum.is() && xEnum->hasMoreElements() )
        {
            Reference< XSimpleRegistry > xTempReg;
            xEnum->nextElement() >>= xTempReg;
            if( xTempReg.is() )
            {
                Sequence< OUString > seq2 = retrieveAsciiValueList( xTempReg, keyName );

                if( seq2.getLength() )
                {
                    sal_Int32 n1Len = seq.getLength();
                    sal_Int32 n2Len = seq2.getLength();

                    seq.realloc( n1Len + n2Len );
                    const OUString *pSource = seq2.getConstArray();
                    OUString *pTarget = seq.getArray();
                    for( int i = 0 ; i < n2Len ; i ++ )
                    {
                        pTarget[i+n1Len] = pSource[i];
                    }
                }
            }
        }
    }
    else if( xReg.is () )
    {
        try
        {
            Reference< XRegistryKey > rRootKey = xReg->getRootKey();
            if( rRootKey.is() )
            {
                Reference<XRegistryKey > xKey = rRootKey->openKey(keyName);
                if( xKey.is() )
                {
                    seq = xKey->getAsciiListValue();
                }
            }
        }
        catch( InvalidRegistryException & )
        {
        }
        catch (InvalidValueException &)
        {
        }
    }
    return seq;
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

typedef boost::unordered_set
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
        { g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt ); }
    virtual ~ServiceEnumeration_Impl()
        { g_moduleCount.modCnt.release( &g_moduleCount.modCnt ); }

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

//==================================================================================================
class PropertySetInfo_Impl : public WeakImplHelper1< beans::XPropertySetInfo >
{
    Sequence< beans::Property > m_properties;

public:
    inline PropertySetInfo_Impl( Sequence< beans::Property > const & properties ) SAL_THROW(())
        : m_properties( properties )
        {}

    // XPropertySetInfo impl
    virtual Sequence< beans::Property > SAL_CALL getProperties()
        throw (RuntimeException);
    virtual beans::Property SAL_CALL getPropertyByName( OUString const & name )
        throw (beans::UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName( OUString const & name )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
Sequence< beans::Property > PropertySetInfo_Impl::getProperties()
    throw (RuntimeException)
{
    return m_properties;
}
//__________________________________________________________________________________________________
beans::Property PropertySetInfo_Impl::getPropertyByName( OUString const & name )
    throw (beans::UnknownPropertyException, RuntimeException)
{
    beans::Property const * p = m_properties.getConstArray();
    for ( sal_Int32 nPos = m_properties.getLength(); nPos--; )
    {
        if (p[ nPos ].Name.equals( name ))
            return p[ nPos ];
    }
    throw beans::UnknownPropertyException(
        OUSTR("unknown property: ") + name, Reference< XInterface >() );
}
//__________________________________________________________________________________________________
sal_Bool PropertySetInfo_Impl::hasPropertyByName( OUString const & name )
    throw (RuntimeException)
{
    beans::Property const * p = m_properties.getConstArray();
    for ( sal_Int32 nPos = m_properties.getLength(); nPos--; )
    {
        if (p[ nPos ].Name.equals( name ))
            return sal_True;
    }
    return sal_False;
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
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
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
    Reference<XInterface >          xNext;
};

ImplementationEnumeration_Impl::~ImplementationEnumeration_Impl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
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

typedef boost::unordered_set
<
    OUString,
    hashOWString_Impl,
    equalOWString_Impl
> HashSet_OWString;

typedef boost::unordered_multimap
<
    OUString,
    Reference<XInterface >,
    hashOWString_Impl,
    equalOWString_Impl
> HashMultimap_OWString_Interface;

typedef boost::unordered_map
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
            OSL_FAIL( "IllegalArgumentException caught" );
        }
        catch( const NoSuchElementException & )
        {
            OSL_FAIL( "NoSuchElementException caught" );
        }
    }
}


/*****************************************************************************
    class OServiceManager
*****************************************************************************/
struct OServiceManagerMutex
{
    Mutex m_mutex;
};

extern "C" void SAL_CALL smgrUnloadingListener(void* id);

typedef WeakComponentImplHelper7<
    lang::XMultiServiceFactory, lang::XMultiComponentFactory, lang::XServiceInfo,
    lang::XInitialization,
    container::XSet, container::XContentEnumerationAccess,
    beans::XPropertySet > t_OServiceManager_impl;

class OServiceManager
    : public OServiceManagerMutex
    , public t_OServiceManager_impl
{
public:
    friend void SAL_CALL smgrUnloadingListener(void* id);

    OServiceManager( Reference< XComponentContext > const & xContext );
    virtual ~OServiceManager();

    // XInitialization
    void SAL_CALL initialize( Sequence< Any > const & args )
        throw (Exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    static OUString getImplementationName_Static() throw(::com::sun::star::uno::RuntimeException)
        { return stoc_bootstrap::smgr_getImplementationName(); }
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

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

    // The same as the getAvailableServiceNames, but only uique names
    Sequence< OUString > getUniqueAvailableServiceNames(
        HashSet_OWString & aNameSet );

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual Reference<XEnumeration > SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & Element ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insert( const Any & Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const Any & Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    virtual Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

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
    inline bool is_disposed() const SAL_THROW( (lang::DisposedException) );
    inline void check_undisposed() const SAL_THROW( (lang::DisposedException) );
    virtual void SAL_CALL disposing();

    sal_Bool haveFactoryWithThisImplementation(const OUString& aImplName);

    virtual Sequence< Reference< XInterface > > queryServiceFactories(
        const OUString& aServiceName, Reference< XComponentContext > const & xContext );

    Reference< XComponentContext >  m_xContext;

    Reference< beans::XPropertySetInfo > m_xPropertyInfo;

    sal_Int32 m_nUnloadingListenerId;

    // Does clean up when the unloading mechanism has been set off. It is called from
    // the listener function smgrUnloadingListener.
    void onUnloadingNotify();
    // factories which have been loaded and not inserted( by XSet::insert)
    // are remembered by this set. Those factories
    // are not released on a call to onUnloadingNotify
    HashSet_Ref m_SetLoadedFactories;
private:

    Reference<XEventListener >      getFactoryListener();


    HashMultimap_OWString_Interface m_ServiceMap;
    HashSet_Ref                     m_ImplementationMap;
    HashMap_OWString_Interface      m_ImplementationNameMap;
    Reference<XEventListener >      xFactoryListener;
    bool                            m_bInDisposing;
};


//______________________________________________________________________________
inline bool OServiceManager::is_disposed() const
    SAL_THROW( (lang::DisposedException) )
{
    // ought to be guarded by m_mutex:
    return (m_bInDisposing || rBHelper.bDisposed);
}

//______________________________________________________________________________
inline void OServiceManager::check_undisposed() const
    SAL_THROW( (lang::DisposedException) )
{
    if (is_disposed())
    {
        throw lang::DisposedException(
            OUSTR("service manager instance has already been disposed!"),
            (OWeakObject *)this );
    }
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

typedef WeakComponentImplHelper6<
    lang::XMultiServiceFactory, lang::XMultiComponentFactory, lang::XServiceInfo,
    container::XSet, container::XContentEnumerationAccess,
    beans::XPropertySet > t_OServiceManagerWrapper_impl;

class OServiceManagerWrapper : public OServiceManagerMutex, public t_OServiceManagerWrapper_impl
{
    Reference< XComponentContext > m_xContext;
    Reference< XMultiComponentFactory > m_root;
    inline Reference< XMultiComponentFactory > getRoot() SAL_THROW( (RuntimeException) )
    {
        if (! m_root.is())
        {
            throw lang::DisposedException(
                OUSTR("service manager instance has already been disposed!"),
                Reference< XInterface >() );
        }
        return m_root;
    }

protected:
    virtual void SAL_CALL disposing();

public:
    OServiceManagerWrapper(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( (RuntimeException) );
    virtual ~OServiceManagerWrapper() SAL_THROW(());

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException)
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->getImplementationName(); }
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw (RuntimeException)
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->supportsService( ServiceName ); }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException)
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->getSupportedServiceNames(); }

    // XMultiComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        OUString const & rServiceSpecifier, Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException)
        { return getRoot()->createInstanceWithContext( rServiceSpecifier, xContext ); }
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        OUString const & rServiceSpecifier,
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException)
        { return getRoot()->createInstanceWithArgumentsAndContext( rServiceSpecifier, rArguments, xContext ); }
//      virtual Sequence< OUString > SAL_CALL getAvailableServiceNames()
//          throw (RuntimeException);

    // XMultiServiceFactory
    virtual Sequence< OUString > SAL_CALL getAvailableServiceNames() throw (RuntimeException)
        { return getRoot()->getAvailableServiceNames(); }
    virtual Reference<XInterface > SAL_CALL createInstance(const OUString & name) throw (Exception)
        { return getRoot()->createInstanceWithContext( name, m_xContext ); }
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const OUString & name, const Sequence<Any >& Arguments) throw (Exception)
        { return getRoot()->createInstanceWithArgumentsAndContext( name, Arguments, m_xContext ); }

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw (RuntimeException)
        { return Reference< XElementAccess >(getRoot(), UNO_QUERY_THROW)->getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException)
        { return Reference< XElementAccess >(getRoot(), UNO_QUERY_THROW)->hasElements(); }

    // XEnumerationAccess
    virtual Reference<XEnumeration > SAL_CALL createEnumeration() throw (RuntimeException)
        { return Reference< XEnumerationAccess >(getRoot(), UNO_QUERY_THROW)->createEnumeration(); }

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & Element ) throw (RuntimeException)
        { return Reference< XSet >(getRoot(), UNO_QUERY_THROW)->has( Element ); }
    virtual void SAL_CALL insert( const Any & Element ) throw (lang::IllegalArgumentException, container::ElementExistException, RuntimeException)
        { Reference< XSet >(getRoot(), UNO_QUERY_THROW)->insert( Element ); }
    virtual void SAL_CALL remove( const Any & Element ) throw (lang::IllegalArgumentException, container::NoSuchElementException, RuntimeException)
        { Reference< XSet >(getRoot(), UNO_QUERY_THROW)->remove( Element ); }

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    virtual Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) throw (RuntimeException)
        { return Reference< XContentEnumerationAccess >(getRoot(), UNO_QUERY_THROW)->createContentEnumeration( aServiceName ); }

    // XPropertySet
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (RuntimeException)
        { return Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->getPropertySetInfo(); }

    void SAL_CALL setPropertyValue(const OUString& PropertyName, const Any& aValue)
        throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, RuntimeException);
    Any SAL_CALL getPropertyValue(const OUString& PropertyName)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException);

    void SAL_CALL addPropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->addPropertyChangeListener( PropertyName, aListener ); }
    void SAL_CALL removePropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->removePropertyChangeListener( PropertyName, aListener ); }
    void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->addVetoableChangeListener( PropertyName, aListener ); }
    void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->removeVetoableChangeListener( PropertyName, aListener ); }
};
//__________________________________________________________________________________________________
void SAL_CALL OServiceManagerWrapper::setPropertyValue(
    const OUString& PropertyName, const Any& aValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
           lang::IllegalArgumentException, lang::WrappedTargetException, RuntimeException)
{
    if ( PropertyName == "DefaultContext" )
    {
        Reference< XComponentContext > xContext;
        if (aValue >>= xContext)
        {
            MutexGuard aGuard( m_mutex );
            m_xContext = xContext;
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("no XComponentContext given!") ),
                (OWeakObject *)this, 1 );
        }
    }
    else
    {
        Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->setPropertyValue( PropertyName, aValue );
    }
}
//__________________________________________________________________________________________________
Any SAL_CALL OServiceManagerWrapper::getPropertyValue(
    const OUString& PropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    if ( PropertyName == "DefaultContext" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xContext.is() )
            return makeAny( m_xContext );
        else
            return Any();
    }
    else
    {
        return Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->getPropertyValue( PropertyName );
    }
}
//__________________________________________________________________________________________________
void OServiceManagerWrapper::disposing()
{
    m_xContext.clear();

// no m_root->dispose(), because every context disposes its service manager...
    m_root.clear();
}
//__________________________________________________________________________________________________
OServiceManagerWrapper::~OServiceManagerWrapper() SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
OServiceManagerWrapper::OServiceManagerWrapper(
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
    : t_OServiceManagerWrapper_impl( m_mutex )
    , m_xContext( xContext )
    , m_root( xContext->getServiceManager() )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

    if (! m_root.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager to wrap") ),
            Reference< XInterface >() );
    }
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

/**
 * Create a ServiceManager
 */
OServiceManager::OServiceManager( Reference< XComponentContext > const & xContext )
    : t_OServiceManager_impl( m_mutex )
    , m_xContext( xContext )
    , m_bInDisposing( false )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_nUnloadingListenerId= rtl_addUnloadingListener( smgrUnloadingListener, this);
}

/**
 * Destroy the ServiceManager
 */
OServiceManager::~OServiceManager()
{
    if( m_nUnloadingListenerId != 0)
        rtl_removeUnloadingListener( m_nUnloadingListenerId );

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// Removes entries in m_ServiceMap, m_ImplementationNameMap and m_ImplementationNameMap
// if those entries have not been inserted through XSet::insert. Therefore the entries
// are compared with the entries in m_SetLoadedFactories.
void OServiceManager::onUnloadingNotify()
{
    MutexGuard aGuard( m_mutex);

    typedef HashSet_Ref::const_iterator CIT_S;
    typedef HashMultimap_OWString_Interface::iterator IT_MM;

    CIT_S it_SetEnd= m_SetLoadedFactories.end();
    IT_MM it_end1= m_ServiceMap.end();
    list<IT_MM> listDeleteServiceMap;
    typedef list<IT_MM>::const_iterator CIT_DMM;
    // find occurrences in m_ServiceMap
    for(IT_MM it_i1= m_ServiceMap.begin(); it_i1 != it_end1; ++it_i1)
    {
        if( m_SetLoadedFactories.find( it_i1->second) != it_SetEnd)
        {
            Reference<XUnloadingPreference> xunl( it_i1->second, UNO_QUERY);
            if( xunl.is())
            {
                if( xunl->releaseOnNotification())
                    listDeleteServiceMap.push_front( it_i1);
            }
            else
                listDeleteServiceMap.push_front( it_i1);
        }
    }
    // delete elements from m_ServiceMap
    CIT_DMM it_end2= listDeleteServiceMap.end();
    for( CIT_DMM it_i2= listDeleteServiceMap.begin(); it_i2 != it_end2; ++it_i2)
        m_ServiceMap.erase( *it_i2);

    // find elements in m_ImplementationNameMap
    typedef HashMap_OWString_Interface::iterator IT_M;
    IT_M it_end3= m_ImplementationNameMap.end();
    list<IT_M> listDeleteImplementationNameMap;
    typedef list<IT_M>::const_iterator CIT_DM;
    for( IT_M it_i3= m_ImplementationNameMap.begin();  it_i3 != it_end3; ++it_i3)
    {
        if( m_SetLoadedFactories.find( it_i3->second) != it_SetEnd)
        {
            Reference<XUnloadingPreference> xunl( it_i3->second, UNO_QUERY);
            if( xunl.is())
            {
                if( xunl->releaseOnNotification())
                    listDeleteImplementationNameMap.push_front( it_i3);
            }
            else
                listDeleteImplementationNameMap.push_front( it_i3);
        }
    }
    // delete elements from m_ImplementationNameMap
    CIT_DM it_end4= listDeleteImplementationNameMap.end();
    for( CIT_DM it_i4= listDeleteImplementationNameMap.begin(); it_i4 != it_end4; ++it_i4)
        m_ImplementationNameMap.erase( *it_i4);

    // find elements in m_ImplementationMap
    typedef HashSet_Ref::iterator IT_S;
    IT_S it_end5= m_ImplementationMap.end();
    list<IT_S> listDeleteImplementationMap;
    typedef list<IT_S>::const_iterator CIT_DS;
    for( IT_S it_i5= m_ImplementationMap.begin(); it_i5 != it_end5; ++it_i5)
    {
        if( m_SetLoadedFactories.find( *it_i5) != it_SetEnd)
        {
            Reference<XUnloadingPreference> xunl( *it_i5, UNO_QUERY);
            if( xunl.is())
            {
                if( xunl->releaseOnNotification())
                    listDeleteImplementationMap.push_front( it_i5);
            }
            else
                listDeleteImplementationMap.push_front( it_i5);
        }
    }
    // delete elements from m_ImplementationMap
    CIT_DS it_end6= listDeleteImplementationMap.end();
    for( CIT_DS it_i6= listDeleteImplementationMap.begin(); it_i6 != it_end6; ++it_i6)
        m_ImplementationMap.erase( *it_i6);

    // remove Event listener before the factories are released.
    IT_S it_end7= m_SetLoadedFactories.end();

    Reference<XEventListener> xlistener= getFactoryListener();
    for( IT_S it_i7= m_SetLoadedFactories.begin(); it_i7 != it_end7; ++it_i7)
    {
        Reference<XComponent> xcomp( *it_i7, UNO_QUERY);
        if( xcomp.is())
            xcomp->removeEventListener( xlistener);
    }
    // release the factories in m_SetLoadedFactories
    m_SetLoadedFactories.clear();
}

// XComponent
void OServiceManager::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    t_OServiceManager_impl::dispose();
}

void OServiceManager::disposing()
{
    // dispose all factories
    HashSet_Ref aImpls;
    {
        MutexGuard aGuard( m_mutex );
        m_bInDisposing = true;
        aImpls = m_ImplementationMap;
    }
    HashSet_Ref::iterator aIt = aImpls.begin();
    while( aIt != aImpls.end() )
    {
        try
        {
            Reference<XComponent > xComp( Reference<XComponent >::query( *aIt++ ) );
            if( xComp.is() )
                xComp->dispose();
        }
        catch (const RuntimeException & exc)
        {
#if OSL_DEBUG_LEVEL > 1
            OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "### RuntimeException occurred upon disposing factory: %s", str.getStr() );
#else
            (void) exc; // unused
#endif
        }
    }

    // dispose
    HashSet_Ref aImplMap;
    {
        MutexGuard aGuard( m_mutex );
        // erase all members
        m_ServiceMap = HashMultimap_OWString_Interface();
        aImplMap = m_ImplementationMap;
        m_ImplementationMap = HashSet_Ref();
        m_ImplementationNameMap = HashMap_OWString_Interface();
        m_SetLoadedFactories= HashSet_Ref();
    }

    m_xContext.clear();

    // not only the Event should hold the object
    OSL_ASSERT( m_refCount != 1 );

    // Revoke this service manager as unloading listener
    rtl_removeUnloadingListener( m_nUnloadingListenerId);
    m_nUnloadingListenerId=0;
}

// XPropertySet
Reference<XPropertySetInfo > OServiceManager::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if (! m_xPropertyInfo.is())
    {
        Sequence< beans::Property > seq( 1 );
        seq[ 0 ] = beans::Property(
            OUSTR("DefaultContext"), -1, ::getCppuType( &m_xContext ), 0 );
        Reference< beans::XPropertySetInfo > xInfo( new PropertySetInfo_Impl( seq ) );

        MutexGuard aGuard( m_mutex );
        if (! m_xPropertyInfo.is())
        {
            m_xPropertyInfo = xInfo;
        }
    }
    return m_xPropertyInfo;
}

void OServiceManager::setPropertyValue(
    const OUString& PropertyName, const Any& aValue )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if ( PropertyName == "DefaultContext" )
    {
        Reference< XComponentContext > xContext;
        if (aValue >>= xContext)
        {
            MutexGuard aGuard( m_mutex );
            m_xContext = xContext;
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("no XComponentContext given!") ),
                (OWeakObject *)this, 1 );
        }
    }
    else
    {
        throw UnknownPropertyException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("unknown property ") ) + PropertyName,
            (OWeakObject *)this );
    }
}

Any OServiceManager::getPropertyValue(const OUString& PropertyName)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if ( PropertyName == "DefaultContext" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xContext.is() )
            return makeAny( m_xContext );
        else
            return Any();
    }
    else
    {
        UnknownPropertyException except;
        except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager : unknown property " ) );
        except.Message += PropertyName;
        throw except;
    }
}

void OServiceManager::addPropertyChangeListener(
    const OUString&, const Reference<XPropertyChangeListener >&)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    throw UnknownPropertyException();
}

void OServiceManager::removePropertyChangeListener(
    const OUString&, const Reference<XPropertyChangeListener >&)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    throw UnknownPropertyException();
}

void OServiceManager::addVetoableChangeListener(
    const OUString&, const Reference<XVetoableChangeListener >&)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    throw UnknownPropertyException();
}

void OServiceManager::removeVetoableChangeListener(
    const OUString&, const Reference<XVetoableChangeListener >&)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    throw UnknownPropertyException();
}

// OServiceManager
Reference<XEventListener > OServiceManager::getFactoryListener()
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    if( !xFactoryListener.is() )
        xFactoryListener = new OServiceManager_Listener( this );
    return xFactoryListener;
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getUniqueAvailableServiceNames(
    HashSet_OWString & aNameSet )
{
    check_undisposed();
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
    OUString const & rServiceSpecifier,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
{
    check_undisposed();
#if OSL_DEBUG_LEVEL > 0
    Reference< beans::XPropertySet > xProps( xContext->getServiceManager(), UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xDefContext;
        xProps->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xDefContext;
        OSL_ENSURE(
            xContext == xDefContext,
            "### default context of service manager singleton differs from context holding it!" );
    }
#endif

    Sequence< Reference< XInterface > > factories(
        queryServiceFactories( rServiceSpecifier, xContext ) );
    Reference< XInterface > const * p = factories.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < factories.getLength(); ++nPos )
    {
        try
        {
            Reference< XInterface > const & xFactory = p[ nPos ];
            if (xFactory.is())
            {
                Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
                if (xFac.is())
                {
                    return xFac->createInstanceWithContext( xContext );
                }
                else
                {
                    Reference< XSingleServiceFactory > xFac2( xFactory, UNO_QUERY );
                    if (xFac2.is())
                    {
#if OSL_DEBUG_LEVEL > 1
                        OString aStr( OUStringToOString( rServiceSpecifier, RTL_TEXTENCODING_ASCII_US ) );
                        OSL_TRACE( "### ignoring given context raising service %s !!!", aStr.getStr() );
#endif
                        return xFac2->createInstance();
                    }
                }
            }
        }
        catch (const lang::DisposedException & exc)
        {
#if OSL_DEBUG_LEVEL > 1
            OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "### DisposedException occurred: %s", str.getStr() );
#else
            (void) exc; // unused
#endif
        }
    }

    return Reference< XInterface >();
}
// XMultiComponentFactory
Reference< XInterface > OServiceManager::createInstanceWithArgumentsAndContext(
    OUString const & rServiceSpecifier,
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
{
    check_undisposed();
#if OSL_DEBUG_LEVEL > 0
    Reference< beans::XPropertySet > xProps( xContext->getServiceManager(), UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xDefContext;
        xProps->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xDefContext;
        OSL_ENSURE(
            xContext == xDefContext,
            "### default context of service manager singleton differs from context holding it!" );
    }
#endif

    Sequence< Reference< XInterface > > factories(
        queryServiceFactories( rServiceSpecifier, xContext ) );
    Reference< XInterface > const * p = factories.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < factories.getLength(); ++nPos )
    {
        try
        {
            Reference< XInterface > const & xFactory = p[ nPos ];
            if (xFactory.is())
            {
                Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
                if (xFac.is())
                {
                    return xFac->createInstanceWithArgumentsAndContext( rArguments, xContext );
                }
                else
                {
                    Reference< XSingleServiceFactory > xFac2( xFactory, UNO_QUERY );
                    if (xFac2.is())
                    {
#if OSL_DEBUG_LEVEL > 1
                        OString aStr( OUStringToOString( rServiceSpecifier, RTL_TEXTENCODING_ASCII_US ) );
                        OSL_TRACE( "### ignoring given context raising service %s !!!", aStr.getStr() );
#endif
                        return xFac2->createInstanceWithArguments( rArguments );
                    }
                }
            }
        }
        catch (const lang::DisposedException & exc)
        {
#if OSL_DEBUG_LEVEL > 1
            OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "### DisposedException occurred: %s", str.getStr() );
#else
            (void) exc; // unused
#endif
        }
    }

    return Reference< XInterface >();
}

// XMultiServiceFactory, XMultiComponentFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getAvailableServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    // all names
    HashSet_OWString aNameSet;
    return getUniqueAvailableServiceNames( aNameSet );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstance(
    const OUString& rServiceSpecifier )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithContext(
        rServiceSpecifier, m_xContext );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstanceWithArguments(
    const OUString& rServiceSpecifier,
    const Sequence<Any >& rArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithArgumentsAndContext(
        rServiceSpecifier, rArguments, m_xContext );
}

// XInitialization
void OServiceManager::initialize( Sequence< Any > const & )
    throw (Exception)
{
    check_undisposed();
    OSL_FAIL( "not impl!" );
}

// XServiceInfo
OUString OServiceManager::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool OServiceManager::supportsService(const OUString& ServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
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
    check_undisposed();
    return stoc_bootstrap::smgr_getSupportedServiceNames();
}


Sequence< Reference< XInterface > > OServiceManager::queryServiceFactories(
    const OUString& aServiceName, Reference< XComponentContext > const & )
{
    Sequence< Reference< XInterface > > ret;

    MutexGuard aGuard( m_mutex );
    ::std::pair<
          HashMultimap_OWString_Interface::iterator,
          HashMultimap_OWString_Interface::iterator> p(
              m_ServiceMap.equal_range( aServiceName ) );

    if (p.first == p.second) // no factories
    {
        // no service found, look for an implementation
        HashMap_OWString_Interface::iterator aIt = m_ImplementationNameMap.find( aServiceName );
        if( aIt != m_ImplementationNameMap.end() )
        {
            Reference< XInterface > const & x = aIt->second;
            // an implementation found
            ret = Sequence< Reference< XInterface > >( &x, 1 );
        }
    }
    else
    {
        ::std::vector< Reference< XInterface > > vec;
        vec.reserve( 4 );
        while (p.first != p.second)
        {
            vec.push_back( p.first->second );
            ++p.first;
        }
        ret = Sequence< Reference< XInterface > >(
            vec.empty() ? 0 : &vec[ 0 ], vec.size() );
    }

    return ret;
}

// XContentEnumerationAccess
Reference<XEnumeration > OServiceManager::createContentEnumeration(
    const OUString& aServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    Sequence< Reference< XInterface > > factories(
        OServiceManager::queryServiceFactories( aServiceName, m_xContext ) );
    if (factories.getLength())
        return new ServiceEnumeration_Impl( factories );
    else
        return Reference< XEnumeration >();
}

// XEnumeration
Reference<XEnumeration > OServiceManager::createEnumeration() throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    return new ImplementationEnumeration_Impl( m_ImplementationMap );
}

// XElementAccess
Type OServiceManager::getElementType()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    return ::getCppuType( (const Reference< XInterface > *)0 );
}

// XElementAccess
sal_Bool OServiceManager::hasElements()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    return !m_ImplementationMap.empty();
}

// XSet
sal_Bool OServiceManager::has( const Any & Element )
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if( Element.getValueTypeClass() == TypeClass_INTERFACE )
    {
        Reference<XInterface > xEle( Element, UNO_QUERY_THROW );
        MutexGuard aGuard( m_mutex );
        return m_ImplementationMap.find( xEle ) !=
            m_ImplementationMap.end();
    }
    else if (Element.getValueTypeClass() == TypeClass_STRING)
    {
        OUString const & implName =
            *reinterpret_cast< OUString const * >(Element.getValue());
        MutexGuard aGuard( m_mutex );
        return m_ImplementationNameMap.find( implName ) !=
            m_ImplementationNameMap.end();
    }
    return sal_False;
}

// XSet
void OServiceManager::insert( const Any & Element )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if( Element.getValueTypeClass() != TypeClass_INTERFACE )
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no interface given!") ),
            Reference< XInterface >(), 0 );
    }
    Reference<XInterface > xEle( Element, UNO_QUERY_THROW );

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
        if( !aImplName.isEmpty() )
            m_ImplementationNameMap[ aImplName ] = xEle;

        //put into the service map
        Sequence< OUString > aServiceNames = xInfo->getSupportedServiceNames();
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
    if (is_disposed())
        return;

    Reference<XInterface > xEle;
    if (Element.getValueTypeClass() == TypeClass_INTERFACE)
    {
        xEle.set( Element, UNO_QUERY_THROW );
    }
    else if (Element.getValueTypeClass() == TypeClass_STRING)
    {
        OUString const & implName =
            *reinterpret_cast< OUString const * >(Element.getValue());
        MutexGuard aGuard( m_mutex );
        HashMap_OWString_Interface::const_iterator const iFind(
            m_ImplementationNameMap.find( implName ) );
        if (iFind == m_ImplementationNameMap.end())
        {
            throw NoSuchElementException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("element is not in: ") )
                + implName, static_cast< OWeakObject * >(this) );
        }
        xEle = iFind->second;
    }
    else
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "neither interface nor string given!") ),
            Reference< XInterface >(), 0 );
    }

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
            static_cast< OWeakObject * >(this) );
    }
    //First remove all factories which have been loaded by ORegistryServiceManager.
    m_SetLoadedFactories.erase( *aIt);
    //Remove from the implementation map. It contains all factories of m_SetLoadedFactories
    //which have been added directly through XSet, that is not via ORegistryServiceManager
    m_ImplementationMap.erase( aIt );

    // remove from the implementation name hashmap
    Reference<XServiceInfo > xInfo( Reference<XServiceInfo >::query( xEle ) );
    if( xInfo.is() )
    {
        OUString aImplName = xInfo->getImplementationName();
        if( !aImplName.isEmpty() )
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
class ORegistryServiceManager : public OServiceManager
{
public:
    ORegistryServiceManager( Reference< XComponentContext > const & xContext );
    virtual ~ORegistryServiceManager();

    // XInitialization
    void SAL_CALL initialize(const Sequence< Any >& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException)
        { return stoc_bootstrap::regsmgr_getImplementationName(); }

    Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XMultiServiceFactory
    Sequence< OUString > SAL_CALL getAvailableServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // OServiceManager
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw(::com::sun::star::uno::RuntimeException);
    Any SAL_CALL getPropertyValue(const OUString& PropertyName)
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected:
    //OServiceManager
    Sequence< Reference< XInterface > > queryServiceFactories(
        const OUString& aServiceName, Reference< XComponentContext > const & xContext );
private:
    Reference<XRegistryKey >        getRootKey();
    Reference<XInterface > loadWithImplementationName(
        const OUString & rImplName, Reference< XComponentContext > const & xContext );
    Sequence<OUString>          getFromServiceName(const OUString& serviceName);
    Reference<XInterface > loadWithServiceName(
        const OUString & rImplName, Reference< XComponentContext > const & xContext );
    void                        fillAllNamesFromRegistry( HashSet_OWString & );

    sal_Bool                    m_searchedRegistry;
    Reference<XSimpleRegistry > m_xRegistry;    // readonly property Registry
    Reference<XRegistryKey >    m_xRootKey;

#if OSL_DEBUG_LEVEL > 0
    bool m_init;
#endif
};

/**
 * Create a ServiceManager
 */
ORegistryServiceManager::ORegistryServiceManager( Reference< XComponentContext > const & xContext )
    : OServiceManager( xContext )
    , m_searchedRegistry(sal_False)
#if OSL_DEBUG_LEVEL > 0
    , m_init( false )
#endif
{
}

/**
 * Destroy the ServiceManager
 */
ORegistryServiceManager::~ORegistryServiceManager()
{
}

// XComponent
void ORegistryServiceManager::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    OServiceManager::dispose();
    // dispose
    MutexGuard aGuard( m_mutex );
    // erase all members
    m_xRegistry = Reference<XSimpleRegistry >();
    m_xRootKey = Reference<XRegistryKey >();
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
    const OUString& name, Reference< XComponentContext > const & xContext )
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
            Reference< lang::XMultiServiceFactory > xMgr;
            if (xContext.is())
                xMgr.set( xContext->getServiceManager(), UNO_QUERY_THROW );
            else
                xMgr.set( this );
            ret = createSingleRegistryFactory( xMgr, name, xImpKey );
            insert( makeAny( ret ) );
            // Remember this factory as loaded in contrast to inserted ( XSet::insert)
            // factories. Those loaded factories in this set are candidates for being
            // released on an unloading notification.
            m_SetLoadedFactories.insert( ret);
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
    OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "/SERVICES/" ) );
    buf.append( serviceName );
    return retrieveAsciiValueList( m_xRegistry, buf.makeStringAndClear() );
}

/**
 * Create a service provider from the registry
 */
Reference<XInterface > ORegistryServiceManager::loadWithServiceName(
    const OUString& serviceName, Reference< XComponentContext > const & xContext )
{
    Sequence<OUString> implEntries = getFromServiceName( serviceName );
    for (sal_Int32 i = 0; i < implEntries.getLength(); i++)
    {
        Reference< XInterface > x(
            loadWithImplementationName( implEntries.getConstArray()[i], xContext ) );
        if (x.is())
            return x;
    }

    return Reference<XInterface >();
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

// XInitialization
void ORegistryServiceManager::initialize(const Sequence< Any >& Arguments)
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    if (Arguments.getLength() > 0)
    {
        m_xRootKey.clear();
        Arguments[ 0 ] >>= m_xRegistry;
    }
#if OSL_DEBUG_LEVEL > 0
    // to find all bootstrapping processes to be fixed...
    OSL_ENSURE( !m_init, "### second init of service manager instance!" );
    m_init = true;
#endif
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > ORegistryServiceManager::getAvailableServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    // all names
    HashSet_OWString aNameSet;

    // all names from the registry
    fillAllNamesFromRegistry( aNameSet );

    return OServiceManager::getUniqueAvailableServiceNames( aNameSet );
}

// XServiceInfo
Sequence< OUString > ORegistryServiceManager::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    return stoc_bootstrap::regsmgr_getSupportedServiceNames();
}


// OServiceManager
Sequence< Reference< XInterface > > ORegistryServiceManager::queryServiceFactories(
    const OUString& aServiceName, Reference< XComponentContext > const & xContext )
{
    Sequence< Reference< XInterface > > ret(
        OServiceManager::queryServiceFactories( aServiceName, xContext ) );
    if (ret.getLength())
    {
        return ret;
    }
    else
    {
        MutexGuard aGuard( m_mutex );
        Reference< XInterface > x( loadWithServiceName( aServiceName, xContext ) );
        if (! x.is())
            x = loadWithImplementationName( aServiceName, xContext );
        return Sequence< Reference< XInterface > >( &x, 1 );
    }
}

// XContentEnumerationAccess
Reference<XEnumeration > ORegistryServiceManager::createContentEnumeration(
    const OUString& aServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
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
            loadWithImplementationName( aImplName, m_xContext );
        }
    }
    // call the superclass to enumerate all contents
    return OServiceManager::createContentEnumeration( aServiceName );
}

// OServiceManager
Reference<XPropertySetInfo > ORegistryServiceManager::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if (! m_xPropertyInfo.is())
    {
        Sequence< beans::Property > seq( 2 );
        seq[ 0 ] = beans::Property(
            OUSTR("DefaultContext"), -1, ::getCppuType( &m_xContext ), 0 );
        seq[ 1 ] = beans::Property(
            OUSTR("Registry"), -1, ::getCppuType( &m_xRegistry ),
            beans::PropertyAttribute::READONLY );
        Reference< beans::XPropertySetInfo > xInfo( new PropertySetInfo_Impl( seq ) );

        MutexGuard aGuard( m_mutex );
        if (! m_xPropertyInfo.is())
        {
            m_xPropertyInfo = xInfo;
        }
    }
    return m_xPropertyInfo;
}

Any ORegistryServiceManager::getPropertyValue(const OUString& PropertyName)
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    check_undisposed();
    if ( PropertyName == "Registry" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xRegistry.is() )
            return makeAny( m_xRegistry );
        else
            return Any();
    }
    return OServiceManager::getPropertyValue( PropertyName );
}

/* This is the listener function used by the service manager in order
to implement the unloading mechanism, id is the this pointer of the
service manager instances. On notification, that is the function is being called
by rtl_unloadUnusedModules, the cached factroies are being removed from the
service manager ( except manually inserted factories).
*/
extern "C" void SAL_CALL smgrUnloadingListener(void* id)
{
    stoc_smgr::OServiceManager* pMgr= reinterpret_cast<stoc_smgr::OServiceManager*>( id);
      pMgr->onUnloadingNotify();
}

} // namespace

namespace stoc_bootstrap
{
/**
 * Create the ServiceManager
 */
Reference<XInterface > SAL_CALL OServiceManager_CreateInstance(
    const Reference< XComponentContext > & xContext )
{
    return Reference<XInterface >(
        static_cast< XInterface * >(
            static_cast< OWeakObject * >( new stoc_smgr::OServiceManager( xContext ) ) ) );
}

/**
 * Create the ServiceManager
 */
Reference<XInterface > SAL_CALL ORegistryServiceManager_CreateInstance(
    const Reference< XComponentContext > & xContext )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return Reference<XInterface >(
        static_cast< XInterface * >(
            static_cast< OWeakObject * >( new stoc_smgr::ORegistryServiceManager( xContext ) ) ) );
}

Reference<XInterface > SAL_CALL OServiceManagerWrapper_CreateInstance(
    const Reference< XComponentContext > & xContext )
    throw (Exception)
{
    return (OWeakObject *)new stoc_smgr::OServiceManagerWrapper( xContext );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
