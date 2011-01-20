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


#ifndef _SD_DLGFIELD_HXX
#define _SD_DLGFIELD_HXX


#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <svx/langbox.hxx>
class SvxFieldData;

/*************************************************************************
|*
|* Dialog zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

class SdModifyFieldDlg : public ModalDialog
{
private:
    FixedLine           aGrpType;
    RadioButton         aRbtFix;
    RadioButton         aRbtVar;
    FixedText           maFtLanguage;
    SvxLanguageBox      maLbLanguage;
    FixedText           aFtFormat;
    ListBox             aLbFormat;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    SfxItemSet          maInputSet;

    const SvxFieldData* pField;

    void                FillFormatList();
    void                FillControls();

    DECL_LINK( LanguageChangeHdl, void * );

public:
    SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    ~SdModifyFieldDlg() {}

    SvxFieldData*       GetField();
    SfxItemSet          GetItemSet();
};



#endif      // _SD_DLGFIELD_HXX
