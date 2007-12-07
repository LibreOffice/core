/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: topfrm.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 12:22:13 $
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
#ifndef _SFX_TOPFRM_HXX
#define _SFX_TOPFRM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _SV_DOCKWIN_HXX //autogen
#include <vcl/dockwin.hxx>
#endif
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
    virtual BOOL        InsertDocument( SfxObjectShell *pDoc );
    String              GetWindowData();

    void                SetPresentationMode( BOOL bSet );
    SystemWindow* GetSystemWindow() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void LockResize_Impl( BOOL bLock );
    SAL_DLLPRIVATE void SetMenuBarOn_Impl( BOOL bOn );
    SAL_DLLPRIVATE BOOL IsMenuBarOn_Impl() const;
    SAL_DLLPRIVATE BOOL InsertDocument_Impl( SfxObjectShell *pDoc, USHORT nViewId=0 );
    SAL_DLLPRIVATE SystemWindow* GetTopWindow_Impl() const;
    SAL_DLLPRIVATE void CheckMenuCloser_Impl( MenuBar* pMenubar );
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

