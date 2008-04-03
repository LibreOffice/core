/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FullScreenPane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:34:01 $
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

#ifndef SD_FRAMEWORK_FULL_SCREEN_PANE_HXX
#define SD_FRAMEWORK_FULL_SCREEN_PANE_HXX

#include "FrameWindowPane.hxx"
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

class WorkWindow;

namespace sd { class ViewShellBase; }

namespace sd { namespace framework {

/** The full screen pane creates a pane that covers the complete application
    window, i.e. that hides menu bar, tool bars, status bars.
*/
class FullScreenPane
    : public FrameWindowPane
{
public:
    /** Create a new full screen pane.
        @param rxComponentContext
            Used for creating a new canvas.
        @param rxPaneId
            The resource id of the new pane.
        @param pViewShellWindow
            The top-level parent of this window is used to obtain title and
            icon for the new top-level window.
    */
    FullScreenPane (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::Window* pViewShellWindow);
    virtual ~FullScreenPane (void) throw();

    virtual void SAL_CALL disposing (void);

protected:
    virtual ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas>
        CreateCanvas (void)
        throw (::com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::boost::scoped_ptr<WorkWindow> mpWorkWindow;

    void ExtractArguments (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        sal_Int32& rnScreenNumberReturnValue);
};

} } // end of namespace sd::framework

#endif
