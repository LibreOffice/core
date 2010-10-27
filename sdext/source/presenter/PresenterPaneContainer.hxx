/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SDEXT_PRESENTER_PANE_CONTAINER_HXX
#define SDEXT_PRESENTER_PANE_CONTAINER_HXX

#include "PresenterTheme.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <rtl/ref.hxx>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterPaneBase;
class PresenterSprite;

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::lang::XEventListener
    > PresenterPaneContainerInterfaceBase;
}

/** This class could also be called PresenterPaneAndViewContainer because it
    stores not only references to all panes that belong to the presenter
    screen but stores the views displayed in these panes as well.
*/
class PresenterPaneContainer
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterPaneContainerInterfaceBase
{
public:
    PresenterPaneContainer (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPaneContainer (void);

    virtual void SAL_CALL disposing (void);

    typedef ::boost::function1<void, const css::uno::Reference<css::drawing::framework::XView>&>
        ViewInitializationFunction;

    /** Each pane descriptor holds references to one pane and the view
        displayed in this pane as well as the other information that is used
        to manage the pane window like an XWindow reference, the title, and
        the coordinates.

        A initialization function for the view is stored as well.  This
        function is executed as soon as a view is created.
    */
    class PaneDescriptor
    {
    public:
        typedef ::boost::function<void(bool)> Activator;
        typedef ::boost::function<boost::shared_ptr<PresenterSprite>()> SpriteProvider;
        css::uno::Reference<css::drawing::framework::XResourceId> mxPaneId;
        ::rtl::OUString msViewURL;
        ::rtl::Reference<PresenterPaneBase> mxPane;
        css::uno::Reference<css::drawing::framework::XView> mxView;
        css::uno::Reference<css::awt::XWindow> mxContentWindow;
        css::uno::Reference<css::awt::XWindow> mxBorderWindow;
        ::rtl::OUString msTitleTemplate;
        ::rtl::OUString msAccessibleTitleTemplate;
        ::rtl::OUString msTitle;
        ViewInitializationFunction maViewInitialization;
        double mnLeft;
        double mnTop;
        double mnRight;
        double mnBottom;
        SharedBitmapDescriptor mpViewBackground;
        bool mbIsActive;
        bool mbNeedsClipping;
        bool mbIsOpaque;
        SpriteProvider maSpriteProvider;
        bool mbIsSprite;
        Activator maActivator;
        css::awt::Point maCalloutAnchorLocation;
        bool mbHasCalloutAnchor;

        void SetActivationState (const bool bIsActive);
    };
    typedef ::boost::shared_ptr<PaneDescriptor> SharedPaneDescriptor;
    typedef ::std::vector<SharedPaneDescriptor> PaneList;
    PaneList maPanes;

    void PreparePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::OUString& rsViewURL,
        const ::rtl::OUString& rsTitle,
        const ::rtl::OUString& rsAccessibleTitle,
        const bool bIsOpaque,
        const ViewInitializationFunction& rViewIntialization,
        const double nLeft,
        const double nTop,
        const double nRight,
        const double nBottom);

    SharedPaneDescriptor StorePane (
        const rtl::Reference<PresenterPaneBase>& rxPane);

    SharedPaneDescriptor StoreBorderWindow(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    SharedPaneDescriptor StoreView (
        const css::uno::Reference<css::drawing::framework::XView>& rxView,
        const SharedBitmapDescriptor& rpViewBackground);

    SharedPaneDescriptor RemovePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    SharedPaneDescriptor RemoveView (
        const css::uno::Reference<css::drawing::framework::XView>& rxView);

    void CreateBorderWindow (PaneDescriptor& rDescriptor);

    /** Find the pane whose border window is identical to the given border
        window.
    */
    SharedPaneDescriptor FindBorderWindow (
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    /** Find the pane whose border window is identical to the given content
        window.
    */
    SharedPaneDescriptor FindContentWindow (
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);

    /** Find the pane whose pane URL is identical to the given URL string.
    */
    SharedPaneDescriptor FindPaneURL (const ::rtl::OUString& rsPaneURL);

    /** Find the pane whose resource id is identical to the given one.
    */
    SharedPaneDescriptor FindPaneId (const css::uno::Reference<
        css::drawing::framework::XResourceId>& rxPaneId);

    SharedPaneDescriptor FindViewURL (const ::rtl::OUString& rsViewURL);

    ::rtl::OUString GetPaneURLForViewURL (const ::rtl::OUString& rsViewURL);

    void ToTop (const SharedPaneDescriptor& rpDescriptor);


    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;

    PaneList::const_iterator FindIteratorForPaneURL (const ::rtl::OUString& rsPaneURL);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
