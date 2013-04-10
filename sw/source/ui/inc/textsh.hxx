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



#ifndef _SWTEXTSH_HXX
#define _SWTEXTSH_HXX

#include <basesh.hxx>

class AbstractSvxPostItDialog;
class SwFldMgr;
class SvxHyperlinkItem;

class SwTextShell: public SwBaseShell
{
    SwFldMgr*   pPostItFldMgr;

    void InsertSymbol( SfxRequest& );
    void InsertHyperlink(const SvxHyperlinkItem& rHlnkItem);
    bool InsertMediaDlg( SfxRequest& );
    void ChangeHeaderOrFooter(const String& rStyleName, sal_Bool bHeader, sal_Bool bOn, sal_Bool bShowWarning);

public:
    SFX_DECL_INTERFACE(SW_TEXTSHELL)
    TYPEINFO();

//CHINA001  DECL_LINK( PostItNextHdl, Button * );
//CHINA001  DECL_LINK( PostItPrevHdl, Button * );
//CHINA001  DECL_LINK( RedlineNextHdl, Button * );
//CHINA001  DECL_LINK( RedlinePrevHdl, Button * );
    DECL_LINK( RedlineNextHdl, AbstractSvxPostItDialog * );
    DECL_LINK( RedlinePrevHdl, AbstractSvxPostItDialog * );

    void    Execute(SfxRequest &);
    void    GetState(SfxItemSet &);

    void    ExecInsert(SfxRequest &);
    void    StateInsert(SfxItemSet&);
    void    ExecDelete(SfxRequest &);
    void    ExecEnterNum(SfxRequest &);
    void    ExecBasicMove(SfxRequest &);
    void    ExecMove(SfxRequest &);
    void    ExecMovePage(SfxRequest &);
    void    ExecMoveCol(SfxRequest &);
    void    ExecMoveLingu(SfxRequest &);
    void    ExecMoveMisc(SfxRequest &);
    void    ExecField(SfxRequest &rReq);
    void    ExecSetNumber(SfxRequest &);
    void    StateField(SfxItemSet &);
    void    ExecIdx(SfxRequest &);
    void    GetIdxState(SfxItemSet &);
    void    ExecGlossary(SfxRequest &);

    void    ExecCharAttr(SfxRequest &);
    void    ExecCharAttrArgs(SfxRequest &);
    void    ExecParaAttr(SfxRequest &);
    void    ExecParaAttrArgs(SfxRequest &);
    void    ExecAttr(SfxRequest &);
    void    ExecDB(SfxRequest &);
    void    ExecTransliteration(SfxRequest &);

    void    GetAttrState(SfxItemSet &);

             SwTextShell(SwView &rView);
    virtual ~SwTextShell();
};

#endif
