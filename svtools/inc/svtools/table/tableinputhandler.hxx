/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    class ITableControl;

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

        virtual bool    MouseMove       ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonDown ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    MouseButtonUp   ( ITableControl& _rControl, const MouseEvent& rMEvt ) = 0;
        virtual bool    KeyInput        ( ITableControl& _rControl, const KeyEvent& rKEvt ) = 0;
        virtual bool    GetFocus        ( ITableControl& _rControl ) = 0;
        virtual bool    LoseFocus       ( ITableControl& _rControl ) = 0;
        virtual bool    RequestHelp     ( ITableControl& _rControl, const HelpEvent& rHEvt ) = 0;
        virtual bool    Command         ( ITableControl& _rControl, const CommandEvent& rCEvt ) = 0;
        virtual bool    PreNotify       ( ITableControl& _rControl, NotifyEvent& rNEvt ) = 0;
        virtual bool    Notify          ( ITableControl& _rControl, NotifyEvent& rNEvt ) = 0;

        virtual ~ITableInputHandler() { }
    };
    typedef ::boost::shared_ptr< ITableInputHandler >   PTableInputHandler;

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLEINPUTHANDLER_HXX
