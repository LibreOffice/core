/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uitest/uitest_factory.hxx>
#include <uitest/sfx_uiobject.hxx>

#include <vcl/uitest/factory.hxx>


void SfxUITestFactory::registerSfxTabDialog()
{
    registerUITestFactory(WINDOW_TABDIALOG, FactoryFunction(SfxTabDialogUIObject::create));
}

void SfxUITestFactory::registerSfxTabPage()
{

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
