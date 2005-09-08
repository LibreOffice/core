/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:54:51 $
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
#ifndef _SVX_POSTDLG_HXX
#define _SVX_POSTDLG_HXX

// include ---------------------------------------------------------------

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

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
                     BOOL bPrevNext = FALSE, BOOL bRedline = FALSE );
    ~SvxPostItDialog();

    static USHORT*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    Link                GetPrevHdl() const { return aPrevHdlLink; }
    void                SetPrevHdl( const Link& rLink )
                            { aPrevHdlLink = rLink; }
    Link                GetNextHdl() const { return aNextHdlLink; }
    void                SetNextHdl( const Link& rLink )
                            { aNextHdlLink = rLink; }

    void                EnableTravel(BOOL bNext, BOOL bPrev);
    inline String       GetNote() { return aEditED.GetText(); }
    inline void         SetNote(const String& rTxt) { aEditED.SetText(rTxt); }

    void                ShowLastAuthor(const String& rAuthor, const String& rDate);
    inline void         DontChangeAuthor()  { aAuthorBtn.Enable(FALSE); }
    inline void         HideAuthor()        { aAuthorBtn.Hide(); }
    inline void         SetReadonlyPostIt(BOOL bDisable)
                            {
                                aOKBtn.Enable( !bDisable );
                                aEditED.SetReadOnly( bDisable );
                                aAuthorBtn.Enable( !bDisable );
                            }
    inline BOOL         IsOkEnabled() const { return aOKBtn.IsEnabled(); }

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

