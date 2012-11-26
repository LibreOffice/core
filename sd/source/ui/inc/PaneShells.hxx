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



#ifndef SD_PANE_SHELLS_HXX
#define SD_PANE_SHELLS_HXX


#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include "glob.hxx"

class CommandEvent;

namespace sd {


/** Shell that displays the left pane for Impress.  The shell does not do
    anything else and has especially no slots.
*/
class LeftImpressPaneShell
    : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SD_IF_SDLEFTIMPRESSPANESHELL)

    LeftImpressPaneShell (void);
    virtual ~LeftImpressPaneShell (void);
};




/** Shell that displays the left pane for Draw.  The shell does not do
    anything else and has especially no slots.
*/
class LeftDrawPaneShell
    : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SD_IF_SDLEFTDRAWPANESHELL)

    LeftDrawPaneShell (void);
    virtual ~LeftDrawPaneShell (void);
};




/** Shell that displays the right pane for both Impress and Draw.  The shell
    does not do anything else and has especially no slots.
*/
class ToolPanelPaneShell : public SfxShell
{
public:
    SFX_DECL_INTERFACE( SD_IF_SDTOOLPANELPANESHELL )

    ToolPanelPaneShell();
    virtual ~ToolPanelPaneShell();
};

} // end of namespace sd

#endif
