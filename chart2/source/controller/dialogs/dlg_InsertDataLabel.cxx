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

#include "dlg_InsertDataLabel.hxx"
#include "res_DataLabel.hxx"

namespace chart
{

DataLabelsDialog::DataLabelsDialog(vcl::Window* pWindow, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter) :
    ModalDialog(pWindow
                ,"dlg_DataLabels"
                ,"modules/schart/ui/dlg_DataLabel.ui"),
    m_apDataLabelResources( new DataLabelResources(this, this, rInAttrs) ),
    m_rInAttrs(rInAttrs)
{
    m_apDataLabelResources->SetNumberFormatter( pFormatter );
    Reset();
}

void DataLabelsDialog::Reset()
{
    m_apDataLabelResources->Reset(m_rInAttrs);
}

void DataLabelsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apDataLabelResources->FillItemSet(&rOutAttrs);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
