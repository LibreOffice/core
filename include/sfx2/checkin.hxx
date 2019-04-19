/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

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

    OUString GetComment();
    bool IsMajor();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
