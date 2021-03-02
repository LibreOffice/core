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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SpinButton final : public Control
{
private:
    AutoTimer       maRepeatTimer;
    tools::Rectangle       maUpperRect;
    tools::Rectangle       maLowerRect;
    tools::Rectangle       maFocusRect;
    bool            mbRepeat         : 1;
    bool            mbUpperIn        : 1;
    bool            mbLowerIn        : 1;
    bool            mbInitialUp      : 1;
    bool            mbInitialDown    : 1;
    bool            mbHorz           : 1;
    bool            mbUpperIsFocused : 1;
    tools::Long            mnMinRange;
    tools::Long            mnMaxRange;
    tools::Long            mnValue;
    tools::Long            mnValueStep;

    SAL_DLLPRIVATE tools::Rectangle* ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void       ImplInit( vcl::Window* pParent, WinBits nStyle );
    DECL_DLLPRIVATE_LINK( ImplTimeout, Timer*, void );

public:
    explicit        SpinButton( vcl::Window* pParent, WinBits nStyle );

    void            Up();
    void            Down();

    virtual void    Resize() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    void            SetRangeMin( tools::Long nNewRange );
    tools::Long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax( tools::Long nNewRange );
    tools::Long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    void            SetValue( tools::Long nValue );
    tools::Long            GetValue() const { return mnValue; }
    void            SetValueStep( tools::Long nNewStep ) { mnValueStep = nNewStep; }
    tools::Long            GetValueStep() const { return mnValueStep; }
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

private:
    // moves the focus to the upper or lower rect. Return sal_True if the focus rect actually changed.
    SAL_DLLPRIVATE bool        ImplMoveFocus( bool _bUpper );
    SAL_DLLPRIVATE void        ImplCalcFocusRect( bool _bUpper );

    SAL_DLLPRIVATE bool ImplIsUpperEnabled( ) const
    {
        return mnValue + mnValueStep <= mnMaxRange;
    }
    SAL_DLLPRIVATE bool ImplIsLowerEnabled( ) const
    {
        return mnValue >= mnMinRange + mnValueStep;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
