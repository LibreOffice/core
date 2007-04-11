/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dockwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:19:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXDOCKWIN_HXX
#define _SFXDOCKWIN_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _DOCKWIN_HXX //autogen
#include <vcl/dockwin.hxx>
#endif
#include <sfx2/childwin.hxx>

class SfxSplitWindow;
class SfxDockingWindow_Impl;

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
    virtual BOOL        PrepareToggleFloatingMode();
    virtual void        ToggleFloatingMode();
    virtual void        StartDocking();
    virtual BOOL        Docking( const Point& rPos, Rectangle& rRect );
    virtual void        EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    virtual void        Resizing( Size& rSize );
    virtual void        Paint( const Rectangle& rRect );
    virtual BOOL        Close();
    virtual void        Move();

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE SfxChildWindow* GetChildWindow_Impl()    { return pMgr; }
//#endif

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
    USHORT              GetType() const                         { return pMgr->GetType(); }
    SfxChildAlignment   GetAlignment() const                    { return pMgr->GetAlignment(); }
    void                SetAlignment(SfxChildAlignment eAlign)  { pMgr->SetAlignment(eAlign); }
    Size                GetFloatingSize() const                 { return aFloatSize; }
    void                SetFloatingSize(const Size& rSize)      { aFloatSize=rSize; }

    void                SetMinOutputSizePixel( const Size& rSize );
    Size                GetMinOutputSizePixel() const;
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        FadeIn( BOOL );
    void                AutoShow( BOOL bShow = TRUE );

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void Initialize_Impl();
    SAL_DLLPRIVATE USHORT GetWinBits_Impl() const;
    SAL_DLLPRIVATE void SetItemSize_Impl( const Size& rSize );
    SAL_DLLPRIVATE void Disappear_Impl();
    SAL_DLLPRIVATE void Reappear_Impl();
    SAL_DLLPRIVATE BOOL IsAutoHide_Impl() const;
    SAL_DLLPRIVATE BOOL IsPinned_Impl() const;
    SAL_DLLPRIVATE void AutoShow_Impl( BOOL bShow = TRUE );
    SAL_DLLPRIVATE void Pin_Impl( BOOL bPinned );
    SAL_DLLPRIVATE SfxSplitWindow* GetSplitWindow_Impl() const;
    SAL_DLLPRIVATE void ReleaseChildWindow_Impl();
//#endif
};

#endif // #ifndef _SFXDOCKWIN_HXX

