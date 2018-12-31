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

#ifndef INCLUDED_TOOLKIT_AWT_ANIMATEDIMAGESPEER_HXX
#define INCLUDED_TOOLKIT_AWT_ANIMATEDIMAGESPEER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/container/XContainerListener.hpp>

#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace toolkit
{


    //= AnimatedImagesPeer

    struct AnimatedImagesPeer_Data;
    typedef cppu::ImplInheritanceHelper< VCLXWindow,
                                         css::awt::XAnimation,
                                         css::container::XContainerListener,
                                         css::util::XModifyListener
                                       > AnimatedImagesPeer_Base;

    class AnimatedImagesPeer    :public AnimatedImagesPeer_Base
    {
    public:
        AnimatedImagesPeer();

    protected:
        virtual ~AnimatedImagesPeer() override;

    public:
        // XAnimation
        virtual void SAL_CALL startAnimation(  ) override;
        virtual void SAL_CALL stopAnimation(  ) override;
        virtual sal_Bool SAL_CALL isAnimationRunning(  ) override;

        // VclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
        virtual css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& i_event ) override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& i_event ) override;

        // XComponent
        void SAL_CALL dispose(  ) override;

    protected:
        void ProcessWindowEvent( const VclWindowEvent& i_windowEvent ) override;

    private:
        /** updates our images with the ones from the given XAnimatedImages component
        */
        void    impl_updateImages_nolck( const css::uno::Reference< css::uno::XInterface >& i_animatedImages );

    private:
        AnimatedImagesPeer(const AnimatedImagesPeer&) = delete;
        AnimatedImagesPeer& operator=(const AnimatedImagesPeer&) = delete;

        std::unique_ptr< AnimatedImagesPeer_Data >   m_xData;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_AWT_ANIMATEDIMAGESPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
