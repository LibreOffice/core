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

#ifndef SD_SIDEBAR_PANEL_ID_HXX
#define SD_SIDEBAR_PANEL_ID_HXX

namespace rtl
{
    class OUString;
}


namespace sd { namespace sidebar {

/** List of top level panels that can be shown in the task pane.
*/
enum PanelId
{
    PID__START = 0,
    PID_UNKNOWN = PID__START,
    PID_MASTER_PAGES_ALL,
    PID_MASTER_PAGES_RECENT,
    PID_MASTER_PAGES_USED,
    PID_LAYOUT,
    PID_TABLE_DESIGN,
    PID_ANIMATION_SCHEMES,
    PID_CUSTOM_ANIMATION,
    PID_SLIDE_TRANSITION,
    PID__END = PID_SLIDE_TRANSITION
};

} } // namespace sd::sidebar


#endif
