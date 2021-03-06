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

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <tools/wintypes.hxx>
#include "rtattributes.hxx"
#include "textattributelistener.hxx"

#include <map>


namespace frm
{

    class ORichTextFeatureDispatcher;

    typedef ::cppu::ImplHelper1 <   css::frame::XDispatchProvider
                                >   ORichTextControl_Base;
    class ORichTextControl  :public UnoEditControl
                            ,public ORichTextControl_Base
    {
    public:
        ORichTextControl();

    protected:
        virtual ~ORichTextControl() override;

        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ORichTextControl, UnoEditControl )
        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

        // XControl
        virtual void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& _rParent ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& _rURL, const OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& Requests ) override;

        // UnoControl
        virtual bool   requiresNewPeer( const OUString& _rPropertyName ) const override;
    };

    typedef ::cppu::ImplHelper1 <   css::frame::XDispatchProvider
                                >   ORichTextPeer_Base;
    class ORichTextPeer final :public VCLXWindow
                        ,public ORichTextPeer_Base
                        ,public ITextSelectionListener
    {
    private:
        typedef rtl::Reference<ORichTextFeatureDispatcher> SingleAttributeDispatcher;
        typedef ::std::map< SfxSlotId, SingleAttributeDispatcher >                                                      AttributeDispatchers;
        AttributeDispatchers                                                                                            m_aDispatchers;

    public:
        /** factory method
        */
        static rtl::Reference<ORichTextPeer> Create(
            const css::uno::Reference< css::awt::XControlModel >&         _rxModel,
            vcl::Window* _pParentWindow,
            WinBits _nStyle
        );

        // XInterface
        DECLARE_XINTERFACE( )

    private:
        ORichTextPeer();
        virtual ~ORichTextPeer() override;

        // XView
        void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) override;

        // XVclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& _rPropertyName, const css::uno::Any& _rValue ) override;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        virtual void SAL_CALL dispose( ) override;

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& _rURL, const OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& Requests ) override;

        // ITextSelectionListener
        virtual void    onSelectionChanged() override;

    private:
        SingleAttributeDispatcher implCreateDispatcher( SfxSlotId _nSlotId, const css::util::URL& _rURL );
    };


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
