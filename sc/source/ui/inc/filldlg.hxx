/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

