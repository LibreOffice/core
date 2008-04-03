/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CanvasUpdateRequester.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:06:05 $
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

#ifndef SD_PRESENTER_CANVAS_UPDATE_REQUESTER_HEADER
#define SD_PRESENTER_CANVAS_UPDATE_REQUESTER_HEADER

#include "precompiled_sd.hxx"

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <sal/types.h>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <vector>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

/** Each UpdateRequester handles update requests (calls to
    XCanvas::updateScreen()) for one shared canvas (a canvas that has one or
    more PresenterCanvas wrappers).  Multiple calls are collected and lead
    to a single call to updateScreen.
*/
class CanvasUpdateRequester : private ::boost::noncopyable
{
public:
    /** Return the Canvas UpdateRequester object for the given shared
        canvas.  A new object is created when it does not already exist.
    */
    static ::boost::shared_ptr<CanvasUpdateRequester> Instance (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);

    void RequestUpdate (const sal_Bool bUpdateAll);

private:
    CanvasUpdateRequester (const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);
    ~CanvasUpdateRequester (void);
    class Deleter; friend class Deleter;

    typedef ::std::vector<
        ::std::pair<
            css::uno::Reference<css::rendering::XSpriteCanvas>,
           ::boost::shared_ptr<CanvasUpdateRequester> > > RequesterMap;
    static RequesterMap maRequesterMap;

    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    ULONG mnUserEventId;
    sal_Bool mbUpdateFlag;
    DECL_LINK(Callback, void*);
};

} } // end of namespace ::sd::presenter

#endif
