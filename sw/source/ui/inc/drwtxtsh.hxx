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


#ifndef _SWDRWTXTSH_HXX
#define _SWDRWTXTSH_HXX

#include <sfx2/shell.hxx>
#include "shellid.hxx"

class SdrView;
class SwView;
class SwWrtShell;
class SfxModule;

class SwDrawTextShell: public SfxShell
{
    SwView      &rView;

    SdrView     *pSdrView;

    sal_Bool        bRotate : 1;
    sal_Bool        bSelMove: 1;

    void        SetAttrToMarked(const SfxItemSet& rAttr);
    void        InsertSymbol(SfxRequest& rReq);
    sal_Bool        IsTextEdit();
public:
    SFX_DECL_INTERFACE(SW_DRWTXTSHELL)
    TYPEINFO();

    SwView     &GetView() { return rView; }
    SwWrtShell &GetShell();

                SwDrawTextShell(SwView &rView);
    virtual     ~SwDrawTextShell();

    virtual ::svl::IUndoManager*
                GetUndoManager();

    void        StateDisableItems(SfxItemSet &);

    void        Execute(SfxRequest &);
    void        ExecDraw(SfxRequest &);
    void        GetStatePropPanelAttr(SfxItemSet &);
    void        GetState(SfxItemSet &);
    void        GetDrawTxtCtrlState(SfxItemSet&);

    void        ExecFontWork(SfxRequest& rReq);
    void        StateFontWork(SfxItemSet& rSet);
    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);
    void        ExecDrawLingu(SfxRequest &rReq);
    void        ExecUndo(SfxRequest &rReq);
    void        StateUndo(SfxItemSet &rSet);
    void        ExecClpbrd(SfxRequest &rReq);
    void        StateClpbrd(SfxItemSet &rSet);
    void        StateInsert(SfxItemSet &rSet);
    void        ExecTransliteration(SfxRequest &);

    void        Init();
    void        StateStatusline(SfxItemSet &rSet);
};

#endif
