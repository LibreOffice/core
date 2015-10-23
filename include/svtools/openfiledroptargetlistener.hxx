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

#ifndef INCLUDED_SVTOOLS_OPENFILEDROPTARGETLISTENER_HXX
#define INCLUDED_SVTOOLS_OPENFILEDROPTARGETLISTENER_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/implbase1.hxx>

#include <sot/exchange.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

/** DropTargetListener that takes care of opening a file when it is dropped in the frame.
*/
class SVT_DLLPUBLIC OpenFileDropTargetListener : public ::cppu::WeakImplHelper1< css::datatransfer::dnd::XDropTargetListener >
{
    private:
        /// uno service manager to create necessary services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /// weakreference to target frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::WeakReference< css::frame::XFrame > m_xTargetFrame;

        /// drag/drop info
        DataFlavorExVector* m_pFormats;

    public:
        OpenFileDropTargetListener( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                    const css::uno::Reference< css::frame::XFrame >& xFrame );
        virtual ~OpenFileDropTargetListener();

    public:
        // XEventListener
        virtual void SAL_CALL disposing        ( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // XDropTargetListener
        virtual void SAL_CALL drop             ( const css::datatransfer::dnd::DropTargetDropEvent&      dtde  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dragEnter        ( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dragExit         ( const css::datatransfer::dnd::DropTargetEvent&          dte   ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dragOver         ( const css::datatransfer::dnd::DropTargetDragEvent&      dtde  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent&      dtde  ) throw(css::uno::RuntimeException, std::exception) override;

    private:
        void     implts_BeginDrag( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors );
        void     implts_EndDrag();
        bool     implts_IsDropFormatSupported( SotClipboardFormatId nFormat );
        void     implts_OpenFile( const OUString& rFilePath );
};

#endif // INCLUDED_SVTOOLS_OPENFILEDROPTARGETLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
