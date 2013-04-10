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

#ifndef SFX_SIDEBAR_TOOLBOX_BACKGROUND_HXX
#define SFX_SIDEBAR_TOOLBOX_BACKGROUND_HXX

#include "vcl/window.hxx"

#include <tools/svborder.hxx>


class ToolBox;

namespace sfx2 { namespace sidebar {

class ToolBoxBackground
    : public Window
{
public:
    ToolBoxBackground (Window* pParentWindow);
    virtual ~ToolBoxBackground (void);

    /** Call this method once to
        a) let the ToolBoxBackground object know which ToolBox to
           monitor and
        b) so that position and sizes can be set up.
        @return
            The relative position of the child.
    */
    Point SetToolBoxChild (
        ToolBox* pChild,
        long nX,
        long nY,
        long nWidth,
        long nHeight,
        sal_uInt16 nFlags);

    virtual void Paint (const Rectangle& rRect);
    virtual void DataChanged (const DataChangedEvent& rEvent);

private:
    SvBorder maPadding;

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};


} } // end of namespace sfx2::sidebar

#endif
