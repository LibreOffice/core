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



#ifndef SVTOOLS_PICKER_CALLBACKS_HXX
#define SVTOOLS_PICKER_CALLBACKS_HXX

#include <tools/string.hxx>
#include <sal/types.h>

class Control;

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    class IFilePickerController
    {
    public:
        virtual Control*    getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const = 0;
        virtual void        enableControl( sal_Int16 _nControlId, sal_Bool _bEnable ) = 0;
        virtual String      getCurFilter( ) const = 0;
    };

    // --------------------------------------------------------------------
    class IFilePickerListener
    {
    public:
        virtual void    notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) = 0;
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_PICKER_CALLBACKS_HXX

