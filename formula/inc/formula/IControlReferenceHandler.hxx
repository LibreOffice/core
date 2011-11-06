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


#ifndef FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED
#define FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED

#include "formula/formuladllapi.h"

namespace formula
{
    class RefEdit;
    class RefButton;
    class FORMULA_DLLPUBLIC SAL_NO_VTABLE IControlReferenceHandler
    {
    public:
        virtual void ShowReference(const String& _sRef) = 0;
        virtual void HideReference( sal_Bool bDoneRefMode = sal_True ) = 0;
        virtual void ReleaseFocus( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;
        virtual void ToggleCollapsed( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;
    };
} // formula
#endif // FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED
