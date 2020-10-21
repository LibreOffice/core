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

class TemplateDefaultView final : public TemplateLocalView
{
public:
    TemplateDefaultView(Window *pParent);

    virtual void reload() override;

    virtual void showAllTemplates () override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    void createContextMenu();

private:
    tools::Long    mnItemMaxSize;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
