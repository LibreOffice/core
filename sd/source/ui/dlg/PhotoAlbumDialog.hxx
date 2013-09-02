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
#include <svx/svdobj.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

class SdrTextObj;
class SdDrawDocument;
class SdPage;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace sd
{

class SdPhotoAlbumDialog : public ModalDialog
{
public:
    SdPhotoAlbumDialog(Window* pWindow, SdDrawDocument* pActDoc);
    ~SdPhotoAlbumDialog();

    virtual short Execute();

private:
    static OUString sDirUrl;
    CancelButton*   pCancelBtn;
    PushButton*     pCreateBtn;

    PushButton*     pAddBtn;
    PushButton*     pUpBtn;
    PushButton*     pDownBtn;
    PushButton*     pRemoveBtn;

    ListBox*        pImagesLst;
    FixedImage*     pImg;

    ListBox*    pInsTypeCombo;
    CheckBox*   pASRCheck;

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

    Reference< drawing::XDrawPage > appendNewSlide(AutoLayout aLayout,
        Reference< drawing::XDrawPages > xDrawPages);

    awt::Size createASRSize(const awt::Size& aPicSize, const awt::Size& aMaxSize);

    Reference< graphic::XGraphic> createXGraphicFromUrl(const OUString& sUrl,
        Reference< graphic::XGraphicProvider> xProvider);

    void EnableDisableButtons();

    enum SlideImageLayout
    {
        ONE_IMAGE=0,
        TWO_IMAGES,
        FOUR_IMAGES
    };

};

} // end of namespace sd

#endif // _SD_PHOTOALBUMDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
