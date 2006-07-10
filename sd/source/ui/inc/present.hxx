/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: present.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 11:22:22 $
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


#ifndef _SD_PRESENT_HXX_
#define _SD_PRESENT_HXX_

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class SfxItemSet;
class List;

/*************************************************************************
|* Dialog zum Festlegen von Optionen und Starten der Praesentation
\************************************************************************/
class SdStartPresentationDlg : public ModalDialog
{
private:

    FixedLine           aGrpRange;
    RadioButton         aRbtAll;
    RadioButton         aRbtAtDia;
    RadioButton         aRbtCustomshow;
    ListBox             aLbDias;
    ListBox             aLbCustomshow;

    FixedLine           aGrpKind;
    RadioButton         aRbtStandard;
    RadioButton         aRbtWindow;
    RadioButton         aRbtAuto;
    // FixedText        aFtPause;
    TimeField           aTmfPause;
    CheckBox            aCbxAutoLogo;

    FixedLine           aGrpOptions;
    CheckBox            aCbxManuel;
    CheckBox            aCbxMousepointer;
    CheckBox            aCbxPen;
    CheckBox            aCbxNavigator;
    CheckBox            aCbxAnimationAllowed;
    CheckBox            aCbxChangePage;
    CheckBox            aCbxAlwaysOnTop;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    List*               pCustomShowList;
    const SfxItemSet&   rOutAttrs;

                        DECL_LINK( ChangeRangeHdl, void * );
                        DECL_LINK( ClickWindowPresentationHdl, void * );
                        DECL_LINK( ChangePauseHdl, void * );

public:
                        SdStartPresentationDlg( Window* pWindow,
                                const SfxItemSet& rInAttrs,
                                List& rPageNames,
                                List* pCSList );

    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // _SD_PRESENT_HXX_

