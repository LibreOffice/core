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

#ifndef SDEXT_PRESENTER_PRESENTER_THEME_HXX
#define SDEXT_PRESENTER_PRESENTER_THEME_HXX

#include "PresenterBitmapContainer.hxx"
#include "PresenterConfigurationAccess.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/util/Color.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** A theme is a set of properties describing fonts, colors, and bitmaps to be used to draw
    background, pane borders, and view content.

    At the moment the properties can be accessed via the getPropertyValue() method.

    For a resource URL of a pane or a view you get the name of the
    associated PaneStyle or ViewStyle.

    For the name of pane or view style suffixed with and underscore and the
    name of configuration property, and maybe additionally suffixed by
    another underscore and sub property name you get the associated
    property.

    Example: you want to access the top left bitmap of a pane border
        (simplified code):

    String sStyleName = getPropertyValue("private:resource/pane/Presenter/Pane1");
    XBitmap xBitmap = getPropertyValue(sStyleName + "_TopLeftBitmap");

    For the offset of the bitmap you can call
    Point aOffset = getPropertyValue(sStyleName + "_TopLeftOffset");

    This is work in progress.
*/
class PresenterTheme
{
public:
    PresenterTheme (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::OUString& rsThemeName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    ~PresenterTheme (void);

    bool HasCanvas (void) const;
    void ProvideCanvas (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    ::rtl::OUString GetStyleName (const ::rtl::OUString& rsResourceURL) const;
    ::std::vector<sal_Int32> GetBorderSize (
        const ::rtl::OUString& rsStyleName,
        const bool bOuter) const;

    class Theme;
    class FontDescriptor
    {
    public:
        explicit FontDescriptor (const ::boost::shared_ptr<FontDescriptor>& rpDescriptor);

        ::rtl::OUString msFamilyName;
        ::rtl::OUString msStyleName;
        sal_Int32 mnSize;
        sal_uInt32 mnColor;
        ::rtl::OUString msAnchor;
        sal_Int32 mnXOffset;
        sal_Int32 mnYOffset;
        css::uno::Reference<css::rendering::XCanvasFont> mxFont;

        bool PrepareFont (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    private:
        css::uno::Reference<css::rendering::XCanvasFont> CreateFont (
            const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
            const double nCellSize) const;
        double GetCellSizeForDesignSize (
            const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
            const double nDesignSize) const;
    };
    typedef ::boost::shared_ptr<FontDescriptor> SharedFontDescriptor;

    SharedBitmapDescriptor GetBitmap (
        const ::rtl::OUString& rsStyleName,
        const ::rtl::OUString& rsBitmapName) const;

    SharedBitmapDescriptor GetBitmap (
        const ::rtl::OUString& rsBitmapName) const;

    ::boost::shared_ptr<PresenterBitmapContainer> GetBitmapContainer (void) const;

    SharedFontDescriptor GetFont (
        const ::rtl::OUString& rsStyleName) const;

    static SharedFontDescriptor ReadFont (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
        const ::rtl::OUString& rsFontPath,
        const SharedFontDescriptor& rDefaultFount);

    static bool ConvertToColor (
        const css::uno::Any& rColorSequence,
        sal_uInt32& rColor);

    ::boost::shared_ptr<PresenterConfigurationAccess> GetNodeForViewStyle (
        const ::rtl::OUString& rsStyleName) const;

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    const ::rtl::OUString msThemeName;
    ::boost::shared_ptr<Theme> mpTheme;
    ::boost::shared_ptr<PresenterBitmapContainer> mpBitmapContainer;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;

    ::boost::shared_ptr<Theme> ReadTheme (void);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
