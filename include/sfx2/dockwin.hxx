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
#ifndef INCLUDED_SFX2_DOCKWIN_HXX
#define INCLUDED_SFX2_DOCKWIN_HXX

#include <sal/config.h>
#include <sal/types.h>
#include <vcl/dockwin.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/dllapi.h>
#include <sfx2/childwin.hxx>

class SfxSplitWindow;
class SfxDockingWindow_Impl;

void SFX2_DLLPUBLIC SAL_CALL SfxDockingWindowFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const OUString& rDockingWindowName );
bool SFX2_DLLPUBLIC SAL_CALL IsDockingWindowVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const OUString& rDockingWindowName );

class SFX2_DLLPUBLIC SfxDockingWindow : public DockingWindow
{
private:
    Rectangle               aInnerRect;
    Rectangle               aOuterRect;
    SfxBindings*            pBindings;
    Size                    aFloatSize;
    SfxChildWindow*         pMgr;
    SfxDockingWindow_Impl*  pImp;

    SAL_DLLPRIVATE SfxDockingWindow(SfxDockingWindow &); // not defined
    SAL_DLLPRIVATE void operator =(SfxDockingWindow &); // not defined

protected:
    SfxChildAlignment   CalcAlignment(const Point& rPos, Rectangle& rRect );
    void                CalcSplitPosition(const Point rPos, Rectangle& rRect,
                            SfxChildAlignment eAlign);
    virtual Size        CalcDockingSize(SfxChildAlignment);
    virtual SfxChildAlignment
                        CheckAlignment(SfxChildAlignment,SfxChildAlignment);

    virtual void        Resize();
    virtual bool        PrepareToggleFloatingMode();
    virtual void        ToggleFloatingMode();
    virtual void        StartDocking();
    virtual bool        Docking( const Point& rPos, Rectangle& rRect );
    virtual void        EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );
    virtual void        Resizing( Size& rSize );
    virtual void        Paint( const Rectangle& rRect );
    virtual bool        Close();
    virtual void        Move();

    SAL_DLLPRIVATE SfxChildWindow* GetChildWindow_Impl()    { return pMgr; }

public:
                        SfxDockingWindow( SfxBindings *pBindings,
                                          SfxChildWindow *pCW,
                                          Window* pParent,
                                          WinBits nWinBits=0);
                        SfxDockingWindow( SfxBindings *pBindings,
                                          SfxChildWindow *pCW,
                                          Window* pParent,
                                          const ResId& rResId);
                        ~SfxDockingWindow();

    void                Initialize (SfxChildWinInfo* pInfo);
    virtual void        FillInfo(SfxChildWinInfo&) const;
    virtual void        StateChanged( StateChangedType nStateChange );

    void                SetDockingRects(const Rectangle& rOuter, const Rectangle& rInner)
                            { aInnerRect = rInner; aOuterRect = rOuter; }
    const Rectangle&    GetInnerRect() const                    { return aInnerRect; }
    const Rectangle&    GetOuterRect() const                    { return aOuterRect; }
    SfxBindings&        GetBindings() const                     { return *pBindings; }
    sal_uInt16              GetType() const                         { return pMgr->GetType(); }
    SfxChildAlignment   GetAlignment() const                    { return pMgr->GetAlignment(); }
    void                SetAlignment(SfxChildAlignment eAlign)  { pMgr->SetAlignment(eAlign); }
    Size                GetFloatingSize() const                 { return aFloatSize; }
    void                SetFloatingSize(const Size& rSize)      { aFloatSize=rSize; }

    void                SetMinOutputSizePixel( const Size& rSize );
    Size                GetMinOutputSizePixel() const;
    virtual bool        Notify( NotifyEvent& rNEvt );
    virtual void        FadeIn( sal_Bool );
    void                AutoShow( sal_Bool bShow = sal_True );
    DECL_LINK(TimerHdl, void *);

    SAL_DLLPRIVATE void Initialize_Impl();
    SAL_DLLPRIVATE sal_uInt16 GetWinBits_Impl() const;
    SAL_DLLPRIVATE void SetItemSize_Impl( const Size& rSize );
    SAL_DLLPRIVATE void Disappear_Impl();
    SAL_DLLPRIVATE void Reappear_Impl();
    SAL_DLLPRIVATE sal_Bool IsAutoHide_Impl() const;
    SAL_DLLPRIVATE void AutoShow_Impl( sal_Bool bShow = sal_True );
    SAL_DLLPRIVATE void Pin_Impl( sal_Bool bPinned );
    SAL_DLLPRIVATE void ReleaseChildWindow_Impl();
};

class SfxDockingWrapper : public SfxChildWindow
{
    public:
        SfxDockingWrapper( Window* pParent ,
                           sal_uInt16 nId ,
                           SfxBindings* pBindings ,
                           SfxChildWinInfo* pInfo );

        SFX_DECL_CHILDWINDOW(SfxDockingWrapper);
};

#endif // INCLUDED_SFX2_DOCKWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
