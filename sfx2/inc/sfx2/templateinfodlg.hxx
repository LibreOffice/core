/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __SFX2_TEMPLATEINFODLG_HXX__
#define __SFX2_TEMPLATEINFODLG_HXX__

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

namespace svtools {
    class ODocumentInfoPreview;
}

class SfxTemplateInfoDlg : public ModalDialog
{
public:

    SfxTemplateInfoDlg (Window *pParent = NULL);

    ~SfxTemplateInfoDlg ();

    void loadDocument (const OUString &rURL);

private:

    PushButton maBtnClose;

    Window *mpPreviewView;
    svtools::ODocumentInfoPreview *mpInfoView;
};

#endif // __SFX2_TEMPLATEINFODLG_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
