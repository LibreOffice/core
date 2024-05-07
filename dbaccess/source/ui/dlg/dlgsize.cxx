/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dlgsize.hxx>

namespace dbaui
{

#define DEF_ROW_HEIGHT  45
#define DEF_COL_WIDTH   227

DlgSize::DlgSize(weld::Window* pParent, sal_Int32 nVal, bool bRow, sal_Int32 _nAlternativeStandard )
    : GenericDialogController(pParent, bRow ? u"dbaccess/ui/rowheightdialog.ui"_ustr : u"dbaccess/ui/colwidthdialog.ui"_ustr,
                              bRow ? u"RowHeightDialog"_ustr : u"ColWidthDialog"_ustr)
    , m_nPrevValue(nVal)
    , m_xMF_VALUE(m_xBuilder->weld_metric_spin_button(u"value"_ustr, FieldUnit::CM))
    , m_xCB_STANDARD(m_xBuilder->weld_check_button(u"automatic"_ustr))
{
    sal_Int32 nStandard(bRow ? DEF_ROW_HEIGHT : DEF_COL_WIDTH);
    if ( _nAlternativeStandard > 0 )
        nStandard = _nAlternativeStandard;
    m_xCB_STANDARD->connect_toggled(LINK(this,DlgSize,CbClickHdl));

    bool bDefault = -1 == nVal;
    m_xCB_STANDARD->set_active(bDefault);
    if (bDefault)
    {
        SetValue(nStandard);
        m_nPrevValue = nStandard;
    }
    CbClickHdl(*m_xCB_STANDARD);
}

DlgSize::~DlgSize()
{
}

void DlgSize::SetValue( sal_Int32 nVal )
{
    m_xMF_VALUE->set_value(nVal, FieldUnit::CM );
}

sal_Int32 DlgSize::GetValue() const
{
    if (m_xCB_STANDARD->get_active())
        return -1;
    return static_cast<sal_Int32>(m_xMF_VALUE->get_value( FieldUnit::CM ));
}

IMPL_LINK_NOARG(DlgSize, CbClickHdl, weld::Toggleable&, void)
{
    m_xMF_VALUE->set_sensitive(!m_xCB_STANDARD->get_active());
    if (m_xCB_STANDARD->get_active())
    {
        // don't use getValue as this will use m_xCB_STANDARD->to determine if we're standard
        m_nPrevValue = static_cast<sal_Int32>(m_xMF_VALUE->get_value(FieldUnit::CM));
        m_xMF_VALUE->set_text(u""_ustr);
    }
    else
    {
        SetValue(m_nPrevValue);
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
