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
#include <cppuhelper/implbase1.hxx>
#include <memory>

namespace toolkit
{


    struct AnimatedImagesControlModel_Data;
    typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                                ,   ::com::sun::star::awt::XAnimatedImages
                                                >   AnimatedImagesControlModel_Base;
    class AnimatedImagesControlModel : public AnimatedImagesControlModel_Base
    {
    public:
                                        AnimatedImagesControlModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & i_factory );
                                        AnimatedImagesControlModel( const AnimatedImagesControlModel& i_copySource );

        virtual UnoControlModel*        Clone() const override;

        // XPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XPersistObject
        OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAnimatedImages
        virtual ::sal_Int32 SAL_CALL getStepTime() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setStepTime( ::sal_Int32 _steptime ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getAutoRepeat() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setAutoRepeat( sal_Bool _autorepeat ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getScaleMode() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setScaleMode( ::sal_Int16 _scalemode ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getImageSetCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getImageSet( ::sal_Int32 i_index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL insertImageSet( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< OUString >& i_imageURLs ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL replaceImageSet( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< OUString >& i_imageURLs ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeImageSet( ::sal_Int32 i_index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAnimatedImages::XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
                                        virtual ~AnimatedImagesControlModel();

        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;
        void SAL_CALL                   setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) override;

    private:
        std::unique_ptr< AnimatedImagesControlModel_Data > m_xData;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_ANIMATEDIMAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
