/*************************************************************************
 *
 *  $RCSfile: ChartController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-10-28 15:49:04 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART_CONTROLLER_HXX
#define _CHART_CONTROLLER_HXX

#include "LifeTime.hxx"
#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XCONTEXTMENUINTERCEPTION_HPP_
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase8.hxx>
#endif

// header for class MouseEvent
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif

//=============================================================================
/** this is an example implementation for the service ::com::sun::star::frame::Controller
*/

//.............................................................................
namespace chart
{
//.............................................................................

class WindowController
{
public:
    virtual void execute_Paint( const Rectangle& rRect )=0;
    virtual void execute_MouseButtonDown( const MouseEvent& rMEvt )=0;
    virtual void execute_MouseMove( const MouseEvent& rMEvt )=0;
    virtual void execute_Tracking( const TrackingEvent& rTEvt )=0;
    virtual void execute_MouseButtonUp( const MouseEvent& rMEvt )=0;
    virtual void execute_Resize()=0;
    virtual void execute_Activate()=0;
    virtual void execute_Deactivate()=0;
    virtual void execute_GetFocus()=0;
    virtual void execute_LoseFocus()=0;
    virtual void execute_Command( const CommandEvent& rCEvt )=0;
    virtual bool execute_KeyInput( const KeyEvent& rKEvt )=0;
};

class ChartWindow;
class ChartView;
class DrawModelWrapper;
class DrawViewWrapper;
class NumberFormatterWrapper;
class ChartController   : public ::cppu::WeakImplHelper8 <
         ::com::sun::star::frame::XController   //comprehends XComponent (required interface)
        ,::com::sun::star::frame::XDispatchProvider     //(required interface)
        ,::com::sun::star::view::XSelectionSupplier     //(optional interface)
        ,::com::sun::star::ui::XContextMenuInterception //(optional interface)
        ,::com::sun::star::util::XCloseListener         //(needed for communication with XModel)
        ,::com::sun::star::lang::XServiceInfo
    //  ,public ::com::sun::star::uno::XWeak            // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::uno::XInterface       // implemented by WeakImplHelper(optional interface)
    //  ,public ::com::sun::star::lang::XTypeProvider   // implemented by WeakImplHelper
        ,::com::sun::star::frame::XDispatch
        ,::com::sun::star::awt::XWindow //this is the Window Controller part of this Controller, that will be given to a Frame via setComponent
        >
        , public WindowController
{

private:
    class TheModelRef;
    friend class ChartController::TheModelRef;
    class RefCountable
    {
        public:
            RefCountable();
            virtual ~RefCountable();
            void acquire();
            void release();
        private:
            sal_Int32 volatile      m_nRefCount;
    };
    class TheModel : public RefCountable
    {
        public:
            TheModel( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel > & xModel );

            virtual ~TheModel();

            void        SetOwnerShip( sal_Bool bGetsOwnership );
            void        addListener( ChartController* pController );
            void        removeListener(  ChartController* pController );
            void        tryTermination();
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        getModel() { return m_xModel;}

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >     m_xModel;
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >  m_xCloseable;

            //the ownership between model and controller is not clear at first
            //each controller might consider himself as owner of the model first
            sal_Bool volatile       m_bOwnership;
            //with a XCloseable::close call and during XCloseListener::queryClosing
            //the ownership can be regulated more explicit,
            //if so the ownership is considered to be well known
            sal_Bool volatile       m_bOwnershipIsWellKnown;
    };
    class TheModelRef
    {
        public:
            TheModelRef( TheModel* pTheModel, ::osl::Mutex& rMutex );
            TheModelRef( const TheModelRef& rTheModel, ::osl::Mutex& rMutex );
            TheModelRef& operator=(ChartController::TheModel* pTheModel);
            TheModelRef& operator=(const TheModelRef& rTheModel);
            virtual ~TheModelRef();
            sal_Bool is() const;
                TheModel* operator->() const { return m_pTheModel; }
        private:
            TheModel*               m_pTheModel;
            mutable ::osl::Mutex&   m_rModelMutex;
    };

private:
    mutable ::apphelper::LifeTimeManager    m_aLifeTimeManager;

    mutable ::osl::Mutex    m_aControllerMutex;
    sal_Bool volatile       m_bSuspended;
    sal_Bool volatile       m_bCanClose;


    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;

    //model
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >     m_xFrame;
    mutable ::osl::Mutex    m_aModelMutex;
    TheModelRef             m_aModel;

    //view
    ChartWindow*        m_pChartWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xViewWindow;
    ChartView*          m_pChartView;
    DrawModelWrapper*   m_pDrawModelWrapper;
    DrawViewWrapper*    m_pDrawViewWrapper;

    ::rtl::OUString     m_aSelectedObjectCID;//only single object selection so far

    bool                m_bViewDirty; //states wether the view needs to be rebuild

    NumberFormatterWrapper*         m_pNumberFormatterWrapper;

private:
    //private methods

    sal_Bool            impl_isDisposedOrSuspended();

    sal_Bool SAL_CALL   impl_tryInitializeView() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL       impl_deleteView() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL       impl_rebuildView() throw(::com::sun::star::uno::RuntimeException);

    //executeDispatch methods
    void SAL_CALL       executeDispatch_ObjectProperties();
    void SAL_CALL       executeDispatch_FormatObject(sal_Int32 nSlotId);
    void SAL_CALL       executeDlg_ObjectProperties( const ::rtl::OUString& rObjectCID );

    void SAL_CALL       executeDispatch_ChartType();
    void SAL_CALL       executeDispatch_ObjectToDefault();

    void SAL_CALL       executeDispatch_InsertTitle();
    void SAL_CALL       executeDispatch_InsertLegend();
    void SAL_CALL       executeDispatch_InsertDataLabel();
    void SAL_CALL       executeDispatch_InsertAxis();
    void SAL_CALL       executeDispatch_InsertGrid();
    void SAL_CALL       executeDispatch_InsertStatistic();

    void SAL_CALL       executeDispatch_InsertSpecialCharacter();
    void SAL_CALL       executeDispatch_EditText();

    void                StartTextEdit();
    bool                EndTextEdit();

    //
    void execute_DoubleClick();


public:
    //no default constructor
    ChartController(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartController();

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XServiceInfo
    //-----------------------------------------------------------------

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartController)

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XController (required interface)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        attachFrame( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::frame::XFrame > & xFrame )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        attachModel( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel > & xModel )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL
        getFrame()          throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL
        getModel()          throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL
        getViewData()       throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        restoreViewData( const ::com::sun::star::uno::Any& rValue )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        suspend( sal_Bool bSuspend )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XComponent (base of XController)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        dispose()           throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XDispatchProvider (required interface)
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> SAL_CALL
        queryDispatch( const ::com::sun::star::util::URL& rURL
                            , const rtl::OUString& rTargetFrameName
                            , sal_Int32 nSearchFlags)
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch > > SAL_CALL
        queryDispatches( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::frame::DispatchDescriptor > & xDescripts)
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::view::XSelectionSupplier (optional interface)
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
        select( const ::com::sun::star::uno::Any& rSelection )
                            throw ( com::sun::star::lang::IllegalArgumentException );

    virtual ::com::sun::star::uno::Any  SAL_CALL
        getSelection() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addSelectionChangeListener( const ::com::sun::star::uno::Reference<
                            com::sun::star::view::XSelectionChangeListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                            com::sun::star::view::XSelectionChangeListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::ui::XContextMenuInterception (optional interface)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        registerContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        releaseContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException);


    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    //additional interfaces

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XCloseListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        queryClosing( const ::com::sun::star::lang::EventObject& Source
                            , sal_Bool GetsOwnership )
                            throw (::com::sun::star::util::CloseVetoException
                                 , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        notifyClosing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XEventListener (base of XCloseListener)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XDispatch
    //-----------------------------------------------------------------

    virtual void SAL_CALL
        dispatch( const ::com::sun::star::util::URL& aURL
                    , const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& aArgs )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addStatusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XStatusListener >& xControl
                    , const ::com::sun::star::util::URL& aURL )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeStatusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XStatusListener >& xControl
                    , const ::com::sun::star::util::URL& aURL )
                    throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::awt::XWindow
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        setPosSize( sal_Int32 X, sal_Int32 Y
                    , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
        getPosSize()
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setVisible( sal_Bool Visible )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setEnable( sal_Bool Enable )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setFocus()  throw (::com::sun::star::uno::RuntimeException);

    //----------------
    virtual void SAL_CALL
        addWindowListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XWindowListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeWindowListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XWindowListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addFocusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XFocusListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeFocusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XFocusListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addKeyListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XKeyListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeKeyListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XKeyListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addMouseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeMouseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addMouseMotionListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseMotionListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeMouseMotionListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseMotionListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addPaintListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XPaintListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removePaintListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XPaintListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // chart2::WindowController
    //-----------------------------------------------------------------
    virtual void execute_Paint( const Rectangle& rRect );
    virtual void execute_MouseButtonDown( const MouseEvent& rMEvt );
    virtual void execute_MouseMove( const MouseEvent& rMEvt );
    virtual void execute_Tracking( const TrackingEvent& rTEvt );
    virtual void execute_MouseButtonUp( const MouseEvent& rMEvt );
    virtual void execute_Resize();
    virtual void execute_Activate();
    virtual void execute_Deactivate();
    virtual void execute_GetFocus();
    virtual void execute_LoseFocus();
    virtual void execute_Command( const CommandEvent& rCEvt );
    virtual bool execute_KeyInput( const KeyEvent& rKEvt );
    //-----------------------------------------------------------------
};

//.............................................................................
}  // namespace chart
//.............................................................................

#endif

