/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uitest/sfx_uiobject.hxx>

#include <sfx2/tabdlg.hxx>

SfxTabDialogUIObject::SfxTabDialogUIObject(VclPtr<SfxTabDialog> xTabDialog):
    mxTabDialog(xTabDialog)
{
}

SfxTabDialogUIObject::~SfxTabDialogUIObject()
{
}

StringMap SfxTabDialogUIObject::get_state()
{
}

void SfxTabDialogUIObject::execute(const OUString& /*rAction*/,
        const StringMap& /*rParameters*/)
{

}

UIObjectType SfxTabDialogUIObject::get_type() const
{
}

std::unique_ptr<UIObject> SfxTabDialogUIObject::create(vcl::Window* pWindow)
{
    SfxTabDialog* pDialog = dynamic_cast<SfxTabDialog*>(pWindow);
    assert(pDialog);
    return std::unique_ptr<UIObject>(new SfxTabDialogUIObject(pDialog));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
