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

#ifndef _SV_SPINFLD_HXX
#define _SV_SPINFLD_HXX

#include <tools/solar.h>
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
    virtual Size    CalcMinimumSizeForText(const rtl::OUString &rString) const;
    virtual Size    GetOptimalSize(WindowSizeType eType) const;
    virtual Size    CalcSize( sal_uInt16 nChars ) const;
    virtual void    take_properties(Window &rOther);
};

#endif // _SV_SPINFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
