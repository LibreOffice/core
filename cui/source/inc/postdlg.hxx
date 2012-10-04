/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SVX_POSTDLG_HXX
#define _SVX_POSTDLG_HXX

#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/svmedit.hxx>

// class SvxPostItDialog -------------------------------------------------
/*
    {k:\svx\prototyp\dialog\memo.bmp}

    [Description]
    In this dialog a note can be created or edited. If the
    application holds a list of notes, it can be iterated
    over this list with links.

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
    inline void         HideAuthor()        { aAuthorFT.Hide(); aAuthorBtn.Hide(); }
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
    DECL_LINK(Stamp, void *);
    DECL_LINK(OKHdl, void *);
    DECL_LINK(PrevHdl, void *);
    DECL_LINK(NextHdl, void *);
#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
