/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sfx2/templateinfodlg.hxx>

#include <sfx2/sfxresid.hxx>
#include <svtools/DocumentInfoPreview.hxx>

#include "templateinfodlg.hrc"

SfxTemplateInfoDlg::SfxTemplateInfoDlg (Window *pParent)
    : ModalDialog(pParent,SfxResId(DLG_TEMPLATE_INFORMATION)),
      maBtnClose(this,SfxResId(BTN_TEMPLATE_INFO_CLOSE))
{
}

SfxTemplateInfoDlg::~SfxTemplateInfoDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
