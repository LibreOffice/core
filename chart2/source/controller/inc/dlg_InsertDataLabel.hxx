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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_INSERTDATALABEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_INSERTDATALABEL_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svl/itemset.hxx>
#include <memory>
#include "../dialogs/res_DataLabel.hxx"

class SvNumberFormatter;

namespace chart
{

class DataLabelsDialog : public ModalDialog
{
private:
//     OKButton            m_aBtnOK;
//     CancelButton        m_aBtnCancel;
//     HelpButton          m_aBtnHelp;
    std::unique_ptr < DataLabelResources >    m_apDataLabelResources;

    const SfxItemSet&   m_rInAttrs;

    void Reset();

public:
    DataLabelsDialog(vcl::Window* pParent, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter);

    void FillItemSet(SfxItemSet& rOutAttrs);
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
