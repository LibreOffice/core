/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

namespace dbaui
{
class OChildWindow
{
protected:
    OChildWindow(weld::Container* pParent, const OUString& rUIXMLDescription, const OString& rID);
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;

public:
    virtual ~OChildWindow();

    virtual void GrabFocus() = 0;

    virtual bool HasChildPathFocus() const = 0;

    void Enable(bool bEnable) { m_xContainer->set_sensitive(bEnable); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
