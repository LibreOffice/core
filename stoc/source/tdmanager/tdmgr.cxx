/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_stoc.hxx"
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include "rtl/ustrbuf.hxx"
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "tdmgr_common.hxx"
#include "tdmgr_tdenumeration.hxx"
#include "lrucache.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/XArrayTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

#include <algorithm>
#include <vector>

using namespace std;
using namespace cppu;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

static const sal_Int32 CACHE_SIZE = 512;

#define SERVICENAME "com.sun.star.reflection.TypeDescriptionManager"
#define IMPLNAME    "com.sun.star.comp.stoc.TypeDescriptionManager"

//--------------------------------------------------------------------------------------------------
// exported via tdmgr_common.hxx
extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
Sequence< OUString > SAL_CALL tdmgr_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

OUString SAL_CALL tdmgr_getImplementationName()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
            pImplName = &implName;
        }
    }
    return *pImplName;
}
}

namespace stoc_tdmgr
{
typedef vector< Reference< XHierarchicalNameAccess > > ProviderVector;

class EnumerationImpl;
class ManagerImpl;

//==================================================================================================
class EventListenerImpl : public ImplHelper1< XEventListener >
{
    ManagerImpl *       _pMgr;

public:
    EventListenerImpl( ManagerImpl * pMgr )
        : _pMgr( pMgr )
        {
            ::g_moduleCount.modCnt.acquire( &::g_moduleCount.modCnt );
        }
    virtual ~EventListenerImpl();

    // lifetime delegated to manager
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject & rEvt ) throw(::com::sun::star::uno::RuntimeException);
};

EventListenerImpl::~EventListenerImpl()
{
    ::g_moduleCount.modCnt.release( &::g_moduleCount.modCnt );
}

//==================================================================================================
class ManagerImpl
    : public WeakComponentImplHelper5< XServiceInfo,
                                       XSet,
                                       XHierarchicalNameAccess,
                                       XTypeDescriptionEnumerationAccess,
                                       XInitialization >
{
    friend class EnumerationImpl;
    friend class EventListenerImpl;

    Mutex                               _aComponentMutex;
    Reference< XComponentContext >      _xContext;
    EventListenerImpl                   _aEventListener;

    // elements
    sal_Bool                            _bCaching;
    LRU_CacheAnyByOUString              _aElements;
    // provider chain
    ProviderVector                      _aProviders;

    inline Any getSimpleType( const OUString & rName );

    Reference< XTypeDescription > getInstantiatedStruct(OUString const & name);

protected:
    virtual void SAL_CALL disposing();

public:
    ManagerImpl( Reference< XComponentContext > const & xContext, sal_Int32 nCacheSize );
    virtual ~ManagerImpl();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & args ) throw (Exception, RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & rElement ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insert( const Any & rElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const Any & rElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameAccess
    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);

    // XTypeDescriptionEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescriptionEnumeration > SAL_CALL
    createTypeDescriptionEnumeration(
        const ::rtl::OUString& moduleName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::TypeClass >& types,
        ::com::sun::star::reflection::TypeDescriptionSearchDepth depth )
            throw ( ::com::sun::star::reflection::NoSuchTypeNameException,
                    ::com::sun::star::reflection::InvalidTypeNameException,
                    ::com::sun::star::uno::RuntimeException );
};

//==================================================================================================
class EnumerationImpl
    : public WeakImplHelper1< XEnumeration >
{
    ManagerImpl *       _pMgr;
    size_t              _nPos;

public:
    EnumerationImpl( ManagerImpl * pManager );
    virtual ~EnumerationImpl();

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL nextElement() throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

//##################################################################################################

// lifetime delegated to manager
//__________________________________________________________________________________________________
void EventListenerImpl::acquire() throw()
{
    _pMgr->acquire();
}
//__________________________________________________________________________________________________
void EventListenerImpl::release() throw()
{
    _pMgr->release();
}

// XEventListener
//__________________________________________________________________________________________________
void EventListenerImpl::disposing( const EventObject & rEvt )
    throw(::com::sun::star::uno::RuntimeException)
{
    _pMgr->remove( makeAny( rEvt.Source ) );
}

//##################################################################################################

//__________________________________________________________________________________________________
EnumerationImpl::EnumerationImpl( ManagerImpl * pManager )
    : _pMgr( pManager )
    , _nPos( 0 )
{
    _pMgr->acquire();
}
//__________________________________________________________________________________________________
EnumerationImpl::~EnumerationImpl()
{
    _pMgr->release();
}

// XEnumeration
//__________________________________________________________________________________________________
sal_Bool EnumerationImpl::hasMoreElements()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( _pMgr->_aComponentMutex );
    return (_nPos < _pMgr->_aProviders.size());
}
//__________________________________________________________________________________________________
Any EnumerationImpl::nextElement()
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( _pMgr->_aComponentMutex );
    if (_nPos >= _pMgr->_aProviders.size())
    {
        throw NoSuchElementException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("there is no further element!") ),
            (XWeak *)(OWeakObject *)this );
    }
    return makeAny( _pMgr->_aProviders[_nPos++] );
}

//##################################################################################################

//__________________________________________________________________________________________________
ManagerImpl::ManagerImpl(
    Reference< XComponentContext > const & xContext, sal_Int32 nCacheSize )
    : WeakComponentImplHelper5<
        XServiceInfo, XSet, XHierarchicalNameAccess,
        XTypeDescriptionEnumerationAccess, XInitialization >( _aComponentMutex )
    , _xContext( xContext )
    , _aEventListener( this )
    , _bCaching( sal_True )
    , _aElements( nCacheSize )
{
    ::g_moduleCount.modCnt.acquire( &::g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
ManagerImpl::~ManagerImpl()
{
    OSL_ENSURE( _aProviders.size() == 0, "### still providers left!" );
    OSL_TRACE( "> TypeDescriptionManager shut down. <\n" );
    ::g_moduleCount.modCnt.release( &::g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
void ManagerImpl::disposing()
{
    // called on disposing the tdmgr instance (supposedly from context)
    _bCaching = sal_False;
    _aElements.clear();
    _xContext.clear();
    _aProviders.clear();
}

// XInitialization
//__________________________________________________________________________________________________
void ManagerImpl::initialize(
    const Sequence< Any > & args )
    throw (Exception, RuntimeException)
{
    // additional providers
    Any const * pProviders = args.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < args.getLength(); ++nPos )
    {
        Reference< XHierarchicalNameAccess > xHA( pProviders[ nPos ], UNO_QUERY );
        OSL_ENSURE( xHA.is(), "### no td provider!" );

        if (xHA.is())
        {
            try
            {
                insert( makeAny( xHA ) );
            }
            catch (IllegalArgumentException &)
            {
            }
            catch (ElementExistException &)
            {
            }
        }
    }
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString ManagerImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::tdmgr_getImplementationName();
}
//__________________________________________________________________________________________________
sal_Bool ManagerImpl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > ManagerImpl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::tdmgr_getSupportedServiceNames();
}

// XElementAccess
//__________________________________________________________________________________________________
Type ManagerImpl::getElementType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType( (const Reference< XHierarchicalNameAccess > *)0 );
}
//__________________________________________________________________________________________________
sal_Bool ManagerImpl::hasElements()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( _aComponentMutex );
    return (_aProviders.size() > 0);
}

// XEnumerationAccess
//__________________________________________________________________________________________________
Reference< XEnumeration > ManagerImpl::createEnumeration()
    throw(::com::sun::star::uno::RuntimeException)
{
    return new EnumerationImpl( this );
}

// XSet
//__________________________________________________________________________________________________
sal_Bool SAL_CALL ManagerImpl::has( const Any & rElement )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XHierarchicalNameAccess > xElem;
    if (rElement >>= xElem)
    {
        MutexGuard aGuard( _aComponentMutex );
        return (find( _aProviders.begin(), _aProviders.end(), xElem ) != _aProviders.end());
    }
    return sal_False;
}

//__________________________________________________________________________________________________
void SAL_CALL ManagerImpl::insert( const Any & rElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException)
{
    Reference< XHierarchicalNameAccess > xElem;
    if (! (rElement >>= xElem) || !xElem.is())
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no valid type description provider given!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    MutexGuard aGuard( _aComponentMutex );
    if (find( _aProviders.begin(), _aProviders.end(), xElem ) != _aProviders.end())
    {
        throw ElementExistException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("provider already inserted!") ),
            (XWeak *)(OWeakObject *)this );
    }

    if (! _aProviders.empty())
    {
        // check whether all types are compatible, if possible:
        Reference<reflection::XTypeDescriptionEnumerationAccess> xTDEnumAccess(
            xElem, UNO_QUERY );
        OSL_ENSURE( xTDEnumAccess.is(),
                    "### providers ought to implement "
                    "reflection::XTypeDescriptionEnumerationAccess!" );
        if (xTDEnumAccess.is())
        {
            try
            {
                TypeClass ar [] = {
                    TypeClass_ENUM, TypeClass_TYPEDEF, TypeClass_SEQUENCE,
                    TypeClass_STRUCT, TypeClass_EXCEPTION,
                    /* TypeClass_UNION, TypeClass_ARRAY not supported */
                    TypeClass_INTERFACE,
                    TypeClass_SERVICE,
                    TypeClass_INTERFACE_METHOD, TypeClass_INTERFACE_ATTRIBUTE,
                    TypeClass_PROPERTY, TypeClass_CONSTANT, TypeClass_CONSTANTS,
                    TypeClass_SINGLETON
                };
                Reference<reflection::XTypeDescriptionEnumeration> xTDEnum(
                    xTDEnumAccess->createTypeDescriptionEnumeration(
                        OUString() /* all modules */,
                        Sequence<TypeClass>( ar, ARLEN(ar) ),
                        reflection::TypeDescriptionSearchDepth_INFINITE ) );

                while (xTDEnum->hasMoreElements())
                {
                    Reference<reflection::XTypeDescription> xNewTD;
                    try
                    {
                        xNewTD = xTDEnum->nextTypeDescription();
                    }
                    catch (container::NoSuchElementException & exc)
                    {
                        throw lang::IllegalArgumentException(
                            OUSTR("NoSuchElementException occurred: ") +
                            exc.Message, static_cast<OWeakObject *>(this),
                            -1 /* unknown */ );
                    }

                    try
                    {
                        OUString newName( xNewTD->getName() );
                        Reference<reflection::XTypeDescription> xExistingTD(
                            getByHierarchicalName( newName ), UNO_QUERY );
                        OSL_ASSERT( xExistingTD.is() );
                        // existing, check whether compatible:
                        if (xExistingTD.is())
                        {
                            try
                            {
                                check( xNewTD, xExistingTD );
                            }
                            catch (IncompatibleTypeException & exc)
                            {
                                throw lang::IllegalArgumentException(
                                    OUSTR("Rejecting types due to "
                                          "incompatibility!  ") + exc.m_cause,
                                    static_cast<OWeakObject *>(this), 0 );
                            }
                        }
                    }
                    catch (container::NoSuchElementException &)
                    {
                        // type not in: ok
                    }
                }
            }
            catch (reflection::NoSuchTypeNameException & exc)
            {
                throw lang::IllegalArgumentException(
                    OUSTR("NoSuchTypeNameException occurred: ") + exc.Message,
                    static_cast<OWeakObject *>(this), -1 /* unknown */ );
            }
            catch (reflection::InvalidTypeNameException & exc)
            {
                throw lang::IllegalArgumentException(
                    OUSTR("InvalidTypeNameException occurred: ") + exc.Message,
                    static_cast<OWeakObject *>(this), -1 /* unknown */ );
            }
        }
    }

    _aProviders.push_back( xElem );
    Reference< XComponent > xComp( xElem, UNO_QUERY );
    if (xComp.is())
        xComp->addEventListener( &_aEventListener );
}
//__________________________________________________________________________________________________
void SAL_CALL ManagerImpl::remove( const Any & rElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        Reference< XHierarchicalNameAccess > xElem;
        if (! (rElement >>= xElem))
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("no type description provider given!") ),
                (XWeak *)(OWeakObject *)this, 0 );
        }

        MutexGuard aGuard( _aComponentMutex );
        ProviderVector::iterator iFind( find( _aProviders.begin(), _aProviders.end(), xElem ) );
        if (iFind == _aProviders.end())
        {
            throw NoSuchElementException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("provider not found!") ),
                (XWeak *)(OWeakObject *)this );
        }
        _aProviders.erase( iFind );
    }

    Reference< XComponent > xComp;
    if (rElement >>= xComp)
        xComp->removeEventListener( &_aEventListener );
}

// XTypeDescriptionEnumerationAccess
//__________________________________________________________________________________________________
// virtual
Reference< XTypeDescriptionEnumeration > SAL_CALL
ManagerImpl::createTypeDescriptionEnumeration(
        const OUString & moduleName,
        const Sequence< TypeClass > & types,
        TypeDescriptionSearchDepth depth )
    throw ( NoSuchTypeNameException,
            InvalidTypeNameException,
            RuntimeException )
{
    MutexGuard aGuard( _aComponentMutex );

    TDEnumerationAccessStack aStack;
    ProviderVector::const_iterator it = _aProviders.begin();
    const ProviderVector::const_iterator end = _aProviders.end();
    while ( it != end )
    {
        Reference< XTypeDescriptionEnumerationAccess >xEnumAccess(
            (*it), UNO_QUERY );
        OSL_ENSURE( xEnumAccess.is(),
                    "### no XTypeDescriptionEnumerationAccess!" );
        if ( xEnumAccess.is() )
            aStack.push( xEnumAccess );

        ++it;
    }

    return Reference< XTypeDescriptionEnumeration >(
        new TypeDescriptionEnumerationImpl( moduleName,
                                            types,
                                            depth,
                                            aStack ) );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class SimpleTypeDescriptionImpl
    : public WeakImplHelper1< XTypeDescription >
{
    TypeClass _eTC;
    OUString  _aName;

public:
    SimpleTypeDescriptionImpl( TypeClass eTC, const OUString & rName )
        : _eTC( eTC )
        , _aName( rName )
        {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
};

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass SimpleTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _eTC;
}
//__________________________________________________________________________________________________
OUString SimpleTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

//==================================================================================================
class SequenceTypeDescriptionImpl
    : public WeakImplHelper1< XIndirectTypeDescription >
{
    Reference< XTypeDescription > _xElementTD;

public:
    SequenceTypeDescriptionImpl( const Reference< XTypeDescription > & xElementTD )
        : _xElementTD( xElementTD )
        {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XIndirectTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getReferencedType() throw(::com::sun::star::uno::RuntimeException);
};

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass SequenceTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_SEQUENCE;
}
//__________________________________________________________________________________________________
OUString SequenceTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return (OUString( RTL_CONSTASCII_USTRINGPARAM("[]") ) + _xElementTD->getName());
}

// XIndirectTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > SequenceTypeDescriptionImpl::getReferencedType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _xElementTD;
}

//==================================================================================================
class ArrayTypeDescriptionImpl
    : public WeakImplHelper1< XArrayTypeDescription >
{
    Reference< XTypeDescription > _xElementTD;
    Mutex                         _aDimensionMutex;
    sal_Int32                     _nDimensions;
    Sequence< sal_Int32 >         _seqDimensions;
    OUString                      _sDimensions;

    void initDimensions(const OUString& rSDimensions);
public:
    ArrayTypeDescriptionImpl( const Reference< XTypeDescription > & xElementTD,
                              sal_Int32 nDimensions, const OUString& rSDimensions )
        : _xElementTD( xElementTD )
        , _nDimensions( nDimensions )
        , _seqDimensions( Sequence< sal_Int32 >(nDimensions) )
        , _sDimensions( rSDimensions )
        {
            initDimensions( rSDimensions );
        }
    virtual ~ArrayTypeDescriptionImpl() {}

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XArrayTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getNumberOfDimensions() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int32 > SAL_CALL getDimensions() throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
static sal_Int32 unicodeToInteger( sal_Int8 base, const sal_Unicode *s )
{
    sal_Int32    r = 0;
    sal_Int32    negative = 0;

    if (*s == '-')
       {
        negative = 1;
          s++;
       }
       if (base == 8 && *s == '0')
        s++;
       else if (base == 16 && *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))
        s += 2;

       for (; *s; s++)
       {
           if (*s <= '9' && *s >= '0')
            r = (r * base) + (*s - '0');
          else if (base > 10 && *s <= 'f' && *s >= 'a')
            r = (r * base) + (*s - 'a' + 10);
          else if (base > 10 && *s <= 'F' && *s >= 'A')
            r = (r * base) + (*s - 'A' + 10);
           else
            break;
    }
       if (negative) r *= -1;
    return r;
}
//__________________________________________________________________________________________________
void ArrayTypeDescriptionImpl::initDimensions(const OUString& rSDimensions)
{
    MutexGuard aGuard( _aDimensionMutex );

    sal_Int32 *  pDimensions = _seqDimensions.getArray();
    OUString tmp(rSDimensions);
    sal_Unicode* p = (sal_Unicode*)tmp.getStr()+1;
    sal_Unicode* pOffset = p;
    sal_Int32 len = tmp.getLength() - 1 ;
    sal_Int32 i = 0;

    while ( len > 0)
    {
        pOffset++;
        if (*pOffset == ']')
        {
            *pOffset = '\0';
            pOffset += 2;
            len -= 3;
            pDimensions[i++] = unicodeToInteger(10, p);
            p = pOffset;
        } else
            len--;
    }
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass ArrayTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_ARRAY;
}
//__________________________________________________________________________________________________
OUString ArrayTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return (_xElementTD->getName() + _sDimensions);
}

// XArrayTypeDescription
//__________________________________________________________________________________________________
Reference< XTypeDescription > ArrayTypeDescriptionImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _xElementTD;
}

//__________________________________________________________________________________________________
sal_Int32 ArrayTypeDescriptionImpl::getNumberOfDimensions()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nDimensions;
}

//__________________________________________________________________________________________________
Sequence< sal_Int32 > ArrayTypeDescriptionImpl::getDimensions()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _seqDimensions;
}

//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
inline Any ManagerImpl::getSimpleType( const OUString & rName )
{
    Any aRet;

    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("string") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_STRING, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("long") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_LONG, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("unsigned long") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_UNSIGNED_LONG, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("boolean") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_BOOLEAN, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("char") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_CHAR, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("byte") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_BYTE, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_SHORT, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("unsigned short") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_UNSIGNED_SHORT, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("hyper") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_HYPER, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("unsigned hyper") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_UNSIGNED_HYPER, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("float") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_FLOAT, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("double") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_DOUBLE, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("any") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_ANY, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("void") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_VOID, rName ) );
    else if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("type") ))
        aRet <<= Reference< XTypeDescription >( new SimpleTypeDescriptionImpl( TypeClass_TYPE, rName ) );

    return aRet;
}

namespace {

Reference< XTypeDescription > resolveTypedefs(
    Reference< XTypeDescription > const & type)
{
    Reference< XTypeDescription > resolved(type);
    while (resolved->getTypeClass() == TypeClass_TYPEDEF) {
        resolved = Reference< XIndirectTypeDescription >(
            type, UNO_QUERY_THROW)->getReferencedType();
    }
    return resolved;
}

bool isNonVoidNonExceptionType(Reference< XTypeDescription > const & type) {
    switch (type->getTypeClass()) {
    case TypeClass_BOOLEAN:
    case TypeClass_BYTE:
    case TypeClass_SHORT:
    case TypeClass_UNSIGNED_SHORT:
    case TypeClass_LONG:
    case TypeClass_UNSIGNED_LONG:
    case TypeClass_HYPER:
    case TypeClass_UNSIGNED_HYPER:
    case TypeClass_FLOAT:
    case TypeClass_DOUBLE:
    case TypeClass_CHAR:
    case TypeClass_STRING:
    case TypeClass_TYPE:
    case TypeClass_ANY:
    case TypeClass_SEQUENCE:
    case TypeClass_ENUM:
    case TypeClass_STRUCT:
    case TypeClass_INTERFACE:
        return true;

    default:
        return false;
    }
}

class InstantiatedStruct: public WeakImplHelper1< XStructTypeDescription > {
public:
    InstantiatedStruct(
        Reference< XStructTypeDescription > const & structType,
        std::vector< Reference< XTypeDescription > > const & arguments);

    virtual TypeClass SAL_CALL getTypeClass() throw (RuntimeException)
    { return TypeClass_STRUCT; }

    virtual OUString SAL_CALL getName() throw (RuntimeException);

    virtual Reference< XTypeDescription > SAL_CALL getBaseType()
        throw (RuntimeException)
    { return m_struct->getBaseType(); }

    virtual Sequence< Reference< XTypeDescription > > SAL_CALL getMemberTypes()
        throw (RuntimeException);

    virtual Sequence< OUString > SAL_CALL getMemberNames()
        throw (RuntimeException)
    { return m_struct->getMemberNames(); }

    virtual Sequence< OUString > SAL_CALL getTypeParameters()
        throw (RuntimeException)
    { return Sequence< OUString >(); }

    virtual Sequence< Reference< XTypeDescription > > SAL_CALL
    getTypeArguments() throw (RuntimeException)
    { return m_arguments; }

private:
    Reference< XStructTypeDescription > m_struct;
    Sequence< Reference< XTypeDescription > > m_arguments;
};

InstantiatedStruct::InstantiatedStruct(
    Reference< XStructTypeDescription > const & structType,
    std::vector< Reference< XTypeDescription > > const & arguments):
    m_struct(structType),
    m_arguments(static_cast< sal_Int32 >(arguments.size()))
{
    for (std::vector< Reference< XTypeDescription > >::size_type i = 0;
         i < arguments.size(); ++i)
    {
        m_arguments[static_cast< sal_Int32 >(i)] = arguments[i];
    }
}

OUString InstantiatedStruct::getName() throw (RuntimeException) {
    OUStringBuffer buf(m_struct->getName());
    buf.append(static_cast< sal_Unicode >('<'));
    for (sal_Int32 i = 0; i < m_arguments.getLength(); ++i) {
        if (i != 0) {
            buf.append(static_cast< sal_Unicode >(','));
        }
        buf.append(m_arguments[i]->getName());
    }
    buf.append(static_cast< sal_Unicode >('>'));
    return buf.makeStringAndClear();
}

Sequence< Reference< XTypeDescription > > InstantiatedStruct::getMemberTypes()
    throw (RuntimeException)
{
    Sequence< Reference< XTypeDescription > > types(m_struct->getMemberTypes());
    for (sal_Int32 i = 0; i < types.getLength(); ++i) {
        if (types[i]->getTypeClass() == TypeClass_UNKNOWN) {
            Sequence< OUString > parameters(m_struct->getTypeParameters());
            OSL_ASSERT(parameters.getLength() == m_arguments.getLength());
            for (sal_Int32 j = 0; j < parameters.getLength(); ++j) {
                if (parameters[j] == types[i]->getName()) {
                    types[i] = m_arguments[j];
                    break;
                }
            }
        }
    }
    return types;
}

}

Reference< XTypeDescription > ManagerImpl::getInstantiatedStruct(
    OUString const & name)
{
    sal_Int32 i = name.indexOf('<');
    OSL_ASSERT(i >= 0);
    Reference< XStructTypeDescription > structType(
        getByHierarchicalName(name.copy(0, i)), UNO_QUERY);
    std::vector< Reference< XTypeDescription > > args;
    bool good = structType.is();
    if (good) {
        do {
            ++i; // skip '<' or ','
            sal_Int32 j = i;
            for (sal_Int32 level = 0; j != name.getLength(); ++j) {
                sal_Unicode c = name[j];
                if (c == ',') {
                    if (level == 0) {
                        break;
                    }
                } else if (c == '<') {
                    ++level;
                } else if (c == '>') {
                    if (level == 0) {
                        break;
                    }
                    --level;
                }
            }
            if (j != name.getLength()) {
                Reference< XTypeDescription > type(
                    getByHierarchicalName(name.copy(i, j - i)), UNO_QUERY);
                if (isNonVoidNonExceptionType(resolveTypedefs(type))) {
                    args.push_back(type);
                } else {
                    good = false;
                    break;
                }
            }
            i = j;
        } while (i != name.getLength() && name[i] != '>');
        good = good && i == name.getLength() - 1
            && name[i] == '>' && !args.empty();
    }
    // args.size() cannot exceed SAL_MAX_INT32, as each argument consumes at
    // least one position within an rtl::OUString (which is no longer than
    // SAL_MAX_INT32):
    if (!good
        || (args.size()
            != sal::static_int_cast< sal_uInt32 >(
                structType->getTypeParameters().getLength())))
    {
        throw NoSuchElementException(name, static_cast< OWeakObject * >(this));
    }
    return new InstantiatedStruct(structType, args);
}

// XHierarchicalNameAccess
//__________________________________________________________________________________________________
Any ManagerImpl::getByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    Any aRet;
    if (_bCaching)
        aRet = _aElements.getValue( rName );
    if (rName.getLength() && !aRet.hasValue())
    {
        sal_Int32 nIndex;
        if (rName[0] == '[') // test for sequence
        {
            Reference< XTypeDescription > xElemType(
                getByHierarchicalName( rName.copy( 2 ) ),
                UNO_QUERY_THROW );
            aRet <<= Reference< XTypeDescription >(
                new SequenceTypeDescriptionImpl( xElemType ) );
        }
        else if (rName[rName.getLength()-1] == ']') // test for array
        {
            sal_Int32 nIndex2 = 0, nTokens = 0;
            do { rName.getToken( 0, '[', nIndex2 ); nTokens++; } while( nIndex2 != -1 );
            sal_Int32 nDims = nTokens - 1;
            sal_Int32 dimOffset = rName.indexOf('[');
            Reference< XTypeDescription > xElemType(
                getByHierarchicalName( rName.copy( 0, dimOffset ) ),
                UNO_QUERY_THROW );
            aRet <<= Reference< XTypeDescription >(
                new ArrayTypeDescriptionImpl(
                    xElemType, nDims, rName.copy(dimOffset) ) );
        }
        // test for interface member names:
        else if ((nIndex = rName.indexOf( ':' )) >= 0)
        {
            Reference< XInterfaceTypeDescription > xIfaceTD(
                getByHierarchicalName( rName.copy( 0, nIndex ) ),
                UNO_QUERY_THROW );
            const Sequence< Reference< XInterfaceMemberTypeDescription > > &
                rMembers = xIfaceTD->getMembers();
            const Reference< XInterfaceMemberTypeDescription > * pMembers =
                rMembers.getConstArray();

            for ( sal_Int32 nPos = rMembers.getLength(); nPos--; )
            {
                if (rName == pMembers[nPos]->getName())
                {
                    aRet <<= Reference< XTypeDescription >(
                        pMembers[nPos], UNO_QUERY_THROW );
                    break;
                }
            }
            if (! aRet.hasValue())
            {
                // member not found:
                throw NoSuchElementException(
                    rName, static_cast< OWeakObject * >(this) );
            }
        }
        // test for instantiated polymorphic struct types:
        else if (rName.indexOf('<') >= 0)
        {
            aRet <<= getInstantiatedStruct(rName);
        }
        else if (rName.indexOf( '.' ) < 0) // test for simple/ build in types
        {
            aRet = getSimpleType( rName );
        }

        if (! aRet.hasValue())
        {
            // last, try callback chain
            for ( ProviderVector::const_iterator iPos( _aProviders.begin() );
                  iPos != _aProviders.end(); ++iPos )
            {
                try
                {
                    if ((aRet = (*iPos)->getByHierarchicalName(
                             rName )).hasValue())
                    {
                        break;
                    }
                }
                catch (NoSuchElementException &)
                {
                }
            }
        }

        // update cache
        if (_bCaching && aRet.hasValue())
            _aElements.setValue( rName, aRet );
    }

    if (! aRet.hasValue())
    {
        throw NoSuchElementException(
            rName, static_cast< OWeakObject * >(this) );
    }
    return aRet;
}
//__________________________________________________________________________________________________
sal_Bool ManagerImpl::hasByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        return getByHierarchicalName( rName ).hasValue();
    }
    catch (NoSuchElementException &)
    {
    }
    return sal_False;
}
}

namespace stoc_bootstrap
{
//==================================================================================================
Reference< XInterface > SAL_CALL ManagerImpl_create(
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    sal_Int32 nCacheSize = CACHE_SIZE;
    if (xContext.is()) {
        xContext->getValueByName(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/implementations/" IMPLNAME "/CacheSize"))) >>=
            nCacheSize;
    }

    return Reference< XInterface >( *new stoc_tdmgr::ManagerImpl( xContext, nCacheSize ) );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
