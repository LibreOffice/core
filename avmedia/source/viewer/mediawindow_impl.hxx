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

#ifndef _AVMEDIA_MEDIAWINDOW_IMPL_HXX
#define _AVMEDIA_MEDIAWINDOW_IMPL_HXX

#include <svtools/transfer.hxx>
#include <vcl/syschild.hxx>

#include "mediacontrol.hxx"

namespace com { namespace sun { namespace star { namespace media {
    class XPlayer;
    class XPlayerWindow;
} } } }
class BitmapEx;

namespace avmedia
{
    class MediaWindow;

    namespace priv
    {

        // - MediaWindowControl -


        class MediaWindowControl : public MediaControl
        {
        public:

                    MediaWindowControl( Window* pParent );
                    ~MediaWindowControl();

        protected:

            void    update() SAL_OVERRIDE;
            void    execute( const MediaItem& rItem ) SAL_OVERRIDE;
        };


        // - MediaChildWindow -


        class MediaChildWindow : public SystemChildWindow
        {
        public:

                            MediaChildWindow( Window* pParent );
                            ~MediaChildWindow();

        protected:

            virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
            virtual void    KeyUp( const KeyEvent& rKEvt ) SAL_OVERRIDE;
            virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
        };

        // ------------------.
        // - MediaWindowImpl -


        class MediaEventListenersImpl;

        class MediaWindowImpl : public Control,
                                public DropTargetHelper,
                                public DragSourceHelper

        {
        public:

                            MediaWindowImpl( Window* parent, MediaWindow* pMediaWindow, bool bInternalMediaControl );
            virtual         ~MediaWindowImpl();

            static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const OUString& rURL, const OUString& rReferer );

            void    setURL( const OUString& rURL, OUString const& rTempURL, OUString const& rReferer );

            const   OUString&  getURL() const;

            bool    isValid() const;

            Size    getPreferredSize() const;

            bool    start();

            void    updateMediaItem( MediaItem& rItem ) const;
            void    executeMediaItem( const MediaItem& rItem );

            void            setPosSize( const Rectangle& rRect );

            void            setPointer( const Pointer& rPointer );

        private:

            // Window
            virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
            virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
            virtual void    KeyUp( const KeyEvent& rKEvt ) SAL_OVERRIDE;
            virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
            virtual void    Resize() SAL_OVERRIDE;
            virtual void    StateChanged( StateChangedType ) SAL_OVERRIDE;
            virtual void    Paint( const Rectangle& ) SAL_OVERRIDE; // const
            virtual void    GetFocus() SAL_OVERRIDE;

            // DropTargetHelper
            virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
            virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

            // DragSourceHelper
            virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

            bool    setZoom( ::com::sun::star::media::ZoomLevel eLevel );
            ::com::sun::star::media::ZoomLevel getZoom() const;

            void    stop();

            bool    isPlaying() const;

            double  getDuration() const;

            void    setMediaTime( double fTime );
            double  getMediaTime() const;

            double  getRate() const;

            void    setPlaybackLoop( bool bSet );
            bool    isPlaybackLoop() const;

            void    setMute( bool bSet );
            bool    isMute() const;

            void    setVolumeDB( sal_Int16 nVolumeDB );
            sal_Int16 getVolumeDB() const;

            void    stopPlayingInternal( bool );

            void            onURLChanged();

            OUString                                                                    maFileURL;
            OUString                                                                    mTempFileURL;
            OUString                                                                    maReferer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            MediaWindow*                                                                mpMediaWindow;

            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   mxEventsIf;
            MediaEventListenersImpl*                                                mpEvents;
            MediaChildWindow                                                        maChildWindow;
            MediaWindowControl*                                                     mpMediaWindowControl;
            BitmapEx*                                                               mpEmptyBmpEx;
            BitmapEx*                                                               mpAudioBmpEx;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
