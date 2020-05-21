/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svtools/InterimItemWindow.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC LabelItemWindow final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::Label> m_xLabel;

public:
    LabelItemWindow(vcl::Window* pParent, const OUString& rLabel);
    void set_label(const OUString& rLabel);
    OUString get_label() const;

    void SetOptimalSize();
    virtual void dispose() override;
    virtual ~LabelItemWindow() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
