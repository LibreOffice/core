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



#ifndef DBACCESS_ASYNCMODALDIALOG_HXX
#define DBACCESS_ASYNCMODALDIALOG_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= AsyncDialogExecutor
    //====================================================================
    /** helper class for executing (UNO) dialogs modal, but asynchronously
    */
    class AsyncDialogExecutor
    {
    public:
        /** executes the given dialog asynchronously, but still modal

            @raises IllegalArgumentException
                if the given dialog is <NULL/>
            @todo
                allow for a callback for the result
        */
        static void executeModalDialogAsync(
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XExecutableDialog >& _rxDialog
        );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ASYNCMODALDIALOG_HXX

