/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4     -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/templatelocalview.hxx>

class TemplateDefaultView : public TemplateLocalView
{
public:
    TemplateDefaultView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                        std::unique_ptr<weld::Menu> xMenu);

    virtual void showAllTemplates() override;

    virtual bool KeyInput(const KeyEvent& rKEvt) override;

    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    void createContextMenu();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
