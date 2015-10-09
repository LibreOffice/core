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

#ifndef INCLUDED_TOOLKIT_CONTROLS_ANIMATEDIMAGES_HXX
#define INCLUDED_TOOLKIT_CONTROLS_ANIMATEDIMAGES_HXX

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

namespace toolkit
{


    struct AnimatedImagesControlModel_Data;
    typedef ::cppu::AggImplInheritanceHelper    <   UnoControlModel
                                                ,   css::awt::XAnimatedImages
                                                >   AnimatedImagesControlModel_Base;
    class AnimatedImagesControlModel : public AnimatedImagesControlModel_Base
    {
    public:
                                        AnimatedImagesControlModel( css::uno::Reference< css::uno::XComponentContext > const & i_factory );
                                        AnimatedImagesControlModel( const AnimatedImagesControlModel& i_copySource );

        virtual UnoControlModel*        Clone() const override;

        // XPropertySet
        css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XPersistObject
        OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XAnimatedImages
        virtual ::sal_Int32 SAL_CALL getStepTime() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setStepTime( ::sal_Int32 _steptime ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getAutoRepeat() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setAutoRepeat( sal_Bool _autorepeat ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getScaleMode() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setScaleMode( ::sal_Int16 _scalemode ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getImageSetCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getImageSet( ::sal_Int32 i_index ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL insertImageSet( ::sal_Int32 i_index, const css::uno::Sequence< OUString >& i_imageURLs ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL replaceImageSet( ::sal_Int32 i_index, const css::uno::Sequence< OUString >& i_imageURLs ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeImageSet( ::sal_Int32 i_index ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

        // XAnimatedImages::XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
                                        virtual ~AnimatedImagesControlModel() override;

        css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;
        void SAL_CALL                   setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw (css::uno::Exception, std::exception) override;

    private:
        std::unique_ptr< AnimatedImagesControlModel_Data > m_xData;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_ANIMATEDIMAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
