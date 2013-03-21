/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "PhotoAlbumDialog.hxx"

#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <vcl/msgbox.hxx>




using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace sd
{

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

    pCancelBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CancelHdl));
    pCreateBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, CreateHdl));

    pFileBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, FileHdl));
    pTextBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, TextHdl));
    pUpBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, UpHdl));
    pDownBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, DownHdl));
    pRemoveBtn->SetClickHdl(LINK(this, SdPhotoAlbumDialog, RemoveHdl));

    pImagesLst->SetSelectHdl(LINK(this, SdPhotoAlbumDialog, SelectHdl));
    mpGraphicFilter = new GraphicFilter;

}

SdPhotoAlbumDialog::~SdPhotoAlbumDialog()
{
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, CancelHdl)
{
    EndDialog(0);
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

        // insert text to the first slide
        SdPage* pFirstSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
        pFirstSlide->SetAutoLayout(AUTOLAYOUT_TITLE, sal_True);

        Reference< XComponentContext > xContext(::comphelper::getProcessComponentContext());
        Reference< graphic::XGraphicProvider> xProvider(graphic::GraphicProvider::create(xContext));

        // get the option
        OUString sOpt = pInsTypeCombo->GetEntry(pInsTypeCombo->GetSelectEntryPos());
        if ( sOpt == "Fit to slide")
        {
            setFirstSlide(pFirstSlide);
            for( sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                OUString sUrl = pImagesLst->GetEntry( i );
                if (sUrl != "Text Box")
                {
                    ::comphelper::NamedValueCollection aMediaProperties;
                    aMediaProperties.put( "URL", OUString( sUrl ) );

                    Reference< graphic::XGraphic> xGraphic =
                        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );

                    Reference< drawing::XDrawPage > xSlide;
                    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
                    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->SetAutoLayout(AUTOLAYOUT_NONE, sal_True);

                    Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY
                    );

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Size aPicSize = xShape->getSize();
                    ::awt::Size aPageSize;

                    xSlideProps->getPropertyValue(
                        OUString("Width")) >>= aPageSize.Width;
                    xSlideProps->getPropertyValue(
                        OUString("Height")) >>= aPageSize.Height;
                    aPicSize.Width = aPageSize.Width;
                    aPicSize.Height = aPageSize.Height;
                    ::awt::Point aPicPos;

                    aPicPos.X = (aPageSize.Width - aPicSize.Width)/2;
                    aPicPos.Y = (aPageSize.Height - aPicSize.Height)/2;
                    xShape->setSize(aPicSize);
                    xShape->setPosition(aPicPos);
                    xSlide->add(xShape);
                }
                else // insert an empty slide, with centered text box
                {
                    Reference< drawing::XDrawPage > xSlide;
                    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
                    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->SetAutoLayout(AUTOLAYOUT_ONLY_TEXT, sal_True);
                }
            }
        }
        else if( sOpt == "1 image with title" )
        {
            setFirstSlide(pFirstSlide);
            for( sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i )
            {
                OUString sUrl = pImagesLst->GetEntry( i );
                if ( sUrl != "Text Box" )
                {
                    ::comphelper::NamedValueCollection aMediaProperties;
                    aMediaProperties.put( "URL", OUString( sUrl ) );

                    Reference< graphic::XGraphic> xGraphic =
                        xProvider->queryGraphic( aMediaProperties.getPropertyValues() );

                    Reference< drawing::XDrawPage > xSlide;
                    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
                    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );

                    Reference< beans::XPropertySet > xSlideProps( xSlide, uno::UNO_QUERY );

                    Reference< drawing::XShape > xShape(
                        xShapeFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                        uno::UNO_QUERY
                    );

                    Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
                    xProps->setPropertyValue("Graphic", ::uno::Any(xGraphic));

                    ::awt::Size aPicSize = xShape->getSize();
                    ::awt::Size aPageSize;

                    xSlideProps->getPropertyValue(
                        OUString("Width")) >>= aPageSize.Width;
                    xSlideProps->getPropertyValue(
                        OUString("Height")) >>= aPageSize.Height;
                    aPicSize.Width = aPageSize.Width/2;
                    aPicSize.Height = aPageSize.Height/2;
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
                    Reference< drawing::XDrawPage > xSlide;
                    Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
                    xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
                    SdPage* pSlide = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD);
                    pSlide->SetAutoLayout(AUTOLAYOUT_ONLY_TEXT, sal_True);
                }
            }
        }
        else
        {
            InfoBox aInfo(this, OUString("Function is not implemented!"));
            aInfo.Execute();
        }
        EndDialog(0);
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, FileHdl)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
        SFXWB_GRAPHIC | SFXWB_MULTISELECTION
    );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        Sequence< OUString > aFilesArr = aDlg.GetSelectedFiles();
        if( aFilesArr.getLength() )
        {
            for ( sal_Int32 i = 0; i < aFilesArr.getLength(); ++i )
            {
                pImagesLst->InsertEntry( aFilesArr[i] );
            }
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, TextHdl)
{
    sal_uInt16 nInsertPos = pImagesLst->GetSelectEntryPos() + 1;
    pImagesLst->InsertEntry( OUString("Text Box"), nInsertPos);
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl)
{
    if (pImagesLst->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectEntryPos() != 0)
    {
        OUString sActEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
        sal_uInt16 nActPos = pImagesLst->GetSelectEntryPos();
        OUString sUpperEntry( pImagesLst->GetEntry(nActPos - 1) );

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sUpperEntry );

        pImagesLst->InsertEntry( sActEntry, nActPos - 1 );
        pImagesLst->InsertEntry( sUpperEntry, nActPos );
        pImagesLst->SelectEntryPos(nActPos - 1);
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, DownHdl)
{
    sal_uInt16 nActPos = pImagesLst->GetSelectEntryPos();
    if (pImagesLst->GetEntry(nActPos + 1) != OUString("") )
    {
        OUString sActEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
        OUString sDownEntry( pImagesLst->GetEntry(nActPos + 1) );

        pImagesLst->RemoveEntry( sActEntry );
        pImagesLst->RemoveEntry( sDownEntry );

        pImagesLst->InsertEntry( sDownEntry, nActPos );
        pImagesLst->InsertEntry( sActEntry, nActPos + 1 );

        pImagesLst->SelectEntryPos(nActPos + 1);
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl)
{
    pImagesLst->RemoveEntry( pImagesLst->GetSelectEntryPos() );
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl)
{
    OUString sImgUrl( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
    if (sImgUrl != OUString("Text Box") )
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
        sal_Int32 nPrevWidth = 150;
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
    return 0;
}

void SdPhotoAlbumDialog::setFirstSlide(SdPage* pFirstSlide)
{
    SdrTextObj* pTitleObj; // title
    pTitleObj = dynamic_cast< SdrTextObj* >(pFirstSlide->GetPresObj(PRESOBJ_TITLE));
    pFirstSlide->SetObjText(pTitleObj, NULL, PRESOBJ_TITLE, OUString("Photoalbum"));

    SdrTextObj* pAuthorObj; // author's full name
    pAuthorObj = dynamic_cast< SdrTextObj* >(pFirstSlide->GetPresObj(PRESOBJ_TEXT));

    SvtUserOptions aUserOptions;
    pFirstSlide->SetObjText(pAuthorObj, NULL, PRESOBJ_TEXT, OUString("Author: ") + aUserOptions.GetFullName());
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
