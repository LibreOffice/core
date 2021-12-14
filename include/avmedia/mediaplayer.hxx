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

#ifndef INCLUDED_AVMEDIA_MEDIAPLAYER_HXX
#define INCLUDED_AVMEDIA_MEDIAPLAYER_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <avmedia/avmediadllapi.h>
#include <memory>

namespace avmedia
{

class AVMEDIA_DLLPUBLIC MediaPlayer final : public SfxChildWindow
{
public:
                        MediaPlayer( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );
                        virtual ~MediaPlayer() override;

                        SFX_DECL_CHILDWINDOW_WITHID( MediaPlayer );
};

class MediaWindow;

class AVMEDIA_DLLPUBLIC MediaFloater final : public SfxDockingWindow
{
public:

                            MediaFloater( SfxBindings* pBindings, SfxChildWindow* pCW, vcl::Window* pParent );
                            virtual ~MediaFloater() override;
    virtual void            dispose() override;

    void                    setURL( const OUString& rURL, const OUString& rReferer, bool bPlayImmediately );

    void                    dispatchCurrentURL();

private:

    virtual void            Resize() override;
    virtual void            ToggleFloatingMode() override;

    std::unique_ptr<MediaWindow> mpMediaWindow;
};

inline MediaFloater * getMediaFloater() {
    SfxViewFrame * cur = SfxViewFrame::Current();
    if (cur != nullptr) {
        SfxChildWindow * win = cur->GetChildWindow(
            MediaPlayer::GetChildWindowId());
        if (win != nullptr) {
            return static_cast<MediaFloater *>(win->GetWindow());
        }
    }
    return nullptr;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
