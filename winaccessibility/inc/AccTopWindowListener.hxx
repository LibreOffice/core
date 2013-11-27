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

#ifndef __ACCTOPWINDOWLISTENER_HXX
#define __ACCTOPWINDOWLISTENER_HXX

#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <cppuhelper/weak.hxx>

#include  "AccObjectManagerAgent.hxx"

/**
 * AccEventListener is the general event listener for all top windows. The top windows defined
 * in UNO are: FRAME, WINDOW, DIALOG, MENU, TOOLTIP.
 * It implements the methods of XTopWindowListener and the most important method is windowOpened().
 * In this method, all the accessible objects (including COM object and Uno objects) are created and
 * cached into bridge managers, and they are monitored by listeners for later accessible evnet handling.
 */
class AccTopWindowListener:
            public com::sun::star::awt::XTopWindowListener,
            public ::cppu::OWeakObject

{
private:
    AccObjectManagerAgent accManagerAgent;
    oslInterlockedCount m_refcount;
public:
    AccTopWindowListener();
    virtual ~AccTopWindowListener();
    virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();
    virtual void AddAllListeners(com::sun::star::accessibility::XAccessible* pAccessible,com::sun::star::accessibility::XAccessible* pParentXAcc,HWND pWND );
    //for On-Demand load.
    virtual void handleWindowOpened( com::sun::star::accessibility::XAccessible* pAccessible );
};

#endif
