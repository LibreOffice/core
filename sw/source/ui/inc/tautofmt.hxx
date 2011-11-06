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


#ifndef SW_TAUTOFMT_HXX
#define SW_TAUTOFMT_HXX
#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

class SwTableAutoFmt;
class AutoFmtPreview;
class SwTableAutoFmtTbl;
class SwWrtShell;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

class SwAutoFormatDlg : public SfxModalDialog
{
    FixedLine       aFlFormat;
    ListBox         aLbFormat;
    FixedLine       aFlFormats;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnRename;
    MoreButton      aBtnMore;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRenameTitle;
    String          aStrInvalidFmt;
    AutoFmtPreview* pWndPreview;

    //------------------------
    SwWrtShell*             pShell;
    SwTableAutoFmtTbl*      pTableTbl;
    sal_uInt8                   nIndex;
    sal_uInt8                   nDfltStylePos;
    sal_Bool                    bCoreDataChanged : 1;
    sal_Bool                    bSetAutoFmt : 1;


    void Init( const SwTableAutoFmt* pSelFmt );
    void UpdateChecks( const SwTableAutoFmt&, sal_Bool bEnableBtn );
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( RenameHdl, void * );
    DECL_LINK( SelFmtHdl, void * );

public:
    SwAutoFormatDlg( Window* pParent, SwWrtShell* pShell,
                        sal_Bool bSetAutoFmt = sal_True,
                        const SwTableAutoFmt* pSelFmt = 0 );
    virtual ~SwAutoFormatDlg();

    void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};


#endif // SW_AUTOFMT_HXX

