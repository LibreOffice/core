/*************************************************************************
 *
 *  $RCSfile: tdmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-07 14:48:47 $
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

//  #define TRACE(x) OSL_TRACE(x)
#define TRACE(x)

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include "lrucache.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <stl/algorithm>
#include <stl/vector>



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
template < class T >
inline sal_Bool extract( const Any & rAny, Reference< T > & rDest )
{
    rDest.clear();
    if (! (rAny >>= rDest))
    {
        if (rAny.getValueTypeClass() == TypeClass_INTERFACE)
            rDest = Reference< T >::query( *(const Reference< XInterface > *)rAny.getValue() );
    }
    return rDest.is();
}
//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
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
        {}

    // lifetime delegated to manager
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject & rEvt ) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class ManagerImpl : public WeakImplHelper3< XServiceInfo, XSet, XHierarchicalNameAccess >
{
    friend EnumerationImpl;
    friend EventListenerImpl;

    Mutex                               _aComponentMutex;
    Reference< XMultiServiceFactory >   _xSMgr;
    EventListenerImpl                   _aEventListener;

    // elements
    sal_Bool                            _bCaching;
    LRU_CacheAnyByOUString              _aElements;
    // provider chain
    ProviderVector                      _aProviders;
    sal_Bool                            _bProviderInit;

    inline void initProviders();
    inline Any getSimpleType( const OUString & rName );

public:
    ManagerImpl( const Reference< XMultiServiceFactory > & xSMgr );
    virtual ~ManagerImpl();

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
    MutexGuard aGuard( _pMgr->_aComponentMutex );
    if (rEvt.Source == _pMgr->_xSMgr)
    {
        Reference< XComponent > xComp( _pMgr->_xSMgr, UNO_QUERY );
        OSL_ENSHURE( xComp.is(), "### service manager must implement XComponent!" );
        xComp->removeEventListener( this );
        _pMgr->_bCaching = sal_False;
        _pMgr->_aElements.clear();
        _pMgr->_xSMgr.clear();
    }
    else
    {
        _pMgr->remove( makeAny( rEvt.Source ) );
    }
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
    return makeAny( _pMgr->_aProviders[_nPos] );
}

//##################################################################################################


//==================================================================================================
typelib_TypeDescription * createCTD( const Reference<XTypeDescription > & xType );

//==================================================================================================
extern "C"
{
static void SAL_CALL tdmgr_typelib_callback( void * pContext, typelib_TypeDescription ** ppRet,
                                             rtl_uString * pTypeName )
{
    OSL_ENSHURE( pContext && ppRet && pTypeName, "### null ptr!" );
    if (ppRet)
    {
        if (*ppRet)
        {
            typelib_typedescription_release( *ppRet );
            *ppRet = 0;
        }
        if (pContext && pTypeName)
        {
            try
            {
                Reference< XTypeDescription > xTD;
                if (reinterpret_cast< ManagerImpl * >( pContext )->getByHierarchicalName( pTypeName )
                    >>= xTD)
                {
                    *ppRet = createCTD( xTD );
                }
            }
            catch (...)
            {
            }
        }
#ifdef DEBUG
        if (! *ppRet)
        {
            OSL_TRACE( "### typelib type not accessable: " );
            OString aTypeName( OUStringToOString( pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( aTypeName.getStr() );
            OSL_TRACE( "\n" );
        }
#endif
    }
}
}

//__________________________________________________________________________________________________
ManagerImpl::ManagerImpl( const Reference< XMultiServiceFactory > & xSMgr )
    : _xSMgr( xSMgr )
    , _aEventListener( this )
    , _bCaching( sal_True )
    , _aElements( CACHE_SIZE )
    , _bProviderInit( sal_False )
{
    // register c typelib callback
    typelib_typedescription_registerCallback( this, tdmgr_typelib_callback );

    // listen to service manager vanishing...
    Reference< XComponent > xComp( _xSMgr, UNO_QUERY );
    OSL_ENSHURE( xComp.is(), "### service manager must implement XComponent!" );
    xComp->addEventListener( &_aEventListener );
}
//__________________________________________________________________________________________________
ManagerImpl::~ManagerImpl()
{
    OSL_ENSHURE( _aProviders.size() == 0, "### still providers left!" );
    TRACE( "> TypeDescriptionManager shut down. <\n" );
    MutexGuard aGuard( _aComponentMutex );

    // deregister of c typelib callback
    typelib_typedescription_revokeCallback( this, tdmgr_typelib_callback );
}
//__________________________________________________________________________________________________
inline void ManagerImpl::initProviders()
{
    // looking up service manager for all known provider implementations
    Reference< XContentEnumerationAccess > xEnumAccess( _xSMgr, UNO_QUERY );
    OSL_ENSHURE( xEnumAccess.is(), "### service manager must export XContentEnumerationAccess!" );

    Reference< XEnumeration > xEnum( xEnumAccess->createContentEnumeration(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.TypeDescriptionProvider") ) ) );
    OSL_ENSHURE( xEnum.is(), "### no TypeDescriptionProviders available!" );
    if (xEnum.is())
    {
        while (xEnum->hasMoreElements())
        {
            Any aAny( xEnum->nextElement() );
            if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
            {
                Reference< XServiceInfo > xInfo(
                    *(const Reference< XInterface > *)aAny.getValue(), UNO_QUERY );
                if (xInfo.is() &&
                    !xInfo->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(IMPLNAME) )) // no self insertion
                {
                    Reference< XSingleServiceFactory > xFactory(
                        *(const Reference< XInterface > *)aAny.getValue(), UNO_QUERY );
                    OSL_ENSHURE( xFactory.is(), "### the thing that should not be!" );

                    Reference< XHierarchicalNameAccess > xHA( xFactory->createInstance(), UNO_QUERY );
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
        }
    }
    OSL_ENSHURE( !_aProviders.empty(), "### no typedescription providers found!" );
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString ManagerImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
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
    return stoc_tdmgr::getSupportedServiceNames();
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
    if (extract( rElement, xElem ))
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
    if (! extract( rElement, xElem ))
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
    Reference< XHierarchicalNameAccess > xElem;
    if (! extract( rElement, xElem ))
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
    Reference< XComponent > xComp( xElem, UNO_QUERY );
    if (xComp.is())
        xComp->removeEventListener( &_aEventListener );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class SimpleTypeDescriptionImpl : public WeakImplHelper1< XTypeDescription >
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
class SequenceTypeDescriptionImpl : public WeakImplHelper1< XIndirectTypeDescription >
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
            if (extract( getByHierarchicalName( rName.copy( 2 ) ), xElemType ))
                aRet <<= Reference< XTypeDescription >( new SequenceTypeDescriptionImpl( xElemType ) );
            else
                return Any(); // further lookup makes no sense
        }
        else if ((nIndex = rName.indexOf( ':' )) >= 0) // test for interface member names
        {
            Reference< XInterfaceTypeDescription > xIfaceTD;
            if (extract( getByHierarchicalName( rName.copy( 0, nIndex ) ), xIfaceTD ))
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
            MutexGuard aGuard( _aComponentMutex );
            if (! _bProviderInit)
            {
                initProviders();
                _bProviderInit = sal_True;
            }
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
static Reference< XInterface > SAL_CALL ManagerImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
    throw(::com::sun::star::uno::Exception)
{
    return Reference< XInterface >( *new ManagerImpl( xSMgr ) );
}

}


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
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL = stoc_tdmgr::getSupportedServiceNames();
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

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            stoc_tdmgr::ManagerImpl_create,
            stoc_tdmgr::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


