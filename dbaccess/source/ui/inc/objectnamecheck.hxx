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



#ifndef DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
#define DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

namespace rtl { class OUString; }
namespace dbtools { class SQLExceptionInfo; }

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= IObjectNameCheck
    //====================================================================
    /** interface encapsulating the check for the validity of an object name
    */
    class IObjectNameCheck
    {
    public:
        /** determines whether a given object name is valid

            @param  _rObjectName
                the name to check
            @param  _out_rErrorToDisplay
                output parameter taking an error message describing why the name is not
                valid, if applicable.

            @return
                <TRUE/> if and only if the given name is valid.
        */
        virtual bool    isNameValid(
            const ::rtl::OUString& _rObjectName,
            ::dbtools::SQLExceptionInfo& _out_rErrorToDisplay
        ) const = 0;

    public:
        virtual ~IObjectNameCheck() { }
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_OBJECTNAMECHECK_HXX
