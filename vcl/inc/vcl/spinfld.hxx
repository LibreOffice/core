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

#ifndef _SV_SPINFLD_HXX
#define _SV_SPINFLD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>

// -------------
// - SpinField -
// -------------

class VCL_DLLPUBLIC SpinField : public Edit
{
protected:
    Edit*           mpEdit;
    AutoTimer       maRepeatTimer;
    Rectangle       maUpperRect;
    Rectangle       maLowerRect;
    Rectangle       maDropDownRect; // noch nicht angebunden...
    Link            maUpHdlLink;
    Link            maDownHdlLink;
    Link            maFirstHdlLink;
    Link            maLastHdlLink;
    sal_Bool            mbRepeat:1,
                    mbSpin:1,
                    mbInitialUp:1,
                    mbInitialDown:1,
                    mbNoSelect:1,
                    mbUpperIn:1,
                    mbLowerIn:1,
                    mbInDropDown:1;

    using Window::ImplInit;
    SAL_DLLPRIVATE void   ImplInit( Window* pParent, WinBits nStyle );

private:
    DECL_DLLPRIVATE_LINK( ImplTimeout, Timer* );
    SAL_DLLPRIVATE void   ImplInitSpinFieldData();
    SAL_DLLPRIVATE void   ImplCalcButtonAreas( OutputDevice* pDev, const Size& rOutSz, Rectangle& rDDArea, Rectangle& rSpinUpArea, Rectangle& rSpinDownArea );

protected:
                    SpinField( WindowType nTyp );

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    void            EndDropDown();

    virtual void    FillLayoutData() const;
    Rectangle *     ImplFindPartRect( const Point& rPt );
public:
                    SpinField( Window* pParent, WinBits nWinStyle = 0 );
                    SpinField( Window* pParent, const ResId& rResId );
                    ~SpinField();

    virtual sal_Bool    ShowDropDown( sal_Bool bShow );

    virtual void    Up();
    virtual void    Down();
    virtual void    First();
    virtual void    Last();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    void            SetUpHdl( const Link& rLink ) { maUpHdlLink = rLink; }
    const Link&     GetUpHdl() const { return maUpHdlLink; }
    void            SetDownHdl( const Link& rLink ) { maDownHdlLink = rLink; }
    const Link&     GetDownHdl() const { return maDownHdlLink; }
    void            SetFirstHdl( const Link& rLink ) { maFirstHdlLink = rLink; }
    const Link&     GetFirstHdl() const { return maFirstHdlLink; }
    void            SetLastHdl( const Link& rLink ) { maLastHdlLink = rLink; }
    const Link&     GetLastHdl() const { return maLastHdlLink; }

    virtual Size    CalcMinimumSize() const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    virtual Size    CalcSize( sal_uInt16 nChars ) const;
};

#endif // _SV_SPINFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
