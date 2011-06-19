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

#ifndef SDEXT_PRESENTER_BITMAP_CONTAINER_HXX
#define SDEXT_PRESENTER_BITMAP_CONTAINER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;


namespace sdext { namespace presenter {

/** Manage a set of bitmap groups as they are used for buttons: three
    bitmaps, one for the normal state, one for a mouse over effect and one
    to show that the button has been pressed.
    A bitmap group is defined by some entries in the configuration.
*/
class PresenterBitmapContainer
    : private ::boost::noncopyable
{
public:
    /** There is one bitmap for the normal state, one for a mouse over effect and one
        to show that a button has been pressed.
    */
    class BitmapDescriptor
    {
    public:
        BitmapDescriptor (void);
        BitmapDescriptor (const ::boost::shared_ptr<BitmapDescriptor>& rpDefault);

        enum Mode {Normal, MouseOver, ButtonDown, Disabled, Mask};
        css::uno::Reference<css::rendering::XBitmap> GetNormalBitmap (void) const;
        css::uno::Reference<css::rendering::XBitmap> GetBitmap (
            const Mode eMode,
            const bool bMissingDefaultsToNormal = true) const;
        void SetBitmap (
            const Mode eMode,
            const css::uno::Reference<css::rendering::XBitmap>& rxBitmap);

        sal_Int32 mnWidth;
        sal_Int32 mnHeight;
        sal_Int32 mnXOffset;
        sal_Int32 mnYOffset;
        sal_Int32 mnXHotSpot;
        sal_Int32 mnYHotSpot;
        css::util::Color maReplacementColor;
        enum TexturingMode { Once, Repeat, Stretch };
        TexturingMode meHorizontalTexturingMode;
        TexturingMode meVerticalTexturingMode;

    private:
        css::uno::Reference<css::rendering::XBitmap> mxNormalBitmap;
        css::uno::Reference<css::rendering::XBitmap> mxMouseOverBitmap;
        css::uno::Reference<css::rendering::XBitmap> mxButtonDownBitmap;
        css::uno::Reference<css::rendering::XBitmap> mxDisabledBitmap;
        css::uno::Reference<css::rendering::XBitmap> mxMaskBitmap;
    };

    /** Create a new bitmap container from a section of the configuration.
        @param rxComponentContext
            The component context is used to create new API objects.
        @param rxCanvas
            Bitmaps are created specifically for this canvas.
        @param rsConfigurationBase
            The name of a configuration node whose sub-tree defines the
            bitmap sets.
    */
    PresenterBitmapContainer (
        const ::rtl::OUString& rsConfigurationBase,
        const ::boost::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::rtl::OUString& rsBasePath,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper = NULL);
    PresenterBitmapContainer (
        const css::uno::Reference<css::container::XNameAccess>& rsRootNode,
        const ::boost::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::rtl::OUString& rsBasePath,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper = NULL);
    ~PresenterBitmapContainer (void);

    void Initialize (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext);

    /** Return the bitmap set that is associated with the given name.
    */
    ::boost::shared_ptr<BitmapDescriptor> GetBitmap (const ::rtl::OUString& rsName) const;

    static ::boost::shared_ptr<BitmapDescriptor> LoadBitmap (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
        const ::rtl::OUString& rsPathToBitmapNode,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
        const ::rtl::OUString& rsBitmapBasePath,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::boost::shared_ptr<BitmapDescriptor>& rpDefaultBitmap);

private:
    ::boost::shared_ptr<PresenterBitmapContainer> mpParentContainer;
    typedef ::std::map<rtl::OUString, ::boost::shared_ptr<BitmapDescriptor> > BitmapContainer;
    BitmapContainer maIconContainer;
    ::rtl::OUString msBasePath;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;

    void LoadBitmaps (
        const css::uno::Reference<css::container::XNameAccess>& rsRootNode);
    void ProcessBitmap (
        const ::rtl::OUString& rsKey,
        const css::uno::Reference<css::beans::XPropertySet>& rProperties);
    static ::boost::shared_ptr<BitmapDescriptor> LoadBitmap (
        const css::uno::Reference<css::beans::XPropertySet>& rxProperties,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
        const ::rtl::OUString& rsBasePath,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::boost::shared_ptr<PresenterBitmapContainer::BitmapDescriptor>& rpDefault);
    static BitmapDescriptor::TexturingMode
        StringToTexturingMode (const ::rtl::OUString& rsTexturingMode);
};


typedef PresenterBitmapContainer::BitmapDescriptor PresenterBitmapDescriptor;
typedef ::boost::shared_ptr<PresenterBitmapContainer::BitmapDescriptor> SharedBitmapDescriptor;

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
