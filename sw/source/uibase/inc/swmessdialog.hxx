/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWMESSDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWMESSDIALOG_HXX

#include <vcl/dialog.hxx>

class Edit;
class FixedImage;
class OKButton;
class VclMultiLineEdit;

class SwMessageAndEditDialog : public ModalDialog
{
protected:
    VclPtr<OKButton>         m_pOKPB;
    VclPtr<FixedImage>       m_pImageIM;
    VclPtr<VclMultiLineEdit> m_pPrimaryMessage;
    VclPtr<VclMultiLineEdit> m_pSecondaryMessage;
    VclPtr<Edit>             m_pEdit;
public:
    SwMessageAndEditDialog(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription);
    virtual ~SwMessageAndEditDialog();
    virtual void dispose() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWMESSDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
