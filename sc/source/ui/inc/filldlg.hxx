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



#ifndef SC_FILLDLG_HXX
#define SC_FILLDLG_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include "global.hxx"

class ScDocument;

//----------------------------------------------------------------------------

//CHINA001 #define  FDS_OPT_NONE        0
//CHINA001 #define  FDS_OPT_HORZ        1
//CHINA001 #define  FDS_OPT_VERT        2
//CHINA001
#include "scui_def.hxx" //CHINA001
//============================================================================

class ScFillSeriesDlg : public ModalDialog
{
public:
            ScFillSeriesDlg( Window*        pParent,
                             ScDocument&    rDocument,
                             FillDir        eFillDir,
                             FillCmd        eFillCmd,
                             FillDateCmd    eFillDateCmd,
                             String         aStartStr,
                             double         fStep,
                             double         fMax,
                             sal_uInt16         nPossDir );
            ~ScFillSeriesDlg();

    FillDir     GetFillDir() const          { return theFillDir; }
    FillCmd     GetFillCmd() const          { return theFillCmd; }
    FillDateCmd GetFillDateCmd() const      { return theFillDateCmd; }
    double      GetStart() const            { return fStartVal; }
    double      GetStep() const             { return fIncrement; }
    double      GetMax() const              { return fEndVal; }

    String      GetStartStr() const         { return aEdStartVal.GetText(); }

    void        SetEdStartValEnabled(sal_Bool bFlag=sal_False);

private:
    FixedText       aFtStartVal;
    Edit            aEdStartVal;
    String          aStartStrVal;

    FixedText       aFtEndVal;
    Edit            aEdEndVal;

    FixedText       aFtIncrement;
    Edit            aEdIncrement;
    FixedLine       aFlDirection;
    RadioButton     aBtnDown;
    RadioButton     aBtnRight;
    RadioButton     aBtnUp;
    RadioButton     aBtnLeft;

    FixedLine       aFlSep1;
    FixedLine       aFlType;
    RadioButton     aBtnArithmetic;
    RadioButton     aBtnGeometric;
    RadioButton     aBtnDate;
    RadioButton     aBtnAutoFill;

    FixedLine       aFlSep2;
    FixedLine       aFlTimeUnit;
    RadioButton     aBtnDay;
    RadioButton     aBtnDayOfWeek;
    RadioButton     aBtnMonth;
    RadioButton     aBtnYear;
    sal_Bool        bStartValFlag;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    const String    errMsgInvalidVal;

    //----------------------------------------------------------

    ScDocument& rDoc;
    FillDir     theFillDir;
    FillCmd     theFillCmd;
    FillDateCmd theFillDateCmd;
    double      fStartVal;
    double      fIncrement;
    double      fEndVal;

#ifdef _FILLDLG_CXX
private:
    void Init( sal_uInt16 nPossDir );
    sal_Bool CheckStartVal();
    sal_Bool CheckIncrementVal();
    sal_Bool CheckEndVal();

    DECL_LINK( OKHdl, void * );
    DECL_LINK( DisableHdl, Button * );
#endif
};



#endif // SC_FILLDLG_HXX

