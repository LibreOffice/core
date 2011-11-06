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


#ifndef SCUI_AUTOFMT_HXX
#define SCUI_AUTOFMT_HXX
#include "autofmt.hxx"

class ScAutoFormatDlg : public ModalDialog
{
public:
            ScAutoFormatDlg( Window*                    pParent,
                             ScAutoFormat*              pAutoFormat,
                             const ScAutoFormatData*    pSelFormatData,
                             ScDocument*                pDoc );
            ~ScAutoFormatDlg();

    sal_uInt16 GetIndex() const { return nIndex; }
    String GetCurrFormatName();

private:
    FixedLine       aFlFormat;
    ListBox         aLbFormat;
    ScAutoFmtPreview*   pWndPreview;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;
    FixedLine       aFlFormatting;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    CheckBox        aBtnAdjust;
    PushButton      aBtnRename;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRename;

    //------------------------
    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    sal_uInt16                  nIndex;
    sal_Bool                    bCoreDataChanged;
    sal_Bool                    bFmtInserted;

    void Init           ();
    void UpdateChecks   ();
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( SelFmtHdl, void * );
    DECL_LINK( CloseHdl, PushButton * );
    DECL_LINK( DblClkHdl, void * );
    DECL_LINK( RenameHdl, void *);

};
#endif

