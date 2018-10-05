/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QueryPropertiesDialog.hxx>
#include <strings.hrc>
#include <core_resource.hxx>

#define ALL_STRING DBA_RES(STR_QUERY_LIMIT_ALL)
#define ALL_INT -1

namespace dbaui
{

QueryPropertiesDialog::QueryPropertiesDialog(
    weld::Window* pParent, const bool bDistinct, const sal_Int64 nLimit )
    : GenericDialogController(pParent, "dbaccess/ui/querypropertiesdialog.ui", "QueryPropertiesDialog")
    , m_xRB_Distinct(m_xBuilder->weld_radio_button("distinct"))
    , m_xRB_NonDistinct(m_xBuilder->weld_radio_button("nondistinct"))
    , m_xLB_Limit(m_xBuilder->weld_combo_box("limitbox"))
{
    m_xRB_Distinct->set_active(bDistinct);
    m_xRB_NonDistinct->set_active(!bDistinct);

    m_xLB_Limit->append(OUString::number(ALL_INT), ALL_STRING);
    /// Default values
    sal_Int64 const aDefLimitAry[] =
    {
        5,
        10,
        20,
        50
    };
    for (auto a : aDefLimitAry)
        m_xLB_Limit->append(OUString::number(a), OUString::number(a));
    OUString sInitial = OUString::number(nLimit);
    auto nPos = m_xLB_Limit->find_id(sInitial);
    if (nPos != -1)
        m_xLB_Limit->set_active(nPos);
    else
        m_xLB_Limit->set_entry_text(OUString::number(nLimit));
}

sal_Int64 QueryPropertiesDialog::getLimit() const
{
    OUString sSelectedId = m_xLB_Limit->get_active_id();
    if (!sSelectedId.isEmpty())
        return sSelectedId.toInt64();
    return m_xLB_Limit->get_active_text().toInt64();
}

QueryPropertiesDialog::~QueryPropertiesDialog()
{
}

} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
