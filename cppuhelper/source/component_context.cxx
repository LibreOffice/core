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


#ifdef DIAG
#define CONTEXT_DIAG
#endif

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

#include <vector>
#include <boost/unordered_map.hpp>
#ifdef CONTEXT_DIAG
#include <map>
#endif

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <uno/mapping.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

#include <memory>

#define SMGR_SINGLETON "/singletons/com.sun.star.lang.theServiceManager"
#define TDMGR_SINGLETON "/singletons/com.sun.star.reflection.theTypeDescriptionManager"
#define AC_SINGLETON "/singletons/com.sun.star.security.theAccessController"
#define AC_POLICY "/singletons/com.sun.star.security.thePolicy"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace cppu
{

#ifdef CONTEXT_DIAG
//--------------------------------------------------------------------------------------------------
static OUString val2str( void const * pVal, typelib_TypeDescriptionReference * pTypeRef )
{
    OSL_ASSERT( pVal );
    if (pTypeRef->eTypeClass == typelib_TypeClass_VOID)
        return OUSTR("void");

    OUStringBuffer buf( 64 );
    buf.append( (sal_Unicode)'(' );
    buf.append( pTypeRef->pTypeName );
    buf.append( (sal_Unicode)')' );

    switch (pTypeRef->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( (sal_Int64)*(void **)pVal, 16 );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
        typelib_TypeDescription * pTypeDescr = 0;
        ::typelib_typedescriptionreference_getDescription( &pTypeDescr, pTypeRef );
        OSL_ASSERT( pTypeDescr );
        if (! pTypeDescr->bComplete)
            ::typelib_typedescription_complete( &pTypeDescr );

        typelib_CompoundTypeDescription * pCompType = (typelib_CompoundTypeDescription *)pTypeDescr;
        sal_Int32 nDescr = pCompType->nMembers;

        if (pCompType->pBaseTypeDescription)
        {
            buf.append( val2str( pVal, ((typelib_TypeDescription *)pCompType->pBaseTypeDescription)->pWeakRef ) );
            if (nDescr)
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
        }

        typelib_TypeDescriptionReference ** ppTypeRefs = pCompType->ppTypeRefs;
        sal_Int32 * pMemberOffsets = pCompType->pMemberOffsets;
        rtl_uString ** ppMemberNames = pCompType->ppMemberNames;

        for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
        {
            buf.append( ppMemberNames[ nPos ] );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" = ") );
            typelib_TypeDescription * pMemberType = 0;
            TYPELIB_DANGER_GET( &pMemberType, ppTypeRefs[ nPos ] );
            buf.append( val2str( (char *)pVal + pMemberOffsets[ nPos ], pMemberType->pWeakRef ) );
            TYPELIB_DANGER_RELEASE( pMemberType );
            if (nPos < (nDescr -1))
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
        }

        ::typelib_typedescription_release( pTypeDescr );

        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

        uno_Sequence * pSequence = *(uno_Sequence **)pVal;
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );

        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        sal_Int32 nElements    = pSequence->nElements;

        if (nElements)
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
            char * pElements = pSequence->elements;
            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
                buf.append( val2str( pElements + (nElementSize * nPos), pElementTypeDescr->pWeakRef ) );
                if (nPos < (nElements -1))
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
            }
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
        }
        else
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{}") );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        break;
    }
    case typelib_TypeClass_ANY:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
        buf.append( val2str( ((uno_Any *)pVal)->pData,
                             ((uno_Any *)pVal)->pType ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
        break;
    case typelib_TypeClass_TYPE:
        buf.append( (*(typelib_TypeDescriptionReference **)pVal)->pTypeName );
        break;
    case typelib_TypeClass_STRING:
        buf.append( (sal_Unicode)'\"' );
        buf.append( *(rtl_uString **)pVal );
        buf.append( (sal_Unicode)'\"' );
        break;
    case typelib_TypeClass_ENUM:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        ::typelib_typedescriptionreference_getDescription( &pTypeDescr, pTypeRef );
        OSL_ASSERT( pTypeDescr );
        if (! pTypeDescr->bComplete)
            ::typelib_typedescription_complete( &pTypeDescr );

        sal_Int32 * pValues = ((typelib_EnumTypeDescription *)pTypeDescr)->pEnumValues;
        sal_Int32 nPos = ((typelib_EnumTypeDescription *)pTypeDescr)->nEnumValues;
        while (nPos--)
        {
            if (pValues[ nPos ] == *(sal_Int32 *)pVal)
                break;
        }
        if (nPos >= 0)
            buf.append( ((typelib_EnumTypeDescription *)pTypeDescr)->ppEnumNames[ nPos ] );
        else
            buf.append( (sal_Unicode)'?' );

        ::typelib_typedescription_release( pTypeDescr );
        break;
    }
    case typelib_TypeClass_BOOLEAN:
        if (*(sal_Bool *)pVal)
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("true") );
        else
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("false") );
        break;
    case typelib_TypeClass_CHAR:
        buf.append( (sal_Unicode)'\'' );
        buf.append( *(sal_Unicode *)pVal );
        buf.append( (sal_Unicode)'\'' );
        break;
    case typelib_TypeClass_FLOAT:
        buf.append( *(float *)pVal );
        break;
    case typelib_TypeClass_DOUBLE:
        buf.append( *(double *)pVal );
        break;
    case typelib_TypeClass_BYTE:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( (sal_Int32)*(sal_Int8 *)pVal, 16 );
        break;
    case typelib_TypeClass_SHORT:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( (sal_Int32)*(sal_Int16 *)pVal, 16 );
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( (sal_Int32)*(sal_uInt16 *)pVal, 16 );
        break;
    case typelib_TypeClass_LONG:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( *(sal_Int32 *)pVal, 16 );
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( (sal_Int64)*(sal_uInt32 *)pVal, 16 );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
#if defined(GCC) && defined(SPARC)
        {
            sal_Int64 aVal;
            *(sal_Int32 *)&aVal = *(sal_Int32 *)pVal;
            *((sal_Int32 *)&aVal +1)= *((sal_Int32 *)pVal +1);
            buf.append( aVal, 16 );
        }
#else
        buf.append( *(sal_Int64 *)pVal, 16 );
#endif
        break;
    default:
        buf.append( (sal_Unicode)'?' );
    }

    return buf.makeStringAndClear();
}
//--------------------------------------------------------------------------------------------------
static void dumpEntry( OUString const & key, Any const & value )
{
    OUString val( val2str( value.getValue(), value.getValueTypeRef() ) );
    OString key_str( OUStringToOString( key, RTL_TEXTENCODING_ASCII_US ) );
    OString val_str( OUStringToOString( val, RTL_TEXTENCODING_ASCII_US ) );
    ::fprintf( stderr, "| %s = %s\n", key_str.getStr(), val_str.getStr() );
}
#endif
//--------------------------------------------------------------------------------------------------
static inline void try_dispose( Reference< XInterface > const & xInstance )
    SAL_THROW( (RuntimeException) )
{
    Reference< lang::XComponent > xComp( xInstance, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}
//--------------------------------------------------------------------------------------------------
static inline void try_dispose( Reference< lang::XComponent > const & xComp )
    SAL_THROW( (RuntimeException) )
{
    if (xComp.is())
    {
        xComp->dispose();
    }
}

//==================================================================================================

class DisposingForwarder
    : public WeakImplHelper1< lang::XEventListener >
{
    Reference< lang::XComponent > m_xTarget;

    inline DisposingForwarder( Reference< lang::XComponent > const & xTarget )
        SAL_THROW(())
        : m_xTarget( xTarget )
        { OSL_ASSERT( m_xTarget.is() ); }
public:
    // listens at source for disposing, then disposes target
    static inline void listen(
        Reference< lang::XComponent > const & xSource,
        Reference< lang::XComponent > const & xTarget )
        SAL_THROW( (RuntimeException) );

    virtual void SAL_CALL disposing( lang::EventObject const & rSource )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline void DisposingForwarder::listen(
    Reference< lang::XComponent > const & xSource,
    Reference< lang::XComponent > const & xTarget )
    SAL_THROW( (RuntimeException) )
{
    if (xSource.is())
    {
        xSource->addEventListener( new DisposingForwarder( xTarget ) );
    }
}
//__________________________________________________________________________________________________
void DisposingForwarder::disposing( lang::EventObject const & )
    throw (RuntimeException)
{
    m_xTarget->dispose();
    m_xTarget.clear();
}

//==================================================================================================
struct MutexHolder
{
protected:
    Mutex m_mutex;
};
//==================================================================================================

class ComponentContext
    : private MutexHolder
    , public WeakComponentImplHelper2< XComponentContext,
                                       container::XNameContainer >
{
protected:
    Reference< XComponentContext > m_xDelegate;

    struct ContextEntry
    {
        Any value;
        bool lateInit;

        inline ContextEntry( Any const & value_, bool lateInit_ )
            : value( value_ )
            , lateInit( lateInit_ )
            {}
    };
    typedef ::boost::unordered_map< OUString, ContextEntry * , OUStringHash > t_map;
    t_map m_map;

    Reference< lang::XMultiComponentFactory > m_xSMgr;

protected:
    Any lookupMap( OUString const & rName )
        SAL_THROW( (RuntimeException) );

    virtual void SAL_CALL disposing();
public:
    ComponentContext(
        ContextEntry_Init const * pEntries, sal_Int32 nEntries,
        Reference< XComponentContext > const & xDelegate );
    virtual ~ComponentContext()
        SAL_THROW(());

    // XComponentContext
    virtual Any SAL_CALL getValueByName( OUString const & rName )
        throw (RuntimeException);
    virtual Reference<lang::XMultiComponentFactory> SAL_CALL getServiceManager()
        throw (RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName(
        OUString const & name, Any const & element )
        throw (lang::IllegalArgumentException, container::ElementExistException,
               lang::WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( OUString const & name )
        throw (container::NoSuchElementException,
               lang::WrappedTargetException, RuntimeException);
    // XNameReplace
    virtual void SAL_CALL replaceByName(
        OUString const & name, Any const & element )
        throw (lang::IllegalArgumentException,container::NoSuchElementException,
               lang::WrappedTargetException, RuntimeException);
    // XNameAccess
    virtual Any SAL_CALL getByName( OUString const & name )
        throw (container::NoSuchElementException,
               lang::WrappedTargetException, RuntimeException);
    virtual Sequence<OUString> SAL_CALL getElementNames()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( OUString const & name )
        throw (RuntimeException);
    // XElementAccess
    virtual Type SAL_CALL getElementType() throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException);
};

// XNameContainer
//______________________________________________________________________________
void ComponentContext::insertByName(
    OUString const & name, Any const & element )
    throw (lang::IllegalArgumentException, container::ElementExistException,
           lang::WrappedTargetException, RuntimeException)
{
    t_map::mapped_type entry(
        new ContextEntry(
            element,
            /* lateInit_: */
            name.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("/singletons/") ) &&
            !element.hasValue() ) );
    MutexGuard guard( m_mutex );
    ::std::pair<t_map::iterator, bool> insertion( m_map.insert(
        t_map::value_type( name, entry ) ) );
    if (! insertion.second)
        throw container::ElementExistException(
            OUSTR("element already exists: ") + name,
            static_cast<OWeakObject *>(this) );
}

//______________________________________________________________________________
void ComponentContext::removeByName( OUString const & name )
        throw (container::NoSuchElementException,
               lang::WrappedTargetException, RuntimeException)
{
    MutexGuard guard( m_mutex );
    t_map::iterator iFind( m_map.find( name ) );
    if (iFind == m_map.end())
        throw container::NoSuchElementException(
            OUSTR("no such element: ") + name,
            static_cast<OWeakObject *>(this) );

    delete iFind->second;
    m_map.erase(iFind);
}

// XNameReplace
//______________________________________________________________________________
void ComponentContext::replaceByName(
    OUString const & name, Any const & element )
    throw (lang::IllegalArgumentException,container::NoSuchElementException,
           lang::WrappedTargetException, RuntimeException)
{
    MutexGuard guard( m_mutex );
    t_map::const_iterator const iFind( m_map.find( name ) );
    if (iFind == m_map.end())
        throw container::NoSuchElementException(
            OUSTR("no such element: ") + name,
            static_cast<OWeakObject *>(this) );
    if (name.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("/singletons/") ) &&
        !element.hasValue())
    {
        iFind->second->value.clear();
        iFind->second->lateInit = true;
    }
    else
    {
        iFind->second->value = element;
        iFind->second->lateInit = false;
    }
}

// XNameAccess
//______________________________________________________________________________
Any ComponentContext::getByName( OUString const & name )
    throw (container::NoSuchElementException,
           lang::WrappedTargetException, RuntimeException)
{
    return getValueByName( name );
}

//______________________________________________________________________________
Sequence<OUString> ComponentContext::getElementNames()
    throw (RuntimeException)
{
    MutexGuard guard( m_mutex );
    Sequence<OUString> ret( m_map.size() );
    OUString * pret = ret.getArray();
    sal_Int32 pos = 0;
    t_map::const_iterator iPos( m_map.begin() );
    t_map::const_iterator const iEnd( m_map.end() );
    for ( ; iPos != iEnd; ++iPos )
        pret[pos++] = iPos->first;
    return ret;
}

//______________________________________________________________________________
sal_Bool ComponentContext::hasByName( OUString const & name )
    throw (RuntimeException)
{
    MutexGuard guard( m_mutex );
    return m_map.find( name ) != m_map.end();
}

// XElementAccess
//______________________________________________________________________________
Type ComponentContext::getElementType() throw (RuntimeException)
{
    return ::getVoidCppuType();
}

//______________________________________________________________________________
sal_Bool ComponentContext::hasElements() throw (RuntimeException)
{
    MutexGuard guard( m_mutex );
    return ! m_map.empty();
}

//__________________________________________________________________________________________________
Any ComponentContext::lookupMap( OUString const & rName )
    SAL_THROW( (RuntimeException) )
{
#ifdef CONTEXT_DIAG
    if ( rName == "dump_maps" )
    {
        ::fprintf( stderr, ">>> dumping out ComponentContext %p m_map:\n", this );
        typedef ::std::map< OUString, ContextEntry * > t_sorted; // sorted map
        t_sorted sorted;
        for ( t_map::const_iterator iPos( m_map.begin() ); iPos != m_map.end(); ++iPos )
        {
            sorted[ iPos->first ] = iPos->second;
        }
        {
        for ( t_sorted::const_iterator iPos( sorted.begin() ); iPos != sorted.end(); ++iPos )
        {
            dumpEntry( iPos->first, iPos->second->value );
        }
        }
        return Any();
    }
#endif

    ResettableMutexGuard guard( m_mutex );
    t_map::const_iterator iFind( m_map.find( rName ) );
    if (iFind == m_map.end())
        return Any();

    t_map::mapped_type pEntry = iFind->second;
    if (! pEntry->lateInit)
        return pEntry->value;

    // late init singleton entry
    Reference< XInterface > xInstance;
    guard.clear();

    try
    {
        Any usesService( getValueByName( rName + OUSTR("/service") ) );
        Any args_( getValueByName( rName + OUSTR("/arguments") ) );
        Sequence<Any> args;
        if (args_.hasValue() && !(args_ >>= args))
        {
            args.realloc( 1 );
            args[ 0 ] = args_;
        }

        Reference< lang::XSingleComponentFactory > xFac;
        if (usesService >>= xFac) // try via factory
        {
            xInstance = args.getLength()
                ? xFac->createInstanceWithArgumentsAndContext( args, this )
                : xFac->createInstanceWithContext( this );
        }
        else
        {
            Reference< lang::XSingleServiceFactory > xFac2;
            if (usesService >>= xFac2)
            {
                // try via old XSingleServiceFactory
#if OSL_DEBUG_LEVEL > 0
                ::fprintf(
                    stderr,
                    "### omitting context for service instanciation!\n" );
#endif
                xInstance = args.getLength()
                    ? xFac2->createInstanceWithArguments( args )
                    : xFac2->createInstance();
            }
            else if (m_xSMgr.is()) // optionally service name
            {
                OUString serviceName;
                if ((usesService >>= serviceName) &&
                    !serviceName.isEmpty())
                {
                    xInstance = args.getLength()
                        ? m_xSMgr->createInstanceWithArgumentsAndContext(
                            serviceName, args, this )
                        : m_xSMgr->createInstanceWithContext(
                            serviceName, this );
                }
            }
        }
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception & exc) // rethrow as WrappedTargetRuntimeException
    {
        Any caught( getCaughtException() );
        OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             "exception occurred raising singleton \"") );
        buf.append( rName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\": ") );
        buf.append( exc.Message );
        throw lang::WrappedTargetRuntimeException(
            buf.makeStringAndClear(), static_cast<OWeakObject *>(this),caught );
    }

    if (! xInstance.is())
    {
        throw RuntimeException(
            OUSTR("no service object raising singleton ") + rName,
            static_cast<OWeakObject *>(this) );
    }

    Any ret;
    guard.reset();
    iFind = m_map.find( rName );
    if (iFind != m_map.end())
    {
        pEntry = iFind->second;
        if (pEntry->lateInit)
        {
            pEntry->value <<= xInstance;
            pEntry->lateInit = false;
            return pEntry->value;
        }
        else
            ret = pEntry->value;
    }
    guard.clear();
    try_dispose( xInstance );
    return ret;
}

//__________________________________________________________________________________________________
Any ComponentContext::getValueByName( OUString const & rName )
    throw (RuntimeException)
{
    // to determine the root context:
    if ( rName == "_root" )
    {
        if (m_xDelegate.is())
            return m_xDelegate->getValueByName( rName );
        else
            return makeAny( Reference<XComponentContext>(this) );
    }

    Any ret( lookupMap( rName ) );
    if (!ret.hasValue() && m_xDelegate.is())
    {
        return m_xDelegate->getValueByName( rName );
    }
    return ret;
}
//__________________________________________________________________________________________________
Reference< lang::XMultiComponentFactory > ComponentContext::getServiceManager()
    throw (RuntimeException)
{
    if ( !m_xSMgr.is() )
    {
        throw RuntimeException(
            "null component context service manager",
            static_cast<OWeakObject *>(this) );
    }
    return m_xSMgr;
}
//__________________________________________________________________________________________________
ComponentContext::~ComponentContext()
    SAL_THROW(())
{
#ifdef CONTEXT_DIAG
    ::fprintf( stderr, "> destructed context %p\n", this );
#endif
    t_map::const_iterator iPos( m_map.begin() );
    t_map::const_iterator const iEnd( m_map.end() );
    for ( ; iPos != iEnd; ++iPos )
        delete iPos->second;
    m_map.clear();
}
//__________________________________________________________________________________________________
void ComponentContext::disposing()
{
#ifdef CONTEXT_DIAG
    ::fprintf( stderr, "> disposing context %p\n", this );
#endif

    Reference< lang::XComponent > xTDMgr, xAC, xPolicy; // to be disposed separately

    // dispose all context objects
    t_map::const_iterator iPos( m_map.begin() );
    t_map::const_iterator const iEnd( m_map.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        t_map::mapped_type pEntry = iPos->second;

        // service manager disposed separately
        if (!m_xSMgr.is() ||
            !iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SMGR_SINGLETON) ))
        {
            if (pEntry->lateInit)
            {
                // late init
                MutexGuard guard( m_mutex );
                if (pEntry->lateInit)
                {
                    pEntry->value.clear(); // release factory
                    pEntry->lateInit = false;
                    continue;
                }
            }

            Reference< lang::XComponent > xComp;
            pEntry->value >>= xComp;
            if (xComp.is())
            {
                if ( iPos->first == TDMGR_SINGLETON )
                {
                    xTDMgr = xComp;
                }
                else if ( iPos->first == AC_SINGLETON )
                {
                    xAC = xComp;
                }
                else if ( iPos->first == AC_POLICY )
                {
                    xPolicy = xComp;
                }
                else // dispose immediately
                {
                    xComp->dispose();
                }
            }
        }
    }

    // dispose service manager
    try_dispose( m_xSMgr );
    m_xSMgr.clear();
    // dispose ac
    try_dispose( xAC );
    // dispose policy
    try_dispose( xPolicy );
    // dispose tdmgr; revokes callback from cppu runtime
    try_dispose( xTDMgr );

    iPos = m_map.begin();
    for ( ; iPos != iEnd; ++iPos )
        delete iPos->second;
    m_map.clear();
}
//__________________________________________________________________________________________________
ComponentContext::ComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    : WeakComponentImplHelper2< XComponentContext, container::XNameContainer >(
        m_mutex ),
      m_xDelegate( xDelegate )
{
    for ( sal_Int32 nPos = 0; nPos < nEntries; ++nPos )
    {
        ContextEntry_Init const & rEntry = pEntries[ nPos ];

        if ( rEntry.name == SMGR_SINGLETON )
        {
            rEntry.value >>= m_xSMgr;
        }

        if (rEntry.bLateInitService)
        {
            // singleton entry
            m_map[ rEntry.name ] = new ContextEntry( Any(), true );
            // /service
            m_map[ rEntry.name + OUSTR("/service") ] = new ContextEntry( rEntry.value, false );
            // /initial-arguments are provided as optional context entry
        }
        else
        {
            // only value, no late init factory nor string
            m_map[ rEntry.name ] = new ContextEntry( rEntry.value, false );
        }
    }

    if (!m_xSMgr.is() && m_xDelegate.is())
    {
        // wrap delegate's smgr XPropertySet into new smgr
        Reference< lang::XMultiComponentFactory > xMgr( m_xDelegate->getServiceManager() );
        if (xMgr.is())
        {
            osl_atomic_increment( &m_refCount );
            try
            {
                // create new smgr based on delegate's one
                m_xSMgr.set(
                    xMgr->createInstanceWithContext(
                        OUSTR("com.sun.star.comp.stoc.OServiceManagerWrapper"), xDelegate ),
                    UNO_QUERY );
                // patch DefaultContext property of new one
                Reference< beans::XPropertySet > xProps( m_xSMgr, UNO_QUERY );
                OSL_ASSERT( xProps.is() );
                if (xProps.is())
                {
                    Reference< XComponentContext > xThis( this );
                    xProps->setPropertyValue( OUSTR("DefaultContext"), makeAny( xThis ) );
                }
            }
            catch (...)
            {
                osl_atomic_decrement( &m_refCount );
                throw;
            }
            osl_atomic_decrement( &m_refCount );
            OSL_ASSERT( m_xSMgr.is() );
        }
    }
}


//##################################################################################################
extern "C" { static void s_createComponentContext_v(va_list * pParam)
{
    ContextEntry_Init const  * pEntries     = va_arg(*pParam, ContextEntry_Init const *);
    sal_Int32                  nEntries     = va_arg(*pParam, sal_Int32);
    XComponentContext        * pDelegatee   = va_arg(*pParam, XComponentContext *);
    void                    ** ppContext    = va_arg(*pParam, void **);
    uno::Mapping             * pTarget2curr = va_arg(*pParam, uno::Mapping *);

    Reference<XComponentContext> xDelegate(pDelegatee, SAL_NO_ACQUIRE);
    Reference<XComponentContext> xContext;

    if (nEntries > 0)
    {
        try
        {
            ComponentContext * p = new ComponentContext( pEntries, nEntries, xDelegate );
            xContext.set(p);
            // listen delegate for disposing, to dispose this (wrapping) context first.
            DisposingForwarder::listen( Reference< lang::XComponent >::query( xDelegate ), p );
        }
        catch (Exception & exc)
        {
            (void) exc; // avoid warning about unused variable
            OSL_FAIL( OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            xContext.clear();
        }
    }
    else
    {
        xContext = xDelegate;
    }

    *ppContext = pTarget2curr->mapInterface(xContext.get(), ::getCppuType(&xContext));
}}

Reference< XComponentContext > SAL_CALL createComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    SAL_THROW(())
{
    uno::Environment curr_env(Environment::getCurrent());
    uno::Environment source_env(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))));

    uno::Mapping curr2source(curr_env, source_env);
    uno::Mapping source2curr(source_env, curr_env);

    ContextEntry_Init * mapped_entries = new ContextEntry_Init[nEntries];
    for (sal_Int32 nPos = 0; nPos < nEntries; ++ nPos)
    {
        mapped_entries[nPos].bLateInitService = pEntries[nPos].bLateInitService;
        mapped_entries[nPos].name             = pEntries[nPos].name;

        uno_type_any_constructAndConvert(&mapped_entries[nPos].value,
                                         const_cast<void *>(pEntries[nPos].value.getValue()),
                                         pEntries[nPos].value.getValueTypeRef(),
                                         curr2source.get());
    }

    void * mapped_delegate = curr2source.mapInterface(xDelegate.get(), ::getCppuType(&xDelegate));
    XComponentContext * pXComponentContext = NULL;
    source_env.invoke(s_createComponentContext_v, mapped_entries, nEntries, mapped_delegate, &pXComponentContext, &source2curr);
    delete[] mapped_entries;

    return Reference<XComponentContext>(pXComponentContext, SAL_NO_ACQUIRE);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
