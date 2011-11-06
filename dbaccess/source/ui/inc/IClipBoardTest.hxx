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


#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#define DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace dbaui
{
    class SAL_NO_VTABLE IClipboardTest
    {
    public:
        virtual sal_Bool isCutAllowed()         = 0;
        virtual sal_Bool isCopyAllowed()        = 0;
        virtual sal_Bool isPasteAllowed()       = 0;

        virtual sal_Bool hasChildPathFocus()    = 0;

        virtual void copy()     = 0;
        virtual void cut()      = 0;
        virtual void paste()    = 0;
    };
}
#endif // DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
