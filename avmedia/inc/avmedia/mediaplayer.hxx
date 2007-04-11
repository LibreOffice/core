/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediaplayer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:25:05 $
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

#ifndef _AVMEDIA_MEDIACHILD_HXX
#define _AVMEDIA_MEDIACHILD_HXX

#ifndef _SFXCTRLITEM_HXX
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

// -----------
// - Defines -
// -----------

#define AVMEDIA_MEDIAWINDOW()                                                                                           \
(static_cast< ::avmedia::MediaFloater* >( (                                                                             \
SfxViewFrame::Current() && SfxViewFrame::Current()->GetChildWindow(::avmedia::MediaPlayer::GetChildWindowId())) ?   \
SfxViewFrame::Current()->GetChildWindow(::avmedia::MediaPlayer::GetChildWindowId())->GetWindow() :              \
NULL))

namespace avmedia
{

// ---------------
// - MediaPlayer -
// ---------------

class MediaPlayer : public SfxChildWindow
{
public:
                        MediaPlayer( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );
                        ~MediaPlayer();

                        SFX_DECL_CHILDWINDOW( MediaPlayer );
};

// ----------------
// - MediaFloater -
// ----------------

class MediaWindow;

class MediaFloater : public SfxDockingWindow
{
public:

                            MediaFloater( SfxBindings* pBindings, SfxChildWindow* pCW, Window* pParent );
                            ~MediaFloater();

    void                    setURL( const ::rtl::OUString& rURL, bool bPlayImmediately );
    const ::rtl::OUString&  getURL() const;

    void                    dispatchCurrentURL();

protected:

    virtual void            Resize();
    virtual void            ToggleFloatingMode();

private:

    MediaWindow*            mpMediaWindow;
    Size                    maLastSize;
    long                    mnDummy1;
    long                    mnDummy2;

    void                    implInit();
};

}

#endif
