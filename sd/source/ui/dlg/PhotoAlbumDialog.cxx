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
#include "PhotoAlbumDialog.hrc"

using namespace ::sd;
using namespace ::com::sun::star::uno;

SdPhotoAlbumDialog::SdPhotoAlbumDialog(Window* pWindow)
: ModalDialog(pWindow, "PhotoAlbumCreatorDialog", "sd/ui/photoalbum.ui")
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
