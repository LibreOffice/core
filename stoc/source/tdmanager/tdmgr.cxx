/*************************************************************************
 *
 *  $RCSfile: tdmgr.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:04:23 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _STOC_TDMGR_COMMON_HXX
#include "tdmgr_common.hxx"
#endif
#ifndef _STOC_TDMGR_TDENUMERATION_HXX
#include "tdmgr_tdenumeration.hxx"
#endif
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
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <algorithm>
#include <vector>

using namespace std;
using namespace cppu;
using namespace rtl;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;


namespace stoc_tdmgr
{

static const sal_Int32 CACHE_SIZE = 512;

#define SERVICENAME "com.sun.star.reflection.TypeDescriptionManager"
#define IMPLNAME    "com.sun.star.comp.stoc.TypeDescriptionManager"

//--------------------------------------------------------------------------------------------------
// exported via tdmgr_common.hxx
rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static Sequence< OUString > SAL_CALL tdmgr_getSupportedServiceNames()
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

static OUString SAL_CALL tdmgr_getImplementationName()
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
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
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
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//==================================================================================================
class ManagerImpl
    : public WeakComponentImplHelper5< XServiceInfo,
                                       XSet,
                                       XHierarchicalNameAccess,
                                       XTypeDescriptionEnumerationAccess,
                                       XInitialization >
{
    friend EnumerationImpl;
    friend EventListenerImpl;

    Mutex                               _aComponentMutex;
    Reference< XComponentContext >      _xContext;
    EventListenerImpl                   _aEventListener;

    // elements
    sal_Bool                            _bCaching;
    LRU_CacheAnyByOUString              _aElements;
    // provider chain
    ProviderVector                      _aProviders;

    inline Any getSimpleType( const OUString & rName );

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
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
ManagerImpl::~ManagerImpl()
{
    OSL_ENSURE( _aProviders.size() == 0, "### still providers left!" );
    OSL_TRACE( "> TypeDescriptionManager shut down. <\n" );
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
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
    return tdmgr_getImplementationName();
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
    return tdmgr_getSupportedServiceNames();
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
    if (! (rElement >>= xElem))
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no type description provider given!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    MutexGuard aGuard( _aComponentMutex );
    if (find( _aProviders.begin(), _aProviders.end(), xElem ) != _aProviders.end())
    {
        throw ElementExistException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("provider already inserted!") ),
            (XWeak *)(OWeakObject *)this );
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

        it++;
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
            Reference< XTypeDescription > xElemType;
            if (getByHierarchicalName( rName.copy( 2 ) ) >>= xElemType)
                aRet <<= Reference< XTypeDescription >( new SequenceTypeDescriptionImpl( xElemType ) );
            else
                return Any(); // further lookup makes no sense
        }
        else if (rName[rName.getLength()-1] == ']') // test for array
        {
            sal_Int32 nIndex = 0, nTokens = 0;
            do { rName.getToken( 0, '[', nIndex ); nTokens++; } while( nIndex != -1 );
            sal_Int32 nDims = nTokens - 1;
            sal_Int32 dimOffset = rName.indexOf('[');
            Reference< XTypeDescription > xElemType;
            if (getByHierarchicalName( rName.copy( 0, dimOffset ) ) >>= xElemType)
                aRet <<= Reference< XTypeDescription >( new ArrayTypeDescriptionImpl( xElemType, nDims, rName.copy(dimOffset) ) );
            else
                return Any(); // further lookup makes no sense
        }
        else if ((nIndex = rName.indexOf( ':' )) >= 0) // test for interface member names
        {
            Reference< XInterfaceTypeDescription > xIfaceTD;
            if (getByHierarchicalName( rName.copy( 0, nIndex ) ) >>= xIfaceTD)
            {
                const Sequence< Reference< XInterfaceMemberTypeDescription > > & rMembers =
                    xIfaceTD->getMembers();
                const Reference< XInterfaceMemberTypeDescription > * pMembers =
                    rMembers.getConstArray();

                for ( sal_Int32 nPos = rMembers.getLength(); nPos--; )
                {
                    if (rName == pMembers[nPos]->getName())
                    {
                        aRet <<= Reference< XTypeDescription >::query( pMembers[nPos] );
                        break;
                    }
                }
                if (! aRet.hasValue())
                    return Any(); // further lookup makes no sense
            }
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
                    if ((aRet = (*iPos)->getByHierarchicalName( rName )).hasValue())
                        break;
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
        NoSuchElementException aExc;
        aExc.Message = rName;
        throw aExc;
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

//==================================================================================================
static Reference< XInterface > SAL_CALL ManagerImpl_create(
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    sal_Int32 nCacheSize;
    if (!xContext.is() || !(xContext->getValueByName( OUString(
        RTL_CONSTASCII_USTRINGPARAM("/implementations/" IMPLNAME "/CacheSize") ) ) >>= nCacheSize))
    {
        nCacheSize = CACHE_SIZE;
    }

    return Reference< XInterface >( *new ManagerImpl( xContext, nCacheSize ) );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################
using namespace stoc_tdmgr;

static struct ImplementationEntry g_entries[] =
{
    {
        ManagerImpl_create, tdmgr_getImplementationName,
        tdmgr_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

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
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}
