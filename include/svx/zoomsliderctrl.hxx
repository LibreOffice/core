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

#ifndef INCLUDED_SVX_ZOOMSLIDERCTRL_HXX
#define INCLUDED_SVX_ZOOMSLIDERCTRL_HXX

#include <sfx2/stbitem.hxx>
#include <svx/svxdllapi.h>
#include <memory>

// class SvxZoomSliderControl ----------------------------------------
class SVX_DLLPUBLIC SvxZoomSliderControl : public SfxStatusBarControl
{
private:

    struct SvxZoomSliderControl_Impl;
    std::unique_ptr<SvxZoomSliderControl_Impl> mxImpl;

    sal_uInt16 Offset2Zoom( long nOffset ) const;
    long Zoom2Offset( sal_uInt16 nZoom ) const;

    void repaintAndExecute();
    void forceRepaint() const;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomSliderControl( sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& _rStb );
    virtual ~SvxZoomSliderControl();

    virtual void  StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual void  Paint( const UserDrawEvent& rEvt ) override;
    virtual bool  MouseButtonDown( const MouseEvent & ) override;
    virtual bool  MouseButtonUp( const MouseEvent & ) override;
    virtual bool  MouseMove( const MouseEvent & rEvt ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
