/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmessdialog.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/messagedialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/vclmedit.hxx>

SwMessageAndEditDialog::SwMessageAndEditDialog(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription)
    : ModalDialog(pParent, rID, rUIXMLDescription)
{
    get(m_pOKPB, "ok");
    get(m_pPrimaryMessage, "primarymessage");
    m_pPrimaryMessage->SetPaintTransparent(true);
    get(m_pSecondaryMessage, "secondarymessage");
    m_pSecondaryMessage->SetPaintTransparent(true);
    MessageDialog::SetMessagesWidths(this, m_pPrimaryMessage, m_pSecondaryMessage);
    get(m_pImageIM, "image");
    m_pImageIM->SetImage(WarningBox::GetStandardImage());
    get(m_pEdit, "edit");
}

SwMessageAndEditDialog::~SwMessageAndEditDialog()
{
    disposeOnce();
}

void SwMessageAndEditDialog::dispose()
{
    m_pOKPB.clear();
    m_pImageIM.clear();
    m_pPrimaryMessage.clear();
    m_pSecondaryMessage.clear();
    m_pEdit.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
