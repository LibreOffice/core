/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef SVTOOLS_INC_TABLE_TABLEINPUTHANDLER_HXX
#define SVTOOLS_INC_TABLE_TABLEINPUTHANDLER_HXX

#include <boost/shared_ptr.hpp>

class MouseEvent;
class KeyEvent;
class HelpEvent;
class CommandEvent;
class NotifyEvent;

//........................................................................
namespace svt { namespace table
{
//........................................................................

    class IAbstractTableControl;

    //====================================================================
    //= ITableInputHandler
    //====================================================================
    /** interface for components handling input in a ->TableControl
    */
    class ITableInputHandler
    {
    public:
        // all those methods have the same semantics as the equal-named methods of ->Window,
        // with the additional option to return a boolean value indicating whether
        // the event should be further processed by the ->Window implementations (<FALSE/>),
        // or whether it has been sufficiently handled by the ->ITableInputHandler instance
        // (<FALSE/>).

        virtual bool    MouseMove       ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonDown ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonUp   ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    KeyInput        ( IAbstractTableControl& _rControl, const KeyEvent& rKEvt ) = 0;
        virtual bool    GetFocus        ( IAbstractTableControl& _rControl ) = 0;
        virtual bool    LoseFocus       ( IAbstractTableControl& _rControl ) = 0;
        virtual bool    RequestHelp     ( IAbstractTableControl& _rControl, const HelpEvent& rHEvt ) = 0;
        virtual bool    Command         ( IAbstractTableControl& _rControl, const CommandEvent& rCEvt ) = 0;
        virtual bool    PreNotify       ( IAbstractTableControl& _rControl, NotifyEvent& rNEvt ) = 0;
        virtual bool    Notify          ( IAbstractTableControl& _rControl, NotifyEvent& rNEvt ) = 0;

        virtual ~ITableInputHandler() { }
    };
    typedef ::boost::shared_ptr< ITableInputHandler >   PTableInputHandler;

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLEINPUTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
