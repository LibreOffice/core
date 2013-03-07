/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "cusshow.hxx"

#include "PhotoAlbumDialog.hxx"

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <stdio.h>

namespace sd
{

SdPhotoAlbumDialog::SdPhotoAlbumDialog(Window* pWindow)
: ModalDialog(pWindow, "PhotoAlbumCreatorDialog", "modules/simpress/ui/photoalbum.ui")
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
        ::com::sun::star::uno::Sequence< OUString > aFilesArr = aDlg.GetSelectedFiles();
        if( aFilesArr.getLength() )
        {
            //fprintf( stderr, "Num of selected files: %lu\n", aFilesArr.getLength() );
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
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, UpHdl)
{
    if (pImagesLst->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND
        && pImagesLst->GetSelectEntryPos() != 0)
    {
        OUString sActEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
        OUString sUpperEntry( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos() - 1) );
    }
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, DownHdl)
{
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, RemoveHdl)
{
    return 0;
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl)
{
    OUString sImgUrl( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
    //fprintf( stderr, "Index: %d\n", pImagesLst->GetSelectEntryPos() );
    fprintf( stderr, "Setting image to: \"%s\"\n", OUStringToOString( sImgUrl, RTL_TEXTENCODING_ASCII_US ).getStr());
    //pImg->SetImage( framework::AddonsOptions().GetImageFromURL( sImgUrl, sal_False ) );
    ::com::sun::star::uno::Sequence < sal_Int8 > aBmpSequence;
    ::com::sun::star::uno::Any rImage;
    Bitmap aBmp;
    if (com::sun::star::uno::makeAny(sImgUrl) >>= aBmpSequence)
    {
        SvMemoryStream aData( aBmpSequence.getArray(),
            aBmpSequence.getLength(),
            STREAM_READ );
        aData >> aBmp;
    }
    pImg->SetImage(aBmp);
    return 0;
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
