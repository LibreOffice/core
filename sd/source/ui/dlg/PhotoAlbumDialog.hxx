/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _SD_PHOTOALBUMDIALOG_HXX
#define _SD_PHOTOALBUMDIALOG_HXX

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

class SdPhotoAlbumDialog : public ModalDialog
{
public:
    SdPhotoAlbumDialog(Window* pWindow, SdDrawDocument* pActDoc);
    ~SdPhotoAlbumDialog();

private:
    CancelButton*   pCancelBtn;
    PushButton*     pCreateBtn;

    PushButton*     pFileBtn;
    PushButton*     pTextBtn;
    PushButton*     pUpBtn;
    PushButton*     pDownBtn;
    PushButton*     pRemoveBtn;

    ListBox*        pImagesLst;
    FixedImage*     pImg;

    ListBox*   pInsTypeCombo;

    SdDrawDocument* pDoc;
    GraphicFilter* mpGraphicFilter;

    DECL_LINK(CancelHdl, void*);
    DECL_LINK(CreateHdl, void*);

    DECL_LINK(FileHdl, void*);
    DECL_LINK(TextHdl, void*);
    DECL_LINK(UpHdl, void*);
    DECL_LINK(DownHdl, void*);
    DECL_LINK(RemoveHdl, void*);

    DECL_LINK(SelectHdl, void*);

    void setFirstSlide(SdPage* pFirstSlide);
};

}

#endif // _SD_PHOTOALBUMDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
