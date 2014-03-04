/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/dialog.hxx>

class Edit;
class FixedImage;
class OKButton;
class VclMultiLineEdit;

class SwMessageAndEditDialog : public ModalDialog
{
protected:
    OKButton*         m_pOKPB;
    FixedImage*       m_pImageIM;
    VclMultiLineEdit* m_pPrimaryMessage;
    VclMultiLineEdit* m_pSecondaryMessage;
    Edit*             m_pEdit;
public:
    SwMessageAndEditDialog(Window* pParent, const OString& rID,
        const OUString& rUIXMLDescription);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
