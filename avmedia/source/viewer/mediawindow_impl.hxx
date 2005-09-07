/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediawindow_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:43:57 $
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

#ifndef _AVMEDIA_MEDIAWINDOW_IMPL_HXX
#define _AVMEDIA_MEDIAWINDOW_IMPL_HXX

#include <svtools/transfer.hxx>
#include <vcl/javachild.hxx>

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

        class MediaChildWindow : public JavaChildWindow
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
