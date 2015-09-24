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
#ifndef INCLUDED_CUI_SOURCE_INC_POSTDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_POSTDLG_HXX

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/svmedit.hxx>

// class SvxPostItDialog -------------------------------------------------
/*
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
    SvxPostItDialog(vcl::Window* pParent, const SfxItemSet& rCoreSet,
                     bool bPrevNext = false);
    virtual ~SvxPostItDialog();
    virtual void dispose() SAL_OVERRIDE;

    static const sal_uInt16*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    void                SetPrevHdl( const Link<SvxPostItDialog&,void>& rLink )
                            { aPrevHdlLink = rLink; }
    void                SetNextHdl( const Link<SvxPostItDialog&,void>& rLink )
                            { aNextHdlLink = rLink; }

    void EnableTravel(bool bNext, bool bPrev);
    OUString GetNote()
    {
        return m_pEditED->GetText();
    }
    void SetNote(const OUString& rTxt)
    {
        m_pEditED->SetText(rTxt);
    }
    void ShowLastAuthor(const OUString& rAuthor, const OUString& rDate);
    void DontChangeAuthor()
    {
        m_pAuthorBtn->Enable(false);
    }
    void HideAuthor()
    {
        m_pInsertAuthor->Hide();
    }

private:
    VclPtr<FixedText>          m_pLastEditFT;

    VclPtr<VclMultiLineEdit>   m_pEditED;

    VclPtr<VclContainer>       m_pInsertAuthor;
    VclPtr<PushButton>         m_pAuthorBtn;

    VclPtr<OKButton>           m_pOKBtn;

    VclPtr<PushButton>         m_pPrevBtn;
    VclPtr<PushButton>         m_pNextBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;

    Link<SvxPostItDialog&,void>  aPrevHdlLink;
    Link<SvxPostItDialog&,void>  aNextHdlLink;

    DECL_LINK_TYPED(Stamp, Button*, void);
    DECL_LINK_TYPED(OKHdl, Button*, void);
    DECL_LINK_TYPED(PrevHdl, Button*, void);
    DECL_LINK_TYPED(NextHdl, Button*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
