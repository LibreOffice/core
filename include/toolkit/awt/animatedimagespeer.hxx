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

#ifndef TOOLKIT_ANIMATEDIMAGEPEER_HXX
#define TOOLKIT_ANIMATEDIMAGEPEER_HXX

#include "toolkit/awt/vclxwindow.hxx"

#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <cppuhelper/implbase3.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= AnimatedImagesPeer
    //==================================================================================================================
    struct AnimatedImagesPeer_Data;
    typedef ::cppu::ImplInheritanceHelper3  <   VCLXWindow
                                            ,   ::com::sun::star::awt::XAnimation
                                            ,   ::com::sun::star::container::XContainerListener
                                            ,   ::com::sun::star::util::XModifyListener
                                            >   AnimatedImagesPeer_Base;

    class AnimatedImagesPeer    :public AnimatedImagesPeer_Base
                                ,public ::boost::noncopyable
    {
    public:
        AnimatedImagesPeer();

    protected:
        ~AnimatedImagesPeer();

    public:
        // XAnimation
        virtual void SAL_CALL startAnimation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stopAnimation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isAnimationRunning(  ) throw (::com::sun::star::uno::RuntimeException);

        // VclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& i_event ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& i_event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        void ProcessWindowEvent( const VclWindowEvent& i_windowEvent );

    private:
        /** updates our images with the ones from the givem XAnimatedImages component
        */
        void    impl_updateImages_nolck( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_animatedImages );

    private:
        ::boost::scoped_ptr< AnimatedImagesPeer_Data >   m_pData;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_ANIMATEDIMAGEPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
