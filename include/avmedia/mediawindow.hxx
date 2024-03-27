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

#pragma once

#include <vector>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/media/XPlayerListener.hpp>
#include <comphelper/compbase.hxx>
#include <vcl/vclptr.hxx>
#include <avmedia/avmediadllapi.h>

#define AVMEDIA_FRAMEGRABBER_DEFAULTFRAME -1.0

namespace com::sun::star::frame { class XDispatchProvider; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::media {
    class XPlayer;
    class XPlayerNotifier;
}

namespace vcl { class Window; }
namespace weld { class Window; }
class KeyEvent;
class MouseEvent;
class CommandEvent;
struct AcceptDropEvent;
struct ExecuteDropEvent;
enum class PointerStyle;

namespace avmedia
{
    typedef ::std::vector< ::std::pair< OUString, OUString > > FilterNameVector;

    class MediaItem;

    namespace priv { class MediaWindowImpl; }

    typedef comphelper::WeakComponentImplHelper<css::media::XPlayerListener> PlayerListener_BASE;

    class AVMEDIA_DLLPUBLIC PlayerListener final : public PlayerListener_BASE
    {
    private:
        css::uno::Reference<css::media::XPlayerNotifier> m_xNotifier;
        std::function<void(const css::uno::Reference<css::media::XPlayer>&)> m_aFn;

        using WeakComponentImplHelperBase::disposing;
    public:
        PlayerListener(std::function<void(const css::uno::Reference<css::media::XPlayer>&)> fn);
        virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;
        virtual ~PlayerListener() override;

        virtual void SAL_CALL preferredPlayerWindowSizeAvailable(const css::lang::EventObject& rSource) override;
        virtual void SAL_CALL disposing(const css::lang::EventObject& rSource) override;

        void startListening(const css::uno::Reference<css::media::XPlayerNotifier>& rNotifier);
        void stopListening();

        void callPlayerWindowSizeAvailable(const css::uno::Reference<css::media::XPlayer>& rPlayer) { m_aFn(rPlayer); }
    private:
        void stopListening(std::unique_lock<std::mutex>& rGuard);
    };

    class AVMEDIA_DLLPUBLIC MediaWindow
    {
    public:
                            MediaWindow( vcl::Window* parent, bool bInternalMediaControl );
        virtual             ~MediaWindow();

        void                setURL( const OUString& rURL, const OUString& rReferer );
        const OUString&     getURL() const;

        bool                isValid() const;
        Size                getPreferredSize() const;

        vcl::Window*             getWindow() const;

        void                setPosSize( const tools::Rectangle& rNewRect );

        void                setPointer( PointerStyle aPointer );

        bool                start();

        void                updateMediaItem( MediaItem& rItem ) const;
        void                executeMediaItem( const MediaItem& rItem );

        void                show();
        void                hide();
        bool                isVisible() const;

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

        static FilterNameVector getMediaFilters();
        /// @param o_pbLink if not 0, this is an "insert" dialog: display link
        ///                 checkbox and store its state in *o_pbLink
        static bool         executeMediaURLDialog(weld::Window* pParent, OUString& rURL, bool *const o_pbLink);
        static void         executeFormatErrorBox(weld::Window* pParent);
        static bool         isMediaURL(std::u16string_view rURL, const OUString& rReferer, bool bDeep = false,
                                       const rtl::Reference<PlayerListener>& xPreferredPixelSizeListener = nullptr);

        static css::uno::Reference< css::media::XPlayer > createPlayer( const OUString& rURL, const OUString& rReferer, const OUString* pMimeType = nullptr );

        static css::uno::Reference<css::graphic::XGraphic>
        grabFrame(const css::uno::Reference<css::media::XPlayer>& rPlayer,
                  const css::uno::Reference<css::graphic::XGraphic>& rGraphic = nullptr);

        static css::uno::Reference< css::graphic::XGraphic > grabFrame(const OUString& rURL, const OUString& rReferer,
            const OUString& sMimeType, const rtl::Reference<PlayerListener>& xPreferredPixelSizeListener);

        static void dispatchInsertAVMedia(const css::uno::Reference<css::frame::XDispatchProvider>&,
                                          const css::awt::Size& rSize, const OUString& rURL, bool bLink);

    private:
        MediaWindow(const MediaWindow&) = delete;
        MediaWindow& operator =( const MediaWindow& ) = delete;

        VclPtr<priv::MediaWindowImpl>                 mpImpl;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
