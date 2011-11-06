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


#ifndef DBAUI_ACCESSIBLE_HELPER_IFACE_HXX
#define DBAUI_ACCESSIBLE_HELPER_IFACE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace dbaui
{
    class SAL_NO_VTABLE IAccessibleHelper
    {
    protected:
        /** isEditable returns the current editable state
            @return true if it is editable otherwise false
        */
        virtual sal_Bool isEditable() const = 0;
    public:
        /** notifies all listeners that this object has changed
            @param  _nEventId   the event id
            @param  _aOldValue  the old value
            @param  _aNewValue  the new value
        */
        virtual void notifyAccessibleEvent( sal_Int16 _nEventId,
                                            const ::com::sun::star::uno::Any& _aOldValue,
                                            const ::com::sun::star::uno::Any& _aNewValue) = 0;
    };
}
#endif // DBAUI_ACCESSIBLE_HELPER_IFACE_HXX
