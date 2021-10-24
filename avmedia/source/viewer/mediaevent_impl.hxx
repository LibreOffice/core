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

#include <avmedia/mediawindow.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <vcl/vclptr.hxx>

#include <mutex>

namespace avmedia::priv
    {

        // - MediaEventListenersImpl -

        class MediaEventListenersImpl : public ::cppu::WeakImplHelper< css::awt::XKeyListener,
                                                                       css::awt::XMouseListener,
                                                                       css::awt::XMouseMotionListener,
                                                                       css::awt::XFocusListener >
        {
        public:

        explicit MediaEventListenersImpl( vcl::Window& rNotifyWindow );
        virtual ~MediaEventListenersImpl() override;

        void        cleanUp();

        protected:

            // XKeyListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
            virtual void SAL_CALL keyPressed( const css::awt::KeyEvent& e ) override;
            virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& e ) override;

            // XMouseListener
            virtual void SAL_CALL mousePressed( const css::awt::MouseEvent& e ) override;
            virtual void SAL_CALL mouseReleased( const css::awt::MouseEvent& e ) override;
            virtual void SAL_CALL mouseEntered( const css::awt::MouseEvent& e ) override;
            virtual void SAL_CALL mouseExited( const css::awt::MouseEvent& e ) override;

            // XMouseMotionListener
            virtual void SAL_CALL mouseDragged( const css::awt::MouseEvent& e ) override;
            virtual void SAL_CALL mouseMoved( const css::awt::MouseEvent& e ) override;

            // XFocusListener
            virtual void SAL_CALL focusGained( const css::awt::FocusEvent& e ) override;
            virtual void SAL_CALL focusLost( const css::awt::FocusEvent& e ) override;

        private:

            VclPtr<vcl::Window>     mpNotifyWindow;
            mutable std::mutex      maMutex;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
