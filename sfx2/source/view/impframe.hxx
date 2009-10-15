/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impframe.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _SFX_IMPFRAME_HXX
#define _SFX_IMPFRAME_HXX

#include <svl/cancel.hxx>

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>                  // SvBorder

class SfxViewFrame;
class SfxObjectShell;
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <cppuhelper/weak.hxx>

#include <sfx2/viewsh.hxx>
#include <sfx2/sfxuno.hxx>

#ifndef FRAME_SEARCH_PARENT
#define FRAME_SEARCH_PARENT     0x00000001
#define FRAME_SEARCH_SELF       0x00000002
#define FRAME_SEARCH_CHILDREN   0x00000004
#define FRAME_SEARCH_CREATE     0x00000008
#endif

class SfxFrame_Impl : public SfxBroadcaster, public SvCompatWeakBase, public SfxListener
{
friend class SfxFrame;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;
    String                    aFrameIdName;
    sal_uInt32                nType;
    sal_uInt32                nHistoryPos;
    SfxViewFrame*             pCurrentViewFrame;
    SfxObjectShell*           pCurrentObjectShell;
    SfxFrameDescriptor*       pDescr;
    sal_uInt16                nFrameId;
    sal_uInt16                nLocks;
    sal_Bool                  bCloseOnUnlock : 1;
    sal_Bool                  bClosing : 1;
    sal_Bool                  bPrepClosing : 1;
    sal_Bool                  bInCancelTransfers : 1;
    sal_Bool                  bOwnsBindings : 1;
    sal_Bool                  bReleasingComponent : 1;
    sal_Bool                  bFocusLocked : 1;
    sal_Bool                  bInPlace : 1;
    SfxCancelManager*         pCancelMgr;
    SfxCancellable*           pLoadCancellable;
    SfxFrame*                 pFrame;
    const SfxItemSet*         pSet;
    SfxWorkWindow*            pWorkWin;
    SvBorder                  aBorder;

    SfxFrame_Impl( SfxFrame* pAntiImplP ) :
                              SvCompatWeakBase( pAntiImplP ),
                              nType( 0L ),
                              nHistoryPos( 0 ),
                              pCurrentViewFrame( NULL ),
                              pCurrentObjectShell( NULL ),
                              pDescr( NULL ),
                              nFrameId( 0 ),
                              nLocks( 0 ),
                              bCloseOnUnlock( sal_False ),
                              bClosing(sal_False),
                              bPrepClosing(sal_False),
                              bInCancelTransfers( sal_False ),
                              bOwnsBindings( sal_False ),
                              bReleasingComponent( sal_False ),
                              bFocusLocked( sal_False ),
                              bInPlace( sal_False ),
                              pCancelMgr( 0 ),
                              pLoadCancellable( 0 ),
                              pFrame( pAntiImplP ),
                              pSet( 0 ),
                              pWorkWin( 0 )
                              {}

    virtual ~SfxFrame_Impl() { delete pCancelMgr; delete pLoadCancellable; }
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif

