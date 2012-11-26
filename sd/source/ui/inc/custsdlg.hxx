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




#ifndef _SD_CUSTSDLG_HXX
#define _SD_CUSTSDLG_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/dialog.hxx>

class SdDrawDocument;
class SdCustomShow;

//------------------------------------------------------------------------

class SdCustomShowDlg : public ModalDialog
{
private:
    ListBox         aLbCustomShows;
    CheckBox        aCbxUseCustomShow;
    PushButton      aBtnNew;
    PushButton      aBtnEdit;
    PushButton      aBtnRemove;
    PushButton      aBtnCopy;
    HelpButton      aBtnHelp;
    PushButton      aBtnStartShow;
    OKButton        aBtnOK;

    SdDrawDocument& rDoc;
    List*           pCustomShowList;
    SdCustomShow*   pCustomShow;
    bool            bModified;

    void            CheckState();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( StartShowHdl, Button* );

public:
                SdCustomShowDlg( Window* pWindow, SdDrawDocument& rDrawDoc );
                ~SdCustomShowDlg();

    bool        IsModified() const { return( bModified ); }
    bool        IsCustomShow() const;
};


//------------------------------------------------------------------------

class SdDefineCustomShowDlg : public ModalDialog
{
private:
    FixedText       aFtName;
    Edit            aEdtName;
    FixedText       aFtPages;
    MultiListBox    aLbPages;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    FixedText       aFtCustomPages;
    SvTreeListBox   aLbCustomPages;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    SdDrawDocument& rDoc;
    SdCustomShow*&  rpCustomShow;
    bool            bModified;
    String          aOldName;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( OKHdl, Button* );

public:

                    SdDefineCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS );
                    ~SdDefineCustomShowDlg();

    bool            IsModified() const { return( bModified ); }
};

#endif // _SD_CUSTSDLG_HXX

