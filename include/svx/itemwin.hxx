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
#ifndef INCLUDED_SVX_ITEMWIN_HXX
#define INCLUDED_SVX_ITEMWIN_HXX

#include <svtools/toolbarmenu.hxx>
#include <svx/svxdllapi.h>
#include <svx/xtable.hxx>
#include <vcl/customweld.hxx>

class SfxObjectShell;
class ValueSet;
class SvxLineStyleToolBoxControl;

class SvxLineBox final : public WeldToolbarPopup
{
    rtl::Reference<SvxLineStyleToolBoxControl> mxControl;
    std::unique_ptr<ValueSet> mxLineStyleSet;
    std::unique_ptr<weld::CustomWeld> mxLineStyleSetWin;

    void FillControl();
    void Fill(const XDashListRef& pList);

    DECL_LINK(SelectHdl, ValueSet*, void);

    virtual void GrabFocus() override;

public:
    SvxLineBox(SvxLineStyleToolBoxControl* pControl, weld::Widget* pParent, int nInitialIndex);
    virtual ~SvxLineBox() override;
};

namespace SvxFillTypeBox
{
SVX_DLLPUBLIC void Fill(weld::ComboBox& rListBox);
}

namespace SvxFillAttrBox
{
SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XHatchListRef& pList);
SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XGradientListRef& pList);
SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XBitmapListRef& pList);
SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XPatternListRef& pList);
}

#endif // INCLUDED_SVX_ITEMWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
