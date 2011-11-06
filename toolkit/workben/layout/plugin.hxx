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


#ifndef _LAYOUT_PLUGIN_HXX
#define _LAYOUT_PLUGIN_HXX

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

namespace svx {
class DialControl;
}

class PluginDialog : public ModalDialog
{
private:
    FixedImage aHeaderImage;
    FixedText aHeaderText;
    FixedLine aHeaderLine;
    Plugin aPlugin;
    svx::DialControl& aDialControl;
    OKButton aOKBtn;
    CancelButton aCancelBtn;
    HelpButton aHelpBtn;

public:
    PluginDialog( Window* pParent );
    ~PluginDialog();
};

#include <layout/layout-post.hxx>

#endif /* _LAYOUT_PLUGIN_HXX */

