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



#ifndef SVX_FMSCRIPTINGENV_HXX
#define SVX_FMSCRIPTINGENV_HXX

/** === begin UNO includes === **/
#include <com/sun/star/script/XEventAttacherManager.hpp>
/** === end UNO includes === **/
#include <rtl/ref.hxx>

class FmFormModel;
//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= IFormScriptingEnvironment
    //====================================================================
    /** describes the interface implemented by a component which handles scripting requirements
        in a form/control environment.
    */
    class SAL_NO_VTABLE IFormScriptingEnvironment : public ::rtl::IReference
    {
    public:
        /** registers an XEventAttacherManager whose events should be monitored and handled

            @param _rxManager
                the XEventAttacherManager to monitor. Must not be <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if attaching as script listener to the manager fails with a RuntimeException itself
        */
        virtual void registerEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** registers an XEventAttacherManager whose events should not be monitored and handled anymore

            @param _rxManager
                the XEventAttacherManager which was previously registered. Must not ne <NULL/>.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if <arg>_rxManager</arg> is <NULL/>
            @throws ::com::sun::star::lang::DisposedException
                if the instance is already disposed
            @throws ::com::sun::star::uno::RuntimeException
                if removing as script listener from the manager fails with a RuntimeException itself
        */
        virtual void revokeEventAttacherManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >& _rxManager ) = 0;

        /** disposes the scripting environment instance
        */
        virtual void dispose() = 0;

        virtual ~IFormScriptingEnvironment();
    };
    typedef ::rtl::Reference< IFormScriptingEnvironment >   PFormScriptingEnvironment;

    //====================================================================
    /** creates a default component implementing the IFormScriptingEnvironment interface
    */
    PFormScriptingEnvironment   createDefaultFormScriptingEnvironment( FmFormModel& _rFormModel );

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FMSCRIPTINGENV_HXX

