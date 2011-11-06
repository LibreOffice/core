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


#include "unotools/unotoolsdllapi.h"

#ifndef UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX
#define UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace utl
{
//........................................................................

    //====================================================================
    //= ITerminationListener
    //====================================================================
    /** non-UNO version of the <type scope="com.sun.star.frame">XTerminateListener</type>
    */
    class ITerminationListener
    {
    public:
        virtual bool    queryTermination() const = 0;
        virtual void    notifyTermination() = 0;
    };

    //====================================================================
    //= DesktopTerminationObserver
    //====================================================================
    /** a class which allows non-UNO components to observe the desktop (aka application)
        for it's shutdown
    */
    class UNOTOOLS_DLLPUBLIC DesktopTerminationObserver
    {
    public:
        /** registers a listener which should be notified when the desktop terminates
            (which means the application is shutting down)
        */
        static void    registerTerminationListener( ITerminationListener* _pListener );

        /** revokes a termination listener
        */
        static void    revokeTerminationListener( ITerminationListener* _pListener );

    private:
        DesktopTerminationObserver();   // never implemented, only static methods
    };

//........................................................................
} // namespace utl
//........................................................................

#endif // UNOTOOLS_INC_UNOTOOLS_DESKTOPTERMINATIONOBSERVER_HXX

