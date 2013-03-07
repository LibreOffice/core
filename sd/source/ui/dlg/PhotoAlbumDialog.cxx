/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "PhotoAlbumDialog.hxx"

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <stdio.h>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <unotools/ucbstreamhelper.hxx>

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
    Reference< drawing::XDrawPagesSupplier > xDPS( pDoc->getUnoModel(), uno::UNO_QUERY );
    Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY );

    for(sal_Int16 i = 0; i < pImagesLst->GetEntryCount(); ++i)
    {
        SdPage* pNewPage = NULL;

        Reference< drawing::XDrawPage > xSlide;
        Reference< container::XIndexAccess > xIndexAccess( xDrawPages, uno::UNO_QUERY );
        xSlide = xDrawPages->insertNewByIndex( xIndexAccess->getCount() );
        pNewPage = pDoc->GetSdPage(xIndexAccess->getCount(), PK_STANDARD);
        //SdrObject* pSdrObj = pNewPage->CreateDefaultPresObj(PRESOBJ_IMAGE, true);
        //Reference< drawing::XShape > xShape();
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
                fprintf( stderr, "Filename: %s\n", OUStringToOString( aFilesArr[i], RTL_TEXTENCODING_ASCII_US ).getStr() );
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
        fprintf( stderr, "Moving entry %s up\n", OUStringToOString( sActEntry, RTL_TEXTENCODING_ASCII_US ).getStr() );
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
        fprintf( stderr, "Moving entry %s down\n", OUStringToOString( sActEntry, RTL_TEXTENCODING_ASCII_US ).getStr() );

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
        fprintf( stderr, "Setting image to: \"%s\"\n", OUStringToOString( sImgUrl, RTL_TEXTENCODING_ASCII_US ).getStr());

        GraphicFilter aCurFilter;
        Graphic aGraphic;
        INetURLObject aURLObj( sImgUrl );

        sal_uInt16 nFilter = GRFILTER_FORMAT_DONTKNOW;

        if ( aURLObj.HasError() || INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
        {
            aURLObj.SetSmartProtocol( INET_PROT_FILE );
            aURLObj.SetSmartURL( sImgUrl );
        }

        ErrCode nRet = ERRCODE_NONE;

        sal_uInt32 nFilterImportFlags = GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG;
        // remote?
        if ( INET_PROT_FILE != aURLObj.GetProtocol() )
        {
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( sImgUrl, STREAM_READ );

            if( pStream )
                nRet = mpGraphicFilter->ImportGraphic( aGraphic, sImgUrl, *pStream, nFilter, NULL, nFilterImportFlags );
            else
                nRet = mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
                delete pStream;
        }
        else
        {
            nRet = mpGraphicFilter->ImportGraphic( aGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
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
    return 0;
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
