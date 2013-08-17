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

#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#define _DBACCESS_UI_CALLBACKS_HXX_

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <com/sun/star/container/XContainer.hpp>

class SvTreeListEntry;
class String;
class Point;
class PopupMenu;
class Control;
struct AcceptDropEvent;
struct ExecuteDropEvent;

namespace cppu { class OInterfaceContainerHelper; }

namespace dbaui
{

    class IController;
    // IControlActionListener
    class SAL_NO_VTABLE IControlActionListener
    {
    public:
        /** requests a quick help text to display
            @return <FALSE/> if the default quick help text should be used
        */
        virtual sal_Bool    requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const = 0;

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

    protected:
        ~IControlActionListener() {}
    };

    // IContextMenuProvider
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

    protected:
        ~IContextMenuProvider() {}
    };

    // IDragTransferableListener
    class SAL_NO_VTABLE IDragTransferableListener
    {
    public:
        /// called when a drag operation done with a Transferable has been finished
        virtual void        dragFinished( ) = 0;

    protected:
        ~IDragTransferableListener() {}
    };

}   // namespace dbaui

#endif // _DBACCESS_UI_CALLBACKS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
