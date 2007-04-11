/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediawindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:25:25 $
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

#ifndef _AVMEDIA_MEDIAWINDOW_HXX
#define _AVMEDIA_MEDIAWINDOW_HXX

#include <memory>
#include <vector>
#include <tools/gen.hxx>

#ifndef _COM_SUN_STAR_MEDIA_ZOOMLEVEL_HPP_
#include <com/sun/star/media/ZoomLevel.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

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
