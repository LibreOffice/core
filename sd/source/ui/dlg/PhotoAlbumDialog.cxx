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

#include <vcl/msgbox.hxx>
#include <svx/unoshape.hxx>

#include "PhotoAlbumDialog.hxx"
#include "strings.hrc"
#include "sdresid.hxx"

namespace sd
{

OUString SdPhotoAlbumDialog::sDirUrl;

SdPhotoAlbumDialog::SdPhotoAlbumDialog(Window* pWindow, SdDrawDocument* pActDoc)
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
    sDirUrl = "";
    pAddBtn->GrabFocus();
    pImagesLst->Clear();
}

SdPhotoAlbumDialog::~SdPhotoAlbumDialog()
{
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CancelHdl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CreateHdl)
{
    if (pImagesLst->GetEntryCount() == 0)
    {
        WarningBox aWarning(this, WB_OK, SD_RESSTR(STR_PHOTO_ALBUM_EMPTY_WARNING));
        aWarning.Execute();
    }
    else
    {
        Reference< drawing::XDrawPagesSupplier > xDPS( pDoc->getUnoModel(), uno::UNO_QUERY );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
        Reference< lang::XMultiServiceFactory > xShapeFactory( pDoc->getUnoModel(), uno::UNO_QUERY );

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // get the option
        sal_uInt16 nOpt = pInsTypeCombo->GetSelectEntryPos();
        if ( nOpt == ONE_IMAGE )
        {
            OUString sUrl;
            for( sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                OUString* pData = (OUString*) pImagesLst->GetEntryData(i);
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
                    OUString("Width")) >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    OUString("Height")) >>= aPageSize.Height;

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
            }
        }
        else if( nOpt == TWO_IMAGES )
        {
            OUString sUrl1("");
            OUString sUrl2("");

            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); i+=2 )
            {
                OUString* pData = NULL;

                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    OUString("Width")) >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    OUString("Height")) >>= aPageSize.Height;

                // grab the left one
                void* pD1 = pImagesLst->GetEntryData(i);
                pData = (OUString*) pD1;
                sUrl1 = pData ? OUString(*pData) : "";
                // grab the right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = (OUString*) pD2;
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
                OUString* pData = NULL;
                // create the slide
                Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
                Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                //Slide dimensions
                ::awt::Size aPageSize;

                xSlideProps->getPropertyValue(
                    OUString("Width")) >>= aPageSize.Width;
                xSlideProps->getPropertyValue(
                    OUString("Height")) >>= aPageSize.Height;

                // grab the upper left one
                void* pD1 = pImagesLst->GetEntryData(i);
                pData = (OUString*) pD1;
                sUrl1 = pData ? OUString(*pData) : "";

                // grab the upper right one
                void* pD2 = pImagesLst->GetEntryData(i+1);
                pData = (OUString*) pD2;
                sUrl2 = pData ? OUString(*pData) : "";

                // grab the lower left one
                void* pD3 = pImagesLst->GetEntryData(i+2);
                pData = (OUString*) pD3;
                sUrl3 = pData ? OUString(*pData) : "";

                // grab the lower right one
                void* pD4 = pImagesLst->GetEntryData(i+3);
                pData = (OUString*) pD4;
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
                }
            }
        }
        else
        {
            InfoBox aInfo(this, OUString("Function is not implemented!"));
            aInfo.Execute();
        }
        EndDialog();
        return 0;
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, FileHdl)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
        SFXWB_GRAPHIC | SFXWB_MULTISELECTION
    );
    // Read configuration
    OUString sUrl(".");
    try
    {
        sUrl = officecfg::Office::Impress::Pictures::Path::get();
    }
    catch(const Exception&)
    {
        OSL_FAIL("Could not find config for Create Photo Album function");
    }


    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    if (sUrl != "")
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
            try
            {
                boost::shared_ptr< comphelper::ConfigurationChanges > batch(
                    comphelper::ConfigurationChanges::create());
                officecfg::Office::Impress::Pictures::Path::set(sUrl, batch);
                batch->commit();
            }
            catch(const Exception&)
            {
                OSL_FAIL("Could not find config for Create Photo Album function");
            }

            for ( sal_Int32 i = 0; i < aFilesArr.getLength(); i++ )
            {
                // Store full path, show filename only. Use INetURLObject to display spaces in filename correctly
                INetURLObject aUrl = INetURLObject(aFilesArr[i]);
                sal_Int16 nPos = pImagesLst->InsertEntry( aUrl.GetLastName(INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8) );
                pImagesLst->SetEntryData(nPos, (OUString*) new OUString(aUrl.GetMainURL(INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8)));
            }
        }
    }
    EnableDisableButtons();
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl)
{
    if (pImagesLst->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectEntryPos() != 0)
    {
        sal_uInt16 nActPos = pImagesLst->GetSelectEntryPos();
        OUString sActEntry( pImagesLst->GetEntry(nActPos) );
        // actual data
        OUString* pActData = (OUString*) pImagesLst->GetEntryData(nActPos);
        OUString sAct(*pActData);

        OUString sUpperEntry( pImagesLst->GetEntry(nActPos - 1) );
        // upper data
        OUString* pUpperData = (OUString*) pImagesLst->GetEntryData(nActPos - 1);
        OUString sUpper(*pUpperData);

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sUpperEntry );

        pImagesLst->InsertEntry( sActEntry, nActPos - 1 );
        pImagesLst->SetEntryData( nActPos - 1, (void*) new OUString(sAct));

        pImagesLst->InsertEntry( sUpperEntry, nActPos );
        pImagesLst->SetEntryData( nActPos, (void*) new OUString(sUpper));

        pImagesLst->SelectEntryPos(nActPos - 1);
    }

    EnableDisableButtons();
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, DownHdl)
{
    sal_uInt16 nActPos = pImagesLst->GetSelectEntryPos();
    if (pImagesLst->GetEntry(nActPos + 1) != OUString("") )
    {
        OUString sActEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
        OUString* pActData = (OUString*) pImagesLst->GetEntryData(pImagesLst->GetSelectEntryPos());
        OUString sAct(*pActData);

        OUString sDownEntry( pImagesLst->GetEntry(nActPos + 1) );
        OUString* pDownData = (OUString*) pImagesLst->GetEntryData(nActPos + 1);
        OUString sDown(*pDownData);

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sDownEntry );

        pImagesLst->InsertEntry( sDownEntry, nActPos );
        pImagesLst->SetEntryData( nActPos, (void*) new OUString(sDown));

        pImagesLst->InsertEntry( sActEntry, nActPos + 1 );
        pImagesLst->SetEntryData( nActPos + 1, (void*) new OUString(sAct));

        pImagesLst->SelectEntryPos(nActPos + 1);

    }
    EnableDisableButtons();
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl)
{
    pImagesLst->RemoveEntry( pImagesLst->GetSelectEntryPos() );
    pImg->SetImage(Image());

    EnableDisableButtons();
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl)
{
    OUString* pData = (OUString*) pImagesLst->GetEntryData(pImagesLst->GetSelectEntryPos());
    OUString sImgUrl = pData ? OUString(*pData) : "";

    if (sImgUrl != SD_RESSTR(STR_PHOTO_ALBUM_TEXTBOX))
    {
        GraphicFilter aCurFilter;
        Graphic aGraphic;
        INetURLObject aURLObj( sImgUrl );

        sal_uInt16 nFilter = GRFILTER_FORMAT_DONTKNOW;

        if ( aURLObj.HasError() || INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
        {
            aURLObj.SetSmartProtocol( INET_PROT_FILE );
            aURLObj.SetSmartURL( sImgUrl );
        }

        sal_uInt32 nFilterImportFlags = GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG;
        // remote?
        if ( INET_PROT_FILE != aURLObj.GetProtocol() )
        {
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( sImgUrl, STREAM_READ );

            if( pStream )
                mpGraphicFilter->ImportGraphic( aGraphic, sImgUrl, *pStream, nFilter, NULL, nFilterImportFlags );
            else
                mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
            delete pStream;
        }
        else
        {
            mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
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
        pImg->SetImage( aBmp );
    }
    else
    {
        pImg->SetImage(Image());
    }
    EnableDisableButtons();
    return 0;
}

Reference< drawing::XDrawPage > SdPhotoAlbumDialog::appendNewSlide(AutoLayout aLayout,
    Reference< drawing::XDrawPages > xDrawPages
)
{
    Reference< drawing::XDrawPage > xSlide; // Create the slide
    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
    pSlide->SetAutoLayout(aLayout, sal_True); // Set the layout here
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

Reference< drawing::XShape > SdPhotoAlbumDialog::createXShapeFromUrl(const OUString& sUrl,
    Reference< lang::XMultiServiceFactory > xShapeFactory,
    Reference< graphic::XGraphicProvider> xProvider
)
{
    //First, we create an XGraphic
    ::comphelper::NamedValueCollection aMediaProperties;
    aMediaProperties.put( "URL", OUString( sUrl ) );
    Reference< graphic::XGraphic> xGraphic =
        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
    //And then, we can create the XShape from the XGraphic
    Reference< drawing::XShape > xShape(
        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
        uno::UNO_QUERY
    );

    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

    return xShape; // Image loaded into XShape
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
