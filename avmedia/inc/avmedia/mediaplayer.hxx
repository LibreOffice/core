/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef _AVMEDIA_MEDIACHILD_HXX
#define _AVMEDIA_MEDIACHILD_HXX

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <avmedia/avmediadllapi.h>

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

class AVMEDIA_DLLPUBLIC MediaPlayer : public SfxChildWindow
{
public:
                        MediaPlayer( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
                        ~MediaPlayer();

                        SFX_DECL_CHILDWINDOW_WITHID( MediaPlayer );
};

// ----------------
// - MediaFloater -
// ----------------

class MediaWindow;

class AVMEDIA_DLLPUBLIC MediaFloater : public SfxDockingWindow
{
public:

                            MediaFloater( SfxBindings* pBindings, SfxChildWindow* pCW, Window* pParent );
                            ~MediaFloater();

    void                    setURL( const OUString& rURL, bool bPlayImmediately );
    const OUString&         getURL() const;

    void                    dispatchCurrentURL();

protected:

    virtual void            Resize();
    virtual void            ToggleFloatingMode();

private:

    MediaWindow*            mpMediaWindow;
    Size                    maLastSize;

    AVMEDIA_DLLPRIVATE void implInit();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
