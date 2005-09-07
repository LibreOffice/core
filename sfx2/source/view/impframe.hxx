/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impframe.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:27:12 $
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

#ifndef _SFX_IMPFRAME_HXX
#define _SFX_IMPFRAME_HXX

#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
#endif

#pragma hdrstop

#include "frame.hxx"
#include "loadenv.hxx"
#include "viewfrm.hxx"                  // SvBorder

class SfxViewFrame;
class SfxObjectShell;
class SfxExplorerBrowserConfig;

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <viewsh.hxx>
#include <sfxuno.hxx>

#ifndef FRAME_SEARCH_PARENT
#define FRAME_SEARCH_PARENT     0x00000001
#define FRAME_SEARCH_SELF       0x00000002
#define FRAME_SEARCH_CHILDREN   0x00000004
#define FRAME_SEARCH_CREATE     0x00000008
#endif

class SfxFrame_Impl : public SfxBroadcaster, public SvCompatWeakBase, public SfxListener
{
friend class SfxFrame;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                     xFrame;
    String                  aFrameIdName;
    sal_uInt32              nType;
    sal_uInt32              nHistoryPos;
    SfxViewFrame*           pCurrentViewFrame;
    SfxObjectShell*         pCurrentObjectShell;
    SfxFrameDescriptor*     pDescr;
    SfxExplorerBrowserConfig* pBrowserCfg;
    sal_uInt16              nFrameId;
    sal_uInt16              nLocks;
    sal_Bool                bCloseOnUnlock : 1;
    sal_Bool                bClosing : 1;
    sal_Bool                bPrepClosing : 1;
    sal_Bool                bInCancelTransfers : 1;
    sal_Bool                bOwnsBindings : 1;
    sal_Bool                bReleasingComponent : 1;
    sal_Bool                bFocusLocked : 1;
    sal_Bool                bInPlace : 1;
    sal_uInt16              nHasBrowser;
    SfxCancelManager*       pCancelMgr;
    SfxCancellable*         pLoadCancellable;
    SfxFrame*               pFrame;
    const SfxItemSet*       pSet;
    SfxWorkWindow*          pWorkWin;
    SvBorder                aBorder;

                            SfxFrame_Impl( SfxFrame* pAntiImplP ) :
                                SvCompatWeakBase( pAntiImplP ),
                                pFrame( pAntiImplP ),
                                bClosing(sal_False),
                                bPrepClosing(sal_False),
                                nType( 0L ),
                                nHistoryPos( 0 ),
                                nFrameId( 0 ),
                                pCurrentObjectShell( NULL ),
                                pCurrentViewFrame( NULL ),
                                bInCancelTransfers( sal_False ),
                                bCloseOnUnlock( sal_False ),
                                bOwnsBindings( sal_False ),
                                bReleasingComponent( sal_False ),
                                bFocusLocked( sal_False ),
                                bInPlace( sal_False ),
                                nLocks( 0 ),
                                pBrowserCfg( NULL ),
                                pDescr( NULL ),
                                nHasBrowser( SFX_BEAMER_OFF ),
                                pCancelMgr( 0 ),
                                pLoadCancellable( 0 ),
                                pSet( 0 ),
                                pWorkWin( 0 )
                            {}

                            ~SfxFrame_Impl() { delete pCancelMgr;
                                               delete pLoadCancellable; }
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif

