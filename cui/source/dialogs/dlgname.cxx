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

#include <vcl/svapp.hxx>
#include <dlgname.hxx>
#include <defdlgname.hxx>
#include <dialmgr.hxx>

/*************************************************************************
|*
|* Dialog for editing a name
|*
\************************************************************************/

SvxNameDialog::SvxNameDialog(weld::Window* pParent, const OUString& rName, const OUString& rDesc)
    : GenericDialogController(pParent, "cui/ui/namedialog.ui", "NameDialog")
    , m_xEdtName(m_xBuilder->weld_entry("name_entry"))
    , m_xFtDescription(m_xBuilder->weld_label("description_label"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    m_xFtDescription->set_label(rDesc);
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxNameDialog, ModifyHdl, weld::Entry&, void)
{
    // Do not allow empty names
    if (m_aCheckNameHdl.IsSet())
        m_xBtnOK->set_sensitive(!m_xEdtName->get_text().isEmpty() && m_aCheckNameHdl.Call(*this));
    else
        m_xBtnOK->set_sensitive(!m_xEdtName->get_text().isEmpty());
}

// #i68101#
// Dialog for editing Object Name
// plus uniqueness-callback-linkHandler

SvxObjectNameDialog::SvxObjectNameDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, "cui/ui/objectnamedialog.ui", "ObjectNameDialog")
    , m_xEdtName(m_xBuilder->weld_entry("object_name_entry"))
    , m_xBtnOK(m_xBuilder->weld_button("ok"))
{
    // set name
    m_xEdtName->set_text(rName);
    m_xEdtName->select_region(0, -1);

    // activate name
    ModifyHdl(*m_xEdtName);
    m_xEdtName->connect_changed(LINK(this, SvxObjectNameDialog, ModifyHdl));
}

IMPL_LINK_NOARG(SvxObjectNameDialog, ModifyHdl, weld::Entry&, void)
{
    if (aCheckNameHdl.IsSet())
    {
        m_xBtnOK->set_sensitive(aCheckNameHdl.Call(*this));
    }
}

// #i68101#
// Dialog for editing Object Title and Description

SvxObjectTitleDescDialog::SvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle,
    const OUString& rDescription)
    : GenericDialogController(pParent, "cui/ui/objecttitledescdialog.ui", "ObjectTitleDescDialog")
    , m_xEdtTitle(m_xBuilder->weld_entry("object_title_entry"))
    , m_xEdtDescription(m_xBuilder->weld_text_view("desc_entry"))
{
    //lock height to initial height
    m_xEdtDescription->set_size_request(-1, m_xEdtDescription->get_text_height() * 5);
    // set title & desc
    m_xEdtTitle->set_text(rTitle);
    m_xEdtDescription->set_text(rDescription);

    // activate title
    m_xEdtTitle->select_region(0, -1);
}

/*************************************************************************
|*
|* dialog for cancelling, saving or adding
|*
\************************************************************************/

SvxMessDialog::SvxMessDialog( vcl::Window* pWindow, const OUString& rText, const OUString& rDesc, Image const * pImg )
    : ModalDialog(pWindow, "MessBox", "cui/ui/messbox.ui")
    , pImage(nullptr)
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
    disposeOnce();
}

void SvxMessDialog::dispose()
{
    delete pImage;
    pImage = nullptr;
    pFtDescription.clear();
    pBtn1.clear();
    pBtn2.clear();
    pFtImage.clear();
    ModalDialog::dispose();
}

/*************************************************************************/

IMPL_LINK_NOARG(SvxMessDialog, Button1Hdl, Button*, void)
{
    EndDialog( RET_BTN_1 );
}

/*************************************************************************/

IMPL_LINK_NOARG(SvxMessDialog, Button2Hdl, Button*, void)
{
    EndDialog( RET_BTN_2 );
}

/*************************************************************************/

void SvxMessDialog::SetButtonText( SvxMessDialogButton nBtnId, const OUString& rNewTxt )
{
    switch ( nBtnId )
    {
        case SvxMessDialogButton::N1:
            pBtn1->SetText( rNewTxt );
            break;

        case SvxMessDialogButton::N2:
            pBtn2->SetText( rNewTxt );
            break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
