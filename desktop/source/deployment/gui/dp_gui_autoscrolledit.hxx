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


#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX

#include "svtools/svmedit2.hxx"
#include "svl/lstner.hxx"

namespace dp_gui {

/** This control shows automatically the vertical scroll bar if text is inserted,
    that does not fit into the text area. In the resource one uses MultiLineEdit
    and needs to set VScroll = TRUE
*/
class AutoScrollEdit : public ExtMultiLineEdit, public SfxListener
{
public:
    AutoScrollEdit( Window* pParent, const ResId& rResId );
    ~AutoScrollEdit();

    using ExtMultiLineEdit::Notify;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

} // namespace dp_gui

#endif
