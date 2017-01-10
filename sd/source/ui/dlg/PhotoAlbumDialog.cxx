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
#include "PhotoAlbumDialog.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

namespace sd
{

OUString SdPhotoAlbumDialog::sDirUrl;

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
    get(pCapCheck, "cap_check");
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

    mpGraphicFilter = new GraphicFilter;
    sDirUrl.clear();
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
    pCapCheck.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, CancelHdl, Button*, void)
{
    Close();
}

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, CreateHdl, Button*, void)
{
    if (pImagesLst->GetEntryCount() == 0)
    {
        ScopedVclPtrInstance< WarningBox > aWarning(this, WB_OK, SD_RESSTR(STR_PHOTO_ALBUM_EMPTY_WARNING));
        aWarning->Execute();
    }
    else
    {
        Reference< drawing::XDrawPagesSupplier > xDPS( pDoc->getUnoModel(), uno::UNO_QUERY );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
        Reference< lang::XMultiServiceFactory > xShapeFactory( pDoc->getUnoModel(), uno::UNO_QUERY );

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // get the option
        const sal_Int32 nOpt = pInsTypeCombo->GetSelectEntryPos();
        if ( nOpt == ONE_IMAGE )
        {
            OUString sUrl;
            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                OUString const * pData = static_cast<OUString const *>(pImagesLst->GetEntryData(i));
                sUrl = *pData;

                Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );

                Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl, xProvider);

                Image aImg(xGraphic);
                // Save the original size, multiplied with 100
                ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                Reference< drawing::XShape > xShape(
                    xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                    uno::UNO_QUERY);

                Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    "Width") >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    "Height") >>= aPageSize.Height;

                ::awt::Point aPicPos;

                if(pASRCheck->IsChecked())
                {
                    // Resize the image, with keeping ASR
                    aPicSize = createASRSize(aPicSize, aPageSize);
                }
                else
                {
                    aPicSize.Width = aPageSize.Width;
                    aPicSize.Height = aPageSize.Height;
                }

                xShape->setSize(aPicSize);
                aPicPos.X = (aPageSize.Width - aPicSize.Width)/2;
                aPicPos.Y = (aPageSize.Height - aPicSize.Height)/2;

                xShape->setPosition(aPicPos);
                xSlide->add(xShape);
                if(pCapCheck->IsChecked())
                    createCaption( aPageSize );
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
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
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
                sUrl1 = pData ? OUString(*pData) : "";
                // grab the right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = static_cast<OUString const *>(pD2);
                sUrl2 = pData ? OUString(*pData) : "";

                if( !sUrl1.isEmpty() )
                {

                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                }

                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                    if(pCapCheck->IsChecked())
                        createCaption( aPageSize );

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
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
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
                sUrl1 = pData ? OUString(*pData) : "";

                // grab the upper right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = static_cast<OUString *>(pD2);
                sUrl2 = pData ? OUString(*pData) : "";

                // grab the lower left one
                void* pD3 = pImagesLst->GetEntryData(i+2);
                pData = static_cast<OUString*>(pD3);
                sUrl3 = pData ? OUString(*pData) : "";

                // grab the lower right one
                void* pD4 = pImagesLst->GetEntryData(i+3);
                pData = static_cast<OUString*>(pD4);
                sUrl4 = pData ? OUString(*pData) : "";

                if( !sUrl1.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl1, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                }
                if( !sUrl2.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl2, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                }
                if( !sUrl3.isEmpty() )
                {

                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl3, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                }
                if( !sUrl4.isEmpty() )
                {
                    Reference< graphic::XGraphic > xGraphic = createXGraphicFromUrl(sUrl4, xProvider);

                    Image aImg(xGraphic);
                    // Save the original size, multiplied with 100
                    ::awt::Size aPicSize(aImg.GetSizePixel().Width()*100, aImg.GetSizePixel().Height()*100);

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY);

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
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
                    xSlide->add(xShape);
                    if(pCapCheck->IsChecked())
                        createCaption( aPageSize );

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

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, FileHdl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
        SFXWB_GRAPHIC | SFXWB_MULTISELECTION
    );
    // Read configuration
    OUString sUrl(officecfg::Office::Impress::Pictures::Path::get());

    INetURLObject aFile( SvtPathOptions().GetUserConfigPath() );
    if (!sUrl.isEmpty())
        aDlg.SetDisplayDirectory(sUrl);
    else
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

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
                sal_Int16 nPos = pImagesLst->InsertEntry( aUrl.GetLastName(INetURLObject::DECODE_WITH_CHARSET) );
                pImagesLst->SetEntryData(nPos, new OUString(aUrl.GetMainURL(INetURLObject::NO_DECODE)));
            }
        }
    }
    EnableDisableButtons();
}

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, UpHdl, Button*, void)
{
    if (pImagesLst->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectEntryPos() != 0)
    {
        const sal_Int32 nActPos = pImagesLst->GetSelectEntryPos();
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

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, DownHdl, Button*, void)
{
    const sal_Int32 nActPos = pImagesLst->GetSelectEntryPos();
    if (!pImagesLst->GetEntry(nActPos + 1).isEmpty())
    {
        OUString sActEntry( pImagesLst->GetSelectEntry() );
        OUString* pActData = static_cast<OUString*>(pImagesLst->GetSelectEntryData());
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

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, RemoveHdl, Button*, void)
{
    pImagesLst->RemoveEntry( pImagesLst->GetSelectEntryPos() );
    pImg->SetImage(Image());

    EnableDisableButtons();
}

IMPL_LINK_NOARG_TYPED(SdPhotoAlbumDialog, SelectHdl, ListBox&, void)
{
    OUString* pData = static_cast<OUString*>(pImagesLst->GetSelectEntryData());
    OUString sImgUrl = pData ? OUString(*pData) : "";

    if (sImgUrl != SD_RESSTR(STR_PHOTO_ALBUM_TEXTBOX))
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
        sal_Int32 nPrevWidth = 200;
        sal_Int32 nPrewHeight = 150;

        double nXRatio = (double) nPrevWidth / nBmpWidth;
        double nYRatio = (double) nPrewHeight / nBmpHeight;
        if ( nXRatio < nYRatio )
            aBmp.Scale( nXRatio, nXRatio );
        else
            aBmp.Scale( nYRatio, nYRatio );

        aBmp.Convert( BMP_CONVERSION_24BIT );
        pImg->SetImage(Image(aBmp));
    }
    else
    {
        pImg->SetImage(Image());
    }
    EnableDisableButtons();
}

Reference< drawing::XDrawPage > SdPhotoAlbumDialog::appendNewSlide(AutoLayout aLayout,
    Reference< drawing::XDrawPages > xDrawPages
)
{
    Reference< drawing::XDrawPage > xSlide; // Create the slide
    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
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

void SdPhotoAlbumDialog::createCaption(const awt::Size& aPageSize )
{
    Point CapPos;
    Size CapSize;

    CapSize.Width() = aPageSize.Width;
    CapSize.Height() = aPageSize.Height/6;
    CapPos.X() = 0;
    CapPos.Y() = aPageSize.Height - CapSize.Height();
    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD );
    Rectangle rRect(CapPos,CapSize);
    SdrObject* pSdrObj = pSlide->CreatePresObj(PRESOBJ_TITLE,false,rRect);
    SdrModel* pSdrModel = pDoc->AllocModel();
    SfxItemSet aSet(pSdrModel->GetItemPool() );
    aSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
    aSet.Put( XFillColorItem( "", Color(COL_BLACK) ) );
    aSet.Put( XFillTransparenceItem( 20 ) );
    pSdrObj->SetMergedItemSetAndBroadcast(aSet);
}

Reference< graphic::XGraphic> SdPhotoAlbumDialog::createXGraphicFromUrl(const OUString& sUrl,
    Reference< graphic::XGraphicProvider> xProvider
)
{
    // The same as above, except this returns an XGraphic from the image URL
    ::comphelper::NamedValueCollection aMediaProperties;
    aMediaProperties.put( "URL", OUString( sUrl ) );
    Reference< graphic::XGraphic> xGraphic =
        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
    return xGraphic;
}

short SdPhotoAlbumDialog::Execute()
{
    return ModalDialog::Execute();
}

void SdPhotoAlbumDialog::EnableDisableButtons()
{
    pRemoveBtn->Enable(pImagesLst->GetSelectEntryCount() > 0);
    pUpBtn->Enable(pImagesLst->GetSelectEntryCount() > 0 &&
                   pImagesLst->GetSelectEntryPos() != 0);
    pDownBtn->Enable(pImagesLst->GetSelectEntryCount() > 0 &&
                     pImagesLst->GetSelectEntryPos() < pImagesLst->GetEntryCount()-1);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
