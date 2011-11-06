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



#ifndef _VERSDLG_HXX
#define _VERSDLG_HXX

// #include *****************************************************************

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#include <svtools/svmedit.hxx>
#include <sfx2/basedlgs.hxx>

class SfxObjectShell;
class SfxViewFrame;
struct SfxVersionInfo;

class SfxVersionsTabListBox_Impl  : public SvTabListBox
{
    virtual void                KeyInput( const KeyEvent& rKeyEvent );

public:
                                SfxVersionsTabListBox_Impl(
                                    Window* pParent, const ResId& );
};

class SfxVersionTableDtor;
class SfxVersionDialog : public SfxModalDialog
{
    FixedLine                   aNewGroup;
    PushButton                  aSaveButton;
    CheckBox                    aSaveCheckBox;
    FixedLine                   aExistingGroup;
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    FixedText                   aCommentText;
    SfxVersionsTabListBox_Impl  aVersionBox;
    CancelButton                aCloseButton;
    PushButton                  aOpenButton;
    PushButton                  aViewButton;
    PushButton                  aDeleteButton;
    PushButton                  aCompareButton;
    HelpButton                  aHelpButton;
    SfxViewFrame*               pViewFrame;
    SfxVersionTableDtor*        mpTable;
    LocaleDataWrapper*          mpLocaleWrapper;
    sal_Bool                    mbIsSaveVersionOnClose;

    DECL_LINK(                  DClickHdl_Impl, Control* );
    DECL_LINK(                  SelectHdl_Impl, Control* );
    DECL_LINK(                  ButtonHdl_Impl, Button* );
    void                        Init_Impl();
    void                        Open_Impl();
    void                        RecalcDateColumn();

public:
                                SfxVersionDialog ( SfxViewFrame* pFrame, sal_Bool );
    virtual                     ~SfxVersionDialog ();
    sal_Bool                    IsSaveVersionOnClose() const { return mbIsSaveVersionOnClose; }
};

class SfxViewVersionDialog_Impl : public SfxModalDialog
{
    FixedText                   aDateTimeText;
    FixedText                   aSavedByText;
    MultiLineEdit               aEdit;
    OKButton                    aOKButton;
    CancelButton                aCancelButton;
    PushButton                  aCloseButton;
    HelpButton                  aHelpButton;
    SfxVersionInfo*             pInfo;

    DECL_LINK(                  ButtonHdl, Button* );

public:
                                SfxViewVersionDialog_Impl( Window *pParent,
                                    SfxVersionInfo& rInfo, sal_Bool bEdit );
 };

#endif
