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


#ifndef _SWDRAWSH_HXX
#define _SWDRAWSH_HXX

#include "drwbassh.hxx"

class SwDrawBaseShell;

class SwDrawShell: public SwDrawBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_DRAWSHELL)

                SwDrawShell(SwView &rView);

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        ExecDrawDlg(SfxRequest& rReq);
    void        ExecDrawAttrArgs(SfxRequest& rReq);
    void        GetDrawAttrState(SfxItemSet &rSet);

    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);

    // #123922# added helper methods to handle applying graphic data to draw objects
    SdrObject* IsSingleFillableNonOLESelected();
    void InsertPictureFromFile(SdrObject& rObject);
};

#endif
