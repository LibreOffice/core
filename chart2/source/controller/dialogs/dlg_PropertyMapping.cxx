/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dlg_PropertyMapping.hxx"

#include "DialogModel.hxx"

using namespace com::sun::star;

namespace chart {

namespace {

long pListBoxTabs[] =
{
    3,
    0, 75, 150
};

std::vector<OUString> getEntries()
{
    const char* pEntries[] =
    {
        "FillColor\tColor",
        "BorderColor\tColor"
    };

    std::vector<OUString> aRet;
    for(size_t i = 0; i < SAL_N_ELEMENTS(pEntries); ++i)
    {
        aRet.push_back(OUString::createFromAscii(pEntries[i]));
    }

    return aRet;
}

}

PropertyMappingDlg::PropertyMappingDlg(Window* pParent, uno::Reference< chart2::XChartType > xChartType )
    : ModalDialog(pParent, "PropertyMappingDialog",
        "modules/schart/ui/dlg_PropertyMapping.ui")
{
    get(mpMappingTable, "LST_PROPERTY_MAPPING");
    get(mpBtnOk, "ok");
    get(mpBtnCancel, "cancel");

    mpMappingTable->SetTabs( pListBoxTabs, MAP_APPFONT );
    uno::Sequence< OUString > aPropRoles = xChartType->getSupportedPropertyRoles();
    for(sal_Int32 i = 0, n = aPropRoles.getLength(); i < n; ++i)
    {
        OUString aUIString = DialogModel::ConvertRoleFromInternalToUI(aPropRoles[i]);
        mpMappingTable->InsertEntry(aUIString);
    }
    mpBtnOk->SetClickHdl( LINK( this, PropertyMappingDlg, OkBtnHdl ) );
    mpBtnCancel->SetClickHdl( LINK( this, PropertyMappingDlg, CancelBtnHdl ) );
}

OUString PropertyMappingDlg::getSelectedEntry()
{
    if(mpMappingTable->GetSelectionCount())
    {
        SvTreeListEntry* pEntry = mpMappingTable->FirstSelected();
        OUString aText = mpMappingTable->GetEntryText(pEntry, 0);
        return aText;
    }

    return OUString();
}

IMPL_LINK_NOARG(PropertyMappingDlg, OkBtnHdl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(PropertyMappingDlg, CancelBtnHdl)
{
    EndDialog(RET_CANCEL);
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
