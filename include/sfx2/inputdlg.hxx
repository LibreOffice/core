/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX
#define INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX

#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>

class SFX2_DLLPUBLIC InputDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Button> m_xHelp;
    std::unique_ptr<weld::Button> m_xOk;
    std::function<bool(OUString)> mCheckEntry;
    DECL_LINK(EntryChangedHdl, weld::Entry&, void);

public:
    InputDialog(weld::Widget* pParent, const OUString& rLabelText);
    OUString GetEntryText() const;
    void SetEntryText(const OUString& rStr);
    void HideHelpBtn();
    void SetEntryMessageType(weld::EntryMessageType aType);
    void SetTooltip(const OUString& rStr);
    void setCheckEntry(std::function<bool(OUString)> aFunc);
};

#endif // INCLUDED_SFX2_SOURCE_INC_INPUTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
