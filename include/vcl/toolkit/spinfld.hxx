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

#include <vcl/dllapi.h>
#include <vcl/toolkit/edit.hxx>
#include <vcl/timer.hxx>

class VCL_DLLPUBLIC SpinField : public Edit
{
public:
    explicit        SpinField( vcl::Window* pParent, WinBits nWinStyle, WindowType nType = WindowType::SPINFIELD );
    virtual         ~SpinField() override;
    SAL_DLLPRIVATE virtual void dispose() override;

    SAL_DLLPRIVATE virtual bool ShowDropDown( bool bShow );

    SAL_DLLPRIVATE virtual void Up();
    SAL_DLLPRIVATE virtual void Down();
    SAL_DLLPRIVATE virtual void First();
    SAL_DLLPRIVATE virtual void Last();

    SAL_DLLPRIVATE virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    SAL_DLLPRIVATE virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    SAL_DLLPRIVATE virtual void MouseMove( const MouseEvent& rMEvt ) override;
    SAL_DLLPRIVATE virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    SAL_DLLPRIVATE virtual void Draw( OutputDevice* pDev, const Point& rPos,SystemTextColorFlags nFlags ) override;
    SAL_DLLPRIVATE virtual void Resize() override;
    SAL_DLLPRIVATE virtual void StateChanged( StateChangedType nType ) override;
    SAL_DLLPRIVATE virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    SAL_DLLPRIVATE virtual bool PreNotify( NotifyEvent& rNEvt ) override;

    void            SetUpHdl( const Link<SpinField&,void>& rLink ) { maUpHdlLink = rLink; }
    void            SetDownHdl( const Link<SpinField&,void>& rLink ) { maDownHdlLink = rLink; }

    SAL_DLLPRIVATE virtual Size CalcMinimumSize() const override;
    SAL_DLLPRIVATE virtual Size CalcMinimumSizeForText(const OUString &rString) const override;
    SAL_DLLPRIVATE virtual Size GetOptimalSize() const override;
    SAL_DLLPRIVATE virtual Size CalcSize(sal_Int32 nChars) const override;

    SAL_DLLPRIVATE virtual FactoryFunction GetUITestFactory() const override;

    SAL_DLLPRIVATE void SetUpperEnabled(bool bEnabled);
    SAL_DLLPRIVATE void SetLowerEnabled(bool bEnabled);
    SAL_DLLPRIVATE bool IsUpperEnabled() const { return mbUpperEnabled; }
    SAL_DLLPRIVATE bool IsLowerEnabled() const { return mbLowerEnabled; }

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void   ImplInit( vcl::Window* pParent, WinBits nStyle );

    SAL_DLLPRIVATE virtual bool EventNotify( NotifyEvent& rNEvt ) override;

    SAL_DLLPRIVATE void EndDropDown();

    SAL_DLLPRIVATE virtual void FillLayoutData() const override;
    SAL_DLLPRIVATE tools::Rectangle * ImplFindPartRect( const Point& rPt );

private:
    DECL_DLLPRIVATE_LINK( ImplTimeout, Timer*, void );
    SAL_DLLPRIVATE void   ImplInitSpinFieldData();
    SAL_DLLPRIVATE void   ImplCalcButtonAreas( const OutputDevice* pDev, const Size& rOutSz, tools::Rectangle& rDDArea, tools::Rectangle& rSpinUpArea, tools::Rectangle& rSpinDownArea );

    tools::Rectangle       maUpperRect;
    tools::Rectangle       maLowerRect;
    tools::Rectangle       maDropDownRect; // not yet attached ...

    VclPtr<Edit>    mpEdit;
    AutoTimer       maRepeatTimer;
    Link<SpinField&,void>  maUpHdlLink;
    Link<SpinField&,void>  maDownHdlLink;
    bool            mbRepeat:1,
                    mbSpin:1,
                    mbInitialUp:1,
                    mbInitialDown:1,
                    mbUpperIn:1,
                    mbLowerIn:1,
                    mbInDropDown:1,
                    mbUpperEnabled:1,
                    mbLowerEnabled:1;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
