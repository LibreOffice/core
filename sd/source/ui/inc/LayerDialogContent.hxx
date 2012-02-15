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



#ifndef SD_LAYER_DIALOG_CONTENT_HXX
#define SD_LAYER_DIALOG_CONTENT_HXX

#include <sfx2/dockwin.hxx>
#include "LayerTabBar.hxx"
#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

namespace sd {

class ViewShellBase;

class LayerDialogContent
    : public SfxDockingWindow
{
public:
    LayerDialogContent (
        SfxBindings* pBindings,
        SfxChildWindow *pCW,
        ::Window* pParent,
        const SdResId& rSdResId,
        ViewShellBase& rBase);
    virtual ~LayerDialogContent (void);

protected:
    virtual sal_Bool Close (void);
    virtual void Resize (void);

private:
    LayerTabBar maLayerTabBar;
};

} // end of namespace sd

#endif

