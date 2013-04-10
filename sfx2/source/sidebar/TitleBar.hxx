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

#ifndef SFX_SIDEBAR_TITLE_BAR_HXX
#define SFX_SIDEBAR_TITLE_BAR_HXX

#include "Paint.hxx"

#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>


namespace sfx2 { namespace sidebar {

class TitleBar
    : public Window
{
public:
    TitleBar (
        const ::rtl::OUString& rsTitle,
        Window* pParentWindow,
        const sidebar::Paint& rInitialBackgroundPaint);
    virtual ~TitleBar (void);

    void SetTitle (const ::rtl::OUString& rsTitle);

    virtual void Paint (const Rectangle& rUpdateArea);
    virtual void DataChanged (const DataChangedEvent& rEvent);
    virtual void SetPosSizePixel (
        long nX,
        long nY,
        long nWidth,
        long nHeight,
        sal_uInt16 nFlags = WINDOW_POSSIZE_ALL);

    ToolBox& GetToolBox (void);

protected:
    ToolBox maToolBox;

    virtual Rectangle GetTitleArea (const Rectangle& rTitleBarBox) = 0;
    virtual void PaintDecoration (const Rectangle& rTitleBarBox) = 0;
    virtual void PaintFocus (const Rectangle& rFocusBox);
    virtual sidebar::Paint GetBackgroundPaint (void) = 0;
    virtual Color GetTextColor (void) = 0;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);

private:
    ::rtl::OUString msTitle;

    void PaintTitle (const Rectangle& rTitleBox);
    DECL_LINK(SelectionHandler, ToolBox*);
};


} } // end of namespace sfx2::sidebar

#endif
