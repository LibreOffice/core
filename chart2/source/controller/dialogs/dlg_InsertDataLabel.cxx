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
#include "dlg_InsertDataLabel.hrc"
#include "ResId.hxx"
#include "ObjectNameProvider.hxx"
#include "res_DataLabel.hxx"

namespace chart
{

DataLabelsDialog::DataLabelsDialog(Window* pWindow, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter) :
    ModalDialog(pWindow, SchResId(DLG_DATA_DESCR)),
    m_aBtnOK(this, SchResId(BTN_OK)),
    m_aBtnCancel(this, SchResId(BTN_CANCEL)),
    m_aBtnHelp(this, SchResId(BTN_HELP)),
    m_apDataLabelResources( new DataLabelResources(this,rInAttrs) ),
    m_rInAttrs(rInAttrs)
{
    FreeResource();
    this->SetText( ObjectNameProvider::getName_ObjectForAllSeries( OBJECTTYPE_DATA_LABELS ) );

    m_apDataLabelResources->SetNumberFormatter( pFormatter );
    Reset();
}

DataLabelsDialog::~DataLabelsDialog()
{
}

void DataLabelsDialog::Reset()
{
    m_apDataLabelResources->Reset(m_rInAttrs);
}

void DataLabelsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apDataLabelResources->FillItemSet(rOutAttrs);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
