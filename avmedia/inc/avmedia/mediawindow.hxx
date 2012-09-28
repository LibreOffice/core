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

#ifndef _AVMEDIA_MEDIAWINDOW_HXX
#define _AVMEDIA_MEDIAWINDOW_HXX

#include <memory>
#include <vector>
#include <tools/gen.hxx>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <avmedia/avmediadllapi.h>

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
    typedef ::std::vector< ::std::pair< OUString, OUString > > FilterNameVector;

    class MediaItem;

    namespace priv { class MediaWindowImpl; }

    // ---------------
    // - MediaWindow -
    // ---------------

    class AVMEDIA_DLLPUBLIC MediaWindow
    {
    public:
                            MediaWindow( Window* parent, bool bInternalMediaControl );
        virtual             ~MediaWindow();

        void                setURL( const OUString& rURL );
        const OUString&     getURL() const;

        bool                isValid() const;
        Size                getPreferredSize() const;

        Window*             getWindow() const;

        void                setPosSize( const Rectangle& rNewRect );

        void                setPointer( const Pointer& rPointer );

        bool                start();

        void                updateMediaItem( MediaItem& rItem ) const;
        void                executeMediaItem( const MediaItem& rItem );

        void                show();
        void                hide();

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
        /// @param o_pbLink if not 0, this is an "insert" dialog: display link
        ///                 checkbox and store its state in *o_pbLink
        static bool         executeMediaURLDialog( Window* pParent,
                OUString& rURL, bool *const o_pbLink );
        static void         executeFormatErrorBox( Window* pParent );
        static bool         isMediaURL( const OUString& rURL, bool bDeep = false, Size* pPreferredSizePixel = NULL );

        static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const OUString& rURL );

        static ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > grabFrame( const OUString& rURL,
                                                                                                  bool bAllowToCreateReplacementGraphic = false,
                                                                                                  double fMediaTime = AVMEDIA_FRAMEGRABBER_DEFAULTFRAME );

    private:

                    // default: disabled copy/assignment
        AVMEDIA_DLLPRIVATE MediaWindow(const MediaWindow&);
        AVMEDIA_DLLPRIVATE MediaWindow& operator =( const MediaWindow& );

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   mxIFace;
        priv::MediaWindowImpl*                                                  mpImpl;
    };
}

#endif // _AVMEDIA_MEDIAWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
