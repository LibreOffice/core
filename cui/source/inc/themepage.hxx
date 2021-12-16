/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <sfx2/tabdlg.hxx>

#include <memory>

class ColorListBox;

/// Tab page for themes
class SvxThemePage : public SfxTabPage
{
    static const WhichRangesContainer m_pRanges;

    std::unique_ptr<weld::Entry> m_xThemeName;
    std::unique_ptr<weld::Entry> m_xColorSetName;
    std::unique_ptr<ColorListBox> m_xDk1;
    std::unique_ptr<ColorListBox> m_xLt1;
    std::unique_ptr<ColorListBox> m_xDk2;
    std::unique_ptr<ColorListBox> m_xLt2;
    std::unique_ptr<ColorListBox> m_xAccent1;
    std::unique_ptr<ColorListBox> m_xAccent2;
    std::unique_ptr<ColorListBox> m_xAccent3;
    std::unique_ptr<ColorListBox> m_xAccent4;
    std::unique_ptr<ColorListBox> m_xAccent5;
    std::unique_ptr<ColorListBox> m_xAccent6;
    std::unique_ptr<ColorListBox> m_xHlink;
    std::unique_ptr<ColorListBox> m_xFolHlink;

public:
    SvxThemePage(weld::Container* pPage, weld::DialogController* pController,
                 const SfxItemSet& rInAttrs);
    virtual ~SvxThemePage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet*);
    static WhichRangesContainer GetRanges() { return m_pRanges; }

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
