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
#ifndef INCLUDED_SVX_FNTCTL_HXX
#define INCLUDED_SVX_FNTCTL_HXX

#include <sfx2/mnuitem.hxx>
#include <svl/lstner.hxx>
#include <svx/svxdllapi.h>



class FontNameMenu;
class SfxBindings;

// class SvxFontMenuControl ----------------------------------------------

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxFontMenuControl : public SfxMenuControl, public SfxListener
{
private:
    FontNameMenu*   pMenu;
    Menu&           rParent;

    SVX_DLLPRIVATE void         FillMenu();
    DECL_LINK_TYPED( MenuSelect, FontNameMenu *, void);
    SVX_DLLPRIVATE virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState ) override;
    SVX_DLLPRIVATE virtual void     Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

public:
    SvxFontMenuControl( sal_uInt16 nId, Menu&, SfxBindings& );
    virtual ~SvxFontMenuControl();

    SFX_DECL_MENU_CONTROL();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
