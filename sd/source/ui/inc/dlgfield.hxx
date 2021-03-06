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

#pragma once

#include <vcl/weld.hxx>
#include <memory>
#include <svl/itemset.hxx>

class SvxFieldData;
class SvxLanguageBox;

/**
 * dialog to adjust field-commands
 */
class SdModifyFieldDlg : public weld::GenericDialogController
{
private:
    SfxItemSet          m_aInputSet;
    const SvxFieldData* m_pField;

    std::unique_ptr<weld::RadioButton> m_xRbtFix;
    std::unique_ptr<weld::RadioButton> m_xRbtVar;
    std::unique_ptr<SvxLanguageBox> m_xLbLanguage;
    std::unique_ptr<weld::ComboBox> m_xLbFormat;

    void                FillFormatList();
    void                FillControls();

    DECL_LINK(LanguageChangeHdl, weld::ComboBox&, void);

public:
    SdModifyFieldDlg(weld::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet);
    virtual ~SdModifyFieldDlg() override;

    SvxFieldData*       GetField();
    SfxItemSet          GetItemSet() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
