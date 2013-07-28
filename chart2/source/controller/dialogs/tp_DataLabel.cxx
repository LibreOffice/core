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

#include "tp_DataLabel.hxx"

#include "TabPages.hrc"
#include "ResId.hxx"

namespace chart
{

DataLabelsTabPage::DataLabelsTabPage(Window* pWindow, const SfxItemSet& rInAttrs) :
        SfxTabPage(pWindow, SchResId(TP_DATA_DESCR), rInAttrs),
        m_aDataLabelResources(this, rInAttrs)
{
    FreeResource();
}

DataLabelsTabPage::~DataLabelsTabPage()
{
}

SfxTabPage* DataLabelsTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new DataLabelsTabPage(pWindow, rOutAttrs);
}

sal_Bool DataLabelsTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    return m_aDataLabelResources.FillItemSet(rOutAttrs);
}

void DataLabelsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_aDataLabelResources.Reset(rInAttrs);
}

void DataLabelsTabPage::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_aDataLabelResources.SetNumberFormatter( pFormatter );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
