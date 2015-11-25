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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTCONTROLLER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTCONTROLLER_HXX

#include "LifeTime.hxx"
#include "CommandDispatchContainer.hxx"
#include "SelectionHelper.hxx"

#include <svx/svdtypes.hxx>
#include <vcl/timer.hxx>
#include <vcl/event.hxx>

#include <cppuhelper/implbase.hxx>

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
#include <set>

class SdrModel;

namespace svt
{
    class AcceleratorExecute;
}

namespace svx { namespace sidebar {
    class SelectionChangeHandler;
}}

class DropTargetHelper;

namespace com { namespace sun { namespace star {
namespace graphic {
    class XGraphic;
}
}}}

namespace chart
{

class UndoGuard;
class ChartWindow;
class DrawModelWrapper;
class DrawViewWrapper;
class ReferenceSizeProvider;
class ViewElementListProvider;

enum ChartDrawMode { CHARTDRAW_INSERT, CHARTDRAW_SELECT };


class ChartController   : public ::cppu::WeakImplHelper <
         ::com::sun::star::frame::XController   //comprehends XComponent (required interface)
        ,::com::sun::star::frame::XDispatchProvider     //(required interface)
        ,::com::sun::star::view::XSelectionSupplier     //(optional interface)
        ,::com::sun::star::ui::XContextMenuInterception //(optional interface)
        ,::com::sun::star::util::XCloseListener         //(needed for communication with XModel)
        ,::com::sun::star::lang::XServiceInfo
        ,::com::sun::star::frame::XDispatch
        ,::com::sun::star::awt::XWindow //this is the Window Controller part of this Controller, that will be given to a Frame via setComponent
        ,::com::sun::star::lang::XMultiServiceFactory
        ,::com::sun::star::util::XModifyListener
        ,::com::sun::star::util::XModeChangeListener
        ,::com::sun::star::frame::XLayoutManagerListener
        >
{
    friend class DrawCommandDispatch;
    friend class ShapeController;

public:
    //no default constructor
    explicit ChartController(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartController();

    OUString GetContextName();

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ::com::sun::star::frame::XController (required interface)
    virtual void SAL_CALL
        attachFrame( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::frame::XFrame > & xFrame )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        attachModel( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel > & xModel )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL
        getFrame()          throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL
        getModel()          throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL
        getViewData()       throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        restoreViewData( const ::com::sun::star::uno::Any& rValue )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        suspend( sal_Bool bSuspend )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XComponent (base of XController)
    virtual void SAL_CALL
        dispose()           throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::frame::XDispatchProvider (required interface)
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> SAL_CALL
        queryDispatch( const ::com::sun::star::util::URL& rURL
                            , const OUString& rTargetFrameName
                            , sal_Int32 nSearchFlags)
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch > > SAL_CALL
        queryDispatches( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::frame::DispatchDescriptor > & xDescripts)
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::view::XSelectionSupplier (optional interface)
    virtual sal_Bool SAL_CALL
        select( const ::com::sun::star::uno::Any& rSelection )
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any  SAL_CALL
        getSelection() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addSelectionChangeListener( const ::com::sun::star::uno::Reference<
                            com::sun::star::view::XSelectionChangeListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeSelectionChangeListener( const ::com::sun::star::uno::Reference<
                            com::sun::star::view::XSelectionChangeListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::ui::XContextMenuInterception (optional interface)
    virtual void SAL_CALL
        registerContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        releaseContextMenuInterceptor( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::ui::XContextMenuInterceptor > & xInterceptor)
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //additional interfaces

    // ::com::sun::star::util::XCloseListener
    virtual void SAL_CALL
        queryClosing( const ::com::sun::star::lang::EventObject& Source
                            , sal_Bool GetsOwnership )
                            throw (::com::sun::star::util::CloseVetoException
                                 , ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        notifyClosing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
                            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::frame::XDispatch

    virtual void SAL_CALL
        dispatch( const ::com::sun::star::util::URL& aURL
                    , const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue >& aArgs )
                    throw (::com::sun::star::uno::RuntimeException,
                           std::exception) override;

    virtual void SAL_CALL
        addStatusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XStatusListener >& xControl
                    , const ::com::sun::star::util::URL& aURL )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeStatusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XStatusListener >& xControl
                    , const ::com::sun::star::util::URL& aURL )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XWindow
    virtual void SAL_CALL
        setPosSize( sal_Int32 X, sal_Int32 Y
                    , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
        getPosSize()
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        setVisible( sal_Bool Visible )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        setEnable( sal_Bool Enable )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        setFocus()  throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addWindowListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XWindowListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeWindowListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XWindowListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addFocusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XFocusListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeFocusListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XFocusListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addKeyListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XKeyListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeKeyListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XKeyListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addMouseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeMouseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addMouseMotionListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseMotionListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeMouseMotionListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XMouseMotionListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        addPaintListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XPaintListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removePaintListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::awt::XPaintListener >& xListener )
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstance( const OUString& aServiceSpecifier )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments( const OUString& ServiceSpecifier,
                                     const ::com::sun::star::uno::Sequence<
                                         ::com::sun::star::uno::Any >& Arguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::util::XModeChangeListener
    virtual void SAL_CALL modeChanged(
        const ::com::sun::star::util::ModeChangeEvent& _rSource )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    // ::com::sun::star::frame::XLayoutManagerListener
    virtual void SAL_CALL layoutEvent(
        const ::com::sun::star::lang::EventObject& aSource,
        ::sal_Int16 eLayoutEvent,
        const ::com::sun::star::uno::Any& aInfo )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // WindowController stuff
    void PrePaint(vcl::RenderContext& rRenderContext);
    void execute_Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    void execute_MouseButtonDown( const MouseEvent& rMEvt );
    void execute_MouseMove( const MouseEvent& rMEvt );
    void execute_MouseButtonUp( const MouseEvent& rMEvt );
    void execute_Resize();
    void execute_Command( const CommandEvent& rCEvt );
    bool execute_KeyInput( const KeyEvent& rKEvt );

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
    bool requestQuickHelp(
        ::Point aAtLogicPosition, bool bIsBalloonHelp,
        OUString & rOutQuickHelpText, ::com::sun::star::awt::Rectangle & rOutEqualRect );

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    static bool isObjectDeleteable( const ::com::sun::star::uno::Any& rSelection );

    void setDrawMode( ChartDrawMode eMode ) { m_eDrawMode = eMode; }

    bool isShapeContext() const;

    ViewElementListProvider getViewElementListProvider();
    DrawModelWrapper* GetDrawModelWrapper();

    DECL_LINK_TYPED( NotifyUndoActionHdl, SdrUndoAction*, void );

public:
    //private

private:
    DrawViewWrapper* GetDrawViewWrapper();

private:
    class TheModelRef;
    friend class ChartController::TheModelRef;
    class TheModel : public salhelper::SimpleReferenceObject
    {
        public:
            explicit TheModel( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel > & xModel );

            virtual ~TheModel();

            void        SetOwnership( bool bGetsOwnership );
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
            bool m_bOwnership;
    };
    class TheModelRef
    {
        public:
            TheModelRef( TheModel* pTheModel, ::osl::Mutex& rMutex );
            TheModelRef( const TheModelRef& rTheModel, ::osl::Mutex& rMutex );
            TheModelRef& operator=(ChartController::TheModel* pTheModel);
            TheModelRef& operator=(const TheModelRef& rTheModel);
            virtual ~TheModelRef();
            bool is() const;
                TheModel* operator->() const { return m_pTheModel; }
        private:
            TheModel*               m_pTheModel;
            ::osl::Mutex&   m_rModelMutex;
    };

private:
    mutable ::apphelper::LifeTimeManager m_aLifeTimeManager;

    bool m_bSuspended;
    bool m_bCanClose;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xCC;

    //model
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    mutable ::osl::Mutex m_aModelMutex;
    TheModelRef m_aModel;

    //view
    VclPtr<ChartWindow> m_pChartWindow;
    css::uno::Reference<css::awt::XWindow> m_xViewWindow;
    css::uno::Reference<css::uno::XInterface> m_xChartView;
    std::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;
    DrawViewWrapper* m_pDrawViewWrapper;

    Selection m_aSelection;
    SdrDragMode m_eDragMode;

    Timer m_aDoubleClickTimer;
    bool m_bWaitingForDoubleClick;
    bool m_bWaitingForMouseUp;

    bool m_bConnectingToView;
    bool m_bDisposed;

    ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > m_xUndoManager;
    std::unique_ptr< UndoGuard > m_pTextActionUndoGuard;
    /// needed for dispatching URLs in FeatureStateEvents
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;

    std::unique_ptr< ::svt::AcceleratorExecute > m_apAccelExecute;

    CommandDispatchContainer m_aDispatchContainer;

    std::unique_ptr< DropTargetHelper > m_apDropTargetHelper;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XLayoutManagerEventBroadcaster > m_xLayoutManagerEventBroadcaster;

    ChartDrawMode m_eDrawMode;

    rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;

private:
    //private methods

    bool impl_isDisposedOrSuspended() const;
    ReferenceSizeProvider* impl_createReferenceSizeProvider();
    void impl_adaptDataSeriesAutoResize();

    void impl_createDrawViewController();
    void impl_deleteDrawViewController();

    //executeDispatch methods
    void executeDispatch_ObjectProperties();
    void executeDispatch_FormatObject( const OUString& rDispatchCommand );
    void executeDlg_ObjectProperties( const OUString& rObjectCID );
    bool executeDlg_ObjectProperties_withoutUndoGuard( const OUString& rObjectCID, bool bSuccessOnUnchanged );

    void executeDispatch_ChartType();

    void executeDispatch_InsertTitles();
    void executeDispatch_InsertLegend();
    void executeDispatch_DeleteLegend();
    void executeDispatch_OpenLegendDialog();
    void executeDispatch_InsertAxes();
    void executeDispatch_InsertGrid();

    void executeDispatch_InsertMenu_DataLabels();
    void executeDispatch_InsertMenu_Trendlines();
    void executeDispatch_InsertMenu_MeanValues();

    void executeDispatch_InsertMeanValue();
    void executeDispatch_InsertTrendline();
    void executeDispatch_InsertTrendlineEquation( bool bInsertR2=false );
    void executeDispatch_InsertErrorBars( bool bYError );

    void executeDispatch_InsertR2Value();
    void executeDispatch_DeleteR2Value();

    void executeDispatch_DeleteMeanValue();
    void executeDispatch_DeleteTrendline();
    void executeDispatch_DeleteTrendlineEquation();
    void executeDispatch_DeleteErrorBars( bool bYError );

    void executeDispatch_InsertDataLabels();
    void executeDispatch_InsertDataLabel();
    void executeDispatch_DeleteDataLabels();
    void executeDispatch_DeleteDataLabel();

    void executeDispatch_ResetAllDataPoints();
    void executeDispatch_ResetDataPoint();

    void executeDispatch_InsertAxis();
    void executeDispatch_InsertAxisTitle();
    void executeDispatch_InsertMajorGrid();
    void executeDispatch_InsertMinorGrid();
    void executeDispatch_DeleteAxis();
    void executeDispatch_DeleteMajorGrid();
    void executeDispatch_DeleteMinorGrid();

    void executeDispatch_InsertSpecialCharacter();
    void executeDispatch_EditText( const Point* pMousePixel = nullptr );
    void executeDispatch_SourceData();
    void executeDispatch_MoveSeries( bool bForward );

    void StartTextEdit( const Point* pMousePixel = nullptr );
    bool EndTextEdit();

    void executeDispatch_View3D();
    void executeDispatch_PositionAndSize();

    void executeDispatch_EditData();

    void executeDispatch_NewArrangement();
    void executeDispatch_ScaleText();

    void executeDispatch_Paste();
    void executeDispatch_Copy();
    void executeDispatch_Cut();
    bool executeDispatch_Delete();
    void executeDispatch_ToggleLegend();
    void executeDispatch_ToggleGridHorizontal();
    void executeDispatch_ToggleGridVertical();

    void impl_ShapeControllerDispatch( const ::com::sun::star::util::URL& rURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    DECL_LINK_TYPED( DoubleClickWaitingHdl, Timer*, void );
    void execute_DoubleClick( const Point* pMousePixel = nullptr );
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

    static std::set< OUString > impl_getAvailableCommands();

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
