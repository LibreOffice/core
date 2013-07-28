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
#ifndef _CHART_CONTROLLER_HXX
#define _CHART_CONTROLLER_HXX

#include "LifeTime.hxx"
#include "ServiceMacros.hxx"
#include "CommandDispatchContainer.hxx"
#include "SelectionHelper.hxx"

// header for enum SdrDragMode
#include <svx/svdtypes.hxx>
// header for class Timer
#include <vcl/timer.hxx>
// header for class MouseEvent
#include <vcl/event.hxx>

#include <cppuhelper/implbase12.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>

#include <memory>
#include <boost/shared_ptr.hpp>
#include <set>

class SdrModel;

namespace svt
{
    class AcceleratorExecute;
}

class DropTargetHelper;

namespace com { namespace sun { namespace star {
namespace graphic {
    class XGraphic;
}
}}}

namespace chart
{

class UndoGuard;

enum ChartDrawMode { CHARTDRAW_INSERT, CHARTDRAW_SELECT };

class WindowController
{
public:
    virtual ~WindowController() {};

    virtual void PrePaint()=0;
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

    /** get help text to be shown in a quick help

        @param aAtLogicPosition the position in logic coordinates (of the
                                window) of the mouse cursor to determine for
                                which object help is requested.

        @param bIsBalloonHelp determines whether to return the long text version
                              (balloon help) or the shorter one (quick help).

        @param rOutQuickHelpText is filled with the quick help text

        @param rOutEqualRect is filled with a rectangle that denotes the region
                             in which the quick help does not change.

        @return </sal_True>, if a quick help should be shown.
     */
    virtual bool requestQuickHelp(
        ::Point aAtLogicPosition, bool bIsBalloonHelp,
        OUString & rOutQuickHelpText, ::com::sun::star::awt::Rectangle & rOutEqualRect ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() = 0;
};

class ChartWindow;
class DrawModelWrapper;
class DrawViewWrapper;
class ReferenceSizeProvider;

class ChartController   : public ::cppu::WeakImplHelper12 <
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
        ,::com::sun::star::lang::XMultiServiceFactory
        ,::com::sun::star::util::XModifyListener
        ,::com::sun::star::util::XModeChangeListener
        ,::com::sun::star::frame::XLayoutManagerListener
        >
        , public WindowController
{
    friend class DrawCommandDispatch;
    friend class ShapeController;

public:
    //no default constructor
    ChartController(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartController();

    // ::com::sun::star::lang::XServiceInfo

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartController)

    // ::com::sun::star::frame::XController (required interface)
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

    // ::com::sun::star::lang::XComponent (base of XController)
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

    // ::com::sun::star::frame::XDispatchProvider (required interface)
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> SAL_CALL
        queryDispatch( const ::com::sun::star::util::URL& rURL
                            , const OUString& rTargetFrameName
                            , sal_Int32 nSearchFlags)
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch > > SAL_CALL
        queryDispatches( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::frame::DispatchDescriptor > & xDescripts)
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::view::XSelectionSupplier (optional interface)
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

    // ::com::sun::star::ui::XContextMenuInterception (optional interface)
    virtual void SAL_CALL
        registerContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        releaseContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException);

    //additional interfaces

    // ::com::sun::star::util::XCloseListener
    virtual void SAL_CALL
        queryClosing( const ::com::sun::star::lang::EventObject& Source
                            , sal_Bool GetsOwnership )
                            throw (::com::sun::star::util::CloseVetoException
                                 , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        notifyClosing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XDispatch

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

    // ::com::sun::star::awt::XWindow
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

    // ::com::sun::star::lang XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstance( const OUString& aServiceSpecifier )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments( const OUString& ServiceSpecifier,
                                     const ::com::sun::star::uno::Sequence<
                                         ::com::sun::star::uno::Any >& Arguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
            throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModeChangeListener
    virtual void SAL_CALL modeChanged(
        const ::com::sun::star::util::ModeChangeEvent& _rSource )
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XLayoutManagerListener
    virtual void SAL_CALL layoutEvent(
        const ::com::sun::star::lang::EventObject& aSource,
        ::sal_Int16 eLayoutEvent,
        const ::com::sun::star::uno::Any& aInfo )
        throw (::com::sun::star::uno::RuntimeException);

    // chart2::WindowController
    virtual void PrePaint();
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

    virtual bool requestQuickHelp(
        ::Point aAtLogicPosition, bool bIsBalloonHelp,
        OUString & rOutQuickHelpText, ::com::sun::star::awt::Rectangle & rOutEqualRect );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    static bool isObjectDeleteable( const ::com::sun::star::uno::Any& rSelection );

    void setDrawMode( ChartDrawMode eMode ) { m_eDrawMode = eMode; }
    ChartDrawMode getDrawMode() const { return m_eDrawMode; }

    bool isShapeContext() const;

    DECL_LINK( NotifyUndoActionHdl, SdrUndoAction* );

public:
    //private

private:
    DrawModelWrapper* GetDrawModelWrapper();
    DrawViewWrapper* GetDrawViewWrapper();

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

            void        SetOwnership( sal_Bool bGetsOwnership );
            void        addListener( ChartController* pController );
            void        removeListener(  ChartController* pController );
            void        tryTermination();
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        getModel() const { return m_xModel;}

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
            ::osl::Mutex&   m_rModelMutex;
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
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XInterface >                  m_xChartView;
    ::boost::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;
    DrawViewWrapper*    m_pDrawViewWrapper;

    Selection           m_aSelection;
    SdrDragMode         m_eDragMode;

    Timer               m_aDoubleClickTimer;
    bool volatile       m_bWaitingForDoubleClick;
    bool volatile       m_bWaitingForMouseUp;

    bool volatile       m_bConnectingToView;

    ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager >    m_xUndoManager;
    ::std::auto_ptr< UndoGuard >                                                    m_pTextActionUndoGuard;
    /// needed for dispatching URLs in FeatureStateEvents
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;

    ::std::auto_ptr< ::svt::AcceleratorExecute >  m_apAccelExecute;

    CommandDispatchContainer m_aDispatchContainer;

    ::std::auto_ptr< DropTargetHelper >           m_apDropTargetHelper;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XLayoutManagerEventBroadcaster >
                                                  m_xLayoutManagerEventBroadcaster;

    ChartDrawMode       m_eDrawMode;

private:
    //private methods

    sal_Bool            impl_isDisposedOrSuspended() const;
    ::std::auto_ptr< ReferenceSizeProvider > impl_createReferenceSizeProvider();
    void                impl_adaptDataSeriesAutoResize();

    void                impl_createDrawViewController();
    void                impl_deleteDrawViewController();

    //executeDispatch methods
    void SAL_CALL       executeDispatch_ObjectProperties();
    void SAL_CALL       executeDispatch_FormatObject( const OUString& rDispatchCommand );
    void SAL_CALL       executeDlg_ObjectProperties( const OUString& rObjectCID );
    bool                executeDlg_ObjectProperties_withoutUndoGuard( const OUString& rObjectCID, bool bOkClickOnUnchangedDialogSouldBeRatedAsSuccessAlso );

    void SAL_CALL       executeDispatch_ChartType();

    void                executeDispatch_InsertTitles();
    void                executeDispatch_InsertLegend();
    void                executeDispatch_DeleteLegend();
    void                executeDispatch_OpenLegendDialog();
    void                executeDispatch_InsertAxes();
    void                executeDispatch_InsertGrid();

    void                executeDispatch_InsertMenu_DataLabels();
    void                executeDispatch_InsertMenu_Trendlines();
    void                executeDispatch_InsertMenu_MeanValues();

    void                executeDispatch_InsertMeanValue();
    void                executeDispatch_InsertTrendline();
    void                executeDispatch_InsertTrendlineEquation( bool bInsertR2=false );
    void                executeDispatch_InsertErrorBars( bool bYError );

    void                executeDispatch_InsertR2Value();
    void                executeDispatch_DeleteR2Value();

    void                executeDispatch_DeleteMeanValue();
    void                executeDispatch_DeleteTrendline();
    void                executeDispatch_DeleteTrendlineEquation();
    void                executeDispatch_DeleteErrorBars( bool bYError );

    void                executeDispatch_InsertDataLabels();
    void                executeDispatch_InsertDataLabel();
    void                executeDispatch_DeleteDataLabels();
    void                executeDispatch_DeleteDataLabel();

    void                executeDispatch_ResetAllDataPoints();
    void                executeDispatch_ResetDataPoint();

    void                executeDispatch_InsertAxis();
    void                executeDispatch_InsertAxisTitle();
    void                executeDispatch_InsertMajorGrid();
    void                executeDispatch_InsertMinorGrid();
    void                executeDispatch_DeleteAxis();
    void                executeDispatch_DeleteMajorGrid();
    void                executeDispatch_DeleteMinorGrid();

    void SAL_CALL       executeDispatch_InsertSpecialCharacter();
    void SAL_CALL       executeDispatch_EditText( const Point* pMousePixel = NULL );
    void SAL_CALL       executeDispatch_SourceData();
    void SAL_CALL       executeDispatch_MoveSeries( sal_Bool bForward );

    void                StartTextEdit( const Point* pMousePixel = NULL );
    bool                EndTextEdit();

    void SAL_CALL       executeDispatch_View3D();
    void SAL_CALL       executeDispatch_PositionAndSize();

    void                executeDispatch_EditData();

    void                executeDispatch_NewArrangement();
    void                executeDispatch_ScaleText();

    void                executeDispatch_Paste();
    void                executeDispatch_Copy();
    void                executeDispatch_Cut();
    bool                executeDispatch_Delete();
    void                executeDispatch_ToggleLegend();
    void                executeDispatch_ToggleGridHorizontal();
    void                executeDispatch_ToggleGridVertical();

    void impl_ShapeControllerDispatch( const ::com::sun::star::util::URL& rURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    DECL_LINK( DoubleClickWaitingHdl, void* );
    void execute_DoubleClick( const Point* pMousePixel = NULL );
    void startDoubleClickWaiting();
    void stopDoubleClickWaiting();

    void impl_selectObjectAndNotiy();
    void impl_notifySelectionChangeListeners();
    void impl_invalidateAccessible();
    void impl_initializeAccessible();
    void impl_initializeAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XInitialization >& xInit );

    //sets the model member to null if it equals the parameter
    //returns true if successful
    bool impl_releaseThisModel( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XInterface > & xModel );

    enum eMoveOrResizeType
    {
        MOVE_OBJECT,
        CENTERED_RESIZE_OBJECT
    };
    /// @return </sal_True>, if resize/move was successful
    bool impl_moveOrResizeObject(
        const OUString & rCID, eMoveOrResizeType eType, double fAmountLogicX, double fAmountLogicY );
    bool impl_DragDataPoint( const OUString & rCID, double fOffset );

    ::std::set< OUString > impl_getAvailableCommands();

    /** Creates a helper accesibility class that must be initialized via XInitialization.  For
        parameters see

        The returned object should not be used directly.  Instead a proxy object
        should use this helper to retrieve its children and add them to its own
        children.
     */
    ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleContext >
        impl_createAccessibleTextContext();

    void impl_PasteGraphic( ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > & xGraphic,
                            const ::Point & aPosition );
    void impl_PasteShapes( SdrModel* pModel );
    void impl_PasteStringAsTextShape( const OUString& rString, const ::com::sun::star::awt::Point& rPosition );
    void impl_SetMousePointer( const MouseEvent & rEvent );

    void impl_ClearSelection();

    void impl_switchDiagramPositioningToExcludingPositioning();
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
