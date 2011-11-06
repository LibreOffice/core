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



#ifndef SVX_DATABASELOCATIONINPUT_HXX
#define SVX_DATABASELOCATIONINPUT_HXX

#include "svx/svxdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

class PushButton;
class String;
namespace svt { class OFileURLControl; }
namespace comphelper { class ComponentContext; }

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseLocationInputController
    //====================================================================
    class DatabaseLocationInputController_Impl;
    /** helper class to control controls needed to input a database location

        If you allow, in your dialog, to save a database document, then you usually
        have a OFileURLControl for inputting the actual location, and a push button
        to browse for a location.

        This helper class controls such two UI elements.
    */
    class SVX_DLLPUBLIC DatabaseLocationInputController
    {
    public:
        DatabaseLocationInputController(
            const ::comphelper::ComponentContext&   _rContext,
            ::svt::OFileURLControl&                 _rLocationInput,
            PushButton&                             _rBrowseButton
        );
        ~DatabaseLocationInputController();

        /** sets the given URL at the input control, after translating it into a system path
        */
        void    setURL( const String& _rURL );

        /** returns the current database location, in form of an URL (not a system path)
        */
        String  getURL() const;

        /** prepares committing the database location entered in the input field

            Effectively, this method checks whether the file in the location already
            exists, and if so, it asks the user whether to overwrite it.

            If the method is called multiple times, this check only happens when the location
            changed since the last call.
        */
        bool    prepareCommit();

    private:
        ::std::auto_ptr< DatabaseLocationInputController_Impl >
                m_pImpl;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_DATABASELOCATIONINPUT_HXX
