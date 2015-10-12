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
#ifndef INCLUDED_SVX_ZOOMCTRL_HXX
#define INCLUDED_SVX_ZOOMCTRL_HXX

#include <sfx2/stbitem.hxx>
#include <svx/svxdllapi.h>
#include <sfx2/zoomitem.hxx>
#include <vcl/image.hxx>

class SVX_DLLPUBLIC SvxZoomStatusBarControl : public SfxStatusBarControl
{
private:
    sal_uInt16         nZoom;
    SvxZoomEnableFlags nValueSet;

public:
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState ) override;
    virtual void    Paint( const UserDrawEvent& rEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomStatusBarControl( sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb );

};

class SVX_DLLPUBLIC SvxZoomPageStatusBarControl : public SfxStatusBarControl
{
private:
    Image maImage;
public:
    virtual void    Paint( const UserDrawEvent& rEvt ) override;
    virtual bool    MouseButtonDown( const MouseEvent & rEvt ) override;

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomPageStatusBarControl(sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
