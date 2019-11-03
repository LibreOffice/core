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

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

class DocumentConverterDialog : public weld::GenericDialogController
{
public:
    typedef std::vector<std::shared_ptr<const SfxFilter>> FilterList;

    DocumentConverterDialog(weld::Widget* pParent);

    virtual short run() override;

private:
    DECL_LINK(ImportDocTypesSelChange, weld::ComboBox&, void);

    std::unique_ptr<weld::ComboBox> m_xImportDocTypes;
    std::unique_ptr<weld::ComboBox> m_xExportDocTypes;

    std::vector<std::tuple<OUString, OUString, OUString>> m_aServiceList;
    std::unordered_map<OUString, FilterList> m_aImportLists;
    std::unordered_map<OUString, FilterList> m_aExportLists;
};

#endif // INCLUDED_CUI_INC_DOCUMENTCONVERTERDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
