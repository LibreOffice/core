/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterTheme.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:06:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
