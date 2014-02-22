/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <comphelper/eventattachermgr.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
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
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

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

    bool    operator<( const AttachedObject_Impl & ) const;
    bool    operator==( const AttachedObject_Impl & ) const;
};

struct AttacherIndex_Impl
{
    ::std::deque< ScriptEventDescriptor > aEventList;
    ::std::deque< AttachedObject_Impl > aObjList;

    bool    operator<( const AttacherIndex_Impl & ) const;
    bool    operator==( const AttacherIndex_Impl & ) const;
};


class ImplEventAttacherManager
    : public WeakImplHelper2< XEventAttacherManager, XPersistObject >
{
    friend class AttacherAllListener_Impl;
    ::std::deque< AttacherIndex_Impl >  aIndex;
    Mutex aLock;
    
    OInterfaceContainerHelper           aScriptListeners;
    
    Reference< XEventAttacher2 >        xAttacher;
    Reference< XComponentContext >      mxContext;
    Reference< XIdlReflection >         mxCoreReflection;
    Reference< XIntrospection >         mxIntrospection;
    Reference< XTypeConverter >         xConverter;
    sal_Int16                           nVersion;
public:
    ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                              const Reference< XComponentContext > xContext );
    ~ImplEventAttacherManager();

    
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








class AttacherAllListener_Impl : public WeakImplHelper1< XAllListener >
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

    
    virtual void SAL_CALL firing(const AllEventObject& Event) throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException );

    
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException );
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




void SAL_CALL AttacherAllListener_Impl::firing(const AllEventObject& Event)
    throw( RuntimeException )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = (OWeakObject *)mpManager; 
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    
    OInterfaceIteratorHelper aIt( mpManager->aScriptListeners );
    while( aIt.hasMoreElements() )
        ((XScriptListener *)aIt.next())->firing( aScriptEvent );
}



void AttacherAllListener_Impl::convertToEventReturn( Any & rRet, const Type & rRetType )
    throw( CannotConvertException )
{
    
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



Any SAL_CALL AttacherAllListener_Impl::approveFiring( const AllEventObject& Event )
    throw( InvocationTargetException, RuntimeException )
{
    ScriptEvent aScriptEvent;
    aScriptEvent.Source         = (OWeakObject *)mpManager; 
    aScriptEvent.ListenerType   = Event.ListenerType;
    aScriptEvent.MethodName     = Event.MethodName;
    aScriptEvent.Arguments      = Event.Arguments;
    aScriptEvent.Helper         = Event.Helper;
    aScriptEvent.ScriptType     = aScriptType;
    aScriptEvent.ScriptCode     = aScriptCode;

    Any aRet;
    
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
                    
                    Reference< XInterface > x;
                    aRet >>= x;
                    if( x.is() )
                        return aRet;
                    }
                    break;

                case TypeClass_BOOLEAN:
                    
                    if( !(*(sal_Bool*)aRet.getValue()) )
                        return aRet;
                    break;

                case TypeClass_STRING:
                    
                    if( !((OUString*)aRet.getValue())->isEmpty() )
                        return aRet;
                    break;

                    
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



void SAL_CALL AttacherAllListener_Impl::disposing(const EventObject& )
    throw( RuntimeException )
{
    
}



Reference< XEventAttacherManager > createEventAttacherManager( const Reference< XComponentContext > & rxContext )
    throw( Exception )
{
    Reference< XIntrospection > xIntrospection = Introspection::create( rxContext );
    return new ImplEventAttacherManager( xIntrospection, rxContext );
}


ImplEventAttacherManager::ImplEventAttacherManager( const Reference< XIntrospection > & rIntrospection,
                                                    const Reference< XComponentContext > xContext )
    : aScriptListeners( aLock )
    , mxContext( xContext )
    , mxIntrospection( rIntrospection )
    , nVersion(0)
{
    if ( xContext.is() )
    {
        Reference< XInterface > xIFace( xContext->getServiceManager()->createInstanceWithContext(
             OUString( "com.sun.star.script.EventAttacher" ), xContext)  );
        if ( xIFace.is() )
        {
            xAttacher = Reference< XEventAttacher2 >::query( xIFace );
        }
        xConverter = Converter::create(xContext);
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
    
    if( !mxCoreReflection.is() )
    {
        mxCoreReflection = theCoreReflection::get(mxContext);
    }
    return mxCoreReflection;
}



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

namespace {

class DetachObject : public std::unary_function<AttachedObject_Impl, void>
{
    ImplEventAttacherManager& mrMgr;
    sal_Int32 mnIdx;
public:
    DetachObject(ImplEventAttacherManager& rMgr, sal_Int32 nIdx) :
        mrMgr(rMgr), mnIdx(nIdx) {}

    void operator() (AttachedObject_Impl& rObj)
    {
        mrMgr.detach(mnIdx, rObj.xTarget);
    }
};

class AttachObject : public std::unary_function<AttachedObject_Impl, void>
{
    ImplEventAttacherManager& mrMgr;
    sal_Int32 mnIdx;
public:
    AttachObject(ImplEventAttacherManager& rMgr, sal_Int32 nIdx) :
        mrMgr(rMgr), mnIdx(nIdx) {}

    void operator() (AttachedObject_Impl& rObj)
    {
        mrMgr.attach(mnIdx, rObj.xTarget, rObj.aHelper);
    }
};

}



void SAL_CALL ImplEventAttacherManager::registerScriptEvent
(
    sal_Int32 nIndex,
    const ScriptEventDescriptor& ScriptEvent
)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );

    
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;

    ScriptEventDescriptor aEvt = ScriptEvent;
    sal_Int32 nLastDot = aEvt.ListenerType.lastIndexOf('.');
    if (nLastDot != -1)
        aEvt.ListenerType = aEvt.ListenerType.copy(nLastDot+1);
    (*aIt).aEventList.push_back( aEvt );

    
    ::std::deque< AttachedObject_Impl >::iterator aObjIt =  (*aIt).aObjList.begin();
    ::std::deque< AttachedObject_Impl >::iterator aObjEnd = (*aIt).aObjList.end();
    while( aObjIt != aObjEnd )
    {
        
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

        ++aObjIt;
    }
}


void SAL_CALL ImplEventAttacherManager::registerScriptEvents
(
    sal_Int32 nIndex,
    const Sequence< ScriptEventDescriptor >& ScriptEvents
)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );

    
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    ::std::for_each(aList.begin(), aList.end(), DetachObject(*this, nIndex));

    const ScriptEventDescriptor* pArray = ScriptEvents.getConstArray();
    sal_Int32 nLen = ScriptEvents.getLength();
    for( sal_Int32 i = 0 ; i < nLen ; i++ )
        registerScriptEvent( nIndex, pArray[ i ] );

    ::std::for_each(aList.begin(), aList.end(), AttachObject(*this, nIndex));
}


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
    ::std::for_each(aList.begin(), aList.end(), DetachObject(*this, nIndex));

    OUString aLstType = ListenerType;
    sal_Int32 nLastDot = aLstType.lastIndexOf('.');
    if (nLastDot != -1)
        aLstType = aLstType.copy(nLastDot+1);

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

        ++aEvtIt;
    }
    ::std::for_each(aList.begin(), aList.end(), AttachObject(*this, nIndex));
}


void SAL_CALL ImplEventAttacherManager::revokeScriptEvents(sal_Int32 nIndex )
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    ::std::for_each(aList.begin(), aList.end(), DetachObject(*this, nIndex));
    (*aIt).aEventList.clear();
    ::std::for_each(aList.begin(), aList.end(), AttachObject(*this, nIndex));
}


void SAL_CALL ImplEventAttacherManager::insertEntry(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException )
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
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    ::std::deque< AttachedObject_Impl > aList = (*aIt).aObjList;
    ::std::for_each(aList.begin(), aList.end(), DetachObject(*this, nIndex));
    aIndex.erase( aIt );
}


Sequence< ScriptEventDescriptor > SAL_CALL ImplEventAttacherManager::getScriptEvents(sal_Int32 nIndex)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    ::std::deque<AttacherIndex_Impl>::iterator aIt = implCheckIndex( nIndex );

    Sequence< ScriptEventDescriptor > aSeq( (*aIt).aEventList.size() );
    ScriptEventDescriptor * pArray = aSeq.getArray();

    ::std::deque< ScriptEventDescriptor >::iterator aEvtIt =    (*aIt).aEventList.begin();
    ::std::deque< ScriptEventDescriptor >::iterator aEvtEnd =   (*aIt).aEventList.end();
    sal_Int32 i = 0;
    while( aEvtIt != aEvtEnd )
    {
        pArray[i++] = *aEvtIt;
        ++aEvtIt;
    }
    return aSeq;
}


void SAL_CALL ImplEventAttacherManager::attach(sal_Int32 nIndex, const Reference< XInterface >& xObject, const Any & Helper)
    throw( IllegalArgumentException, ServiceNotRegisteredException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    if( nIndex < 0 || !xObject.is() )
        throw IllegalArgumentException();

    if( static_cast< ::std::deque< AttacherIndex_Impl >::size_type>(nIndex) >= aIndex.size() )
    {
        
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

    Sequence<com::sun::star::script::EventListener> aEvents(aCurrentPosition->aEventList.size());
    std::deque<ScriptEventDescriptor>::iterator itr = aCurrentPosition->aEventList.begin();
    std::deque<ScriptEventDescriptor>::iterator itrEnd = aCurrentPosition->aEventList.end();
    ::com::sun::star::script::EventListener* p = aEvents.getArray();
    size_t i = 0;
    for (; itr != itrEnd; ++itr)
    {
        com::sun::star::script::EventListener aListener;
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
        
    }
}


void SAL_CALL ImplEventAttacherManager::detach(sal_Int32 nIndex, const Reference< XInterface >& xObject)
    throw( IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    
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
                ++aEvtIt;
            }
            aCurrentPosition->aObjList.erase( aObjIt );
            break;
        }
        ++aObjIt;
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



OUString SAL_CALL ImplEventAttacherManager::getServiceName(void)
    throw( RuntimeException )
{
    return OUString( "com.sun.star.uno.script.EventAttacherManager" );
}

void SAL_CALL ImplEventAttacherManager::write(const Reference< XObjectOutputStream >& OutStream)
    throw( IOException, RuntimeException )
{
    Guard< Mutex > aGuard( aLock );
    
    Reference< XMarkableStream > xMarkStream( OutStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    
    OutStream->writeShort( 2 );

    
    sal_Int32 nObjLenMark = xMarkStream->createMark();
    OutStream->writeLong( 0L );

    OutStream->writeLong( aIndex.size() );

    
    ::std::deque<AttacherIndex_Impl>::iterator aIt = aIndex.begin();
    ::std::deque<AttacherIndex_Impl>::iterator aEnd = aIndex.end();
    while( aIt != aEnd )
    {
        
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

            ++aEvtIt;
        }
        ++aIt;
    }

    
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
    
    Reference< XMarkableStream > xMarkStream( InStream, UNO_QUERY );
    if( !xMarkStream.is() )
        return;

    
    nVersion = InStream->readShort();

    
    
    sal_Int32 nLen = InStream->readLong();

    
    sal_Int32 nObjLenMark = xMarkStream->createMark();

    
    sal_Int32 nItemCount = InStream->readLong();

    for( sal_Int32 i = 0 ; i < nItemCount ; i++ )
    {
        insertEntry( i );
        
        sal_Int32 nSeqLen = InStream->readLong();

        
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

    
    sal_Int32 nRealLen = xMarkStream->offsetToMark( nObjLenMark );
    if( nRealLen != nLen )
    {
        
        
        if( nRealLen > nLen || nVersion == 1 )
        {
            OSL_FAIL( "ImplEventAttacherManager::read(): Fatal Error, wrong object length" );
        }
        else
        {   
            
            sal_Int32 nSkipCount = nLen - nRealLen;
            InStream->skipBytes( nSkipCount );
        }
    }
    xMarkStream->jumpToFurthest();
    xMarkStream->deleteMark( nObjLenMark );
}

} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
