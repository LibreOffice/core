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
#pragma once

#include <sfx2/basedlgs.hxx>

// class SvxPostItDialog -------------------------------------------------
/*
    [Description]
    In this dialog a note can be created or edited. If the
    application holds a list of notes, it can be iterated
    over this list with links.

    [Items]
    <SvxPostItAuthorItem><SID_ATTR_POSTIT_AUTHOR>
    <SvxPostItDateItem><SID_ATTR_POSTIT_DATE>
    <SvxPostItTextItem><SID_ATTR_POSTIT_TEXT>
*/

class SvxPostItDialog : public SfxDialogController
{
public:
    SvxPostItDialog(weld::Widget* pParent, const SfxItemSet& rCoreSet,
                    bool bPrevNext);
    virtual ~SvxPostItDialog() override;

    static const sal_uInt16*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return m_xOutSet.get(); }

    void                SetPrevHdl( const Link<SvxPostItDialog&,void>& rLink )
                            { m_aPrevHdlLink = rLink; }
    void                SetNextHdl( const Link<SvxPostItDialog&,void>& rLink )
                            { m_aNextHdlLink = rLink; }

    void EnableTravel(bool bNext, bool bPrev);
    OUString GetNote() const
    {
        return m_xEditED->get_text();
    }
    void SetNote(const OUString& rTxt)
    {
        m_xEditED->set_text(rTxt);
    }
    void ShowLastAuthor(const OUString& rAuthor, const OUString& rDate);
    void DontChangeAuthor()
    {
        m_xAuthorBtn->set_sensitive(false);
    }
    void HideAuthor()
    {
        m_xInsertAuthor->hide();
    }
    void set_title(const OUString& rTitle)
    {
        m_xDialog->set_title(rTitle);
    }
    std::shared_ptr<weld::Dialog> const & GetDialog() const
    {
        return m_xDialog;
    }

private:
    const SfxItemSet&   m_rSet;
    std::unique_ptr<SfxItemSet>     m_xOutSet;

    Link<SvxPostItDialog&,void>  m_aPrevHdlLink;
    Link<SvxPostItDialog&,void>  m_aNextHdlLink;

    std::unique_ptr<weld::Label>    m_xLastEditFT;
    std::unique_ptr<weld::Label>    m_xAltTitle;
    std::unique_ptr<weld::TextView> m_xEditED;
    std::unique_ptr<weld::Widget>   m_xInsertAuthor;
    std::unique_ptr<weld::Button>   m_xAuthorBtn;
    std::unique_ptr<weld::Button>   m_xOKBtn;
    std::unique_ptr<weld::Button>   m_xPrevBtn;
    std::unique_ptr<weld::Button>   m_xNextBtn;

    DECL_LINK(Stamp, weld::Button&, void);
    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(PrevHdl, weld::Button&, void);
    DECL_LINK(NextHdl, weld::Button&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
