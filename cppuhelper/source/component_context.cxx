 /*************************************************************************
 *
 *  $RCSfile: component_context.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:54:25 $
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

#ifdef DIAG
#define CONTEXT_DIAG
#endif

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

#include <vector>
#ifdef CONTEXT_DIAG
#include <map>
#endif

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <hash_map>

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
template< class T >
static inline beans::PropertyValue createPropertyValue(
    OUString const & name, T const & value )
    SAL_THROW( () )
{
    return beans::PropertyValue( name, -1, makeAny( value ), beans::PropertyState_DIRECT_VALUE );
}
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

#if defined( MACOSX ) && ( __GNUC__ < 3 )
// Initialize static template data here to break circular reference to libstatic
ClassData1 WeakImplHelper1< lang::XEventListener >::s_aCD(1);
#endif

class DisposingForwarder
    : public WeakImplHelper1< lang::XEventListener >
{
    Reference< lang::XComponent > m_xTarget;

    inline DisposingForwarder( Reference< lang::XComponent > const & xTarget )
        SAL_THROW( () )
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
void DisposingForwarder::disposing( lang::EventObject const & rSource )
    throw (RuntimeException)
{
    m_xTarget->dispose();
    m_xTarget.clear();
}

//==================================================================================================
struct MutexHolder
{
    Mutex m_mutex;
};
//==================================================================================================

#if defined( MACOSX ) && ( __GNUC__ < 3 )
// Initialize static template data here to break circular reference to libstatic
ClassData1 WeakComponentImplHelper1< XComponentContext >::s_aCD(1);
#endif

class ComponentContext
    : public MutexHolder
    , public WeakComponentImplHelper1< XComponentContext >
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
    typedef ::std::hash_map< OUString, ContextEntry *, OUStringHash > t_map;
    t_map m_map;

    Reference< lang::XMultiComponentFactory > m_xSMgr;

protected:
    void throw_RT(
        OUString const & str1, OUString const & str2 = OUString() )
        SAL_THROW( (RuntimeException ) );
    void throw_RT(
        OUString const & str1, OUString const & str2,
        OUString const & str3, OUString const & str4 = OUString() )
        SAL_THROW( (RuntimeException ) );

    Any lookupMap( OUString const & rName )
        SAL_THROW( (RuntimeException) );
    virtual Sequence< Any > readInitialArguments( const OUString & rName )
        SAL_THROW( (RuntimeException) );

    virtual void SAL_CALL disposing();
public:
    ComponentContext(
        ContextEntry_Init const * pEntries, sal_Int32 nEntries,
        Reference< XComponentContext > const & xDelegate );
    virtual ~ComponentContext()
        SAL_THROW( () );

    // XComponentContext
    virtual Any SAL_CALL getValueByName( OUString const & rName )
        throw (RuntimeException);
    virtual Reference< lang::XMultiComponentFactory > SAL_CALL getServiceManager()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
void ComponentContext::throw_RT(
    OUString const & str1, OUString const & str2 )
    SAL_THROW( (RuntimeException ) )
{
    OUStringBuffer buf( 64 );
    buf.append( str1 );
    buf.append( str2 );
    OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
    OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
    ::fprintf( stderr, "### %s\n", str.getStr() );
#endif
    throw RuntimeException( msg, (OWeakObject *)this );
}
//__________________________________________________________________________________________________
void ComponentContext::throw_RT(
    OUString const & str1, OUString const & str2,
    OUString const & str3, OUString const & str4 )
    SAL_THROW( (RuntimeException ) )
{
    OUStringBuffer buf( 64 );
    buf.append( str1 );
    buf.append( str2 );
    buf.append( str3 );
    buf.append( str4 );
    OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
    OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
    ::fprintf( stderr, "### %s\n", str.getStr() );
#endif
    throw RuntimeException( msg, (OWeakObject *)this );
}
//__________________________________________________________________________________________________
Sequence< Any > ComponentContext::readInitialArguments(
    const OUString & rName )
    SAL_THROW( (RuntimeException) )
{
    Any args( ComponentContext::getValueByName( rName + OUSTR("/arguments") ) );
    if (::getCppuType( (Sequence< Any > const *)0 ) == args.getValueType())
    {
        return Sequence< Any >( *(Sequence< Any > const *)args.getValue() );
    }
    else
    {
        return Sequence< Any >();
    }
}
//__________________________________________________________________________________________________
Any ComponentContext::lookupMap( OUString const & rName )
    SAL_THROW( (RuntimeException) )
{
#ifdef CONTEXT_DIAG
    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dump_maps") ))
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

    /** map is anytime untouched, if an uninit value will be inited, synch is done on mutex.
    */
    t_map::const_iterator const iFind( m_map.find( rName ) );
    if (iFind != m_map.end())
    {
        ContextEntry * pEntry = iFind->second;
        if (pEntry->lateInit)
        {
            // late init singleton entry
            Reference< XInterface > xInstance;

            try
            {
                Any usesService( getValueByName( rName + OUSTR("/service") ) );
                Sequence< Any > args( readInitialArguments( rName ) );

                Reference< lang::XSingleComponentFactory > xFac;
                if (usesService >>= xFac) // try via factory
                {
                    xInstance = args.getLength()
                        ? xFac->createInstanceWithArgumentsAndContext( args, this )
                        : xFac->createInstanceWithContext( this );
                }
                else
                {
                    Reference< lang::XSingleServiceFactory > xFac;
                    if (usesService >>= xFac) // try via old XSingleServiceFactory
                    {
#if OSL_DEBUG_LEVEL > 0
                        ::fprintf( stderr, "### omitting context for service instanciation!\n" );
#endif
                        xInstance = args.getLength()
                            ? xFac->createInstanceWithArguments( args )
                            : xFac->createInstance();
                    }
                    else if (m_xSMgr.is()) // optionally service name
                    {
                        OUString serviceName;
                        if ((usesService >>= serviceName) && serviceName.getLength())
                        {
                            xInstance = args.getLength()
                                ? m_xSMgr->createInstanceWithArgumentsAndContext( serviceName, args, this )
                                : m_xSMgr->createInstanceWithContext( serviceName, this );
                        }
                    }
                }
            }
            catch (RuntimeException &)
            {
                throw;
            }
            catch (Exception & exc) // rethrow as RuntimeException
            {
                throw_RT(
                    OUSTR("exception occured raising singleton \""), rName,
                    OUSTR("\": "), exc.Message );
            }

            if (! xInstance.is())
            {
                throw_RT(
                    OUSTR("no service object raising singleton "), rName );
            }

            ClearableMutexGuard guard( m_mutex );
            if (pEntry->lateInit)
            {
                pEntry->value.setValue( &xInstance, ::getCppuType( &xInstance ) );
                pEntry->lateInit = false;
            }
            else // inited in the meantime
            {
                guard.clear();
                // service has entered the context in the meantime
                // => try to dispose this object
                try_dispose( xInstance );
            }
        }

        return pEntry->value;
    }
    else
    {
        return Any();
    }
}
//__________________________________________________________________________________________________
Any ComponentContext::getValueByName( OUString const & rName )
    throw (RuntimeException)
{
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
    return m_xSMgr;
}
//__________________________________________________________________________________________________
ComponentContext::~ComponentContext()
    SAL_THROW( () )
{
#ifdef CONTEXT_DIAG
    ::fprintf( stderr, "> destructed context %p\n", this );
#endif
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
    t_map::const_iterator iEnd( m_map.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        ContextEntry * pEntry = iPos->second;

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
                if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(TDMGR_SINGLETON) ))
                {
                    xTDMgr = xComp;
                }
                else if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(AC_SINGLETON) ))
                {
                    xAC = xComp;
                }
                else if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(AC_POLICY) ))
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

    // everything is disposed, hopefully nobody accesses the context anymore...
    iPos = m_map.begin();
    while (iPos != iEnd)
    {
        delete iPos->second;
        ++iPos;
    }
    m_map.clear();
}
//__________________________________________________________________________________________________
ComponentContext::ComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    : WeakComponentImplHelper1< XComponentContext >( m_mutex )
    , m_xDelegate( xDelegate )
{
    for ( sal_Int32 nPos = 0; nPos < nEntries; ++nPos )
    {
        ContextEntry_Init const & rEntry = pEntries[ nPos ];

        if (rEntry.name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SMGR_SINGLETON) ))
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
            osl_incrementInterlockedCount( &m_refCount );
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
                osl_decrementInterlockedCount( &m_refCount );
                throw;
            }
            osl_decrementInterlockedCount( &m_refCount );
            OSL_ASSERT( m_xSMgr.is() );
        }
    }
}

//==================================================================================================
class ConfigurationComponentContext : public ComponentContext
{
protected:
    Reference< lang::XMultiServiceFactory > m_xCfgProvider;

    typedef ::std::hash_map< OUString, Reference< XInterface >, OUStringHash > t_singletons;
    t_singletons m_singletons;

protected:
    Reference< container::XNameAccess > getCfgNode( OUString const & rName )
        SAL_THROW( (RuntimeException) );
    Reference< XInterface > createSingletonFromCfg( OUString const & rName )
        SAL_THROW( (RuntimeException) );

    Sequence< Any > readInitialArguments( const OUString & rName )
        SAL_THROW( (RuntimeException) );

    virtual void SAL_CALL disposing();

public:
    inline ConfigurationComponentContext(
        ContextEntry_Init const * pEntries, sal_Int32 nEntries,
        Reference< XComponentContext > const & xDelegate )
        : ComponentContext( pEntries, nEntries, xDelegate )
        {}

    // XComponentContext
    Any SAL_CALL getValueByName( OUString const & rName )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
Reference< container::XNameAccess > ConfigurationComponentContext::getCfgNode(
    OUString const & rName )
    SAL_THROW( (RuntimeException) )
{
    if (! m_xCfgProvider.is())
    {
        Reference< lang::XMultiServiceFactory > xCfgProvider;

        // don't run into recursion trouble
        lookupMap( OUSTR("/singletons/com.sun.star.bootstrap.theConfigurationProvider") ) >>= xCfgProvider;
        if (!xCfgProvider.is() && m_xDelegate.is())
        {
            m_xDelegate->getValueByName(
                OUSTR("/singletons/com.sun.star.bootstrap.theConfigurationProvider") ) >>= xCfgProvider;
        }

        if (xCfgProvider.is())
        {
            ClearableMutexGuard guard( m_mutex );
            if (! m_xCfgProvider.is())
            {
                m_xCfgProvider = xCfgProvider;
            }
            else
            {
                guard.clear();
                try_dispose( xCfgProvider );
            }
        }
        else
        {
            throw RuntimeException(
                OUSTR("\"/singletons/com.sun.star.bootstrap.theConfigurationProvider\" not available!"),
                (OWeakObject *)this );
        }
    }

    try
    {
        Sequence< Any > args( 1 );
        args[ 0 ] <<= createPropertyValue( OUSTR("nodepath"), OUSTR("/uno.components") + rName );
        Reference< container::XNameAccess > xNA( m_xCfgProvider->createInstanceWithArguments(
            OUSTR("com.sun.star.configuration.ConfigurationAccess"), args ), UNO_QUERY );
        return xNA;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception & exc)
    {
#ifdef CONTEXT_DIAG
        OString str( OUStringToOString( rName, RTL_TEXTENCODING_ASCII_US ) );
        OString str2( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, "### accessing node %s from cfg failed: %s\n", str.getStr(), str2.getStr() );
#endif
        return Reference< container::XNameAccess >();
    }
}
//__________________________________________________________________________________________________
Sequence< Any > ConfigurationComponentContext::readInitialArguments(
    const OUString & rName )
    SAL_THROW( (RuntimeException) )
{
    Sequence< Any > args( ComponentContext::readInitialArguments( rName ) );
    if (!args.getLength() && m_xCfgProvider.is()) // no recursion trouble
    {
        Reference< container::XNameAccess > xNA(
            getCfgNode( rName + OUSTR("/arguments") ) );
        if (xNA.is())
        {
            ::std::vector< Any > ar;
            ar.reserve( 3 );

            sal_Int32 nNum = 0;
            for (;;)
            {
                try
                {
                    Any arg( xNA->getByName( OUString::valueOf( nNum++ ) ) );
                    ar.push_back( arg );
                }
                catch (RuntimeException &)
                {
                    throw;
                }
                catch (Exception &)
                {
                    break;
                }
            }

            return Sequence< Any >( &ar[ 0 ], ar.size() );
        }
    }
    return args;
}
//__________________________________________________________________________________________________
Reference< XInterface > ConfigurationComponentContext::createSingletonFromCfg(
    OUString const & rName )
    SAL_THROW( (RuntimeException) )
{
    OUString serviceName;

    Reference< container::XNameAccess > xNA( getCfgNode( rName ) );
    if (! xNA.is())
    {
        return Reference< XInterface >();
    }

    if (! (xNA->getByName( OUSTR("service") ) >>= serviceName))
    {
        throw_RT(
            OUSTR("missing \"service\" entry for singleton "), rName );
    }
    if (! m_xSMgr.is())
    {
        throw_RT(
            OUSTR("no service manager instance available creating singleton "), rName );
    }

    Sequence< Any > args( readInitialArguments( rName ) );

    Reference< XInterface > xInstance;
    try
    {
        xInstance = args.getLength()
            ? m_xSMgr->createInstanceWithArgumentsAndContext( serviceName, args, this )
            : m_xSMgr->createInstanceWithContext( serviceName, this );
        if (xInstance.is())
        {
            return xInstance;
        }
        throw_RT( OUSTR("no service object raising singleton \""), rName );
        // is here for dummy
        return Reference< XInterface >();
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception & exc)
    {
        throw_RT(
            OUSTR("exception occured raising singleton \""), rName,
            OUSTR("\": "), exc.Message );
    }
}
//__________________________________________________________________________________________________
Any ConfigurationComponentContext::getValueByName( OUString const & rName )
    throw (RuntimeException)
{
    Any ret( lookupMap( rName ) );
    if (ret.hasValue())
    {
        return ret;
    }

#ifdef CONTEXT_DIAG
    if (rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dump_maps") ))
    {
        ::fprintf( stderr, ">>> dumping out ConfigurationComponentContext %p m_singletons:\n", this );
        typedef ::std::map< OUString, Any > t_sorted; // sorted map
        t_sorted sorted;
        {
        MutexGuard guard( m_mutex );
        for ( t_singletons::const_iterator iPos( m_singletons.begin() ); iPos != m_singletons.end(); ++iPos )
        {
            sorted[ iPos->first ] = makeAny( iPos->second );
        }
        }
        {
        for ( t_sorted::const_iterator iPos( sorted.begin() ); iPos != sorted.end(); ++iPos )
        {
            dumpEntry( iPos->first, iPos->second );
        }
        }
        if (m_xDelegate.is())
        {
            // dump more contexts
            return m_xDelegate->getValueByName( rName );
        }
        return Any();
    }
#endif

    // analyze name
    if (rName.getLength() > sizeof("/singletons/") &&
        0 == rName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("/singletons/") )) // singleton lookup
    {
        {
        // try singletons map
        MutexGuard guard( m_mutex );
        t_singletons::const_iterator const iFind( m_singletons.find( rName ) );
        if (iFind != m_singletons.end()) // entry in map
        {
            return makeAny( iFind->second );
        }
        }

        Reference< XInterface > xInstance( createSingletonFromCfg( rName ) );

        if (xInstance.is())
        {
            // inserted in the meantime?
            ClearableMutexGuard guard( m_mutex );
            t_singletons::const_iterator const iFind( m_singletons.find( rName ) );
            if (iFind == m_singletons.end())
            {
                ::std::pair< t_singletons::iterator, bool > insertion(
                    m_singletons.insert( t_singletons::value_type( rName, xInstance ) ) );
                OSL_ENSURE( insertion.second, "### inserting new singleton failed?!" );
                return makeAny( xInstance );
            }
            else // inited in the meantime
            {
                guard.clear();
                // => try to dispose created object
                try_dispose( xInstance );
                return makeAny( iFind->second );
            }
        }
    }
    else // try regular config lookup
    {
        sal_Int32 last = rName.lastIndexOf( '/' );
        if (last >= 0)
        {
            Reference< container::XNameAccess > xNA( getCfgNode( rName.copy( 0, last ) ) );
            if (xNA.is())
            {
                try
                {
                    return xNA->getByName( rName.copy( last +1 ) );
                }
                catch (RuntimeException &)
                {
                    throw;
                }
                catch (Exception & exc)
                {
#ifdef CONTEXT_DIAG
                    OString str( OUStringToOString( rName, RTL_TEXTENCODING_ASCII_US ) );
                    OString str2( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
                    ::fprintf( stderr, "### accessing node %s from cfg failed: %s\n", str.getStr(), str2.getStr() );
#endif
                }
            }
        }
    }

    if (m_xDelegate.is())
    {
        return m_xDelegate->getValueByName( rName );
    }

    return Any();
}
//__________________________________________________________________________________________________
void ConfigurationComponentContext::disposing()
{
#ifdef CONTEXT_DIAG
    ::fprintf( stderr, "> disposing cfg context %p\n", this );
#endif

    Reference< XInterface > xSMgr, xTDMgr, xAC, xPolicy;

    // first dispose all context objects
    t_singletons::const_iterator iPos( m_singletons.begin() );
    t_singletons::const_iterator iEnd( m_singletons.end() );
    while (iPos != iEnd)
    {
        // to be disposed separately
        if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SMGR_SINGLETON) ))
        {
            xSMgr = iPos->second;
        }
        else if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(TDMGR_SINGLETON) ))
        {
            xTDMgr = iPos->second;
        }
        else if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(AC_SINGLETON) ))
        {
            xAC = iPos->second;
        }
        else if (iPos->first.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(AC_POLICY) ))
        {
            xPolicy = iPos->second;
        }
        else // dispose immediately
        {
            try_dispose( iPos->second );
        }
        ++iPos;
    }
    m_singletons.clear();

    // dispose service manager
    try_dispose( xSMgr );
    // dispose ac
    try_dispose( xAC );
    // dispose policy
    try_dispose( xPolicy );
    // dispose tdmgr; revokes callback from cppu runtime
    try_dispose( xTDMgr );

    // dispose context values map
    ComponentContext::disposing();
}

//==================================================================================================
Reference< XComponentContext > SAL_CALL createInitialCfgComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    SAL_THROW( () )
{
    try
    {
        ConfigurationComponentContext * p =
            new ConfigurationComponentContext( pEntries, nEntries, xDelegate );
        Reference< XComponentContext > xContext( p );
        // listen delegate for disposing, to dispose this (wrapping) context first.
        DisposingForwarder::listen( Reference< lang::XComponent >::query( xDelegate ), p );
        return xContext;
    }
    catch (Exception & exc)
    {
        OSL_ENSURE( 0, OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        return Reference< XComponentContext >();
    }
}

//##################################################################################################
Reference< XComponentContext > SAL_CALL createComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    SAL_THROW( () )
{
    if (nEntries > 0)
    {
        try
        {
            ComponentContext * p = new ComponentContext( pEntries, nEntries, xDelegate );
            Reference< XComponentContext > xContext( p );
            // listen delegate for disposing, to dispose this (wrapping) context first.
            DisposingForwarder::listen( Reference< lang::XComponent >::query( xDelegate ), p );
            return xContext;
        }
        catch (Exception & exc)
        {
            OSL_ENSURE( 0, OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            return Reference< XComponentContext >();
        }
    }
    else
    {
        return xDelegate;
    }
}

}
