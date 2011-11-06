/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _AVMEDIA_MEDIAWINDOW_HXX
#define _AVMEDIA_MEDIAWINDOW_HXX

#include <memory>
#include <vector>
#include <tools/gen.hxx>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/uno/XInterface.hpp>

// -----------
// - Defines -
// -----------

#define AVMEDIA_FRAMEGRABBER_DEFAULTFRAME -1.0

// ------------------------
// - Forward Declarations -
// ------------------------

class Window;
class KeyEvent;
class MouseEvent;
class CommandEvent;
class PopupMenu;
class Pointer;
struct AcceptDropEvent;
struct ExecuteDropEvent;

namespace rtl { class OUString; }

/* Declaration of MediaWindow class */

namespace avmedia
{
    typedef ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > > FilterNameVector;

    class MediaItem;

    namespace priv { class MediaWindowImpl; }

    // ---------------
    // - MediaWindow -
    // ---------------

    class MediaWindow
    {
    public:
                            MediaWindow( Window* parent, bool bInternalMediaControl );
        virtual             ~MediaWindow();

        void                setURL( const ::rtl::OUString& rURL );
        const ::rtl::OUString&  getURL() const;

        bool                isValid() const;
//
        bool                hasPreferredSize() const;
        Size                getPreferredSize() const;

        Window*             getWindow() const;

        void                setPosSize( const Rectangle& rNewRect );
        Rectangle           getPosSize() const;

        void                setPointer( const Pointer& rPointer );
        const Pointer&      getPointer() const;

        bool                setZoom( ::com::sun::star::media::ZoomLevel eLevel );
        ::com::sun::star::media::ZoomLevel  getZoom() const;

        bool                start();
        void                stop();

        bool                isPlaying() const;

        double              getDuration() const;

        void                setMediaTime( double fTime );
        double              getMediaTime() const;

        void                setStopTime( double fTime );
        double              getStopTime() const;

        void                setRate( double fRate );
        double              getRate() const;

        void                setPlaybackLoop( bool bSet );
        bool                isPlaybackLoop() const;

        void                setMute( bool bSet );
        bool                isMute() const;

        void                updateMediaItem( MediaItem& rItem ) const;
        void                executeMediaItem( const MediaItem& rItem );

        void                show();
        void                hide();

        void                enable();
        void                disable();

    public:

        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& rMEvt );

        virtual void        KeyInput( const KeyEvent& rKEvt );
        virtual void        KeyUp( const KeyEvent& rKEvt );

        virtual void        Command( const CommandEvent& rCEvt );

        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    public:

        static void         getMediaFilters( FilterNameVector& rFilterNameVector );
        static bool         executeMediaURLDialog( Window* pParent, ::rtl::OUString& rURL, bool bInsertDialog = true );
        static void         executeFormatErrorBox( Window* pParent );
        static bool         isMediaURL( const ::rtl::OUString& rURL, bool bDeep = false, Size* pPreferredSizePixel = NULL );

        static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const ::rtl::OUString& rURL );

        static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > grabFrame( const ::rtl::OUString& rURL,
                                                                                                  bool bAllowToCreateReplacementGraphic = false,
                                                                                                  double fMediaTime = AVMEDIA_FRAMEGRABBER_DEFAULTFRAME );

    private:

                    // default: disabled copy/assignment
        MediaWindow(const MediaWindow&);
        MediaWindow& operator =( const MediaWindow& );

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   mxIFace;
        priv::MediaWindowImpl*                                                  mpImpl;
    };
}

#endif // _AVMEDIA_MEDIAWINDOW_HXX
