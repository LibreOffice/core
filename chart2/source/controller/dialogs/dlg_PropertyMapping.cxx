/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dlg_PropertyMapping.hxx"

namespace chart {

namespace {

static long pListBoxTabs[] =
{
    3,
    0, 0, 75
};

}

PropertyMappingDlg::PropertyMappingDlg(Window* pParent)
    : ModalDialog(pParent, "PropertyMappingDialog",
        "modules/schart/ui/dlg_PropertyMapping.ui")
{
    get(mpMappingTable, "LST_PROPERTY_MAPPING");

    mpMappingTable->SetTabs( pListBoxTabs, MAP_APPFONT );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
