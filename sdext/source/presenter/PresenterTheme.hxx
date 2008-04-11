/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterTheme.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SDEXT_PRESENTER_PRESENTER_THEME_HXX
#define SDEXT_PRESENTER_PRESENTER_THEME_HXX

#include "PresenterConfigurationAccess.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
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

    void SAL_CALL disposing (void);

    bool HasCanvas (void) const;
    void ProvideCanvas (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    css::uno::Any getPropertyValue (const ::rtl::OUString& rsPropertyName);

    ::rtl::OUString GetPaneStyle (const ::rtl::OUString& rsResourceURL) const;
    ::rtl::OUString GetViewStyle (const ::rtl::OUString& rsResourceURL) const;

    class BitmapContainer;
    class FontDescriptor;
    class BitmapDescriptor;
    class Theme;
    class PropertyMap;

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    const ::rtl::OUString msThemeName;
    ::boost::shared_ptr<BitmapContainer> mpBitmapContainer;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    ::boost::shared_ptr<PropertyMap> mpPropertyMap;

    void ReadTheme (void);
};

} } // end of namespace ::sd::presenter

#endif
