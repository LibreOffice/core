/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>

#include <cuires.hrc>

#include "dlgname.hxx"
#include "defdlgname.hxx"
#include <dialmgr.hxx>

/*************************************************************************
|*
|* Dialog for editing a name
|*
\************************************************************************/

SvxNameDialog::SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc ) :
    ModalDialog     ( pWindow, "NameDialog", "cui/ui/namedialog.ui" )
{
    get(pBtnOK, "ok");
    get(pFtDescription, "description_label");
    get(pEdtName, "name_entry");

    pFtDescription->SetText( rDesc );
    pEdtName->SetText( rName );
    pEdtName->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    ModifyHdl(&pEdtName);
    pEdtName->SetModifyHdl(LINK(this, SvxNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxNameDialog, ModifyHdl)
{
    if(aCheckNameHdl.IsSet())
        pBtnOK->Enable(aCheckNameHdl.Call(this) > 0);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Name
// plus uniqueness-callback-linkHandler

SvxObjectNameDialog::SvxObjectNameDialog(
    Window* pWindow,
    const String& rName) :
    ModalDialog     ( pWindow, "ObjectNameDialog", "cui/ui/objectnamedialog.ui" )
{
    get(pBtnOK, "ok");
    get(pEdtName, "object_name_entry");
    // set name
    pEdtName->SetText(rName);

    // activate name
    pEdtName->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    ModifyHdl(&pEdtName);
    pEdtName->SetModifyHdl(LINK(this, SvxObjectNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxObjectNameDialog, ModifyHdl)
{
    if(aCheckNameHdl.IsSet())
    {
        pBtnOK->Enable(aCheckNameHdl.Call(this) > 0);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description

SvxObjectTitleDescDialog::SvxObjectTitleDescDialog(
    Window* pWindow,
    const String& rTitle,
    const String& rDescription) :
    ModalDialog     ( pWindow, "ObjectTitleDescDialog", "cui/ui/objecttitledescdialog.ui" )
{
    get(pEdtTitle, "object_title_entry");
    get(pEdtDescription, "desc_entry");
    //lock height to initial height
    pEdtDescription->set_height_request(pEdtDescription->get_preferred_size().Height());
    // set title & desc
    pEdtTitle->SetText(rTitle);
    pEdtDescription->SetText(rDescription);

    // activate title
    pEdtTitle->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
}

///////////////////////////////////////////////////////////////////////////////////////////////

/*************************************************************************
|*
|* dialog for cancelling, saving or adding
|*
\************************************************************************/

SvxMessDialog::SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg )
    : ModalDialog(pWindow, "MessBox", "cui/ui/messbox.ui")
    , pImage(NULL)
{
    get(pBtn1, "mess_box_btn1");
    get(pBtn2, "mess_box_btn2");
    get(pFtDescription, "mess_box_description");
    get(pFtImage, "mess_box_image");
    if( pImg )
    {
        pImage = new Image( *pImg );
        pFtImage->SetImage( *pImage );
        pFtImage->SetStyle( ( pFtImage->GetStyle()/* | WB_NOTABSTOP */) & ~WB_3DLOOK );
        pFtImage->Show();
    }

    SetText( rText );
    pFtDescription->SetText( rDesc );

    pBtn1->SetClickHdl( LINK( this, SvxMessDialog, Button1Hdl ) );
    pBtn2->SetClickHdl( LINK( this, SvxMessDialog, Button2Hdl ) );
}

SvxMessDialog::~SvxMessDialog()
{
    delete pImage;
}

/*************************************************************************/

IMPL_LINK_NOARG_INLINE_START(SvxMessDialog, Button1Hdl)
{
    EndDialog( RET_BTN_1 );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxMessDialog, Button1Hdl)

/*************************************************************************/

IMPL_LINK_NOARG_INLINE_START(SvxMessDialog, Button2Hdl)
{
    EndDialog( RET_BTN_2 );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxMessDialog, Button2Hdl)

/*************************************************************************/

void SvxMessDialog::SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt )
{
    switch ( nBtnId )
    {
        case MESS_BTN_1:
            pBtn1->SetText( rNewTxt );
            break;

        case MESS_BTN_2:
            pBtn2->SetText( rNewTxt );
            break;

        default:
            OSL_FAIL( "Invalid button number!!!" );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
