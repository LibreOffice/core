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

#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

class EditView;

namespace frm
{



    //= ORichTextFeatureDispatcher

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::frame::XDispatch
                                    >   ORichTextFeatureDispatcher_Base;

    class ORichTextFeatureDispatcher    :public ::comphelper::OBaseMutex
                                        ,public ORichTextFeatureDispatcher_Base
    {
    private:
        ::com::sun::star::util::URL         m_aFeatureURL;
        ::cppu::OInterfaceContainerHelper   m_aStatusListeners;
        EditView*                           m_pEditView;
        bool                                m_bDisposed;

    protected:
              EditView*   getEditView()       { return m_pEditView; }
        const EditView*   getEditView() const { return m_pEditView; }

    protected:
        inline const ::com::sun::star::util::URL&       getFeatureURL() const { return m_aFeatureURL; }
        inline       ::cppu::OInterfaceContainerHelper& getStatusListeners() { return m_aStatusListeners; }
        inline       bool                               isDisposed() const { return m_bDisposed; }
        inline       void                               checkDisposed() const { if ( isDisposed() ) throw ::com::sun::star::lang::DisposedException(); }

    protected:
        ORichTextFeatureDispatcher( EditView& _rView, const ::com::sun::star::util::URL&  _rURL );
        virtual ~ORichTextFeatureDispatcher( );

    public:
        /// clean up resources associated with this instance
        void    dispose();

        // invalidate the feature, re-retrieve it's state, and broadcast changes, if necessary
        void    invalidate();

    protected:
        // overridables
        virtual void    newStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener );
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify );
        virtual void    invalidateFeatureState_Broadcast();

        // to be overridden, and filled with the info special do your derived class
        virtual ::com::sun::star::frame::FeatureStateEvent
                        buildStatusEvent() const;

        void    doNotify(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener,
                    const ::com::sun::star::frame::FeatureStateEvent& _rEvent
                ) const SAL_THROW(());

    protected:
        // XDispatch
        virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_FEATUREDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
