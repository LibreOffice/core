/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SD_SOURCE_UI_DLG_PHOTOALBUMDIALOG_HXX
#define INCLUDED_SD_SOURCE_UI_DLG_PHOTOALBUMDIALOG_HXX

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
    SdPhotoAlbumDialog(vcl::Window* pWindow, SdDrawDocument* pActDoc);
    virtual ~SdPhotoAlbumDialog();
    virtual void dispose() override;

    virtual short Execute() override;

private:
    static OUString sDirUrl;
    VclPtr<CancelButton>   pCancelBtn;
    VclPtr<PushButton>     pCreateBtn;

    VclPtr<PushButton>     pAddBtn;
    VclPtr<PushButton>     pUpBtn;
    VclPtr<PushButton>     pDownBtn;
    VclPtr<PushButton>     pRemoveBtn;

    VclPtr<ListBox>        pImagesLst;
    VclPtr<FixedImage>     pImg;

    VclPtr<ListBox>    pInsTypeCombo;
    VclPtr<CheckBox>   pASRCheck;
    VclPtr<CheckBox>   pASRCheckCrop;
    VclPtr<CheckBox>   pCapCheck;
    VclPtr<CheckBox>   pInsertAsLinkCheck;

    SdDrawDocument* pDoc;
    GraphicFilter* mpGraphicFilter;

    DECL_LINK_TYPED(CancelHdl, Button*, void);
    DECL_LINK_TYPED(CreateHdl, Button*, void);

    DECL_LINK_TYPED(FileHdl, Button*, void);
    DECL_LINK_TYPED(UpHdl, Button*, void);
    DECL_LINK_TYPED(DownHdl, Button*, void);
    DECL_LINK_TYPED(RemoveHdl, Button*, void);

    DECL_LINK_TYPED(SelectHdl, ListBox&, void);

    Reference< drawing::XDrawPage > appendNewSlide(AutoLayout aLayout,
        const Reference< drawing::XDrawPages >& xDrawPages);

    static awt::Size createASRSize(const awt::Size& aPicSize, const awt::Size& aMaxSize);
    static awt::Size createASRSizeCrop(const awt::Size& aPicSize, const awt::Size& aMaxSize);
    void createCaption(const awt::Size& aPageSize);
    static Reference< graphic::XGraphic> createXGraphicFromUrl(const OUString& sUrl,
        const Reference< graphic::XGraphicProvider>& xProvider);

    void EnableDisableButtons();

    enum SlideImageLayout
    {
        ONE_IMAGE=0,
        TWO_IMAGES,
        FOUR_IMAGES
    };

};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_DLG_PHOTOALBUMDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
