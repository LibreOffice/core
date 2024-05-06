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

#include "tp_Trendline.hxx"

namespace chart
{

TrendlineTabPage::TrendlineTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_Trendline.ui"_ustr, u"TP_TRENDLINE"_ustr, &rInAttrs)
    , m_aTrendlineResources(*m_xBuilder, rInAttrs)
{
}

std::unique_ptr<SfxTabPage> TrendlineTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<TrendlineTabPage>(pPage, pController, *rOutAttrs);
}

bool TrendlineTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    m_aTrendlineResources.FillItemSet( rOutAttrs );
    return true;
}

void TrendlineTabPage::Reset( const SfxItemSet* rInAttrs )
{
    m_aTrendlineResources.Reset( *rInAttrs );
}

void TrendlineTabPage::SetNumFormatter( SvNumberFormatter* pNumFormatter )
{
    m_aTrendlineResources.SetNumFormatter( pNumFormatter );
}

void TrendlineTabPage::SetNbPoints( sal_Int32 nNbPoints )
{
    m_aTrendlineResources.SetNbPoints( nNbPoints );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
