/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callbacks.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:23:59 $
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

#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#define _DBACCESS_UI_CALLBACKS_HXX_

#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

class CommandEvent;
class SvLBoxEntry;
class String;
class Point;
struct AcceptDropEvent;
struct ExecuteDropEvent;

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= IControlActionListener
    //====================================================================
    class IControlActionListener
    {
    public:
        /** handler for context menu requests
            @return <TRUE/> if the request was handled
        */
        virtual sal_Bool    requestContextMenu( const CommandEvent& _rEvent ) = 0;

        /** requests a quick help text to display
            @return <FALSE/> if the default quick help text should be used
        */
        virtual sal_Bool    requestQuickHelp( const SvLBoxEntry* _pEntry, String& _rText ) const = 0;

        /** handler for StartDrag requests
            @return <TRUE/> if a drag operation was started
        */
        virtual sal_Bool    requestDrag( sal_Int8 _nAction, const Point& _rPosPixel ) = 0;

        /** check whether or no a drop request should be accepted
        */
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors ) = 0;

        /** execute a drop request
        */
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt ) = 0;
    };

    //====================================================================
    //= IDragTransferableListener
    //====================================================================
    class IDragTransferableListener
    {
    public:
        /// called when a drag operation done with a Transferable has been finished
        virtual void        dragFinished( ) = 0;
    };

    //====================================================================
    //= IContainerFoundListener
    //====================================================================
    class SAL_NO_VTABLE IContainerFoundListener
    {
    public:
        /// called when a container was found
        virtual void containerFound( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer) = 0;
    };

    //====================================================================
    //= IViewChangeListener
    //====================================================================
    class SAL_NO_VTABLE IViewChangeListener
    {
    public:
        /// called when the preview mode was changed
        virtual void previewChanged( sal_Int32 _nMode) = 0;
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBACCESS_UI_CALLBACKS_HXX_

