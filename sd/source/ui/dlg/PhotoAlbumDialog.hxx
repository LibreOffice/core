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

#include <tools/link.hxx>
#include <sdpage.hxx>
#include <pres.hxx>
#include <drawdoc.hxx>

#include <vcl/weld.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/graphctl.hxx>
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

class SdPhotoAlbumDialog : public weld::GenericDialogController
{
public:
    SdPhotoAlbumDialog(weld::Window* pWindow, SdDrawDocument* pActDoc);
    virtual ~SdPhotoAlbumDialog() override;

private:
    SdDrawDocument* m_pDoc;
    GraphicFilter* m_pGraphicFilter;

    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::Button> m_xCreateBtn;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xUpBtn;
    std::unique_ptr<weld::Button> m_xDownBtn;
    std::unique_ptr<weld::Button> m_xRemoveBtn;
    std::unique_ptr<weld::TreeView> m_xImagesLst;
    std::unique_ptr<SvxGraphCtrl> m_xImg;
    std::unique_ptr<weld::ComboBoxText> m_xInsTypeCombo;
    std::unique_ptr<weld::CheckButton> m_xASRCheck;
    std::unique_ptr<weld::CheckButton> m_xASRCheckCrop;
    std::unique_ptr<weld::CheckButton> m_xCapCheck;
    std::unique_ptr<weld::CheckButton> m_xInsertAsLinkCheck;

    DECL_LINK(CancelHdl, weld::Button&, void);
    DECL_LINK(CreateHdl, weld::Button&, void);

    DECL_LINK(FileHdl, weld::Button&, void);
    DECL_LINK(UpHdl, weld::Button&, void);
    DECL_LINK(DownHdl, weld::Button&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(TypeSelectHdl, weld::ComboBoxText&, void);

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
