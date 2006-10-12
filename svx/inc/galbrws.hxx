/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galbrws.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:42:58 $
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

#ifndef _SVX_GALBRWS_HXX_
#define _SVX_GALBRWS_HXX_

#ifndef _SFXCTRLITEM_HXX
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// -----------
// - Defines -
// -----------

#define GALLERYBROWSER() ((GalleryBrowser*)( SfxViewFrame::Current()->GetChildWindow(GalleryChildWindow::GetChildWindowId())->GetWindow()))

// -------------------------
// - SvxGalleryChildWindow -
// -------------------------

class SVX_DLLPUBLIC GalleryChildWindow : public SfxChildWindow
{
public:
                        GalleryChildWindow( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );
                        ~GalleryChildWindow();

                        SFX_DECL_CHILDWINDOW( GalleryChildWindow );
};

// ------------------
// - FormModel -
// ------------------

class FmFormModel;

// ------------------
// - GalleryBrowser -
// ------------------

class GalleryBrowser1;
class GalleryBrowser2;
class GallerySplitter;
class Gallery;

class GalleryBrowser : public SfxDockingWindow
{
    friend class GalleryBrowser1;
    friend class GalleryBrowser2;
    friend class GallerySplitter;
    using Window::KeyInput;

private:

    Size                    maLastSize;
    GallerySplitter*        mpSplitter;
    GalleryBrowser1*        mpBrowser1;
    GalleryBrowser2*        mpBrowser2;
    Gallery*                mpGallery;
    long                    mnDummy1;
    long                    mnDummy2;
    long                    mnDummy3;

    void                    InitSettings();

    virtual BOOL            Close();
    virtual void            Resize();
    virtual void            GetFocus();

                            DECL_LINK( SplitHdl, void* );

protected:

    void                    ThemeSelectionHasChanged();

public:

                            GalleryBrowser( SfxBindings* pBindings, SfxChildWindow* pCW,
                                            Window* pParent, const ResId& rResId );
                            ~GalleryBrowser();

    INetURLObject           GetURL() const;
    String                  GetFilterName() const;
    Graphic                 GetGraphic() const;
    BOOL                    GetVCDrawModel( FmFormModel& rModel ) const;
    BOOL                    IsLinkage() const;

    BOOL                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};

#endif // _SVX_GALBRWS_HXX_
