/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: topfrm.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _SFX_TOPFRM_HXX
#define _SFX_TOPFRM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Reference.h>
#include <vcl/dockwin.hxx>
#include <vcl/wrkwin.hxx>

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

class SfxViewShell;
class SfxTopFrame;
class SfxTopFrame_Impl;
namespace svtools { class AsynchronLink; }

// class SfxTopFrame -----------------------------------------------------

class SFX2_DLLPUBLIC SfxTopFrame : public SfxFrame
{
friend class SfxTopWindow_Impl;
friend class SfxTopFrameWindow_Impl;
friend class SfxTopFrame_Impl;

    SfxTopFrame_Impl*   pImp;
    Window*             pWindow;
    SAL_DLLPRIVATE SfxTopFrame( Window* pWindow=NULL, sal_Bool bHidden = sal_False );
    SAL_DLLPRIVATE virtual ~SfxTopFrame();
public:
                        TYPEINFO();

    static SfxTopFrame* Create( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame );
    static SfxTopFrame* Create( SfxObjectShell* pDoc=0, USHORT nViewId=0, BOOL bHidden=sal_False, const SfxItemSet* pSet=NULL );
    static SfxTopFrame* Create( SfxObjectShell* pDoc, Window* pWindow, USHORT nViewId=0, BOOL bHidden=sal_False, const SfxItemSet* pSet=NULL );

    virtual Window&     GetWindow() const;
    virtual BOOL        Close();
    String              GetWindowData();

    void                SetPresentationMode( BOOL bSet );
    SystemWindow* GetSystemWindow() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE BOOL InsertDocument_Impl( SfxObjectShell& rDoc );
    SAL_DLLPRIVATE void LockResize_Impl( BOOL bLock );
    SAL_DLLPRIVATE void SetMenuBarOn_Impl( BOOL bOn );
    SAL_DLLPRIVATE BOOL IsMenuBarOn_Impl() const;
    SAL_DLLPRIVATE SystemWindow* GetTopWindow_Impl() const;
    SAL_DLLPRIVATE void CheckMenuCloser_Impl( MenuBar* pMenubar );
    SAL_DLLPRIVATE void PositionWindow_Impl( const Rectangle& rWinArea ) const;
//#endif
};

// class SfxTopViewFrame -------------------------------------------------

class SfxTopViewFrame_Impl;
class SFX2_DLLPUBLIC SfxTopViewFrame : public SfxViewFrame
{
friend class SfxTopWindow_Impl;
friend class SfxTopViewFrame_Impl;

    SfxTopViewFrame_Impl*   pImp;
    svtools::AsynchronLink* pCloser;

protected:

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void            Activate( BOOL bUI );
    virtual void            Deactivate( BOOL bUI );
    virtual BOOL            Close();
    virtual                 ~SfxTopViewFrame();

public:
                            TYPEINFO();
                            SFX_DECL_INTERFACE(SFX_INTERFACE_SFXTOPFRM)

                            SfxTopViewFrame( SfxFrame* pFrame, SfxObjectShell *pDoc=NULL, USHORT nViewId=0 );

    virtual void            SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );
    SAL_DLLPRIVATE virtual BOOL SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    virtual String          UpdateTitle();

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void Exec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE SfxTopFrame* GetTopFrame_Impl()
                            { return (SfxTopFrame*) GetFrame(); }
//#endif
};

#endif // #ifndef _SFX_TOPFRM_HXX

