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
#ifndef _SVX_POSTDLG_HXX
#define _SVX_POSTDLG_HXX

// include ---------------------------------------------------------------

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/svmedit.hxx>

// class SvxPostItDialog -------------------------------------------------
/*
    {k:\svx\prototyp\dialog\memo.bmp}

    [Beschreibung]
    In diesem Dialog kann eine Notiz erstellt oder bearbeitet werden. Wenn die
    Applikation eine Liste von Notizen haelt, kann mit Hilfe von Links, ueber
    diese Liste iteriert werden.

    [Items]
    <SvxPostitAuthorItem><SID_ATTR_POSTIT_AUTHOR>
    <SvxPostitDateItem><SID_ATTR_POSTIT_DATE>
    <SvxPostitTextItem><SID_ATTR_POSTIT_TEXT>
*/

class SvxPostItDialog : public SfxModalDialog
{
public:
    SvxPostItDialog( Window* pParent, const SfxItemSet& rCoreSet,
                     sal_Bool bPrevNext = sal_False, sal_Bool bRedline = sal_False );
    ~SvxPostItDialog();

    static sal_uInt16*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    Link                GetPrevHdl() const { return aPrevHdlLink; }
    void                SetPrevHdl( const Link& rLink )
                            { aPrevHdlLink = rLink; }
    Link                GetNextHdl() const { return aNextHdlLink; }
    void                SetNextHdl( const Link& rLink )
                            { aNextHdlLink = rLink; }

    void                EnableTravel(sal_Bool bNext, sal_Bool bPrev);
    inline String       GetNote() { return aEditED.GetText(); }
    inline void         SetNote(const String& rTxt) { aEditED.SetText(rTxt); }

    void                ShowLastAuthor(const String& rAuthor, const String& rDate);
    inline void         DontChangeAuthor()  { aAuthorBtn.Enable(sal_False); }
    inline void         HideAuthor()        { aAuthorBtn.Hide(); }
    inline void         SetReadonlyPostIt(sal_Bool bDisable)
                            {
                                aOKBtn.Enable( !bDisable );
                                aEditED.SetReadOnly( bDisable );
                                aAuthorBtn.Enable( !bDisable );
                            }
    inline sal_Bool         IsOkEnabled() const { return aOKBtn.IsEnabled(); }

private:
    FixedLine           aPostItFL;
    FixedText           aLastEditLabelFT;
    FixedInfo           aLastEditFT;

    FixedText           aEditFT;
    MultiLineEdit       aEditED;

    FixedText           aAuthorFT;
    PushButton          aAuthorBtn;

    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    ImageButton         aPrevBtn;
    ImageButton         aNextBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;

    Link                aPrevHdlLink;
    Link                aNextHdlLink;

#ifdef _SVX_POSTDLG_CXX
    DECL_LINK( Stamp, Button* );
    DECL_LINK( OKHdl, Button* );
    DECL_LINK( PrevHdl, Button* );
    DECL_LINK( NextHdl, Button* );
#endif
};


#endif

