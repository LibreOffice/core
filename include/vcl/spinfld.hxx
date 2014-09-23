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

#ifndef INCLUDED_VCL_SPINFLD_HXX
#define INCLUDED_VCL_SPINFLD_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/edit.hxx>
#include <vcl/timer.hxx>


// - SpinField -


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
    bool            mbRepeat:1,
                    mbSpin:1,
                    mbInitialUp:1,
                    mbInitialDown:1,
                    mbNoSelect:1,
                    mbUpperIn:1,
                    mbLowerIn:1,
                    mbInDropDown:1;

    using Window::ImplInit;
    SAL_DLLPRIVATE void   ImplInit( vcl::Window* pParent, WinBits nStyle );

private:
    DECL_DLLPRIVATE_LINK( ImplTimeout, Timer* );
    SAL_DLLPRIVATE void   ImplInitSpinFieldData();
    SAL_DLLPRIVATE void   ImplCalcButtonAreas( OutputDevice* pDev, const Size& rOutSz, Rectangle& rDDArea, Rectangle& rSpinUpArea, Rectangle& rSpinDownArea );

protected:
    explicit        SpinField( WindowType nTyp );

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

    void            EndDropDown();

    virtual void    FillLayoutData() const SAL_OVERRIDE;
    Rectangle *     ImplFindPartRect( const Point& rPt );

public:
    explicit        SpinField( vcl::Window* pParent, WinBits nWinStyle = 0 );
    explicit        SpinField( vcl::Window* pParent, const ResId& );
    virtual         ~SpinField();

    virtual bool    ShowDropDown( bool bShow );

    virtual void    Up();
    virtual void    Down();
    virtual void    First();
    virtual void    Last();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void            SetUpHdl( const Link& rLink ) { maUpHdlLink = rLink; }
    const Link&     GetUpHdl() const { return maUpHdlLink; }
    void            SetDownHdl( const Link& rLink ) { maDownHdlLink = rLink; }
    const Link&     GetDownHdl() const { return maDownHdlLink; }
    void            SetFirstHdl( const Link& rLink ) { maFirstHdlLink = rLink; }
    const Link&     GetFirstHdl() const { return maFirstHdlLink; }
    void            SetLastHdl( const Link& rLink ) { maLastHdlLink = rLink; }
    const Link&     GetLastHdl() const { return maLastHdlLink; }

    virtual Size    CalcMinimumSize() const SAL_OVERRIDE;
    virtual Size    CalcMinimumSizeForText(const OUString &rString) const SAL_OVERRIDE;
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
    virtual Size    CalcSize(sal_Int32 nChars) const SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_SPINFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
