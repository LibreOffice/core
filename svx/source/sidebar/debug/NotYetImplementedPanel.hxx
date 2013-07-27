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

#ifndef SVX_SIDEBAR_DEBUG_NOT_YET_IMPLEMENTED_PANEL_HXX
#define SVX_SIDEBAR_DEBUG_NOT_YET_IMPLEMENTED_PANEL_HXX

#include <vcl/window.hxx>
#include <vcl/fixed.hxx>


namespace svx { namespace sidebar {


/** Display a panel as placeholder for a not-yet-implemented panel.
*/
class NotYetImplementedPanel
    : public Window
{
public:
    NotYetImplementedPanel (::Window* pParent);
    virtual ~NotYetImplementedPanel (void);

private:
    FixedText maMessageControl;
};

} } // end of namespace ::svx::sidebar

#endif
