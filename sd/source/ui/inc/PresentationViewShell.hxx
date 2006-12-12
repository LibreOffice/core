/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresentationViewShell.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:36:56 $
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

#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#define SD_PRESENTATION_VIEW_SHELL_HXX

#ifndef SD_DRAW_VIEW_SHELL
#include "DrawViewShell.hxx"
#endif

namespace sd {

/** This view shell is responsible for showing the presentation of an
    Impress document.
*/
class PresentationViewShell
    : public DrawViewShell
{
public:
    TYPEINFO();

    SFX_DECL_VIEWFACTORY(PresViewShell);
    SFX_DECL_INTERFACE( SD_IF_SDPRESVIEWSHELL )

    PresentationViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL);
    static void     CreateFullScreenShow(ViewShell* pOriginShell, SfxRequest& rReq );

    /** This method is used by a simple class that passes some
        arguments from the creator of the new view shell to the new view
        shell object by waiting for its asynchronous creation.
        @param pFrameView
            The frame view that is typically used by the creating object and
            that shall be shared by the created view shell.
        @param rRequest
            A request from which some arguments are extracted by the
            FuSlideShow object.  It usually comes from an Execute() method
            that initiated the creation of the new presentation view shell.
        @param nPageNumber
            The number of the page at which to start the show.
    */
    void FinishInitialization (
        FrameView* pFrameView,
        SfxRequest& rRequest,
        USHORT nPageNumber);

protected:
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, BOOL bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);

private:
    Rectangle       maOldVisArea;
    sal_Bool        mbShowStarted;

    PresentationViewShell (
        SfxViewFrame* pFrame,
        ::Window* pParentWindow,
        const DrawViewShell& rShell);
    virtual ~PresentationViewShell (void);

    virtual void Activate (BOOL bIsMDIActivate);
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
};

} // end of namespace sd

#endif
