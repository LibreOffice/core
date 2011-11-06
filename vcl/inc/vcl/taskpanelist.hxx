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



#ifndef _SV_TASKPANELIST_HXX
#define _SV_TASKPANELIST_HXX

#include <vcl/dllapi.h>

#include <vector>
#include <vcl/window.hxx>

class VCL_DLLPUBLIC TaskPaneList
{
    ::std::vector<Window *> mTaskPanes;
    Window *FindNextPane( Window *pWindow, sal_Bool bForward = sal_True );
    Window *FindNextFloat( Window *pWindow, sal_Bool bForward = sal_True );
    Window *FindNextSplitter( Window *pWindow, sal_Bool bForward = sal_True );

//#if 0 // _SOLAR__PRIVATE
public:
    sal_Bool IsInList( Window *pWindow );
//#endif

public:
    TaskPaneList();
    ~TaskPaneList();

    void AddWindow( Window *pWindow );
    void RemoveWindow( Window *pWindow );
    sal_Bool HandleKeyEvent( KeyEvent aKeyEvent );
};

#endif
