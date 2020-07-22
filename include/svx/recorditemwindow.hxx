/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/InterimItemWindow.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC RecordItemWindow : public InterimItemWindow
{
public:
    RecordItemWindow(vcl::Window* _pParent);
    virtual void dispose() override;
    virtual ~RecordItemWindow() override;

    void set_text(const OUString& rText) { m_xWidget->set_text(rText); }

private:
    virtual void PositionFired(sal_Int64 nRecord) = 0;

    std::unique_ptr<weld::Entry> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivatedHdl, weld::Entry&, bool);
    // for invalidating our content when losing the focus
    DECL_LINK(FocusOutHdl, weld::Widget&, void);

    void FirePosition(bool bForce);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
