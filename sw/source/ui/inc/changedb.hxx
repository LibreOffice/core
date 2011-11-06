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


#ifndef _CHANGEDB_HXX
#define _CHANGEDB_HXX


#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <svtools/svtreebx.hxx>
#include <svx/stddlg.hxx>
#include "dbtree.hxx"
#include <vcl/fixed.hxx>

class SwFldMgr;
class SwView;
class SwWrtShell;
struct SwDBData;

/*--------------------------------------------------------------------
     Beschreibung: Datenbank an Feldern austauschen
 --------------------------------------------------------------------*/

class SwChangeDBDlg: public SvxStandardDialog
{
    FixedLine       aDBListFL;
    FixedText       aUsedDBFT;
    FixedText       aAvailDBFT;
    SvTreeListBox   aUsedDBTLB;
    SwDBTreeList    aAvailDBTLB;
    PushButton      aAddDBPB;
    FixedInfo       aDescFT;
    FixedText       aDocDBTextFT;
    FixedText       aDocDBNameFT;
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
//  PushButton      aChangeBT;

    ImageList       aImageList;
    ImageList       aImageListHC;

    SwWrtShell      *pSh;
    SwFldMgr        *pMgr;

    DECL_LINK(TreeSelectHdl, SvTreeListBox* pBox = 0);
    DECL_LINK(ButtonHdl, Button* pBtn);
    DECL_LINK(AddDBHdl, PushButton*);

    virtual void    Apply();
    void            UpdateFlds();
    void            FillDBPopup();
    SvLBoxEntry*    Insert(const String& rDBName);
    void            ShowDBName(const SwDBData& rDBData);

public:
    SwChangeDBDlg(SwView& rVw);
    ~SwChangeDBDlg();
};

#endif
