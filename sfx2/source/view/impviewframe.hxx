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



#ifndef SFX2_IMPVIEWFRAME_HXX
#define SFX2_IMPVIEWFRAME_HXX

#include "sfx2/viewfrm.hxx"

#include <svtools/asynclink.hxx>
#include <vcl/window.hxx>

#include <boost/optional.hpp>

struct SfxViewFrame_Impl
{
    SvBorder            aBorder;
    Size                aMargin;
    Size                aSize;
    String              aFrameTitle;
    String              aActualURL;
    SfxFrame&           rFrame;
    svtools::AsynchronLink* pReloader;
    Window*             pWindow;
    SfxViewFrame*       pActiveChild;
    Window*             pFocusWin;
    sal_uInt16          nDocViewNo;
    sal_uInt16          nCurViewId;
    sal_Bool            bResizeInToOut:1;
    sal_Bool            bDontOverwriteResizeInToOut:1;
    sal_Bool            bObjLocked:1;
    sal_Bool            bReloading:1;
    sal_Bool            bIsDowning:1;
    sal_Bool            bModal:1;
    sal_Bool            bEnabled:1;
    sal_Bool            bWindowWasEnabled:1;
    sal_Bool            bActive;
    String              aFactoryName;
    ::boost::optional< bool >
                        aHasToolPanels;

                        SfxViewFrame_Impl( SfxFrame& i_rFrame )
                        : rFrame( i_rFrame )
                        , pReloader(0 )
                        , pWindow( 0 )
                        , bWindowWasEnabled(sal_True)
                        , bActive( sal_False )
                        {
                        }

                        ~SfxViewFrame_Impl()
                        {
                            delete pReloader;
                        }
};

class SfxFrameViewWindow_Impl : public Window
{
    sal_Bool            bActive;
    SfxViewFrame*   pFrame;

public:
                        SfxFrameViewWindow_Impl( SfxViewFrame* p, Window& rParent, WinBits nBits=0 ) :
                            Window( &rParent, nBits | WB_BORDER | WB_CLIPCHILDREN ),
                            bActive( sal_False ),
                            pFrame( p )
                        {
                            p->GetFrame().GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
                        }

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
};

#endif // SFX2_IMPVIEWFRAME_HXX

