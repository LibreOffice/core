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

#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#define _DBACCESS_UI_CALLBACKS_HXX_

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <com/sun/star/container/XContainer.hpp>

class CommandEvent;
class SvLBoxEntry;
class String;
class Point;
class PopupMenu;
class Control;
struct AcceptDropEvent;
struct ExecuteDropEvent;

namespace cppu { class OInterfaceContainerHelper; }

//........................................................................
namespace dbaui
{
//........................................................................

    class IController;
    //====================================================================
    //= IControlActionListener
    //====================================================================
    class SAL_NO_VTABLE IControlActionListener
    {
    public:
        /** requests a quick help text to display
            @return <FALSE/> if the default quick help text should be used
        */
        virtual sal_Bool    requestQuickHelp( const SvLBoxEntry* _pEntry, String& _rText ) const = 0;

        /** handler for StartDrag requests
            @return <TRUE/> if a drag operation was started
        */
        virtual sal_Bool    requestDrag( sal_Int8 _nAction, const Point& _rPosPixel ) = 0;

        /** check whether or not a drop request should be accepted
        */
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors ) = 0;

        /** execute a drop request
        */
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt ) = 0;
    };

    //====================================================================
    //= IContextMenuProvider
    //====================================================================
    class SAL_NO_VTABLE IContextMenuProvider
    {
    public:
        /** returns the context menu for the control

            Note that the menu does not need to care for the controls selection, or its
            state in general.
            The control itself will, using the controller provided by getCommandController,
            disable menu entries as needed.
        */
        virtual PopupMenu*      getContextMenu( Control& _rControl ) const = 0;

        /** returns the controller which is responsible for providing states of certain features,
            and executing them.
        */
        virtual IController&    getCommandController() = 0;

        /** returns the container of registered context menu interceptors, or NULL if the implementation
            does not support context menu interception
        */
        virtual ::cppu::OInterfaceContainerHelper*
                                getContextMenuInterceptors() = 0;

        /** returns the current selection in the given control

            This selection is used for filling a ContextMenuExecuteEvent event for the given
            control.
        */
        virtual ::com::sun::star::uno::Any
                                getCurrentSelection( Control& _rControl ) const = 0;
    };

    //====================================================================
    //= IDragTransferableListener
    //====================================================================
    class SAL_NO_VTABLE IDragTransferableListener
    {
    public:
        /// called when a drag operation done with a Transferable has been finished
        virtual void        dragFinished( ) = 0;
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // _DBACCESS_UI_CALLBACKS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
