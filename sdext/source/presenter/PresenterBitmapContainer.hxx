/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterBitmapContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:55:39 $
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

#ifndef SDEXT_PRESENTER_BITMAP_CONTAINER_HXX
#define SDEXT_PRESENTER_BITMAP_CONTAINER_HXX

#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <vector>

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
    class BitmapSet
    {
    public:
        css::uno::Reference<css::rendering::XBitmap> mxNormalIcon;
        css::uno::Reference<css::rendering::XBitmap> mxMouseOverIcon;
        css::uno::Reference<css::rendering::XBitmap> mxButtonDownIcon;

        /** Fill in missing bitmaps by making mxNormalIcon lighter or
            darker.
        */
        void Update (
            const css::uno::Reference<css::rendering::XGraphicDevice>& rxDevice);
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
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::rtl::OUString& rsConfigurationBase);
    ~PresenterBitmapContainer (void);

    /** Return the bitmap set that is associated with the given name.
    */
    BitmapSet GetButtons (const ::rtl::OUString& rsName) const;

private:
    typedef ::std::map<rtl::OUString,BitmapSet> BitmapContainer;
    BitmapContainer maIconContainer;

    void LoadButtons (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::rtl::OUString& rsConfigurationBase);
    void ProcessBitmap (
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues,
        const ::rtl::OUString& rsLocation,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxBitmapLoader);
};

} } // end of namespace ::sdext::presenter

#endif
