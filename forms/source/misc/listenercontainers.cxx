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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_MISC_LISTENERCONTAINERS_HXX
#include "listenercontainers.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= ResetListeners
    //====================================================================
    //---------------------------------------------------------------------
    bool ResetListeners::implTypedNotify( const Reference< XResetListener >& _rxListener,
        const EventObject& _rEvent )   SAL_THROW( ( Exception ) )
    {
        switch ( m_eCurrentNotificationType )
        {
        case eApproval:
            if ( !_rxListener->approveReset( _rEvent ) )
                return false;
            return true;
        case eFinal:
            _rxListener->resetted( _rEvent );
            break;
        default:
            OSL_ENSURE( sal_False, "ResetListeners::implNotify: invalid notification type!" );
        }
        return true;
    }

//........................................................................
} // namespace frm
//........................................................................

