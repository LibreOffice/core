/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <officecfg/Office/Impress.hxx>
#include <svx/svdview.hxx>
#include <vcl/msgbox.hxx>
#include <svx/unoshape.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xfillit.hxx>
#include <xmloff/autolayout.hxx>

#include "PhotoAlbumDialog.hxx"
#include <strings.hrc>
#include <sdresid.hxx>

namespace sd
{

SdPhotoAlbumDialog::SdPhotoAlbumDialog(vcl::Window* pWindow, SdDrawDocument* pActDoc)
: ModalDialog(pWindow, "PhotoAlbumCreatorDialog", "modules/simpress/ui/photoalbum.ui"),
  pDoc(pActDoc)
{
    get(pCancelBtn, "cancel_btn");
    get(pCreateBtn, "create_btn");

    get(pAddBtn, "add_btn");
    get(pUpBtn, "up_btn");
    get(pDownBtn, "down_btn");
    get(pRemoveBtn, "rem_btn");

    get(pImagesLst, "images_tree");
    get(pImg, "preview_img");

    get(pInsTypeCombo, "opt_combo");
    get(pASRCheck, "asr_check");
    get(pASRCheckCrop, "asr_check_crop");
    get(pCapCheck, "cap_check");
    get(pInsertAsLinkCheck, "insert_as_link_check");

    pCancelBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CancelHdl));
    pCreateBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CreateHdl));

    pAddBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, FileHdl));
    pUpBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, UpHdl));
    pUpBtn->Disable();
    pDownBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, DownHdl));
    pDownBtn->Disable();
    pRemoveBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, RemoveHdl));
    pRemoveBtn->Disable();
    pImagesLst->SetSelectHdl(LINK(this, SdPhotoAlbumDialog, SelectHdl));
    pInsTypeCombo->SetSelectHdl(LINK(this, SdPhotoAlbumDialog, TypeSelectHdl));

    mpGraphicFilter = new GraphicFilter;
    pAddBtn->GrabFocus();
    pImagesLst->Clear();
}

SdPhotoAlbumDialog::~SdPhotoAlbumDialog()
{
    disposeOnce();
}

void SdPhotoAlbumDialog::dispose()
{
    pCancelBtn.clear();
    pCreateBtn.clear();
    pAddBtn.clear();
    pUpBtn.clear();
    pDownBtn.clear();
    pRemoveBtn.clear();
    pImagesLst.clear();
    pImg.clear();
    pInsTypeCombo.clear();
    pASRCheck.clear();
    pASRCheckCrop.clear();
    pCapCheck.clear();
    pInsertAsLinkCheck.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CancelHdl, Button*, void)
{
    Close();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CreateHdl, Button*, void)
{
    if (pImagesLst->GetEntryCount() == 0)
    {
        ScopedVclPtrInstance< WarningBox > aWarning(this, MessBoxStyle::Ok, SdResId(STR_PHOTO_ALBUM_EMPTY_WARNING));
        aWarning->Execute();
    }
    else
    {
        Reference< drawing::XDrawPagesSupplier > xDPS( pDoc->getUnoModel(), uno::UNO_QUERY );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
        Reference< lang::XMultiServiceFactory > xShapeFactory( pDoc->getUnoModel(), uno::UNO_QUERY );

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // determine if to use Captions (use TitleObject) and choose the correct AutoLayout
        // from the beginning
        const bool bCreateCaptions(pCapCheck->IsChecked());
        const bool bInsertAsLink(pInsertAsLinkCheck->IsChecked());
        const AutoLayout aAutoLayout(bCreateCaptions ? AUTOLAYOUT_TITLE_ONLY : AUTOLAYOUT_NONE);

        // get the option
        const sal_Int32 nOpt = pInsTypeCombo->GetSelectedEntryPos();
        if ( nOpt == ONE_IMAGE )
        {
            OUString sUrl;
            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                OUString const * pData = static_cast<OUString const *>(pImagesLst->GetEntryData(i));
                sUrl = *pData;

                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );

                Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl, xProvider);

                Graphic aImg(xGraphic);
                // Save the original size, multiplied with 100
                ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                Reference< drawing::XShape > xShape(
                    xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                    uno::UNO_QUERY);

                Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                if (bInsertAsLink)
                    xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl));
                else
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                ::awt::Point aPicPos;

                if(pASRCheck->IsChecked() && !pASRCheckCrop->IsChecked())
                {
                    // Resize the image, with keeping ASR
                    aPicSize = createASRSize(aPicSize, aPageSize);
                }
                else if(pASRCheckCrop->IsChecked())
                {
                    aPicSize = createASRSizeCrop(aPicSize, aPageSize);
                }

                xShape->setSize(aPicSize);
                aPicPos.X = (aPageSize.Width - aPicSize.Width)/2;
                aPicPos.Y = (aPageSize.Height - aPicSize.Height)/2;

                xShape->setPosition(aPicPos);
                try
                {
                    xSlide->add(xShape);
                    if (bCreateCaptions)
                        createCaption( aPageSize );
                }
                catch (const css::uno::Exception& exc)
                {
                    SAL_WARN( "sd", exc );
                }
            }
        }
        else if( nOpt == TWO_IMAGES )
        {
            OUString sUrl1("");
            OUString sUrl2("");

            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); i+=2 )
            {
                OUString const * pData = nullptr;

                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                // grab the left one
                void* pD1 = pImagesLst->GetEntryData(i);
                pData = static_cast<OUString const *>(pD1);
                sUrl1 = pData ? *pData : "";
                // grab the right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = static_cast<OUString const *>(pD2);
                sUrl2 = pData ? *pData : "";

                if( !sUrl1.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl1));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/2 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }

                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl2));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/2 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);

                    try
                    {
                        xSlide->add(xShape);
                        if(bCreateCaptions)
                            createCaption( aPageSize );
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }
            }
        }
        else if( nOpt == FOUR_IMAGES )
        {
            OUString sUrl1("");
            OUString sUrl2("");
            OUString sUrl3("");
            OUString sUrl4("");

            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); i+=4 )
            {
                OUString* pData = nullptr;
                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(aAutoLayout, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                // grab the upper left one
                void* pD1 = pImagesLst->GetEntryData(i);
                pData = static_cast<OUString*>(pD1);
                sUrl1 = pData ? *pData : "";

                // grab the upper right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = static_cast<OUString *>(pD2);
                sUrl2 = pData ? *pData : "";

                // grab the lower left one
                void* pD3 = pImagesLst->GetEntryData(i+2);
                pData = static_cast<OUString*>(pD3);
                sUrl3 = pData ? *pData : "";

                // grab the lower right one
                void* pD4 = pImagesLst->GetEntryData(i+3);
                pData = static_cast<OUString*>(pD4);
                sUrl4 = pData ? *pData : "";

                if( !sUrl1.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl1));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }
                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl2));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }
                if( !sUrl3.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl3, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl3));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2);
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2 + aPageSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }
                if( !sUrl4.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl4, xProvider);

                    Graphic aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    if (bInsertAsLink)
                        xProps->setPropertyValue("GraphicURL", ::uno::Any(sUrl4));
                    else
                        xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Point aPicPos;

                    if(pASRCheck->IsChecked())
                    {
                        // Resize the image, with keeping ASR
                        aPicSize = createASRSize(aPicSize, ::awt::Size(aPageSize.Width/2 - 100, aPageSize.Height/2 - 100));
                    }
                    else
                    {
                        aPicSize.Width = aPageSize.Width/2 - 100;
                        aPicSize.Height = aPageSize.Height/2 - 100;
                    }
                    xShape->setSize(aPicSize);
                    aPicPos.X = (aPageSize.Width/4 - aPicSize.Width/2) + aPageSize.Width/2;
                    aPicPos.Y = aPageSize.Height/4 - aPicSize.Height/2 + aPageSize.Height/2;

                    xShape->setPosition(aPicPos);
                    try
                    {
                        xSlide->add(xShape);
                        if(bCreateCaptions)
                            createCaption( aPageSize );
                    }
                    catch (const css::uno::Exception& exc)
                    {
                        SAL_WARN( "sd", exc );
                    }
                }
            }
        }
        else
        {
            ScopedVclPtrInstance< InfoBox > aInfo(this, OUString("Function is not implemented!"));
            aInfo->Execute();
        }
        EndDialog();
    }
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, FileHdl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_PREVIEW,
        FileDialogFlags::Graphic | FileDialogFlags::MultiSelection, this
    );
    // Read configuration
    OUString sUrl(officecfg::Office::Impress::Pictures::Path::get());

    INetURLObject aFile( SvtPathOptions().GetUserConfigPath() );
    if (!sUrl.isEmpty())
        aDlg.SetDisplayDirectory(sUrl);
    else
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        Sequence< OUString > aFilesArr = aDlg.GetSelectedFiles();
        if( aFilesArr.getLength() )
        {
            sUrl = aDlg.GetDisplayDirectory();
            // Write out configuration
            {
                std::shared_ptr< comphelper::ConfigurationChanges > batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Impress::Pictures::Path::set(sUrl, batch);
                batch->commit();
            }

            for ( sal_Int32 i = 0; i < aFilesArr.getLength(); i++ )
            {
                // Store full path, show filename only. Use INetURLObject to display spaces in filename correctly
                INetURLObject aUrl = INetURLObject(aFilesArr[i]);
                sal_Int16 nPos = pImagesLst->InsertEntry( aUrl.GetLastName(INetURLObject::DecodeMechanism::WithCharset) );
                pImagesLst->SetEntryData(nPos, new OUString(aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE)));
            }
        }
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl, Button*, void)
{
    if (pImagesLst->GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectedEntryPos() != 0)
    {
        const sal_Int32 nActPos = pImagesLst->GetSelectedEntryPos();
        OUString sActEntry( pImagesLst->GetEntry(nActPos) );
        // actual data
        OUString* pActData = static_cast<OUString*>(pImagesLst->GetEntryData(nActPos));
        OUString sAct(*pActData);

        OUString sUpperEntry( pImagesLst->GetEntry(nActPos - 1) );
        // upper data
        OUString* pUpperData = static_cast<OUString*>(pImagesLst->GetEntryData(nActPos - 1));
        OUString sUpper(*pUpperData);

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sUpperEntry );

        pImagesLst->InsertEntry( sActEntry, nActPos - 1 );
        pImagesLst->SetEntryData( nActPos - 1, new OUString(sAct));

        pImagesLst->InsertEntry( sUpperEntry, nActPos );
        pImagesLst->SetEntryData( nActPos, new OUString(sUpper));

        pImagesLst->SelectEntryPos(nActPos - 1);
    }

    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, DownHdl, Button*, void)
{
    const sal_Int32 nActPos = pImagesLst->GetSelectedEntryPos();
    if (!pImagesLst->GetEntry(nActPos + 1).isEmpty())
    {
        OUString sActEntry( pImagesLst->GetSelectedEntry() );
        OUString* pActData = static_cast<OUString*>(pImagesLst->GetSelectedEntryData());
        OUString sAct(*pActData);

        OUString sDownEntry( pImagesLst->GetEntry(nActPos + 1) );
        OUString* pDownData = static_cast<OUString*>(pImagesLst->GetEntryData(nActPos + 1));
        OUString sDown(*pDownData);

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sDownEntry );

        pImagesLst->InsertEntry( sDownEntry, nActPos );
        pImagesLst->SetEntryData( nActPos, new OUString(sDown));

        pImagesLst->InsertEntry( sActEntry, nActPos + 1 );
        pImagesLst->SetEntryData( nActPos + 1, new OUString(sAct));

        pImagesLst->SelectEntryPos(nActPos + 1);

    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl, Button*, void)
{
    pImagesLst->RemoveEntry( pImagesLst->GetSelectedEntryPos() );
    pImg->SetImage(Image());

    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl, ListBox&, void)
{
    OUString* pData = static_cast<OUString*>(pImagesLst->GetSelectedEntryData());
    OUString sImgUrl = pData ? *pData : "";

    if (sImgUrl != SdResId(STR_PHOTO_ALBUM_TEXTBOX))
    {
        GraphicFilter aCurFilter;
        Graphic aGraphic;
        INetURLObject aURLObj( sImgUrl );

        sal_uInt16 nFilter = GRFILTER_FORMAT_DONTKNOW;

        if ( aURLObj.HasError() || INetProtocol::NotValid == aURLObj.GetProtocol() )
        {
            aURLObj.SetSmartProtocol( INetProtocol::File );
            aURLObj.SetSmartURL( sImgUrl );
        }

        GraphicFilterImportFlags nFilterImportFlags = GraphicFilterImportFlags::SetLogsizeForJpeg;
        // remote?
        if ( INetProtocol::File != aURLObj.GetProtocol() )
        {
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( sImgUrl, StreamMode::READ );

            if( pStream )
                mpGraphicFilter->ImportGraphic( aGraphic, sImgUrl, *pStream, nFilter, nullptr, nFilterImportFlags );
            else
                mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
            delete pStream;
        }
        else
        {
            mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
        }

        Bitmap aBmp = aGraphic.GetBitmap();
        sal_Int32 nBmpWidth  = aBmp.GetSizePixel().Width();
        sal_Int32 nBmpHeight = aBmp.GetSizePixel().Height();

        double nXRatio = (double) 200 / nBmpWidth;
        double nYRatio = (double) 150 / nBmpHeight;
        if ( nXRatio < nYRatio )
            aBmp.Scale( nXRatio, nXRatio );
        else
            aBmp.Scale( nYRatio, nYRatio );

        aBmp.Convert( BmpConversion::N24Bit );
        pImg->SetImage(Image(aBmp));
    }
    else
    {
        pImg->SetImage(Image());
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, TypeSelectHdl, ListBox&, void)
{
    // Enable "Fill Slide" only for one image
    // If we want to have it for other images too, we need to implement the actual cropping.
    bool const bEnable = pInsTypeCombo->GetSelectedEntryPos() == ONE_IMAGE;
    pASRCheckCrop->Enable(bEnable);
    if (!bEnable)
        pASRCheckCrop->Check(false);
}

Reference< drawing::XDrawPage > SdPhotoAlbumDialog::appendNewSlide(AutoLayout aLayout,
    const Reference< drawing::XDrawPages >& xDrawPages
)
{
    Reference< drawing::XDrawPage > xSlide; // Create the slide
    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PageKind::Standard)-1, PageKind::Standard);
    pSlide->SetAutoLayout(aLayout, true); // Set the layout here
    return xSlide;
}

awt::Size SdPhotoAlbumDialog::createASRSize(const awt::Size& aPicSize, const awt::Size& aMaxSize)
{
    double resizeWidth = aPicSize.Width;
    double resizeHeight = aPicSize.Height;
    double aspect = resizeWidth/resizeHeight;

    if( resizeWidth > aMaxSize.Width )
    {
        resizeWidth = aMaxSize.Width;
        resizeHeight = resizeWidth / aspect;
    }

    if( resizeHeight > aMaxSize.Height )
    {
        aspect = resizeWidth/resizeHeight;
        resizeHeight = aMaxSize.Height;
        resizeWidth = resizeHeight * aspect;
    }
    return awt::Size(resizeWidth, resizeHeight);
}

awt::Size SdPhotoAlbumDialog::createASRSizeCrop(const awt::Size& aPicSize, const awt::Size& aMaxSize)
{
    double resizeWidth = aPicSize.Width;
    double resizeHeight = aPicSize.Height;
    double imgAspect = resizeWidth / resizeHeight;
    double windowAspectRatio = static_cast<double>(aMaxSize.Width) / aMaxSize.Height ;


    //When both sides of an image are bigger than canvas size, image would be downscaled.
    if( resizeWidth > aMaxSize.Width && resizeHeight > aMaxSize.Height )
    {
        if( imgAspect > windowAspectRatio )
        {
            resizeHeight = aMaxSize.Height;
            resizeWidth = aMaxSize.Height * imgAspect;
        }
        else
        {
            resizeHeight = aMaxSize.Width / imgAspect;
            resizeWidth = aMaxSize.Width;
        }

    }
    //In all other cases image is upscaled
    else
    {
        if( imgAspect > windowAspectRatio )
        {
            resizeHeight = aMaxSize.Height;
            resizeWidth = aMaxSize.Height * imgAspect;
        }
        else
        {
            resizeWidth = aMaxSize.Width;
            resizeHeight = aMaxSize.Width / imgAspect;
        }
    }
    return awt::Size(resizeWidth, resizeHeight);
}

void SdPhotoAlbumDialog::createCaption(const awt::Size& aPageSize )
{
    Point CapPos;
    Size CapSize;

    CapSize.Width() = aPageSize.Width;
    CapSize.Height() = aPageSize.Height/6;
    CapPos.X() = 0;
    CapPos.Y() = aPageSize.Height - CapSize.Height();
    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PageKind::Standard)-1, PageKind::Standard );

    // try to get existing PresObj
    const ::tools::Rectangle rRect(CapPos,CapSize);
    SdrObject* pSdrObj = pSlide->GetPresObj(PRESOBJ_TITLE);

    if(!pSdrObj)
    {
        // if not exists, create. Beware: It is already inserted to the SdPage
        pSdrObj = pSlide->CreatePresObj(PRESOBJ_TITLE,false,rRect);
    }
    else
    {
        // if exists, bring to front and position it
        const size_t nObjNum(pSlide->GetObjCount());

        if(nObjNum)
        {
            pSlide->SetObjectOrdNum(pSdrObj->GetOrdNum(), nObjNum - 1);
        }

        pSdrObj->SetSnapRect(rRect);
    }

    if(pSdrObj)
    {
        // set color, style and some transparency
        SfxItemSet aSet(pDoc->GetItemPool() );

        aSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
        aSet.Put( XFillColorItem( "", Color(COL_BLACK) ) );
        aSet.Put( XFillTransparenceItem( 20 ) );
        pSdrObj->SetMergedItemSetAndBroadcast(aSet);
    }
}

Reference< graphic::XGraphic> SdPhotoAlbumDialog::createXGraphicFromUrl(const OUString& sUrl,
    const Reference< graphic::XGraphicProvider>& xProvider
)
{
    // The same as above, except this returns an XGraphic from the image URL
    ::comphelper::NamedValueCollection aMediaProperties;
    aMediaProperties.put( "URL", sUrl );
    Reference< graphic::XGraphic> xGraphic =
        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
    return xGraphic;
}

void SdPhotoAlbumDialog::EnableDisableButtons()
{
    pRemoveBtn->Enable(pImagesLst->GetSelectedEntryCount() > 0);
    pUpBtn->Enable(pImagesLst->GetSelectedEntryCount() > 0 &&
                   pImagesLst->GetSelectedEntryPos() != 0);
    pDownBtn->Enable(pImagesLst->GetSelectedEntryCount() > 0 &&
                     pImagesLst->GetSelectedEntryPos() < pImagesLst->GetEntryCount()-1);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
