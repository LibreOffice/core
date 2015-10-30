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

#ifndef INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX
#define INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase.hxx>
#include "fmslotinvalidator.hxx"


namespace svx
{

    typedef ::cppu::WeakImplHelper <   css::frame::XStatusListener
                                    >   FmTextControlFeature_Base;

    class FmTextControlFeature : public FmTextControlFeature_Base
    {
    private:
        css::uno::Reference< css::frame::XDispatch >
                                        m_xDispatcher;
        css::util::URL                  m_aFeatureURL;
        css::uno::Any                   m_aFeatureState;
        SfxSlotId                       m_nSlotId;
        ISlotInvalidator*               m_pInvalidator;
        bool                            m_bFeatureEnabled;

    public:
        /** constructs an FmTextControlFeature object
        @param _rxDispatcher
            the dispatcher which the instance should work with
        @param _rFeatureURL
            the URL which the instance should be responsible for
        */
        FmTextControlFeature(
            const css::uno::Reference< css::frame::XDispatch >& _rxDispatcher,
            const css::util::URL& _rFeatureURL,
            SfxSlotId _nId,
            ISlotInvalidator* _pInvalidator
        );

        /// determines whether the feature we're responsible for is currently enabled
        inline  bool                                isFeatureEnabled( ) const { return m_bFeatureEnabled; }
        inline  const css::uno::Any&   getFeatureState( ) const { return m_aFeatureState; }

        /** dispatches the feature URL to the dispatcher
        */
        void    dispatch() const;

        /** dispatches the feature URL to the dispatcher, with passing the given arguments
        */
        void    dispatch( const css::uno::Sequence< css::beans::PropertyValue >& _rArgs ) const;

        /// releases any resources associated with this instance
        void    dispose();

    protected:
        virtual ~FmTextControlFeature();

    protected:
        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& State ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FMTEXTCONTROLFEATURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
