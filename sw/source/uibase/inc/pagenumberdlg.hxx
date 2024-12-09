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

#include <sfx2/basedlgs.hxx>
#include <svx/pagenumberlistbox.hxx>

class SwWrtShell;

/// Page number wizard for easy header/footer page number insertion
class SwPageNumberDlg final : public SfxDialogController
{
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xCancel;
    std::unique_ptr<weld::ComboBox> m_xPageNumberPosition;
    std::unique_ptr<weld::ComboBox> m_xPageNumberAlignment;
    std::unique_ptr<weld::CheckButton> m_xMirrorOnEvenPages;
    std::unique_ptr<weld::CheckButton> m_xIncludePageTotal;
    std::unique_ptr<weld::CheckButton> m_xIncludePageRangeTotal;
    std::unique_ptr<SvxPageNumberListBox> m_xPageNumberTypeLB;

    std::unique_ptr<weld::Image> m_xPreviewImage;

    int m_aPageNumberPosition;
    int m_aPageNumberAlignment;
    SvxNumType m_nPageNumberType;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(IncludePageTotalChangeHdl, weld::Toggleable&, void);
    DECL_LINK(IncludePageRangeTotalChangeHdl, weld::Toggleable&, void);
    DECL_LINK(PositionSelectHdl, weld::ComboBox&, void);
    DECL_LINK(AlignmentSelectHdl, weld::ComboBox&, void);
    DECL_LINK(NumberTypeSelectHdl, weld::ComboBox&, void);

    void updateImage();

public:
    SwPageNumberDlg(weld::Window* pParent);
    int GetPageNumberPosition() const { return m_aPageNumberPosition; }
    int GetPageNumberAlignment() const { return m_aPageNumberAlignment; }
    bool GetMirrorOnEvenPages();
    bool GetIncludePageTotal();
    bool GetIncludePageRangeTotal() const;
    SvxNumType GetPageNumberType() const { return m_nPageNumberType; }
    void SetPageNumberType(SvxNumType nSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
