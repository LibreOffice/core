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

#ifndef INCLUDED_SVTOOLS_TABLE_TABLEINPUTHANDLER_HXX
#define INCLUDED_SVTOOLS_TABLE_TABLEINPUTHANDLER_HXX

#include <memory>

class MouseEvent;
class KeyEvent;


namespace svt { namespace table
{


    class ITableControl;


    //= ITableInputHandler

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

        virtual bool    MouseMove       ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonDown ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonUp   ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    KeyInput        ( ITableControl& _rControl, const KeyEvent& rKEvt ) = 0;
        virtual bool    GetFocus        ( ITableControl& _rControl ) = 0;
        virtual bool    LoseFocus       ( ITableControl& _rControl ) = 0;

        virtual ~ITableInputHandler() { }
    };
    typedef std::shared_ptr< ITableInputHandler >   PTableInputHandler;


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_TABLE_TABLEINPUTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
