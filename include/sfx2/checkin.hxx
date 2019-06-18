/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SFX2_CHECKIN_HXX
#define INCLUDED_SFX2_CHECKIN_HXX

#include <vcl/weld.hxx>

class SfxCheckinDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::TextView> m_xCommentED;
    std::unique_ptr<weld::CheckButton> m_xMajorCB;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_LINK(OKHdl, weld::Button&, void);

public:
    SfxCheckinDialog(weld::Window* pParent);
    virtual ~SfxCheckinDialog() override;

    OUString GetComment() const;
    bool IsMajor() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
