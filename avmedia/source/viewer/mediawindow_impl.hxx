/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediawindow_impl.hxx,v $
 * $Revision: 1.3 $
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

            void            update();

            void            setPosSize( const Rectangle& rRect );

            void            setPointer( const Pointer& rPointer );
            const Pointer&  getPointer() const;

            bool            hasInternalMediaControl() const;

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
