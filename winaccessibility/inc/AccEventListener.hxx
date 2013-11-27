/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef __ACCEVENTLISTENER_HXX
#define __ACCEVENTLISTENER_HXX

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weak.hxx>
#include <vos/mutex.hxx>

class AccObjectManagerAgent;
using namespace ::com::sun::star::uno;
/**
 * AccEventListener is the general event listener for all controls. It defines the
 * procedure of all the event handling and provides the basic support for some simple
 * methods.
 */
class AccEventListener:
            public com::sun::star::accessibility::XAccessibleEventListener,
            public ::cppu::OWeakObject
{
private:
    oslInterlockedCount m_refcount;
protected:
    //accessible owner's pointer
    com::sun::star::accessibility::XAccessible* pAccessible;
    //agent pointer for objects' manager
    AccObjectManagerAgent* pAgent;
    //disposed state indicator
    bool  m_isDisposed;
    mutable ::vos::OMutex aRemoveMutex;
public:
    AccEventListener( com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent);
    virtual ~AccEventListener();

    //AccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    //for name changed event
    virtual void SAL_CALL handleNameChangedEvent(Any name);

    //for description changed event
    virtual void SAL_CALL handleDescriptionChangedEvent(Any desc);

    //for state changed event
    virtual void SAL_CALL handleStateChangedEvent (Any oldValue, Any newValue);
    virtual void SAL_CALL setComponentState(short state, bool enable);
    virtual void SAL_CALL fireStatePropertyChange(short state, bool set
                                                     );
    virtual void SAL_CALL fireStateFocusdChange(bool enable);

    //for bound rect changed event
    virtual void SAL_CALL handleBoundrectChangedEvent();

    //for visible data changed event
    virtual void SAL_CALL handleVisibleDataChangedEvent();

    //for interface
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    //get the accessible role of pAccessible
    virtual short SAL_CALL getRole();
    //get the accessible parent's role
    virtual short SAL_CALL getParentRole();
public:
    void removeMeFromBroadcaster();
};

#endif
