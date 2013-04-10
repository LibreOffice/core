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

#ifndef _SVX_SIDEBAR_POPUP_CONTROL_HXX_
#define _SVX_SIDEBAR_POPUP_CONTROL_HXX_

#include "svx/svxdllapi.h"
#include <vcl/ctrl.hxx>

namespace svx { namespace sidebar {

/** Base class for sidebar related popup controls.
    A PopupControl is typically a child of a PopupContainer and
    provides the actual content of a popup.
    This base class takes care of painting the proper background and
    border for sidebar popups.
    Specialize by derivation.
*/
class SVX_DLLPUBLIC PopupControl
    : public Control
{
public :
    PopupControl (
        Window* pParent,
        const ResId& rResId);
    virtual ~PopupControl (void);

    virtual void Paint (const Rectangle& rect);
};

} } // end of namespace svx::sidebar

#endif
