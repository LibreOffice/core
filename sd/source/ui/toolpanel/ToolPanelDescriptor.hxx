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



#ifndef SD_TOOL_PANEL_DESCRIPTOR_HXX
#define SD_TOOL_PANEL_DESCRIPTOR_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <memory>

class Window;

namespace sd { namespace toolpanel {


/** Collection of information the describes entries of the tool
    panel.  A descriptor owns the control it is associated with.
*/
class ToolPanelDescriptor
{
public:
    /** Create a new descriptor for the given control.
        @param pControl
    */
    ToolPanelDescriptor (::std::auto_ptr< ::Window> pControl,
        const String& rTitle);

    ~ToolPanelDescriptor (void);

    /** Return the height of the title bar.
        @return
            The title bar height is returned in pixels.
    */
    int GetTitleBarHeight (void) const;


    void SetPositionAndSize (const Point& aPosition,
        const Size& aSize);
    void SetPositionAndSize (const Rectangle& aBox);
    void SetWeight (double nWeight);

    Window* GetControl (void) const;
    const String& GetTitle (void) const;
    const Rectangle& GetTitleBox (void) const;
    Rectangle GetPositionAndSize (void) const;
    double GetWeight (void) const;

    int GetTotalHeight (void) const;
    int GetWindowHeight (void) const;

private:
    ::std::auto_ptr< ::Window> mpControl;
    String msTitle;
    Rectangle maTitleBox;
    double mnWeight;
    int mnTotalHeight;

    /// Do not use! Assignment operator is not supported.
    const ToolPanelDescriptor& operator= (
        const ToolPanelDescriptor& aDescriptor);
};

} } // end of namespace ::sd::toolpanel

#endif
