/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX
#define INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX

#include <vcl/weld.hxx>

class DocumentConverterDialog : public weld::GenericDialogController
{
public:
    DocumentConverterDialog(weld::Widget* pParent);

    virtual short run() override;

private:
    std::unique_ptr<weld::Entry> m_xEdittext;
};

#endif // INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
