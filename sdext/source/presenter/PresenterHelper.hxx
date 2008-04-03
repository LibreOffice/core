/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:59:27 $
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

#ifndef SDEXT_PRESENTER_VIEW_HELPER_HXX
#define SDEXT_PRESENTER_VIEW_HELPER_HXX

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** Collection of helper functions that do not fit in anywhere else.
    Provide access to frequently used strings of the drawing framework.
*/
class PresenterHelper
    : ::boost::noncopyable
{
public:
    static const ::rtl::OUString msPaneURLPrefix;
    static const ::rtl::OUString msCenterPaneURL;
    static const ::rtl::OUString msFullScreenPaneURL;

    static const ::rtl::OUString msViewURLPrefix;
    static const ::rtl::OUString msPresenterScreenURL;
    static const ::rtl::OUString msSlideSorterURL;

    static const ::rtl::OUString msResourceActivationEvent;
    static const ::rtl::OUString msResourceDeactivationEvent;

    static const ::rtl::OUString msDefaultPaneStyle;
    static const ::rtl::OUString msDefaultViewStyle;

    /** Return the slide show controller of a running presentation that has
        the same document as the given framework controller.
        @return
            When no presentation is running this method returns an empty reference.
    */
    static css::uno::Reference<css::presentation::XSlideShowController> GetSlideShowController (
        const css::uno::Reference<css::frame::XController>& rxController);

    /** Load a bitmap from a file (or other place) that has the given URL
        and return it.
        @param rxContext
            The component context is used to create the necessary
            temporarily used services to load the graphic object.
        @param rsURL
            URL of a file or other place that points to a bitmap resource.
    */
    static css::uno::Reference<css::graphic::XGraphic> GetGraphic (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::OUString& rsURL);

private:
    PresenterHelper (void);
    ~PresenterHelper (void);
};

} } // end of namespace presenter

#endif
