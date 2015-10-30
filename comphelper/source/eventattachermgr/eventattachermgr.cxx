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
#include <comphelper/eventattachermgr.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XEngineListener.hpp>
#include <com/sun/star/script/XEventAttacher2.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>

#include <deque>
#include <algorithm>
#include <functional>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace cppu;
using namespace osl;


namespace comphelper
{


struct AttachedObject_Impl
{
    Reference< XInterface >                 xTarget;
    Sequence< Reference< XEventListener > > aAttachedListenerSeq;
    Any                                     aHelper;
};

struct AttacherIndex_Impl
{
    ::std::deque< ScriptEventDescriptor > aEventList;
    ::std::deque< AttachedObject_Impl > aObjList;
};


class ImplEventAttacherManager
    : public WeakImplHelper< XEventAttacherManager, XPersistObject >
{
    friend class AttacherAllListener_Impl;
    ::std::deque< AttacherIndex_Impl >  aIndex;
    Mutex aLock;
    // Container for the ScriptListener
    OInterfaceContainerHelper           aScriptListeners;
    // Instance of EventAttacher
    Reference< XEventAttacher2 >        xAttacher;
    Reference< XComponentContext >      mxContext;
    Reference< XIdlReflection >         mxCoreReflection;
    Reference< XIntrospection >         mxIntrospection;
    Reference< XTypeConverter >         xConverter;
    sal_Int16                           nVersion;
public:
    ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                              const Reference< XComponentContext >& rContext );
    virtual ~ImplEventAttacherManager();

    // Methods of XEventAttacherManager
    virtual void SAL_CALL registerScriptEvent(sal_Int32 Index, const ScriptEventDescriptor& ScriptEvent)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL registerScriptEvents(sal_Int32 Index, const Sequence< ScriptEventDescriptor >& ScriptEvents)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL revokeScriptEvent(sal_Int32 Index, const OUString& ListenerType, const OUString& EventMethod, const OUString& removeListenerParam)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL revokeScriptEvents(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL insertEntry(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEntry(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual Sequence< ScriptEventDescriptor > SAL_CALL getScriptEvents(sal_Int32 Index)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL attach(sal_Int32 Index, const Reference< XInterface >& Object, const Any& Helper)
        throw( IllegalArgumentException, ServiceNotRegisteredException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL detach(sal_Int32 nIndex, const Reference< XInterface >& xObject)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL addScriptListener(const Reference< XScriptListener >& aListener)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeScriptListener(const Reference< XScriptListener >& Listener)
        throw( IllegalArgumentException, RuntimeException, std::exception ) override;

    // Methods of XPersistObject
    virtual OUString SAL_CALL getServiceName() throw( RuntimeException, std::exception ) override;
    virtual void SAL_CALL write(const Reference< XObjectOutputStream >& OutStream) throw( IOException, RuntimeException, std::exception ) override;
    virtual void SAL_CALL read(const Reference< XObjectInputStream >& InStream) throw( IOException, RuntimeException, std::exception ) override;

private:
    Reference< XIdlReflection > getReflection() throw( Exception );

    /** checks if <arg>_nIndex</arg> is a valid index, throws an <type>IllegalArgumentException</type> if not
    @param _nIndex
        the index to check
    @return
        the iterator pointing to the position indicated by the index
    */
    ::std::deque<AttacherIndex_Impl>::iterator implCheckIndex( sal_Int32 _nIndex );
};






// Implementation of an EventAttacher-subclass 'AllListeners', which
// only passes individual events of the general AllListeners.
class AttacherAllListener_Impl : public WeakImplHelper< XAllListener >
{
    ImplEventAttacherManager*           mpManager;
    Reference< XEventAttacherManager >  xManager;
    OUString                            aScriptType;
    OUString                            aScriptCode;

    void convertToEventReturn( Any & rRet, const Type & rRetType )
        throw( CannotConvertException );
public:
    AttacherAllListener_Impl( ImplEventAttacherManager* pManager_, const OUString &rScriptType_,
                                const OUString & rScriptCode_ );

    // Methods of XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) throw( RuntimeException, std::exception ) override;
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException, std::exception ) override;

    // Methods of XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException, std::exception ) override;
};


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
{
}



// Methods of XAllListener
void SAL_CALL AttacherAllListener_Impl::firing(const AllEventObject& Event)
    throw( RuntimeException, std::exception )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = static_cast<OWeakObject *>(mpManager); // get correct XInterface
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    // Iterate over all listeners and pass events.
    OInterfaceIteratorHelper aIt( mpManager->aScriptListeners );
    while( aIt.hasMoreElements() )
        static_cast<XScriptListener *>(aIt.next())->firing( aScriptEvent );
}


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

// Methods of XAllListener
Any SAL_CALL AttacherAllListener_Impl::approveFiring( const AllEventObject& Event )
    throw( InvocationTargetException, RuntimeException, std::exception )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = static_cast<OWeakObject *>(mpManager); // get correct XInterface
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    Any aRet;
    // Iterate over all listeners and pass events.
    OInterfaceIteratorHelper aIt( mpManager->aScriptListeners );
    while( aIt.hasMoreElements() )
    {
        aRet = static_cast<XScriptListener *>(aIt.next())->approveFiring( aScriptEvent );
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
                    if( !(*static_cast<sal_Bool const *>(aRet.getValue())) )
                        return aRet;
                    break;

                case TypeClass_STRING:
                    // none empty string -> return
                    if( !(static_cast<OUString const *>(aRet.getValue()))->isEmpty() )
                        return aRet;
                    break;

                    // none zero number -> return
                case TypeClass_FLOAT:           if( *static_cast<float const *>(aRet.getValue()) )    return aRet; break;
                case TypeClass_DOUBLE:          if( *static_cast<double const *>(aRet.getValue()) )   return aRet; break;
                case TypeClass_BYTE:            if( *static_cast<sal_uInt8 const *>(aRet.getValue()) )    return aRet; break;
                case TypeClass_SHORT:           if( *static_cast<sal_Int16 const *>(aRet.getValue()) )    return aRet; break;
                case TypeClass_LONG:            if( *static_cast<sal_Int32 const *>(aRet.getValue()) )    return aRet; break;
                case TypeClass_UNSIGNED_SHORT:  if( *static_cast<sal_uInt16 const *>(aRet.getValue()) )   return aRet; break;
                case TypeClass_UNSIGNED_LONG:   if( *static_cast<sal_uInt32 const *>(aRet.getValue()) )   return aRet; break;

                default:
                    OSL_ASSERT(false);
                    break;
            }
        }
        catch (const CannotConvertException&)
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
                try
                {
                    convertToEventReturn( aRet, aRetType );
                }
                catch (const CannotConvertException& e)
                {
                    throw css::lang::WrappedTargetRuntimeException(
                        "wrapped CannotConvertException " + e.Message,
                        css::uno::Reference<css::uno::XInterface>(), makeAny(e));
                }
            }
        }
    }
    return aRet;
}

// Methods of XEventListener
void SAL_CALL AttacherAllListener_Impl::disposing(const EventObject& )
    throw( RuntimeException, std::exception )
{
    // It is up to the container to release the object
}

// Constructor method for EventAttacherManager
Reference< XEventAttacherManager > createEventAttacherManager( const Reference< XComponentContext > & rxContext )
    throw( Exception )
{
    Reference< XIntrospection > xIntrospection = theIntrospection::get( rxContext );
    return new ImplEventAttacherManager( xIntrospection, rxContext );
}


ImplEventAttacherManager::ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                                                    const Reference< XComponentContext >& rContext )
    : aScriptListeners( aLock )
    , mxContext( rContext )
    , mxIntrospection( rIntrospection )
    , nVersion(0)
{
    if ( rContext.is() )
    {
        Reference< XInterface > xIFace( rContext->getServiceManager()->createInstanceWithContext(
             "com.sun.star.script.EventAttacher", rContext)  );
        if ( xIFace.is() )
        {
            xAttacher = Reference< XEventAttacher2 >::query( xIFace );
        }
        xConverter = Converter::create(rContext);
    }

    Reference< XInitialization > xInit( xAttacher, UNO_QUERY );
    if( xInit.is() )
    {
        Sequence< Any > Arguments( 1 );
        Arguments[0] <<= rIntrospection;
        xInit->initialize( Arguments );
    }
}


ImplEventAttacherManager::~ImplEventAttacherManager()
{
}

Reference< XIdlReflection > ImplEventAttacherManager::getReflection() throw( Exception )
{
    Guard< Mutex > aGuard( aLock );
    // Do we already have a service? If not, create one.
    if( !mxCoreReflection.is() )
    {
        mxCoreReflection = theCoreReflection::get(mxContext);
    }
    return mxCoreReflection;
}



::std::deque<AttacherIndex_Impl>::iterator ImplEventAttacherManager::implCheckIndex( sal_Int32 _nIndex )
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

// Methods of XEventAttacherManager
void SAL_CALL ImplEventAttacherManager::registerScriptEvent
(
    sal_Int32 nIndex,
    const ScriptEventDescriptor& ScriptEvent
)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );

    // Examine the index and apply the array
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;

    ScriptEventDescriptor aEvt = ScriptEvent;
    sal_Int32 nLastDot = aEvt.ListenerType.lastIndexOf('.');
    if (nLastDot != -1)
        aEvt.ListenerType = aEvt.ListenerType.copy(nLastDot+1);
    aIt->aEventList.push_back( aEvt );

    // register new new Event
    for( auto& rObj : aIt->aObjList )
    {
        // resize
        sal_Int32 nPos = rObj.aAttachedListenerSeq.getLength();
        rObj.aAttachedListenerSeq.realloc( nPos + 1 );
        Reference< XEventListener >* pArray = rObj.aAttachedListenerSeq.getArray();
        Reference< XAllListener > xAll =
            new AttacherAllListener_Impl( this, ScriptEvent.ScriptType, ScriptEvent.ScriptCode );
        try
        {
        pArray[nPos] = xAttacher->attachSingleEventListener( rObj.xTarget, xAll,
                        rObj.aHelper, ScriptEvent.ListenerType,
                        ScriptEvent.AddListenerParam, ScriptEvent.EventMethod );
        }
        catch( Exception& )
        {
        }
    }
}


void SAL_CALL ImplEventAttacherManager::registerScriptEvents
(
    sal_Int32 nIndex,
    const Sequence< ScriptEventDescriptor >& ScriptEvents
)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );

    // Examine the index and apply the array
    ::std::deque< AttachedObject_Impl > aList = implCheckIndex( nIndex )->aObjList;
    for( const auto& rObj : aList )
        this->detach( nIndex, rObj.xTarget );

    const ScriptEventDescriptor* pArray = ScriptEvents.getConstArray();
    sal_Int32 nLen = ScriptEvents.getLength();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
        registerScriptEvent( nIndex, pArray[ i ] );

    for( const auto& rObj : aList )
        this->attach( nIndex, rObj.xTarget, rObj.aHelper );
}


void SAL_CALL ImplEventAttacherManager::revokeScriptEvent
(
    sal_Int32 nIndex,
    const OUString& ListenerType,
    const OUString& EventMethod,
    const OUString& ToRemoveListenerParam
)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );

    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = aIt->aObjList;
    for( const auto& rObj : aList )
        this->detach( nIndex, rObj.xTarget );

    OUString aLstType = ListenerType;
    sal_Int32 nLastDot = aLstType.lastIndexOf('.');
    if (nLastDot != -1)
        aLstType = aLstType.copy(nLastDot+1);

    ::std::deque< ScriptEventDescriptor >::const_iterator aEvtEnd = aIt->aEventList.end();
    for( std::deque< ScriptEventDescriptor >::iterator aEvtIt =  aIt->aEventList.begin();
         aEvtIt != aEvtEnd;
         ++aEvtIt )
    {
        if( aLstType              == aEvtIt->ListenerType &&
            EventMethod           == aEvtIt->EventMethod &&
            ToRemoveListenerParam == aEvtIt->AddListenerParam )
        {
            aIt->aEventList.erase( aEvtIt );
            break;
        }
    }

    for( const auto& rObj : aList )
        this->attach( nIndex, rObj.xTarget, rObj.aHelper );
}


void SAL_CALL ImplEventAttacherManager::revokeScriptEvents(sal_Int32 nIndex )
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = aIt->aObjList;
    for( const auto& rObj : aList )
        this->detach( nIndex, rObj.xTarget );
    aIt->aEventList.clear();
    for( const auto& rObj : aList )
        this->attach( nIndex, rObj.xTarget, rObj.aHelper );
}


void SAL_CALL ImplEventAttacherManager::insertEntry(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    if( nIndex < 0 )
        throw IllegalArgumentException();

    if ( static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() )
        aIndex.resize(nIndex+1);

    AttacherIndex_Impl aTmp;
    aIndex.insert( aIndex.begin() + nIndex, aTmp );
}


void SAL_CALL ImplEventAttacherManager::removeEntry(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = aIt->aObjList;
    for( const auto& rObj : aList )
        this->detach( nIndex, rObj.xTarget );

    aIndex.erase( aIt );
}


Sequence< ScriptEventDescriptor > SAL_CALL ImplEventAttacherManager::getScriptEvents(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    Sequence< ScriptEventDescriptor > aSeq( aIt->aEventList.size() );
    ScriptEventDescriptor * pArray = aSeq.getArray();

    sal_Int32 i = 0;
    for( const auto& rEvt : aIt->aEventList )
    {
        pArray[i++] = rEvt;
    }
    return aSeq;
}


void SAL_CALL ImplEventAttacherManager::attach(sal_Int32 nIndex, const Reference< XInterface >& xObject, const Any & Helper)
    throw( IllegalArgumentException, ServiceNotRegisteredException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    if( nIndex < 0 || !xObject.is() )
        throw IllegalArgumentException();

    if( static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() )
    {
        // read older files
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

    AttachedObject_Impl & rCurObj = aCurrentPosition->aObjList.back();
    rCurObj.aAttachedListenerSeq = Sequence< Reference< XEventListener > >( aCurrentPosition->aEventList.size() );

    if (aCurrentPosition->aEventList.empty())
        return;

    Sequence<css::script::EventListener> aEvents(aCurrentPosition->aEventList.size());
    std::deque<ScriptEventDescriptor>::iterator itr = aCurrentPosition->aEventList.begin();
    std::deque<ScriptEventDescriptor>::iterator itrEnd = aCurrentPosition->aEventList.end();
    css::script::EventListener* p = aEvents.getArray();
    size_t i = 0;
    for (; itr != itrEnd; ++itr)
    {
        css::script::EventListener aListener;
        aListener.AllListener =
            new AttacherAllListener_Impl(this, itr->ScriptType, itr->ScriptCode);
        aListener.Helper = rCurObj.aHelper;
        aListener.ListenerType = itr->ListenerType;
        aListener.EventMethod = itr->EventMethod;
        aListener.AddListenerParam = itr->AddListenerParam;
        p[i++] = aListener;
    }

    try
    {
        rCurObj.aAttachedListenerSeq =
            xAttacher->attachMultipleEventListeners(rCurObj.xTarget, aEvents);
    }
    catch (const Exception&)
    {
        // Fail gracefully.
    }
}


void SAL_CALL ImplEventAttacherManager::detach(sal_Int32 nIndex, const Reference< XInterface >& xObject)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    //return;
    if( nIndex < 0 || static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() || !xObject.is() )
        throw IllegalArgumentException();

    ::std::deque< AttacherIndex_Impl >::iterator aCurrentPosition = aIndex.begin() + nIndex;
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = aCurrentPosition->aObjList.end();
    for( ::std::deque< AttachedObject_Impl >::iterator aObjIt =  aCurrentPosition->aObjList.begin();
         aObjIt != aObjEnd;
         ++aObjIt )
    {
        if( aObjIt->xTarget == xObject )
        {
            Reference< XEventListener > * pArray = aObjIt->aAttachedListenerSeq.getArray();

            sal_Int32 i = 0;
            for( const auto& rEvt : aCurrentPosition->aEventList )
            {
                if( pArray[i].is() )
                {
                    try
                    {
                    xAttacher->removeListener( aObjIt->xTarget, rEvt.ListenerType,
                                               rEvt.AddListenerParam, pArray[i] );
                    }
                    catch( Exception& )
                    {
                    }
                }
                ++i;
            }
            aCurrentPosition->aObjList.erase( aObjIt );
            break;
        }
    }
}

void SAL_CALL ImplEventAttacherManager::addScriptListener(const Reference< XScriptListener >& aListener)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    aScriptListeners.addInterface( aListener );
}

void SAL_CALL ImplEventAttacherManager::removeScriptListener(const Reference< XScriptListener >& aListener)
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    aScriptListeners.removeInterface( aListener );
}


// Methods of XPersistObject
OUString SAL_CALL ImplEventAttacherManager::getServiceName()
    throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.uno.script.EventAttacherManager" );
}

void SAL_CALL ImplEventAttacherManager::write(const Reference< XObjectOutputStream >& OutStream)
    throw( IOException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    // Don't run without XMarkableStream
    Reference< XMarkableStream > xMarkStream( OutStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    // Write out the version
    OutStream->writeShort( 2 );

    // Remember position for length
    sal_Int32 nObjLenMark = xMarkStream->createMark();
    OutStream->writeLong( 0L );

    OutStream->writeLong( aIndex.size() );

    // Write out sequences
    for( const auto& rIx : aIndex )
    {
        OutStream->writeLong( rIx.aEventList.size() );
        for( const auto& rDesc : rIx.aEventList )
        {
            OutStream->writeUTF( rDesc.ListenerType );
            OutStream->writeUTF( rDesc.EventMethod );
            OutStream->writeUTF( rDesc.AddListenerParam );
            OutStream->writeUTF( rDesc.ScriptType );
            OutStream->writeUTF( rDesc.ScriptCode );
        }
    }

    // The length is now known
    sal_Int32 nObjLen = xMarkStream->offsetToMark( nObjLenMark ) -4;
    xMarkStream->jumpToMark( nObjLenMark );
    OutStream->writeLong( nObjLen );
    xMarkStream->jumpToFurthest();
    xMarkStream->deleteMark( nObjLenMark );
}

void SAL_CALL ImplEventAttacherManager::read(const Reference< XObjectInputStream >& InStream)
    throw( IOException, RuntimeException, std::exception )
{
    Guard< Mutex > aGuard( aLock );
    // Don't run without XMarkableStream
    Reference< XMarkableStream > xMarkStream( InStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    // Read in the version
    nVersion = InStream->readShort();

    // At first there's the data according to version 1 --
    // this part needs to be kept in later versions.
    sal_Int32 nLen = InStream->readLong();

    // Position for comparative purposes
    sal_Int32 nObjLenMark = xMarkStream->createMark();

    // Number of read sequences
    sal_Int32 nItemCount = InStream->readLong();

    for( sal_Int32 i = 0 ; i < nItemCount ; i++ )
    {
        insertEntry( i );
        // Read the length of the sequence
        sal_Int32 nSeqLen = InStream->readLong();

        // Display the sequences and read the descriptions
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

    // Have we read the specified length?
    sal_Int32 nRealLen = xMarkStream->offsetToMark( nObjLenMark );
    if( nRealLen != nLen )
    {
        // Only if the StreamVersion is > 1 and the date still follows, can
        // this be true. Otherwise, something is completely gone.
        if( nRealLen > nLen || nVersion == 1 )
        {
            OSL_FAIL( "ImplEventAttacherManager::read(): Fatal Error, wrong object length" );
        }
        else
        {   // TODO: Examine if caching the dates would be useful
            // But for now, it's easier to skip it.
            sal_Int32 nSkipCount = nLen - nRealLen;
            InStream->skipBytes( nSkipCount );
        }
    }
    xMarkStream->jumpToFurthest();
    xMarkStream->deleteMark( nObjLenMark );
}

} // namesapce comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
