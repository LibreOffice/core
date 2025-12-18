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

#include <i18nutil/paper.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <vcl/weld/TreeView.hxx>

#include <vector>

class PageSizePopup;
class ValueSet;

namespace sw::sidebar
{
class PageSizeControl final : public WeldToolbarPopup
{
public:
    explicit PageSizeControl(PageSizePopup* pControl, weld::Widget* pParent);
    virtual void GrabFocus() override;
    virtual ~PageSizeControl() override;

private:
    std::unique_ptr<weld::TreeView> mxPageSizeTreeView;
    std::unique_ptr<weld::Button> mxMoreButton;
    // hidden metric field
    std::unique_ptr<weld::MetricSpinButton> mxWidthHeightField;
    rtl::Reference<PageSizePopup> mxControl;

    std::vector<Paper> maPaperList;

    static void ExecuteSizeChange(const Paper ePaper);

    DECL_LINK(ImplSizeHdl, weld::TreeView&, bool);
    DECL_LINK(MoreButtonClickHdl_Impl, weld::Button&, void);
};

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
