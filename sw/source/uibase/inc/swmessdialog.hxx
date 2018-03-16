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

#include <vcl/weld.hxx>

class SwMessageAndEditDialog : public weld::MessageDialogController
{
protected:
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xOKPB;
public:
    SwMessageAndEditDialog(weld::Window* pParent, const OString& rID,
        const OUString& rUIXMLDescription);
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWMESSDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
