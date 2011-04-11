/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SPIN_HXX
#define _SV_SPIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>

// --------------
// - SpinButton -
// --------------

class VCL_DLLPUBLIC SpinButton : public Control
{
private:
    AutoTimer       maRepeatTimer;
    Rectangle       maUpperRect;
    Rectangle       maLowerRect;
    Rectangle       maFocusRect;
    sal_Bool            mbRepeat         : 1;
    sal_Bool            mbUpperIn        : 1;
    sal_Bool            mbLowerIn        : 1;
    sal_Bool            mbInitialUp      : 1;
    sal_Bool            mbInitialDown    : 1;
    sal_Bool            mbHorz           : 1;
    sal_Bool            mbUpperIsFocused : 1;
    Link            maUpHdlLink;
    Link            maDownHdlLink;
    long            mnMinRange;
    long            mnMaxRange;
    long            mnValue;
    long            mnValueStep;

    SAL_DLLPRIVATE Rectangle* ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void       ImplInit( Window* pParent, WinBits nStyle );
    DECL_DLLPRIVATE_LINK(     ImplTimeout, Timer* );

public:
                    SpinButton( Window* pParent, WinBits nStyle = 0 );
                    SpinButton( Window* pParent, const ResId& rResId );
                    ~SpinButton();

    virtual void    Up();
    virtual void    Down();

    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    GetFocus();
    virtual void    LoseFocus();

    void            SetRangeMin( long nNewRange );
    long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax( long nNewRange );
    long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    Range           GetRange() const { return Range( GetRangeMin(), GetRangeMax() ); }
    void            SetValue( long nValue );
    long            GetValue() const { return mnValue; }
    void            SetValueStep( long nNewStep ) { mnValueStep = nNewStep; }
    long            GetValueStep() const { return mnValueStep; }
    virtual long    PreNotify( NotifyEvent& rNEvt );

    void            SetUpHdl( const Link& rLink ) { maUpHdlLink = rLink; }
    const Link&     GetUpHdl() const   { return maUpHdlLink;   }
    void            SetDownHdl( const Link& rLink ) { maDownHdlLink = rLink; }
    const Link&     GetDownHdl() const { return maDownHdlLink; }

private:
    // moves the focus to the upper or lower rect. Return sal_True if the focus rect actually changed.
    SAL_DLLPRIVATE sal_Bool        ImplMoveFocus( sal_Bool _bUpper );
    SAL_DLLPRIVATE void        ImplCalcFocusRect( sal_Bool _bUpper );

    SAL_DLLPRIVATE inline sal_Bool ImplIsUpperEnabled( ) const
    {
        return mnValue + mnValueStep <= mnMaxRange;
    }
    SAL_DLLPRIVATE inline sal_Bool ImplIsLowerEnabled( ) const
    {
        return mnValue >= mnMinRange + mnValueStep;
    }
};

#endif  // _SV_SPIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
