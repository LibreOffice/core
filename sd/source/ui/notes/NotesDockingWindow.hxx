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



#ifndef SD_TPV_BOTTOM_DOCKING_WINDOW_HXX_
#define SD_TPV_BOTTOM_DOCKING_WINDOW_HXX_

#include <sfx2/dockwin.hxx>

namespace sd { namespace notes {

class EditWindow;

class NotesDockingWindow
    : public SfxDockingWindow
{
public:
    NotesDockingWindow (
        SfxBindings *pBindings,
        SfxChildWindow *pChildWindow,
        Window* pParent);
    virtual ~NotesDockingWindow (void);

    virtual void Paint (const Rectangle& rBoundingBox);
    virtual void Resize (void);

private:
    EditWindow* mpEditWindow;
};

} } // end of namespace ::sd::notes

#endif
