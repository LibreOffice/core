/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/pathoptions.hxx>

#include <stdio.h>

#include "PhotoAlbumDialog.hxx"

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

namespace
{
     Image lcl_getImageFromURL( const OUString& i_rImageURL )
     {
         if ( i_rImageURL.isEmpty() )
             return Image();

        try
        {
             ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
             ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicProvider > xProvider(::com::sun::star::graphic::GraphicProvider::create(xContext));
             ::comphelper::NamedValueCollection aMediaProperties;
             aMediaProperties.put( "URL", i_rImageURL );
             ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > xGraphic = xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
             return Image( xGraphic );
         }
         catch( const ::com::sun::star::uno::Exception& )
         {
             DBG_UNHANDLED_EXCEPTION();
         }
         return Image();
     }
}

IMPL_LINK_NOARG(SdPhotoAlbumDialog, SelectHdl)
{
    OUString aImgUrl( pImagesLst->GetEntry(pImagesLst->GetSelectEntryPos()) );
    fprintf( stderr, "Setting image to: \"%s\"\n", OUStringToOString( aImgUrl, RTL_TEXTENCODING_ASCII_US ).getStr());
    pImg->SetImage(lcl_getImageFromURL(aImgUrl));
    return 0;
}


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
