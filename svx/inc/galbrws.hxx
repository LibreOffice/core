/*************************************************************************
 *
 *  $RCSfile: galbrws.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 09:54:22 $
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

// -----------
// - Defines -
// -----------

#define GALLERYBROWSER() ((GalleryBrowser*)( SfxViewFrame::Current()->GetChildWindow(GalleryChildWindow::GetChildWindowId())->GetWindow()))

// -------------------------
// - SvxGalleryChildWindow -
// -------------------------

class GalleryChildWindow : public SfxChildWindow
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
class Splitter;
class Gallery;

class GalleryBrowser : public SfxDockingWindow
{
    friend class GalleryBrowser1;
    friend class GalleryBrowser2;

private:

    Size                    maLastSize;
    Splitter*               mpSplitter;
    GalleryBrowser1*        mpBrowser1;
    GalleryBrowser2*        mpBrowser2;
    Gallery*                mpGallery;
    long                    mnDummy1;
    long                    mnDummy2;
    long                    mnDummy3;

    virtual BOOL            Close();
    virtual void            Resize();

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
};

#endif // _SVX_GALBRWS_HXX_
