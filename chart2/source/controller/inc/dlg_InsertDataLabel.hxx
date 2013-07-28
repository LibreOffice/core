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
#ifndef CHART2_DLG_INSERT_DATALABELS_GRID_HXX
#define CHART2_DLG_INSERT_DATALABELS_GRID_HXX

// header for class ModalDialog
#include <vcl/dialog.hxx>
// header for class CheckBox
#include <vcl/button.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
//for auto_ptr
#include <memory>

class SvNumberFormatter;

namespace chart
{

class DataLabelResources;
class DataLabelsDialog : public ModalDialog
{
private:
    OKButton            m_aBtnOK;
    CancelButton        m_aBtnCancel;
    HelpButton          m_aBtnHelp;
    ::std::auto_ptr< DataLabelResources >    m_apDataLabelResources;

    const SfxItemSet&   m_rInAttrs;

    void Reset();

public:
    DataLabelsDialog(Window* pParent, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter);
    virtual ~DataLabelsDialog();

    void FillItemSet(SfxItemSet& rOutAttrs);
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
