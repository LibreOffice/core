/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmessdialog.hxx>

SwMessageAndEditDialog::SwMessageAndEditDialog(weld::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription)
    : MessageDialogController(pParent, rUIXMLDescription, rID, u"grid"_ustr)
    , m_xEdit(m_xBuilder->weld_entry(u"edit"_ustr))
    , m_xOKPB(m_xBuilder->weld_button(u"ok"_ustr))
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
