/*************************************************************************
 *
 *  $RCSfile: callbacks.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:05:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#define _DBACCESS_UI_CALLBACKS_HXX_

#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

class CommandEvent;
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

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBACCESS_UI_CALLBACKS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/03/23 10:52:50  fs
 *  new DnD implementations
 *
 *  Revision 1.1  2001/03/22 10:40:44  fs
 *  initial checkin - callbacks
 *
 *
 *  Revision 1.0 22.03.01 10:12:22  fs
 ************************************************************************/

