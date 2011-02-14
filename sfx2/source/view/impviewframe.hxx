/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
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
************************************************************************/

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
    TypeId              aLastType;
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

