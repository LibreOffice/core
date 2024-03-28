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
#pragma once

#include <LifeTime.hxx>
#include <ReferenceSizeProvider.hxx>
#include "CommandDispatchContainer.hxx"
#include "SelectionHelper.hxx"

#include <svx/svdtypes.hxx>
#include <vcl/timer.hxx>

#include <cppuhelper/implbase.hxx>
#include <o3tl/sorted_vector.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XLayoutManagerListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <memory>
#include <string_view>

namespace com::sun::star::accessibility { class XAccessible; }
namespace com::sun::star::accessibility { class XAccessibleContext; }
namespace com::sun::star::awt { class XFocusListener; }
namespace com::sun::star::awt { class XKeyListener; }
namespace com::sun::star::awt { class XMouseListener; }
namespace com::sun::star::awt { class XMouseMotionListener; }
namespace com::sun::star::awt { class XPaintListener; }
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::awt { class XWindowListener; }
namespace com::sun::star::awt { struct Point; }
namespace com::sun::star::document { class XUndoManager; }
namespace com::sun::star::frame { class XDispatch; }
namespace com::sun::star::frame { class XLayoutManagerEventBroadcaster; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::lang { class XInitialization; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XCloseable; }
namespace com::sun::star::view { class XSelectionSupplier; }

class SdrModel;

namespace svt
{
    class AcceleratorExecute;
}

namespace svx::sidebar {
    class SelectionChangeHandler;
}

namespace weld {
    class Window;
}

class DropTargetHelper;

namespace chart
{

class UndoGuard;
class ChartView;
class ChartWindow;
class DrawModelWrapper;
class DrawViewWrapper;
class ViewElementListProvider;
class ViewElementListProvider;
class Diagram;
class AccessibleChartView;
class AccessibleTextHelper;

enum ChartDrawMode { CHARTDRAW_INSERT, CHARTDRAW_SELECT };


class ChartController final : public ::cppu::WeakImplHelper <
         css::frame::XController2   //comprehends XComponent (css::frame::XController is required interface)
        ,css::frame::XDispatchProvider     //(required interface)
        ,css::view::XSelectionSupplier     //(optional interface)
        ,css::ui::XContextMenuInterception //(optional interface)
        ,css::util::XCloseListener         //(needed for communication with XModel)
        ,css::frame::XDispatch
        ,css::awt::XWindow //this is the Window Controller part of this Controller, that will be given to a Frame via setComponent
        ,css::util::XModifyListener
        ,css::util::XModeChangeListener
        ,css::frame::XLayoutManagerListener
        >
{
public:
    ChartController() = delete;
    explicit ChartController(css::uno::Reference< css::uno::XComponentContext > xContext);
    virtual ~ChartController() override;

    OUString GetContextName();

    // css::frame::XController (required interface)
    virtual void SAL_CALL
        attachFrame( const css::uno::Reference< css::frame::XFrame > & xFrame ) override;

    virtual sal_Bool SAL_CALL
        attachModel( const css::uno::Reference< css::frame::XModel > & xModel ) override;

    virtual css::uno::Reference< css::frame::XFrame > SAL_CALL
        getFrame() override;

    virtual css::uno::Reference< css::frame::XModel > SAL_CALL
        getModel() override;

    virtual css::uno::Any SAL_CALL
        getViewData() override;

    virtual void SAL_CALL
        restoreViewData( const css::uno::Any& rValue ) override;

    virtual sal_Bool SAL_CALL
        suspend( sal_Bool bSuspend ) override;

    // css::frame::XController2
    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL getComponentWindow() override;
    virtual OUString SAL_CALL getViewControllerName() override;
    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getCreationArguments() override;
    virtual css::uno::Reference<css::ui::XSidebarProvider> SAL_CALL getSidebar() override;

    // css::lang::XComponent (base of XController)
    virtual void SAL_CALL
        dispose() override;

    virtual void SAL_CALL
        addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;

    virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;

    // css::frame::XDispatchProvider (required interface)
    virtual css::uno::Reference< css::frame::XDispatch> SAL_CALL
        queryDispatch( const css::util::URL& rURL
                            , const OUString& rTargetFrameName
                            , sal_Int32 nSearchFlags) override;

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
        queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor > & xDescripts) override;

    // css::view::XSelectionSupplier (optional interface)
    virtual sal_Bool SAL_CALL
        select( const css::uno::Any& rSelection ) override;

    virtual css::uno::Any  SAL_CALL
        getSelection() override;

    virtual void SAL_CALL
        addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener > & xListener ) override;

    virtual void SAL_CALL
        removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener > & xListener ) override;

    // css::ui::XContextMenuInterception (optional interface)
    virtual void SAL_CALL
        registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor > & xInterceptor) override;

    virtual void SAL_CALL
        releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor > & xInterceptor) override;

    //additional interfaces

    // css::util::XCloseListener
    virtual void SAL_CALL
        queryClosing( const css::lang::EventObject& Source
                            , sal_Bool GetsOwnership ) override;

    virtual void SAL_CALL
        notifyClosing( const css::lang::EventObject& Source ) override;

    // css::util::XEventListener (base of XCloseListener and XModifyListener)
    virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source ) override;

    // css::frame::XDispatch

    virtual void SAL_CALL
        dispatch( const css::util::URL& aURL
                    , const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;

    virtual void SAL_CALL
        addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl
                    , const css::util::URL& aURL ) override;

    virtual void SAL_CALL
        removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl
                    , const css::util::URL& aURL ) override;

    // css::awt::XWindow
    virtual void SAL_CALL
        setPosSize( sal_Int32 X, sal_Int32 Y
                    , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;

    virtual css::awt::Rectangle SAL_CALL
        getPosSize() override;

    virtual void SAL_CALL
        setVisible( sal_Bool Visible ) override;

    virtual void SAL_CALL
        setEnable( sal_Bool Enable ) override;

    virtual void SAL_CALL
        setFocus() override;

    virtual void SAL_CALL
        addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;

    virtual void SAL_CALL
        removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;

    virtual void SAL_CALL
        addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;

    virtual void SAL_CALL
        removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;

    virtual void SAL_CALL
        addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;

    virtual void SAL_CALL
        removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;

    virtual void SAL_CALL
        addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;

    virtual void SAL_CALL
        removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;

    virtual void SAL_CALL
        addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;

    virtual void SAL_CALL
        removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;

    virtual void SAL_CALL
        addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;

    virtual void SAL_CALL
        removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;

    // css::util::XModifyListener
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // css::util::XModeChangeListener
    virtual void SAL_CALL modeChanged(
        const css::util::ModeChangeEvent& _rSource ) override;

    // css::frame::XLayoutManagerListener
    virtual void SAL_CALL layoutEvent(
        const css::lang::EventObject& aSource,
        ::sal_Int16 eLayoutEvent,
        const css::uno::Any& aInfo ) override;

    // WindowController stuff
    void PrePaint();
    void execute_Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
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
        OUString & rOutQuickHelpText, css::awt::Rectangle & rOutEqualRect );

    css::uno::Reference< css::accessibility::XAccessible > CreateAccessible();

    /** Creates a helper accessibility class that must be initialized via initialize().  For
        parameters see

        The returned object should not be used directly.  Instead a proxy object
        should use this helper to retrieve its children and add them to its own
        children.
     */
    rtl::Reference< ::chart::AccessibleTextHelper > createAccessibleTextContext();

    static bool isObjectDeleteable( const css::uno::Any& rSelection );

    void setDrawMode( ChartDrawMode eMode ) { m_eDrawMode = eMode; }

    bool isShapeContext() const;
    bool IsTextEdit() const;

    ViewElementListProvider getViewElementListProvider();
    DrawModelWrapper* GetDrawModelWrapper();
    DrawViewWrapper* GetDrawViewWrapper();
    ChartWindow* GetChartWindow() const;
    weld::Window* GetChartFrame();
    bool isAdditionalShapeSelected() const;
    void SetAndApplySelection(const css::uno::Reference<css::drawing::XShape>& rxShape);
    void StartTextEdit( const Point* pMousePixel = nullptr );

    void NotifyUndoActionHdl( std::unique_ptr<SdrUndoAction> );

    rtl::Reference<::chart::ChartView> const & getChartView() const { return m_xChartView; }

    rtl::Reference<::chart::ChartModel> getChartModel();
    rtl::Reference<::chart::Diagram> getFirstDiagram();

private:
    class TheModel : public salhelper::SimpleReferenceObject
    {
        public:
            explicit TheModel( rtl::Reference<::chart::ChartModel> xModel );

            virtual ~TheModel() override;

            void        addListener( ChartController* pController );
            void        removeListener(  ChartController* pController );
            void        tryTermination();
            const rtl::Reference<::chart::ChartModel>&
                        getModel() const { return m_xModel;}

        private:
            rtl::Reference<::chart::ChartModel>     m_xModel;

            //the ownership between model and controller is not clear at first
            //each controller might consider himself as owner of the model first
            bool m_bOwnership;
    };
    class TheModelRef final
    {
        public:
            TheModelRef( TheModel* pTheModel, ::osl::Mutex& rMutex );
            TheModelRef( const TheModelRef& rTheModel, ::osl::Mutex& rMutex );
            TheModelRef& operator=(ChartController::TheModel* pTheModel);
            TheModelRef& operator=(const TheModelRef& rTheModel);
            ~TheModelRef();
            bool is() const;
            TheModel* operator->() const { return m_xTheModel.get(); }
        private:
            rtl::Reference<TheModel>  m_xTheModel;
            ::osl::Mutex&             m_rModelMutex;
    };

    mutable ::apphelper::LifeTimeManager m_aLifeTimeManager;

    bool m_bSuspended;

    css::uno::Reference< css::uno::XComponentContext> m_xCC;

    //model
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    mutable ::osl::Mutex m_aModelMutex;
    TheModelRef m_aModel;

    //view
    css::uno::Reference<css::awt::XWindow> m_xViewWindow;
    rtl::Reference<::chart::ChartView> m_xChartView;
    std::shared_ptr< DrawModelWrapper > m_pDrawModelWrapper;
    std::unique_ptr<DrawViewWrapper> m_pDrawViewWrapper;

    Selection m_aSelection;
    SdrDragMode m_eDragMode;

    Timer m_aDoubleClickTimer;
    bool m_bWaitingForDoubleClick;
    bool m_bWaitingForMouseUp;
    bool m_bFieldButtonDown;

    bool m_bConnectingToView;
    bool m_bDisposed;

    css::uno::Reference< css::document::XUndoManager > m_xUndoManager;
    std::unique_ptr< UndoGuard > m_pTextActionUndoGuard;

    std::unique_ptr< ::svt::AcceleratorExecute > m_apAccelExecute;

    CommandDispatchContainer m_aDispatchContainer;

    std::unique_ptr< DropTargetHelper > m_apDropTargetHelper;
    css::uno::Reference<
            css::frame::XLayoutManagerEventBroadcaster > m_xLayoutManagerEventBroadcaster;

    ChartDrawMode m_eDrawMode;

    rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;

    bool impl_isDisposedOrSuspended() const;
    ReferenceSizeProvider impl_createReferenceSizeProvider();
    void impl_adaptDataSeriesAutoResize();

    void impl_createDrawViewController();
    void impl_deleteDrawViewController();

    //executeDispatch methods
    void executeDispatch_ObjectProperties();
    void executeDispatch_FormatObject( std::u16string_view rDispatchCommand );
    void executeDlg_ObjectProperties( const OUString& rObjectCID );
    void executeDlg_ObjectProperties_withUndoGuard( std::shared_ptr<UndoGuard> aUndoGuard, const OUString& rObjectCID, bool bSuccessOnUnchanged );

    void executeDispatch_ChartType();

    void executeDispatch_InsertTitles();
    void executeDispatch_InsertLegend();
    void executeDispatch_DeleteLegend();
    void executeDispatch_OpenLegendDialog();
    void executeDispatch_InsertAxes();
    void executeDispatch_InsertGrid();

    void executeDispatch_InsertDataTable();
    void executeDispatch_DeleteDataTable();
    void executeDispatch_OpenInsertDataTableDialog();

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

    bool EndTextEdit();
    css::uno::Sequence< css::uno::Reference<css::chart2::XFormattedString >> GetFormattedTitle(
        const EditTextObject& aEdit, const css::uno::Reference< css::drawing::XShape >& xShape );

    void executeDispatch_View3D();
    void executeDispatch_PositionAndSize( const ::css::uno::Sequence< ::css::beans::PropertyValue >* pArgs = nullptr );

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

    void executeDispatch_LOKSetTextSelection(int nType, int nX, int nY);
    void executeDispatch_LOKPieSegmentDragging(int nOffset);
    void executeDispatch_FillColor(sal_uInt32 nColor);
    void executeDispatch_FillGradient(std::u16string_view sJSONGradient);
    void executeDispatch_LineColor(sal_uInt32 nColor);
    void executeDispatch_LineWidth(sal_uInt32 nWidth);

    void sendPopupRequest(std::u16string_view rCID, tools::Rectangle aRectangle);

    void impl_ShapeControllerDispatch( const css::util::URL& rURL,
        const css::uno::Sequence< css::beans::PropertyValue >& rArgs );

    DECL_LINK( DoubleClickWaitingHdl, Timer*, void );
    void execute_DoubleClick( const Point* pMousePixel, bool &bEditText );
    void startDoubleClickWaiting();
    void stopDoubleClickWaiting();

    void impl_selectObjectAndNotiy();
    void impl_notifySelectionChangeListeners();
    void impl_invalidateAccessible();
    void impl_initializeAccessible();
    void impl_initializeAccessible( AccessibleChartView& xInit );

    //sets the model member to null if it equals the parameter
    //returns true if successful
    bool impl_releaseThisModel( const css::uno::Reference< css::uno::XInterface > & xModel );

    enum eMoveOrResizeType
    {
        MOVE_OBJECT,
        CENTERED_RESIZE_OBJECT
    };
    /// @return </sal_True>, if resize/move was successful
    bool impl_moveOrResizeObject(
        const OUString & rCID, eMoveOrResizeType eType, double fAmountLogicX, double fAmountLogicY );
    bool impl_DragDataPoint( std::u16string_view rCID, double fOffset );

    static const o3tl::sorted_vector< OUString >& impl_getAvailableCommands();

    void impl_PasteGraphic( css::uno::Reference< css::graphic::XGraphic > const & xGraphic,
                            const ::Point & aPosition );
    void impl_PasteShapes( SdrModel* pModel );
    void impl_PasteStringAsTextShape( const OUString& rString, const css::awt::Point& rPosition );
    void impl_SetMousePointer( const MouseEvent & rEvent );

    void impl_ClearSelection();

    void impl_switchDiagramPositioningToExcludingPositioning();
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
