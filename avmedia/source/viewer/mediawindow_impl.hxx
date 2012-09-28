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

#include "mediawindowbase_impl.hxx"
#include "mediacontrol.hxx"

class BitmapEx;

namespace avmedia
{
    namespace priv
    {
        // ----------------------
        // - MediaWindowControl -
        // ----------------------

        class MediaWindowControl : public MediaControl
        {
        public:

                    MediaWindowControl( Window* pParent );
                    ~MediaWindowControl();

        protected:

            void    update();
            void    execute( const MediaItem& rItem );
        };

        // --------------------
        // - MediaChildWindow -
        // --------------------

        class MediaChildWindow : public SystemChildWindow
        {
        public:

                            MediaChildWindow( Window* pParent );
                            ~MediaChildWindow();

        protected:

            virtual void    MouseMove( const MouseEvent& rMEvt );
            virtual void    MouseButtonDown( const MouseEvent& rMEvt );
            virtual void    MouseButtonUp( const MouseEvent& rMEvt );
            virtual void    KeyInput( const KeyEvent& rKEvt );
            virtual void    KeyUp( const KeyEvent& rKEvt );
            virtual void    Command( const CommandEvent& rCEvt );
        };

        // ------------------.
        // - MediaWindowImpl -
        // -------------------

        class MediaEventListenersImpl;

        class MediaWindowImpl : public Control,
                                public MediaWindowBaseImpl,
                                public DropTargetHelper,
                                public DragSourceHelper

        {
        public:

                            MediaWindowImpl( Window* parent, MediaWindow* pMediaWindow, bool bInternalMediaControl );
            virtual         ~MediaWindowImpl();

            virtual void    cleanUp();
            virtual void    onURLChanged();

        public:

            void            setPosSize( const Rectangle& rRect );

            void            setPointer( const Pointer& rPointer );

        protected:

            // Window
            virtual void    MouseMove( const MouseEvent& rMEvt );
            virtual void    MouseButtonDown( const MouseEvent& rMEvt );
            virtual void    MouseButtonUp( const MouseEvent& rMEvt );
            virtual void    KeyInput( const KeyEvent& rKEvt );
            virtual void    KeyUp( const KeyEvent& rKEvt );
            virtual void    Command( const CommandEvent& rCEvt );
            virtual void    Resize();
            virtual void    StateChanged( StateChangedType );
            virtual void    Paint( const Rectangle& ); // const
            virtual void    GetFocus();

            // DropTargetHelper
            virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
            virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

            // DragSourceHelper
            virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );

        private:

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
