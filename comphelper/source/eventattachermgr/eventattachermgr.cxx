/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: eventattachermgr.cxx,v $
 * $Revision: 1.10 $
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

#include <deque>

#if defined( OS2 ) || defined( UNX )
#include <wchar.h>
#endif
#include <osl/mutex.hxx>
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#include <vos/macros.hxx>
#include <comphelper/eventattachermgr.hxx>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XEngineListener.hpp>
#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace cppu;
using namespace osl;
using namespace rtl;

namespace comphelper
{

//-----------------------------------------------------------------------------
struct AttachedObject_Impl
{
    Reference< XInterface >                 xTarget;
    Sequence< Reference< XEventListener > > aAttachedListenerSeq;
    Any                                     aHelper;

    bool    operator<( const AttachedObject_Impl & ) const;
    bool    operator==( const AttachedObject_Impl & ) const;
};

struct AttacherIndex_Impl
{
#ifdef DEQUE_OK
    ::std::deque< ScriptEventDescriptor > aEventList;
#else
    Sequence< ScriptEventDescriptor >   aEventList;
#endif
    ::std::deque< AttachedObject_Impl > aObjList;

    bool    operator<( const AttacherIndex_Impl & ) const;
    bool    operator==( const AttacherIndex_Impl & ) const;
};

#if 0
bool AttachedObject_Impl::operator<( const AttachedObject_Impl & r ) const
{
    VOS_ENSHURE( FALSE, "not implemented" );
    return FALSE;
    return this < &r;
}

bool AttachedObject_Impl::operator==( const AttachedObject_Impl & r ) const
{
    VOS_ENSHURE( FALSE, "not implemented" );
    return this == &r;
}

bool AttacherIndex_Impl::operator<( const AttacherIndex_Impl & r ) const
{
    VOS_ENSHURE( FALSE, "not implemented" );
    return this < &r;
}
bool AttacherIndex_Impl::operator==( const AttacherIndex_Impl & r ) const
{
    VOS_ENSHURE( FALSE, "not implemented" );
    return this == &r;
}
#endif

//-----------------------------------------------------------------------------
class ImplEventAttacherManager
    : public WeakImplHelper2< XEventAttacherManager, XPersistObject >
{
    friend class AttacherAllListener_Impl;
    ::std::deque< AttacherIndex_Impl >  aIndex;
    Mutex aLock;
    // Container fuer die ScriptListener
    OInterfaceContainerHelper           aScriptListeners;
    // EventAttacher-Instanz
    Reference< XEventAttacher >         xAttacher;
    Reference< XMultiServiceFactory >   mxSMgr;
    Reference< XIdlReflection >         mxCoreReflection;
    Reference< XIntrospection >         mxIntrospection;
    Reference< XTypeConverter >         xConverter;
    sal_Int16                           nVersion;
public:
    ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                              const Reference< XMultiServiceFactory > rSMgr );
    ~ImplEventAttacherManager();

    // Methoden von XEventAttacherManager
    virtual void SAL_CALL registerScriptEvent(sal_Int32 Index, const ScriptEventDescriptor& ScriptEvent)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL registerScriptEvents(sal_Int32 Index, const Sequence< ScriptEventDescriptor >& ScriptEvents)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL revokeScriptEvent(sal_Int32 Index, const OUString& ListenerType, const OUString& EventMethod, const OUString& removeListenerParam)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL revokeScriptEvents(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL insertEntry(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL removeEntry(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException );
    virtual Sequence< ScriptEventDescriptor > SAL_CALL getScriptEvents(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL attach(sal_Int32 Index, const Reference< XInterface >& Object, const Any& Helper)
        throw( IllegalArgumentException, ServiceNotRegisteredException, RuntimeException );
    virtual void SAL_CALL detach(sal_Int32 nIndex, const Reference< XInterface >& xObject)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL addScriptListener(const Reference< XScriptListener >& aListener)
        throw( IllegalArgumentException, RuntimeException );
    virtual void SAL_CALL removeScriptListener(const Reference< XScriptListener >& Listener)
        throw( IllegalArgumentException, RuntimeException );

    // Methoden von XPersistObject
    virtual OUString SAL_CALL getServiceName(void) throw( RuntimeException );
    virtual void SAL_CALL write(const Reference< XObjectOutputStream >& OutStream) throw( IOException, RuntimeException );
    virtual void SAL_CALL read(const Reference< XObjectInputStream >& InStream) throw( IOException, RuntimeException );

private:
    Reference< XIdlReflection > getReflection() throw( Exception );

    /** checks if <arg>_nIndex</arg> is a valid index, throws an <type>IllegalArgumentException</type> if not
    @param _nIndex
        the index to check
    @return
        the iterator pointing to the position indicated by the index
    */
    ::std::deque<AttacherIndex_Impl>::iterator implCheckIndex( sal_Int32 _nIndex ) SAL_THROW ( ( IllegalArgumentException ) );
};

//========================================================================
//========================================================================
//========================================================================

// Implementation eines EventAttacher-bezogenen AllListeners, der
// nur einzelne Events an einen allgemeinen AllListener weiterleitet
class AttacherAllListener_Impl : public WeakImplHelper1< XAllListener >
{
    ImplEventAttacherManager*           mpManager;
    Reference< XEventAttacherManager >  xManager;
    OUString                            aScriptType;
    OUString                            aScriptCode;
    sal_Int16                           nVersion;

    void convertToEventReturn( Any & rRet, const Type & rRetType )
        throw( CannotConvertException );
public:
    AttacherAllListener_Impl( ImplEventAttacherManager* pManager_, const OUString &rScriptType_,
                                const OUString & rScriptCode_ );

    // Methoden von XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException );

    // Methoden von XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException );
};

//========================================================================
AttacherAllListener_Impl::AttacherAllListener_Impl
(
    ImplEventAttacherManager*   pManager_,
    const OUString &             rScriptType_,
    const OUString &             rScriptCode_
)
    : mpManager( pManager_ )
    , xManager( pManager_ )
    , aScriptType( rScriptType_ )
    , aScriptCode( rScriptCode_ )
    , nVersion( 2 )
{
}


//========================================================================
// Methoden von XAllListener
void SAL_CALL AttacherAllListener_Impl::firing(const AllEventObject& Event)
    throw( RuntimeException )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = (OWeakObject *)mpManager; // get correct XInterface
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    // ueber alle Listener iterieren und Events senden
    OInterfaceIteratorHelper aIt( mpManager->aScriptListeners );
    while( aIt.hasMoreElements() )
        ((XScriptListener *)aIt.next())->firing( aScriptEvent );
}

//========================================================================
// Convert to the standard event return
void AttacherAllListener_Impl::convertToEventReturn( Any & rRet, const Type & rRetType )
    throw( CannotConvertException )
{
    // no return value? Set to the specified values
    if( rRet.getValueType().getTypeClass() == TypeClass_VOID )
    {
        switch( rRetType.getTypeClass()  )
        {
            case TypeClass_INTERFACE:
                {
                rRet <<= Reference< XInterface >();
                }
                break;

            case TypeClass_BOOLEAN:
                rRet <<= sal_True;
                break;

            case TypeClass_STRING:
                rRet <<= OUString();
                break;

            case TypeClass_FLOAT:           rRet <<= float(0);  break;
            case TypeClass_DOUBLE:          rRet <<= double(0.0);  break;
            case TypeClass_BYTE:            rRet <<= sal_uInt8(0);      break;
            case TypeClass_SHORT:           rRet <<= sal_Int16( 0 );    break;
            case TypeClass_LONG:            rRet <<= sal_Int32( 0 );    break;
            case TypeClass_UNSIGNED_SHORT:  rRet <<= sal_uInt16( 0 );   break;
            case TypeClass_UNSIGNED_LONG:   rRet <<= sal_uInt32( 0 );   break;

            default:
                OSL_ASSERT(false);
                break;
        }
    }
    else if( !rRet.getValueType().equals( rRetType ) )
    {
        if( mpManager->xConverter.is() )
            rRet = mpManager->xConverter->convertTo( rRet, rRetType );
        else
            throw CannotConvertException();
    }
}

//========================================================================
// Methoden von XAllListener
Any SAL_CALL AttacherAllListener_Impl::approveFiring( const AllEventObject& Event )
    throw( InvocationTargetException, RuntimeException )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = (OWeakObject *)mpManager; // get correct XInterface
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    Any aRet;
    // ueber alle Listener iterieren und Events senden
    OInterfaceIteratorHelper aIt( mpManager->aScriptListeners );
    while( aIt.hasMoreElements() )
    {
        aRet = ((XScriptListener *)aIt.next())->approveFiring( aScriptEvent );
        try
        {
            Reference< XIdlClass > xListenerType = mpManager->getReflection()->
                        forName( Event.ListenerType.getTypeName() );
            Reference< XIdlMethod > xMeth = xListenerType->getMethod( Event.MethodName );
            if( xMeth.is() )
            {
                Reference< XIdlClass > xRetType = xMeth->getReturnType();
                Type aRetType(xRetType->getTypeClass(), xRetType->getName());
                convertToEventReturn( aRet, aRetType );
            }

            switch( aRet.getValueType().getTypeClass()  )
            {
                case TypeClass_INTERFACE:
                    {
                    // Interface not null, return
                    Reference< XInterface > x;
                    aRet >>= x;
                    if( x.is() )
                        return aRet;
                    }
                    break;

                case TypeClass_BOOLEAN:
                    // FALSE -> Return
                    if( !(*(sal_Bool*)aRet.getValue()) )
                        return aRet;
                    break;

                case TypeClass_STRING:
                    // none empty string -> return
                    if( ((OUString*)aRet.getValue())->getLength() > 0 )
                        return aRet;
                    break;

                    // none zero number -> return
                case TypeClass_FLOAT:           if( *((float*)aRet.getValue()) )    return aRet; break;
                case TypeClass_DOUBLE:          if( *((double*)aRet.getValue()) )   return aRet; break;
                case TypeClass_BYTE:            if( *((sal_uInt8*)aRet.getValue()) )    return aRet; break;
                case TypeClass_SHORT:           if( *((sal_Int16*)aRet.getValue()) )    return aRet; break;
                case TypeClass_LONG:            if( *((sal_Int32*)aRet.getValue()) )    return aRet; break;
                case TypeClass_UNSIGNED_SHORT:  if( *((sal_uInt16*)aRet.getValue()) )   return aRet; break;
                case TypeClass_UNSIGNED_LONG:   if( *((sal_uInt32*)aRet.getValue()) )   return aRet; break;

                default:
                    OSL_ASSERT(false);
                    break;
            }
        }
        catch( CannotConvertException& )
        {
            // silent ignore conversions errors from a script call
            Reference< XIdlClass > xListenerType = mpManager->getReflection()->
                        forName( Event.ListenerType.getTypeName() );
            Reference< XIdlMethod > xMeth = xListenerType->getMethod( Event.MethodName );
            if( xMeth.is() )
            {
                Reference< XIdlClass > xRetType = xMeth->getReturnType();
                Type aRetType(xRetType->getTypeClass(), xRetType->getName());
                aRet.clear();
                convertToEventReturn( aRet, aRetType );
            }
        }
    }
    return aRet;
}

//========================================================================
// Methoden von XEventListener
void SAL_CALL AttacherAllListener_Impl::disposing(const EventObject& )
    throw( RuntimeException )
{
    // It is up to the container to release the object
}


//========================================================================
//========================================================================
//========================================================================

// Create-Methode fuer EventAttacherManager
Reference< XEventAttacherManager > createEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                                                               const Reference< XMultiServiceFactory > & rSMgr )
    throw( Exception )
{
    return new ImplEventAttacherManager( rIntrospection, rSMgr );
}

// Create-Methode fuer EventAttacherManager
Reference< XEventAttacherManager > createEventAttacherManager( const Reference< XMultiServiceFactory > & rSMgr )
    throw( Exception )
{
    if ( rSMgr.is() )
    {
        Reference< XInterface > xIFace( rSMgr->createInstance( OUString::createFromAscii("com.sun.star.beans.Introspection") ) );
        if ( xIFace.is() )
        {
            Reference< XIntrospection > xIntrospection( xIFace, UNO_QUERY);
            return new ImplEventAttacherManager( xIntrospection, rSMgr );
        }
    }

    return Reference< XEventAttacherManager >();
}

//-----------------------------------------------------------------------------
ImplEventAttacherManager::ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                                                    const Reference< XMultiServiceFactory > rSMgr )
    : aScriptListeners( aLock )
    , mxSMgr( rSMgr )
    , mxIntrospection( rIntrospection )
{
    if ( rSMgr.is() )
    {
        Reference< XInterface > xIFace( rSMgr->createInstance( OUString::createFromAscii("com.sun.star.script.EventAttacher") ) );
        if ( xIFace.is() )
        {
            xAttacher = Reference< XEventAttacher >::query( xIFace );
        }
        xIFace = rSMgr->createInstance( OUString::createFromAscii("com.sun.star.script.Converter") );
        if ( xIFace.is() )
        {
            xConverter = Reference< XTypeConverter >::query( xIFace );
        }
    }

    Reference< XInitialization > xInit( xAttacher, UNO_QUERY );
    if( xInit.is() )
    {
        Sequence< Any > Arguments( 1 );
        Arguments[0] <<= rIntrospection;
        xInit->initialize( Arguments );
    }
}

//-----------------------------------------------------------------------------
ImplEventAttacherManager::~ImplEventAttacherManager()
{
}

Reference< XIdlReflection > ImplEventAttacherManager::getReflection() throw( Exception )
{
    Guard< Mutex > aGuard( aLock );
    // Haben wir den Service schon? Sonst anlegen
    if( !mxCoreReflection.is() )
    {
        Reference< XInterface > xIFace( mxSMgr->createInstance( OUString::createFromAscii("com.sun.star.reflection.CoreReflection") ) );
        mxCoreReflection = Reference< XIdlReflection >( xIFace, UNO_QUERY);
    }
    return mxCoreReflection;
}


//-----------------------------------------------------------------------------
::std::deque<AttacherIndex_Impl>::iterator ImplEventAttacherManager::implCheckIndex( sal_Int32 _nIndex ) SAL_THROW ( ( IllegalArgumentException ) )
{
    if (_nIndex < 0)
        throw IllegalArgumentException();

    ::std::deque<AttacherIndex_Impl>::iterator aIt = aIndex.begin();
    for ( sal_Int32 i = 0; (i < _nIndex) && (aIt != aIndex.end()); ++i, ++aIt )
        ;

    if( aIt == aIndex.end() )
        throw IllegalArgumentException();

    return aIt;
}

//-----------------------------------------------------------------------------
void detachAll_Impl
(
    ImplEventAttacherManager * pMgr,
    sal_Int32 nIdx,
    ::std::deque< AttachedObject_Impl > & rList
)
{
    ::std::deque< AttachedObject_Impl >::iterator aObjIt =  rList.begin();
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = rList.end();
    while( aObjIt != aObjEnd )
    {
        pMgr->detach( nIdx, (*aObjIt).xTarget );
        aObjIt++;
    }
}

//-----------------------------------------------------------------------------
void attachAll_Impl
(
    ImplEventAttacherManager * pMgr,
    sal_Int32 nIdx,
    ::std::deque< AttachedObject_Impl > & rList
)
{
    ::std::deque< AttachedObject_Impl >::iterator aObjIt =  rList.begin();
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = rList.end();
    while( aObjIt != aObjEnd )
    {
        pMgr->attach( nIdx, (*aObjIt).xTarget, (*aObjIt).aHelper );
        aObjIt++;
    }
}

//-----------------------------------------------------------------------------
//*** Methoden von XEventAttacherManager ***
void SAL_CALL ImplEventAttacherManager::registerScriptEvent
(
    sal_Int32 nIndex,
    const ScriptEventDescriptor& ScriptEvent
)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );

    // Index pruefen und Array anpassen
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;

    ScriptEventDescriptor aEvt = ScriptEvent;
    const sal_Unicode* pLastDot = aEvt.ListenerType.getStr();
    pLastDot += rtl_ustr_lastIndexOfChar( pLastDot, '.' );
    if( pLastDot )
        aEvt.ListenerType = pLastDot +1;
#ifdef DEQUE_OK
    (*aIt).aEventList.push_back( aEvt );
#else
    (*aIt).aEventList.realloc( (*aIt).aEventList.getLength() +1 );
    (*aIt).aEventList.getArray()[(*aIt).aEventList.getLength() -1] = aEvt;
#endif

    // register new new Event
    ::std::deque< AttachedObject_Impl >::iterator aObjIt =  (*aIt).aObjList.begin();
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = (*aIt).aObjList.end();
    while( aObjIt != aObjEnd )
    {
        // resize
        sal_Int32 nPos = (*aObjIt).aAttachedListenerSeq.getLength();
        (*aObjIt).aAttachedListenerSeq.realloc( nPos + 1 );
        Reference< XEventListener > * pArray = (*aObjIt).aAttachedListenerSeq.getArray();

        Reference< XAllListener > xAll =
            new AttacherAllListener_Impl( this, ScriptEvent.ScriptType, ScriptEvent.ScriptCode );
        try
        {
        pArray[nPos] = xAttacher->attachSingleEventListener( (*aObjIt).xTarget, xAll,
                        (*aObjIt).aHelper, ScriptEvent.ListenerType,
                        ScriptEvent.AddListenerParam, ScriptEvent.EventMethod );
        }
        catch( Exception& )
        {
        }

        aObjIt++;
    }
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::registerScriptEvents
(
    sal_Int32 nIndex,
    const Sequence< ScriptEventDescriptor >& ScriptEvents
)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );

    // Index pruefen und Array anpassen
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    detachAll_Impl( this, nIndex, aList );

    const ScriptEventDescriptor* pArray = ScriptEvents.getConstArray();
    sal_Int32 nLen = ScriptEvents.getLength();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
        registerScriptEvent( nIndex, pArray[ i ] );

    attachAll_Impl( this, nIndex, aList );
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::revokeScriptEvent
(
    sal_Int32 nIndex,
    const OUString& ListenerType,
    const OUString& EventMethod,
    const OUString& ToRemoveListenerParam
)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );

    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    detachAll_Impl( this, nIndex, aList );

    OUString aLstType = ListenerType;
    const sal_Unicode * pLastDot = aLstType.getStr();
    pLastDot += rtl_ustr_lastIndexOfChar( pLastDot, '.' );
    if( pLastDot )
        aLstType = pLastDot +1;

#ifdef DEQUE_OK
    ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    (*aIt).aEventList.begin();
    ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd =   (*aIt).aEventList.end();
    while( aEvtIt != aEvtEnd )
    {
        if( aLstType            == (*aEvtIt).ListenerType
          && EventMethod            == (*aEvtIt).EventMethod
          && ToRemoveListenerParam  == (*aEvtIt).AddListenerParam )
        {
            (*aIt).aEventList.erase( aEvtIt );
            break;
        }

        aEvtIt++;
    }
#else
    Sequence< ScriptEventDescriptor >& rEventList = (*aIt).aEventList;

            ScriptEventDescriptor* pEventList = rEventList.getArray();
    const   ScriptEventDescriptor* pEventListEnd = pEventList + rEventList.getLength();
    for( ; pEventList < pEventListEnd; ++pEventList )
    {
        if  (   (aLstType               == pEventList->ListenerType )
            &&  (EventMethod            == pEventList->EventMethod      )
            &&  (ToRemoveListenerParam  == pEventList->AddListenerParam)
            )
        {
            ScriptEventDescriptor* pMoveTo = pEventList;
            const ScriptEventDescriptor* pMoveFrom = pMoveTo + 1;
            while (pMoveFrom < pEventListEnd)
            {
                *pMoveTo++ = *pMoveFrom++;
            }
            rEventList.realloc( rEventList.getLength() - 1 );
            break;
        }
    }
#endif
    attachAll_Impl( this, nIndex, aList );
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::revokeScriptEvents(sal_Int32 nIndex )
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    detachAll_Impl( this, nIndex, aList );
#ifdef DEQUE_OK
    (*aIt).aEventList = ::std::deque< ScriptEventDescriptor >();
#else
    (*aIt).aEventList.realloc( 0 );
#endif
    attachAll_Impl( this, nIndex, aList );
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::insertEntry(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    if( nIndex < 0 )
        throw IllegalArgumentException();

//    ::std::deque<AttacherIndex_Impl>::iterator aIt = aIndex.begin();
//    while( nIndex-- )
//        aIt++;

    if ( static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() )
        aIndex.resize(nIndex+1);

    AttacherIndex_Impl aTmp;
    aIndex.insert( aIndex.begin() + nIndex, aTmp );
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::removeEntry(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    detachAll_Impl( this, nIndex, aList );
    aIndex.erase( aIt );
}

//-----------------------------------------------------------------------------
Sequence< ScriptEventDescriptor > SAL_CALL ImplEventAttacherManager::getScriptEvents(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

#ifdef DEQUE_OK
    Sequence< ScriptEventDescriptor > aSeq( (*aIt).aEventList.size() );
    ScriptEventDescriptor * pArray = aSeq.getArray();

    ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    (*aIt).aEventList.begin();
    ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd =   (*aIt).aEventList.end();
    sal_Int32 i = 0;
    while( aEvtIt != aEvtEnd )
    {
        pArray[i++] = *aEvtIt;
        aEvtIt++;
    }
    return aSeq;
#else
    return (*aIt).aEventList;
#endif
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::attach(sal_Int32 nIndex, const Reference< XInterface >& xObject, const Any & Helper)
    throw( IllegalArgumentException, ServiceNotRegisteredException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    if( nIndex < 0 || !xObject.is() )
        throw IllegalArgumentException();

    if( static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() )
    {
        // alte Dateien lesen
        if( nVersion == 1 )
        {
            insertEntry( nIndex );
            attach( nIndex, xObject, Helper );
            return;
        }
        else
            throw IllegalArgumentException();
    }

    ::std::deque< AttacherIndex_Impl >::iterator aCurrentPosition = aIndex.begin() + nIndex;

    AttachedObject_Impl aTmp;
    aTmp.xTarget = xObject;
    aTmp.aHelper = Helper;
    aCurrentPosition->aObjList.push_back( aTmp );

    //::std::deque< AttachedObject_Impl >::iterator aObjIt = (*aIt).aObjList.back();
    AttachedObject_Impl & rCurObj = aCurrentPosition->aObjList.back();
#ifdef DEQUE_OK
    rCurObj.aAttachedListenerSeq = Sequence< Reference< XEventListener > >( aCurrentPosition->aEventList.size() );
#else
    rCurObj.aAttachedListenerSeq = Sequence< Reference< XEventListener > >( aCurrentPosition->aEventList.getLength() );
#endif
    Reference< XEventListener > * pArray = rCurObj.aAttachedListenerSeq.getArray();

#ifdef DEQUE_OK
    ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    aCurrentPosition->aEventList.begin();
    ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd =   aCurrentPosition->aEventList.end();
    sal_Int32 i = 0;
    while( aEvtIt != aEvtEnd )
    {
        Reference< XAllListener > xAll =
            new AttacherAllListener_Impl( this, (*aEvtIt).ScriptType, (*aEvtIt).ScriptCode );
        Reference< XEventListener > xAdapter;
           try
        {
        xAdapter = xAttacher->attachSingleEventListener( rCurObj.xTarget, xAll,
                        rCurObj.aHelper, (*aEvtIt).ScriptType,
                        (*aEvtIt).AddListenerParam, (*aEvtIt).EventMethod );
        }
        catch( Exception& )
        {
        }

        pArray[i++] = xAdapter;
        aEvtIt++;
    }
#else
    sal_Int32 nLen = aCurrentPosition->aEventList.getLength();
    ScriptEventDescriptor * pEL = aCurrentPosition->aEventList.getArray();
    for(sal_Int32 i = 0; i < nLen; ++i )
    {
        Reference< XAllListener > xAll =
            new AttacherAllListener_Impl( this, pEL[i].ScriptType, pEL[i].ScriptCode );
        Reference< XEventListener > xAdapter;
        try
        {
        xAdapter = xAttacher->attachSingleEventListener( rCurObj.xTarget, xAll,
                        rCurObj.aHelper, pEL[i].ListenerType,
                        pEL[i].AddListenerParam, pEL[i].EventMethod );
        }
        catch( Exception& )
        {
        }

        pArray[i] = xAdapter;
    }
#endif
}

//-----------------------------------------------------------------------------
void SAL_CALL ImplEventAttacherManager::detach(sal_Int32 nIndex, const Reference< XInterface >& xObject)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    //return;
    if( nIndex < 0 || static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() || !xObject.is() )
        throw IllegalArgumentException();

    ::std::deque< AttacherIndex_Impl >::iterator aCurrentPosition = aIndex.begin() + nIndex;
    ::std::deque< AttachedObject_Impl >::iterator aObjIt =  aCurrentPosition->aObjList.begin();
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = aCurrentPosition->aObjList.end();
    while( aObjIt != aObjEnd )
    {
        if( (*aObjIt).xTarget == xObject )
        {
            Reference< XEventListener > * pArray = (*aObjIt).aAttachedListenerSeq.getArray();
#ifdef DEQUE_OK

            ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    aCurrentPosition->aEventList.begin();
            ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd =   aCurrentPosition->aEventList.end();
            sal_Int32 i = 0;
            while( aEvtIt != aEvtEnd )
            {
                if( pArray[i].is() )
                {
                    try
                    {
                    xAttacher->removeListener( (*aObjIt).xTarget, (*aEvtIt).ListenerType,
                                                (*aEvtIt).AddListenerParam, pArray[i] );
                    }
                    catch( Exception& )
                    {
                    }
                }
                i++;
                aEvtIt++;
            }
#else
            sal_Int32 nLen = aCurrentPosition->aEventList.getLength();
            ScriptEventDescriptor * pEL = aCurrentPosition->aEventList.getArray();
            for( sal_Int32 i = 0; i < nLen; i++ )
            {
                if( pArray[i].is() )
                {
                    try
                    {
                    xAttacher->removeListener( (*aObjIt).xTarget, pEL[i].ListenerType,
                                                pEL[i].AddListenerParam, pArray[i] );
                    }
                    catch( Exception& )
                    {
                    }
                }
            }
#endif
            aCurrentPosition->aObjList.erase( aObjIt );
            break;
        }
        aObjIt++;
    }
}

void SAL_CALL ImplEventAttacherManager::addScriptListener(const Reference< XScriptListener >& aListener)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    aScriptListeners.addInterface( aListener );
}

void SAL_CALL ImplEventAttacherManager::removeScriptListener(const Reference< XScriptListener >& aListener)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    aScriptListeners.removeInterface( aListener );
}


// Methoden von XPersistObject
OUString SAL_CALL ImplEventAttacherManager::getServiceName(void)
    throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.script.EventAttacherManager") );
}

void SAL_CALL ImplEventAttacherManager::write(const Reference< XObjectOutputStream >& OutStream)
    throw( IOException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    // Ohne XMarkableStream laeuft nichts
    Reference< XMarkableStream > xMarkStream( OutStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    // Version schreiben
    OutStream->writeShort( 2 );

    // Position fuer Laenge merken
    sal_Int32 nObjLenMark = xMarkStream->createMark();
    OutStream->writeLong( 0L );

    OutStream->writeLong( aIndex.size() );

    // Sequences schreiben
    ::std::deque<AttacherIndex_Impl>::iterator aIt = aIndex.begin();
    ::std::deque<AttacherIndex_Impl>::iterator aEnd = aIndex.end();
    while( aIt != aEnd )
    {
#ifdef DEQUE_OK
        // Laenge der Sequence und alle Descriptoren schreiben
        OutStream->writeLong( (*aIt).aEventList.size() );
        ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    (*aIt).aEventList.begin();
        ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd = (*aIt).aEventList.end();
        while( aEvtIt != aEvtEnd )
        {
            const ScriptEventDescriptor& rDesc = (*aEvtIt);
            OutStream->writeUTF( rDesc.ListenerType );
            OutStream->writeUTF( rDesc.EventMethod );
            OutStream->writeUTF( rDesc.AddListenerParam );
            OutStream->writeUTF( rDesc.ScriptType );
            OutStream->writeUTF( rDesc.ScriptCode );

            aEvtIt++;
        }
#else
        sal_Int32 nLen = (*aIt).aEventList.getLength();
        // Laenge der Sequence und alle Descriptoren schreiben
        OutStream->writeLong( nLen );
        ScriptEventDescriptor * pEL = (*aIt).aEventList.getArray();
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            const ScriptEventDescriptor& rDesc = pEL[i];
            OutStream->writeUTF( rDesc.ListenerType );
            OutStream->writeUTF( rDesc.EventMethod );
            OutStream->writeUTF( rDesc.AddListenerParam );
            OutStream->writeUTF( rDesc.ScriptType );
            OutStream->writeUTF( rDesc.ScriptCode );
        }
#endif
        aIt++;
    }

    // Die jetzt bekannte Laenge eintragen
    sal_Int32 nObjLen = xMarkStream->offsetToMark( nObjLenMark ) -4;
    xMarkStream->jumpToMark( nObjLenMark );
    OutStream->writeLong( nObjLen );
    xMarkStream->jumpToFurthest();
    xMarkStream->deleteMark( nObjLenMark );
}

void SAL_CALL ImplEventAttacherManager::read(const Reference< XObjectInputStream >& InStream)
    throw( IOException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    // Ohne XMarkableStream laeuft nichts
    Reference< XMarkableStream > xMarkStream( InStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    // Version lesen
    nVersion = InStream->readShort();

    // Zunaechst kommen die Daten gemaess Version 1,
    // muss auch bei hoeheren Versionen beibehalten werden
    sal_Int32 nLen = InStream->readLong();

    // Position fuer Vergleichszwecke
    sal_Int32 nObjLenMark = xMarkStream->createMark();

    // Anzahl der zu lesenden Sequences
    sal_Int32 nItemCount = InStream->readLong();

    for( sal_Int32 i = 0 ; i < nItemCount ; i++ )
    {
        insertEntry( i );
        // Laenge der Sequence lesen
        sal_Int32 nSeqLen = InStream->readLong();

        // Sequence anlegen und Descriptoren lesen
        Sequence< ScriptEventDescriptor > aSEDSeq( nSeqLen );
        ScriptEventDescriptor* pArray = aSEDSeq.getArray();
        for( sal_Int32 j = 0 ; j < nSeqLen ; j++ )
        {
            ScriptEventDescriptor& rDesc = pArray[ j ];
            rDesc.ListenerType = InStream->readUTF();
            rDesc.EventMethod = InStream->readUTF();
            rDesc.AddListenerParam = InStream->readUTF();
            rDesc.ScriptType = InStream->readUTF();
            rDesc.ScriptCode = InStream->readUTF();
        }
        registerScriptEvents( i, aSEDSeq );
    }

    // Haben wir die angegebene Laenge gelesen?
    sal_Int32 nRealLen = xMarkStream->offsetToMark( nObjLenMark );
    if( nRealLen != nLen )
    {
        // Nur wenn die StreamVersion > 1 ist und noch Daten folgen, kann das
        // Ganze richtig sein. Sonst ist etwas voellig daneben gegangen.
        if( nRealLen > nLen || nVersion == 1 )
        {
            VOS_ENSHURE( sal_False, "ImplEventAttacherManager::read(): Fatal Error, wrong object length" );
        }
        else
        {
            // TODO: Pruefen, ob Zwischen-Speicherung der Daten sinnvoll sein koennte

            // Vorerst einfach nur Skippen
            sal_Int32 nSkipCount = nLen - nRealLen;
            InStream->skipBytes( nSkipCount );
        }
    }
    xMarkStream->jumpToFurthest();
    xMarkStream->deleteMark( nObjLenMark );
}

} // namesapce comphelper


