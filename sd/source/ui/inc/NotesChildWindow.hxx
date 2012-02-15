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



#ifndef SD_NOTES_CHILD_WINDOW_HXX
#define SD_NOTES_CHILD_WINDOW_HXX

#include <sfx2/childwin.hxx>

#define NOTES_CHILD_WINDOW() (                                      \
    static_cast< ::sd::toolpanel::NotesChildWindow*>(               \
        SfxViewFrame::Current()->GetChildWindow(                        \
            ::sd::toolpanel::NotesChildWindow::GetChildWindowId()   \
            )->GetWindow()))


namespace sd { namespace notes {

class NotesChildWindow
    : public SfxChildWindow
{
public:
    NotesChildWindow (::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);
    virtual ~NotesChildWindow (void);

    SFX_DECL_CHILDWINDOW (NotesChildWindow);
};


} } // end of namespaces ::sd::notes

#endif
