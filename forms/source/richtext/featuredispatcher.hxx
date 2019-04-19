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

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>

class EditView;

namespace frm
{

    typedef ::cppu::WeakImplHelper <   css::frame::XDispatch
                                    >   ORichTextFeatureDispatcher_Base;

    class ORichTextFeatureDispatcher    :public ::cppu::BaseMutex
                                        ,public ORichTextFeatureDispatcher_Base
    {
    private:
        css::util::URL                      m_aFeatureURL;
        ::comphelper::OInterfaceContainerHelper2  m_aStatusListeners;
        EditView*                           m_pEditView;
        bool                                m_bDisposed;

    protected:
              EditView*   getEditView()       { return m_pEditView; }
        const EditView*   getEditView() const { return m_pEditView; }

    protected:
        const css::util::URL&       getFeatureURL() const { return m_aFeatureURL; }
        ::comphelper::OInterfaceContainerHelper2& getStatusListeners() { return m_aStatusListeners; }
        bool                               isDisposed() const { return m_bDisposed; }
        void                               checkDisposed() const { if ( isDisposed() ) throw css::lang::DisposedException(); }

    protected:
        ORichTextFeatureDispatcher( EditView& _rView, const css::util::URL&  _rURL );
        virtual ~ORichTextFeatureDispatcher( ) override;

    public:
        /// clean up resources associated with this instance
        void    dispose();

        // invalidate the feature, re-retrieve it's state, and broadcast changes, if necessary
        void    invalidate();

    protected:
        // overridables
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify );
        virtual void    invalidateFeatureState_Broadcast();

        // to be overridden, and filled with the info special do your derived class
        virtual css::frame::FeatureStateEvent
                        buildStatusEvent() const;

        static void     doNotify(
                    const css::uno::Reference< css::frame::XStatusListener >& _rxListener,
                    const css::frame::FeatureStateEvent& _rEvent
                );

        // XDispatch
        virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxControl, const css::util::URL& _rURL ) override;
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& _rxControl, const css::util::URL& _rURL ) override;
    };


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
