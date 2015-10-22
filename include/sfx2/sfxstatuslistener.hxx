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

#ifndef INCLUDED_SFX2_SFXSTATUSLISTENER_HXX
#define INCLUDED_SFX2_SFXSTATUSLISTENER_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/poolitem.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/conditn.hxx>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <cppuhelper/implbase2.hxx>

class SFX2_DLLPUBLIC SfxStatusListener : public ::cppu::WeakImplHelper2<
                          css::frame::XStatusListener,
                          css::lang::XComponent>
{
    public:

        SfxStatusListener( const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& aCommand );
        virtual ~SfxStatusListener();

        // old methods from SfxControllerItem
        sal_uInt16  GetId() const { return m_nSlotID; }
        void    UnBind();
        void    ReBind();

        virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

        // XComponent
        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;

    private:
        SfxStatusListener( const SfxStatusListener& ) = delete;
        SfxStatusListener& operator=( const SfxStatusListener& ) = delete;

        sal_uInt16                                            m_nSlotID;
        css::util::URL                                        m_aCommand;
        css::uno::Reference< css::frame::XDispatchProvider >  m_xDispatchProvider;
        css::uno::Reference< css::frame::XDispatch >          m_xDispatch;
};

#endif // INCLUDED_SFX2_SFXSTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
