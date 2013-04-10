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



#ifndef SD_PANE_CHILD_WINDOWS_HXX
#define SD_PANE_CHILD_WINDOWS_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/taskpane.hxx>

namespace sd {

class PaneChildWindow
    : public SfxChildWindow
{
public:
    PaneChildWindow (
        ::Window* pParentWindow,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo,
        const sal_uInt16 nDockWinTitleResId,
        const sal_uInt16 nTitleBarResId,
        SfxChildAlignment eAlignment);
    virtual ~PaneChildWindow (void);
};




class LeftPaneImpressChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneImpressChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneImpressChildWindow);
};




class LeftPaneDrawChildWindow
    : public PaneChildWindow
{
public:
    LeftPaneDrawChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(LeftPaneDrawChildWindow);
};


} // end of namespace ::sd

#endif
