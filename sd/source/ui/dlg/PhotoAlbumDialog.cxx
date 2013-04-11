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
#include <comphelper/componentcontext.hxx>
#include <comphelper/configurationhelper.hxx>

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

    get(pFileBtn, "file_btn");
    get(pTextBtn, "textbox_btn");
    get(pUpBtn, "up_btn");
    get(pDownBtn, "down_btn");
    get(pRemoveBtn, "rem_btn");

    get(pImagesLst, "images_tree");
    get(pImg, "preview_img");

    get(pInsTypeCombo, "opt_combo");

    get(pASRCheck, "asr_check");
    get(pFilenameLab, "filename_lab");

    pCancelBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CancelHdl));
    pCreateBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CreateHdl));

    pFileBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, FileHdl));
    pTextBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, TextHdl));
    pUpBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, UpHdl));
    pUpBtn->Disable();
    pDownBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, DownHdl));
    pDownBtn->Disable();
    pRemoveBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, RemoveHdl));
    pRemoveBtn->Disable();
    pImagesLst->SetSelectHdl(LINK(this, SdPhotoAlbumDialog, SelectHdl));

    mpGraphicFilter = new GraphicFilter;
    sDirUrl = "";
    pFileBtn->GrabFocus();
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
        InfoBox aInfo(this, OUString("Please add images to the album."));
        aInfo.Execute();
    }
    else
    {
        Reference< drawing::XDrawPagesSupplier > xDPS( pDoc->getUnoModel(), uno::UNO_QUERY );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );
        Reference< lang::XMultiServiceFactory > xShapeFactory( pDoc->getUnoModel(), uno::UNO_QUERY  );

        // Create the title slide
        Reference< drawing::XDrawPage > xTitleSlide = appendNewSlide(AUTOLAYOUT_TITLE, xDrawPages);

        SdPage* pFirstSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
        SvtUserOptions aUserOptions;
        SdrObject* pTitleObj = pFirstSlide->GetPresObj(PRESOBJ_TITLE, 0);
        SvxShapeText* pTitleText = new SvxShapeText(pTitleObj);
        pTitleText->SetShapeType("com.sun.star.presentation.TitleTextShape");
        pTitleText->setString(SD_RESSTR(STR_PHOTO_ALBUM_TITLE));

        SdrObject* pTextObj = pFirstSlide->GetPresObj(PRESOBJ_TEXT, 0);
        SvxShapeText* pTextShape = new SvxShapeText(pTextObj);
        pTextShape->SetShapeType("com.sun.star.presentation.TextShape");
        pTextShape->setString(SD_RESSTR(STR_PHOTO_ALBUM_AUTHOR) + " " + aUserOptions.GetFullName());

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // get the option
        OUString sOpt = pInsTypeCombo->GetEntry(pInsTypeCombo->GetSelectEntryPos());
        if ( sOpt == "Fit to slide")
        {
            OUString sUrl;
            OUString* pData = NULL;
            for( sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                pData = (OUString*) pImagesLst->GetEntryData(i);
                sUrl = *pData;
                if (sUrl != "Text Box")
                {
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
                else // insert an empty slide, with centered text box
                {
                    appendNewSlide(AUTOLAYOUT_ONLY_TEXT, xDrawPages);
                }
            }
            delete pData;
        }
        else if( sOpt == "1 image with title" )
        {
            OUString sUrl;
            OUString* pData = NULL;
            for( sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                pData = (OUString*) pImagesLst->GetEntryData(i);
                sUrl = *pData;
                if ( sUrl != "Text Box" )
                {

                    Reference< drawing::XDrawPage > xSlide = appendNewSlide(AUTOLAYOUT_NONE, xDrawPages);
                    Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );
                    Reference< drawing::XShape > xShape = createXShapeFromUrl(sUrl, xShapeFactory, xProvider);

                    ::awt::Size aPicSize(xShape->getSize());
                    ::awt::Size aPageSize;

                    xSlideProps->getPropertyValue(
                        OUString("Width")) >>= aPageSize.Width;
                    xSlideProps->getPropertyValue(
                        OUString("Height")) >>= aPageSize.Height;

                    ::awt::Point aPicPos;

                    aPicPos.X = (aPageSize.Width - aPicSize.Width)/2;
                    aPicPos.Y = (aPageSize.Height - aPicSize.Height)/2;

                    xShape->setSize(aPicSize);
                    xShape->setPosition(aPicPos);
                    xSlide->add(xShape);

                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->SetAutoLayout(AUTOLAYOUT_TITLE_CONTENT, sal_True);
                }
                else // insert an empty slide, with centered text box
                {
                    appendNewSlide(AUTOLAYOUT_ONLY_TEXT, xDrawPages);
                }
            }
            delete pData;
        }
        else if( sOpt == "2 images" )
        {
            OUString sUrl1("");
            OUString sUrl2("");
            OUString* pData = NULL;

            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); i+=2 )
            {
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
                pData = (OUString*) pImagesLst->GetEntryData(i);
                sUrl1 = pData ? OUString(*pData) : "";
                // grab the right one
                pData = (OUString*) pImagesLst->GetEntryData(i+1);
                sUrl2 = pData ? OUString(*pData) : "";

                if( sUrl1 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(100,100), Point(aPageSize.Width/2-100, aPageSize.Height-100)), sal_True);
                }
                else if( sUrl1.isEmpty()){} // nothing to do
                else
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

                if( sUrl2 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(aPageSize.Width/2 + 100,100), Point(aPageSize.Width-100, aPageSize.Height-100)), sal_True);
                }
                else if( sUrl2.isEmpty()){} // nothing to do
                else
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
            delete pData;
        }
        else if( sOpt == "4 images" )
        {
            OUString sUrl1("");
            OUString sUrl2("");
            OUString sUrl3("");
            OUString sUrl4("");
            OUString* pData = NULL;

            for( sal_Int32 i = 0; i < pImagesLst->GetEntryCount(); i+=4 )
            {
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
                pData = (OUString*) pImagesLst->GetEntryData(i);
                sUrl1 = pData ? OUString(*pData) : "";

                // grab the upper right one
                pData = (OUString*) pImagesLst->GetEntryData(i+1);
                sUrl2 = pData ? OUString(*pData) : "";

                // grab the lower left one
                pData = (OUString*) pImagesLst->GetEntryData(i+2);
                sUrl3 = pData ? OUString(*pData) : "";

                // grab the lower right one
                pData = (OUString*) pImagesLst->GetEntryData(i+3);
                sUrl4 = pData ? OUString(*pData) : "";

                if( sUrl1 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(100,100), Point(aPageSize.Width/2-100, aPageSize.Height/2-100)), sal_True);
                }
                else if( sUrl1.isEmpty()){} // nothing to do
                else
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

                if( sUrl2 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(aPageSize.Width/2 + 100,100), Point(aPageSize.Width-100, aPageSize.Height/2-100)), sal_True);
                }
                else if( sUrl2.isEmpty()){} // nothing to do
                else
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

                if( sUrl3 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(100,aPageSize.Height/2-100), Point(aPageSize.Width/2-100, aPageSize.Height-100)), sal_True);
                }
                else if( sUrl3.isEmpty()){} // nothing to do
                else
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

                if( sUrl4 == "Text Box" ) // create a Text Box
                {
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->CreatePresObj(PRESOBJ_TEXT, sal_False, Rectangle(Point(aPageSize.Width/2 + 100,aPageSize.Height/2 - 100), Point(aPageSize.Width-100, aPageSize.Height-100)), sal_True);
                }
                else if( sUrl4.isEmpty()){} // nothing to do
                else
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
            delete pData;
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
        Reference< XInterface > xCfg = ::comphelper::ConfigurationHelper::openConfig(
            ::comphelper::getProcessComponentContext(),
            OUString("/org.openoffice.Office.Impress/"),
            ::comphelper::ConfigurationHelper::E_READONLY);

        ::comphelper::ConfigurationHelper::readRelativeKey(
            xCfg,
            OUString("Pictures"),
            OUString("Path")) >>= sUrl;
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
                Reference< XInterface > xCfgWriter = ::comphelper::ConfigurationHelper::openConfig(
                    ::comphelper::getProcessComponentContext(),
                    OUString("/org.openoffice.Office.Impress/"),
                    ::comphelper::ConfigurationHelper::E_STANDARD);

                ::comphelper::ConfigurationHelper::writeRelativeKey(
                    xCfgWriter,
                    OUString("Pictures"),
                    OUString("Path"),
                    uno::makeAny(sUrl));

                ::comphelper::ConfigurationHelper::flush(xCfgWriter);
            }
            catch(const Exception&)
            {
                OSL_FAIL("Could not find config for Create Photo Album function");
            }

            if( aFilesArr.getLength() == 1)
            {
                pRemoveBtn->Enable();
                pUpBtn->Disable();
                pDownBtn->Disable();
            }
            else
            {
                pRemoveBtn->Enable();
                pUpBtn->Enable();
                pDownBtn->Enable();
            }

            for ( sal_Int32 i = 0; i < aFilesArr.getLength(); i++ )
            {
                // Store full path, show filename only
                OUString sFileName = aFilesArr[i].copy(aFilesArr[i].lastIndexOf("/")).replaceAll("/","");
                sal_Int16 nPos = pImagesLst->InsertEntry( sFileName );
                pImagesLst->SetEntryData(nPos, (void*)new OUString(aFilesArr[i]));
            }
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, TextHdl)
{
    sal_Int16 nPos;
    sal_Int16 nInsertPos = pImagesLst->GetSelectEntryPos();
    if (nInsertPos < 0)
        nPos = pImagesLst->InsertEntry( OUString("Text Box"));
    else
        nPos = pImagesLst->InsertEntry( OUString("Text Box"), nInsertPos);

    OUString sStr("Text Box");
    pImagesLst->SetEntryData(nPos, (void*)new OUString(sStr));

    if(pImagesLst->GetEntryCount() >= 1)
    {
        pRemoveBtn->Enable();
        pUpBtn->Disable();
        pDownBtn->Disable();
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl)
{
    if (pImagesLst->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectEntryPos() != 0)
    {
        OUString sActEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
        // actual data
        OUString* pActData = (OUString*) pImagesLst->GetEntryData(pImagesLst->GetSelectEntryPos());
        OUString sAct(*pActData);

        sal_uInt16 nActPos = pImagesLst->GetSelectEntryPos();
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

        pDownBtn->Enable();

    }

    if(pImagesLst->GetSelectEntryPos() == 0)
    {
        pDownBtn->Enable();
        pUpBtn->Disable();
    }
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
        pImagesLst->SetEntryData( nActPos, (void*) new OUString(sAct));

        pImagesLst->InsertEntry( sActEntry, nActPos + 1 );
        pImagesLst->SetEntryData( nActPos + 1, (void*) new OUString(sDown));

        pImagesLst->SelectEntryPos(nActPos + 1);

    }

    if(pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos() + 1) != OUString(""))
    {
        pDownBtn->Enable();
        pUpBtn->Enable();
    }
    else
    {
        pDownBtn->Disable();
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl)
{
    pImagesLst->RemoveEntry( pImagesLst->GetSelectEntryPos() );
    pImg->SetImage(Image());
    pFilenameLab->SetText("");

    if(pImagesLst->GetEntryCount() >= 1)
    {
        pRemoveBtn->Enable();
        pUpBtn->Disable();
        pDownBtn->Disable();
    }
    else
    {
        pRemoveBtn->Disable();
        pUpBtn->Disable();
        pDownBtn->Disable();
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl)
{
    OUString* pData = (OUString*) pImagesLst->GetEntryData(pImagesLst->GetSelectEntryPos());
    OUString sImgUrl = pData ? OUString(*pData) : "";
    // Some UI functionality: if you select the first/last item in the list,
    // the Up/Down button gets enabled/diabled (meaning: you cannot move up/down the item)
    // disable/enable moving up
    if(pImagesLst->GetSelectEntryPos() == 0)
    {
        pUpBtn->Disable();
    }
    else
    {
        pUpBtn->Enable();
    }

    // disable/enable moving down
    if(pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos() + 1) == OUString(""))
    {
        pDownBtn->Disable();
    }
    else
    {
        pDownBtn->Enable();
    }

    if (sImgUrl != "Text Box")
    {
        GraphicFilter aCurFilter;
        Graphic aGraphic;
        INetURLObject aURLObj( sImgUrl );
        pFilenameLab->SetText(sImgUrl);

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
        pFilenameLab->SetText("");
    }

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


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
