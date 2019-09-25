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

namespace chart
{

DataLabelsTabPage::DataLabelsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_DataLabel.ui", "tp_DataLabel", &rInAttrs)
    , m_aDataLabelResources(m_xBuilder.get(), pController->getDialog(), rInAttrs)
{
}

std::unique_ptr<SfxTabPage> DataLabelsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<DataLabelsTabPage>(pPage, pController, *rOutAttrs);
}

bool DataLabelsTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    m_aDataLabelResources.FillItemSet(rOutAttrs);
    return true;
}

void DataLabelsTabPage::Reset(const SfxItemSet* rInAttrs)
{
    m_aDataLabelResources.Reset(*rInAttrs);
}

void DataLabelsTabPage::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_aDataLabelResources.SetNumberFormatter( pFormatter );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
