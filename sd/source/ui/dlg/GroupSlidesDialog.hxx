/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _SD_GROUPSLIDESDIALOG_HXX
#define _SD_GROUPSLIDESDIALOG_HXX

#include "tools/link.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/svdotext.hxx>

class SdrTextObj;
class SdDrawDocument;
class SdPage;

namespace sd
{

class SdGroupSlidesDialog : public ModalDialog
{
public:
    SdGroupSlidesDialog(Window* pWindow, SdDrawDocument* pActDoc,
                        const std::vector< SdPage * > &rPages );
    ~SdGroupSlidesDialog();

private:
    CancelButton*   pCancelBtn;

    SdDrawDocument* pDoc;
    std::vector< SdPage * > maPages;

    DECL_LINK(CancelHdl, void*);
};

}

#endif // _SD_GROUPSLIDESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
