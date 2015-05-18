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
#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX

#include "featuredispatcher.hxx"
#include "rtattributes.hxx"
#include "textattributelistener.hxx"


namespace frm
{

    class OAttributeDispatcher  :public ORichTextFeatureDispatcher
                                ,public ITextAttributeListener
    {
    protected:
        IMultiAttributeDispatcher*          m_pMasterDispatcher;
        AttributeId                         m_nAttributeId;

    public:
        /** ctor
            @param _nAttributeId
                the id of the attribute which this instance is responsible for
            @param _rURL
                the URL of the feature which this instance is responsible for
            @param _pMasterDispatcher
                the dispatcher which can execute the given attribute
            @param _pConverter
                an instance which is able to convert between SfxPoolItems and XDispatch-Parameters
                If not <NULL/>, the parametrized version of IMultiAttributeDispatcher::executeAttribute
                will be used.
        */
        OAttributeDispatcher(
            EditView&                           _rView,
            AttributeId                         _nAttributeId,
            const ::com::sun::star::util::URL&  _rURL,
            IMultiAttributeDispatcher*          _pMasterDispatcher
        );

        /// notifies a new attribute state
        void    notifyAttributeState( const AttributeState& _rState );

    protected:
        virtual ~OAttributeDispatcher( );

    protected:
        // XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ITextAttributeListener
        virtual void    onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& _rState ) SAL_OVERRIDE;

        // ORichTextFeatureDispatcher
        virtual void    disposing( ::osl::ClearableMutexGuard& _rClearBeforeNotify ) SAL_OVERRIDE;

        // ORichTextFeatureDispatcher
        virtual ::com::sun::star::frame::FeatureStateEvent  buildStatusEvent() const SAL_OVERRIDE;

        // own overridables
        virtual void    fillFeatureEventFromAttributeState( ::com::sun::star::frame::FeatureStateEvent& _rEvent, const AttributeState& _rState ) const;

    protected:
        /// notifies our feature state to one particular listener
        void    notifyState(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxControl,
                    const AttributeState& _rState
                );
    };


}


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
