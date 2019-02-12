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

#ifndef INCLUDED_AVMEDIA_SOURCE_VIEWER_MEDIAWINDOW_IMPL_HXX
#define INCLUDED_AVMEDIA_SOURCE_VIEWER_MEDIAWINDOW_IMPL_HXX

#include <vcl/transfer.hxx>
#include <vcl/syschild.hxx>

#include <mediacontrol.hxx>

namespace com { namespace sun { namespace star { namespace media {
    class XPlayer;
    class XPlayerWindow;
}}}}

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
}}}}

class BitmapEx;

namespace avmedia
{

class MediaWindow;

namespace priv
{

class MediaWindowControl : public MediaControl
{
public:

    explicit MediaWindowControl( vcl::Window* pParent );

protected:

    void    update() override;
    void    execute( const MediaItem& rItem ) override;
};

class MediaChildWindow : public SystemChildWindow
{
public:

    explicit MediaChildWindow( vcl::Window* pParent );

protected:

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    KeyUp( const KeyEvent& rKEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
};

class MediaEventListenersImpl;

class MediaWindowImpl : public Control, public DropTargetHelper, public DragSourceHelper
{
public:
    MediaWindowImpl(vcl::Window* parent, MediaWindow* pMediaWindow, bool bInternalMediaControl);
    virtual ~MediaWindowImpl() override;

    virtual void dispose() override;

    static css::uno::Reference<css::media::XPlayer> createPlayer(const OUString& rURL, const OUString& rReferer, const OUString* pMimeType);

    void setURL(const OUString& rURL, OUString const& rTempURL, OUString const& rReferer);

    const OUString& getURL() const;

    bool isValid() const;

    Size getPreferredSize() const;

    bool start();

    void updateMediaItem( MediaItem& rItem ) const;
    void executeMediaItem( const MediaItem& rItem );

    void setPosSize( const tools::Rectangle& rRect );

    void setPointer( PointerStyle nPointer );

private:

    // Window
    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void KeyUp( const KeyEvent& rKEvt ) override;
    virtual void Command( const CommandEvent& rCEvt ) override;
    virtual void Resize() override;
    virtual void StateChanged( StateChangedType ) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override; // const
    virtual void GetFocus() override;

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    // DragSourceHelper
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    void stop();

    bool isPlaying() const;

    double getDuration() const;

    void setMediaTime( double fTime );
    double getMediaTime() const;

    void stopPlayingInternal( bool );

    void onURLChanged();

    static css::uno::Reference<css::media::XPlayer> createPlayer(const OUString& rURL, const OUString& rManagerServName,
                                                                 const css::uno::Reference<css::uno::XComponentContext>& xContext);

    OUString maFileURL;
    OUString mTempFileURL;
    OUString maReferer;
    OUString m_sMimeType;
    css::uno::Reference<css::media::XPlayer> mxPlayer;
    css::uno::Reference<css::media::XPlayerWindow> mxPlayerWindow;
    MediaWindow* mpMediaWindow;

    rtl::Reference<MediaEventListenersImpl> mxEvents;
    VclPtr<MediaChildWindow> mpChildWindow;
    VclPtr<MediaWindowControl> mpMediaWindowControl;
    std::unique_ptr<BitmapEx> mpEmptyBmpEx;
    std::unique_ptr<BitmapEx> mpAudioBmpEx;
};

}} // end namespace avmedia::priv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
