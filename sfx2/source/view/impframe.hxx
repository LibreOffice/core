/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SFX_IMPFRAME_HXX
#define _SFX_IMPFRAME_HXX

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

class SfxFrame_Impl : public SfxBroadcaster, public SvCompatWeakBase
{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;
    sal_uInt32                nType;
    SfxViewFrame*             pCurrentViewFrame;
    SfxFrameDescriptor*       pDescr;
    sal_uInt16                nLocks;
    sal_Bool                  bClosing : 1;
    sal_Bool                  bPrepClosing : 1;
    sal_Bool                  bInCancelTransfers : 1;
    sal_Bool                  bOwnsBindings : 1;
    sal_Bool                  bReleasingComponent : 1;
    sal_Bool                  bInPlace : 1;
    SfxFrame*                 pFrame;
    SfxWorkWindow*            pWorkWin;
    SvBorder                  aBorder;
    // formerly SfxTopFrame
    Window*                   pExternalContainerWindow;
    bool                      bHidden;
    bool                      bLockResize;
    bool                      bMenuBarOn;

    SfxFrame_Impl( SfxFrame* pAntiImplP )
        :SvCompatWeakBase( pAntiImplP )
        ,nType( 0L )
        ,pCurrentViewFrame( NULL )
        ,pDescr( NULL )
        ,nLocks( 0 )
        ,bClosing(sal_False)
        ,bPrepClosing(sal_False)
        ,bInCancelTransfers( sal_False )
        ,bOwnsBindings( sal_False )
        ,bReleasingComponent( sal_False )
        ,bInPlace( sal_False )
        ,pFrame( pAntiImplP )
        ,pWorkWin( 0 )
        ,pExternalContainerWindow( NULL )
        ,bHidden( false )
        ,bLockResize( false )
        ,bMenuBarOn( true )
    {
    }

    virtual ~SfxFrame_Impl() { }
};

#endif

