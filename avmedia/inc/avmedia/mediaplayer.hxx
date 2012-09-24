/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
