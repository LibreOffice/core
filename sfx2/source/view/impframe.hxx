/*************************************************************************
 *
 *  $RCSfile: impframe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:29:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    LoadEnvironment_Impl*   pLoadEnv;
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
                                pLoadEnv( NULL ),
                                pCurrentObjectShell( NULL ),
                                pCurrentViewFrame( NULL ),
                                bInCancelTransfers( sal_False ),
                                bCloseOnUnlock( sal_False ),
                                bOwnsBindings( sal_False ),
                                bReleasingComponent( sal_False ),
                                bFocusLocked( sal_False ),
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

