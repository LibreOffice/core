/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgfield.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:26:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
class SvxFieldData;

/*************************************************************************
|*
|* Dialog zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

class SdModifyFieldDlg : public ModalDialog
{
private:
    RadioButton         aRbtFix;
    RadioButton         aRbtVar;
    FixedLine           aGrpType;
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
