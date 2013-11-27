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

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/window.hxx>
#include <toolkit/awt/Vclxwindow.hxx>

#ifndef _SV_SYSDATA_HXX
#if defined( WIN ) || defined( WNT ) || defined( OS2 )
typedef sal_Int32 HWND;
typedef sal_Int32 HMENU;
typedef sal_Int32 HDC;
typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HFONT;
#endif
#include <vcl/sysdata.hxx>
#endif

#include "AccTopWindowListener.hxx"
#include "unomsaaevent.hxx"

#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::awt;
using namespace rtl;
using namespace cppu;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AccTopWindowListener* g_pTop = NULL;
//when proccess exit, call FreeTopWindowListener() in svmain
void FreeTopWindowListener()
{
    if( g_pTop )
    {
        g_pTop->release();
        g_pTop = NULL;
    }
}

/**
 *  As a global method to invoke the handleWindowOpened() method
 */
void handleWindowOpened_impl(long pAcc)
{
    if( g_pTop && pAcc != NULL )
        g_pTop->handleWindowOpened( (com::sun::star::accessibility::XAccessible*)((void*)pAcc) );
}

/**
 *  For the new opened window, generate all the UNO accessible's object, COM object and add
 *  accessible listener to monitor all these objects.
 *  @param pAccessible      the accessible of the new opened window
 */
void AccTopWindowListener::handleWindowOpened( com::sun::star::accessibility::XAccessible* pAccessible )
{
    //get SystemData from window
    VCLXWindow* pvclwindow = (VCLXWindow*)pAccessible;
    Window* window = pvclwindow->GetWindow();
    // The SalFrame of window may be destructed at this time
    const SystemEnvData* systemdata = NULL;
    try
    {
        systemdata = window->GetSystemData();
    }
    catch(...)
    {
        systemdata = NULL;
    }
    Reference<com::sun::star::accessibility::XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
    if(!xContext.is())
    {
        return;
    }
    com::sun::star::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    //Only AccessibleContext exist, add all listeners
    if(pAccessibleContext != NULL && systemdata != NULL)
    {
        accManagerAgent.SaveTopWindowHandle((HWND)systemdata->hWnd,  pAccessible);

        AddAllListeners(pAccessible,NULL,(HWND)systemdata->hWnd);

        if( window->GetStyle() & WB_MOVEABLE )
            accManagerAgent.IncreaseState( pAccessible, -1 /* U_MOVEBLE */ );

        short role = pAccessibleContext->getAccessibleRole();


        if (role == com::sun::star::accessibility::AccessibleRole::POPUP_MENU ||
                role == com::sun::star::accessibility::AccessibleRole::MENU )
        {
            accManagerAgent.NotifyAccEvent(UM_EVENT_MENUPOPUPSTART, pAccessible);
        }

        if (role == com::sun::star::accessibility::AccessibleRole::FRAME ||
                role == com::sun::star::accessibility::AccessibleRole::DIALOG ||
                role == com::sun::star::accessibility::AccessibleRole::WINDOW ||
                role == com::sun::star::accessibility::AccessibleRole::ALERT)
        {
            accManagerAgent.NotifyAccEvent(UM_EVENT_SHOW, pAccessible);
        }
    }
}

AccTopWindowListener::AccTopWindowListener():
        accManagerAgent(),
        m_refcount(1)
{
}

AccTopWindowListener::~AccTopWindowListener()
{
}

/**
 *  It is invoked when a new window is opened, the source of this EventObject is the window
 */
void AccTopWindowListener::windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !e.Source.is())
    {
        return;
    }

    Reference< com::sun::star::accessibility::XAccessible > xAccessible ( e.Source, UNO_QUERY );
    com::sun::star::accessibility::XAccessible* pAccessible = xAccessible.get();
    if ( pAccessible == NULL)
    {
        return;
    }

    handleWindowOpened(pAccessible);

}

/**
 *  Add the accessible event listener to object and all its children objects.
 *  @param  pAccessible     the accessible object
 *  @param  pParentXAcc     the parent of current accessible object
 *  @param  pWND            the handle of top window which current object resides
 */
void AccTopWindowListener::AddAllListeners(com::sun::star::accessibility::XAccessible* pAccessible, com::sun::star::accessibility::XAccessible* pParentXAcc, HWND pWND)
{
    Reference<com::sun::star::accessibility::XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
    if(!xContext.is())
    {
        return;
    }
    com::sun::star::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    if(pAccessibleContext == NULL)
    {
        return;
    }

    accManagerAgent.InsertAccObj( pAccessible, pParentXAcc,pWND );

    if (!accManagerAgent.IsContainer(pAccessible))
    {
        return;
    }


    short role = pAccessibleContext->getAccessibleRole();
    if(com::sun::star::accessibility::AccessibleRole::DOCUMENT == role )
    {
        if(accManagerAgent.IsStateManageDescendant(pAccessible))
        {
            return ;
        }
    }


    int count = pAccessibleContext->getAccessibleChildCount();
    for (int i=0;i<count;i++)
    {
        Reference<com::sun::star::accessibility::XAccessible> mxAccessible
        = pAccessibleContext->getAccessibleChild(i);

        com::sun::star::accessibility::XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != NULL)
        {
            Reference<com::sun::star::accessibility::XAccessibleContext> mxAccessibleContext
            = mpAccessible->getAccessibleContext();
            com::sun::star::accessibility::XAccessibleContext* mpContext = mxAccessibleContext.get();
            if(mpContext != NULL)
            {
                //fprintf(output, "go on add child's children event listener\n");
                AddAllListeners(mpAccessible,pAccessible,pWND);
            }
        }
    }
}

void AccTopWindowListener::windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException)
{
}

/**
 *  Invoke this method when the top window is closed, remove all the objects and its children
 *  from current manager's cache, and remove the COM object and the accessible event listener
 *  assigned to the accessible objects.
 */
void AccTopWindowListener::windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !e.Source.is())
    {
        return;
    }
    Reference< com::sun::star::accessibility::XAccessible > xAccessible ( e.Source, UNO_QUERY );
    com::sun::star::accessibility::XAccessible* pAccessible = xAccessible.get();
    if ( pAccessible == NULL)
    {
        return;
    }


    VCLXWindow* pvclwindow = (VCLXWindow*)pAccessible;
    Window* window = pvclwindow->GetWindow();
    const SystemEnvData* systemdata=window->GetSystemData();

    Reference<com::sun::star::accessibility::XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
    if(!xContext.is())
    {
        return;
    }
    com::sun::star::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();

    short role = -1;
    if(pAccessibleContext != NULL)
    {
        role = pAccessibleContext->getAccessibleRole();

        if (role == com::sun::star::accessibility::AccessibleRole::POPUP_MENU ||
                role == com::sun::star::accessibility::AccessibleRole::MENU)
        {
            accManagerAgent.NotifyAccEvent(UM_EVENT_MENUPOPUPEND, pAccessible);
        }
    }


    accManagerAgent.DeleteChildrenAccObj( pAccessible );
    if( role != com::sun::star::accessibility::AccessibleRole::POPUP_MENU )
        accManagerAgent.DeleteAccObj( pAccessible );

}

void AccTopWindowListener::windowMinimized( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void AccTopWindowListener::windowNormalized( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void AccTopWindowListener::windowActivated( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void AccTopWindowListener::windowDeactivated( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
}

void AccTopWindowListener::disposing( const ::com::sun::star::lang::EventObject&  ) throw (::com::sun::star::uno::RuntimeException)
{
}

//need to investigate further
::com::sun::star::uno::Any SAL_CALL AccTopWindowListener::queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException)
{
    if (aType.equals( ::getCppuType( (Reference< com::sun::star::awt::XTopWindowListener> const *)0 ) ))
    {
        Reference< com::sun::star::awt::XTopWindowListener> xTopListener( static_cast< com::sun::star::awt::XTopWindowListener* >(this));
        return makeAny(xTopListener);
    }
    return Any();
}

void AccTopWindowListener::acquire( ) throw ()
{
    ::osl_incrementInterlockedCount( &m_refcount );
}

void AccTopWindowListener::release() throw ()
{
    // thread-safe decrementation of reference count
    if (0 == ::osl_decrementInterlockedCount( &m_refcount ))
    {
        delete this; // shutdown this object
    }
}
