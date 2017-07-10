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

#ifndef INCLUDED_SFX2_INC_CHARMAPCONTROL_HXX
#define INCLUDED_SFX2_INC_CHARMAPCONTROL_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/charwin.hxx>
#include <vcl/button.hxx>

class SvxCharView;

class SFX2_DLLPUBLIC SfxCharmapCtrl : public SfxPopupWindow
{
public:
    explicit SfxCharmapCtrl(sal_uInt16 nId, const css::uno::Reference< css::frame::XFrame >& rFrame);

    virtual ~SfxCharmapCtrl() override;

    virtual void dispose() override;

private:
    VclPtr<SvxCharView>    m_pRecentCharView[16];
    VclPtr<SvxCharView>    m_pFavCharView[16];
    std::deque<OUString>   maRecentCharList;
    std::deque<OUString>   maRecentCharFontList;
    std::deque<OUString>   maFavCharList;
    std::deque<OUString>   maFavCharFontList;
    VclPtr<Button>         maDlgBtn;

    DECL_LINK(CharClickHdl, SvxCharView*, void);
    DECL_STATIC_LINK(SfxCharmapCtrl, LoseFocusHdl, Control&, void);
    DECL_LINK(OpenDlgHdl, Button*, void);

    void            getFavCharacterList();
    void            updateFavCharControl();

    void            getRecentCharacterList(); //gets both recent char and recent char font list
    void            updateRecentCharacterList(const OUString& rChar, const OUString& rFont);
    void            updateRecentCharControl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
