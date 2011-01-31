/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef TOOLKIT_ANIMATEDIMAGES_HXX
#define TOOLKIT_ANIMATEDIMAGES_HXX

#include "toolkit/controls/unocontrolbase.hxx"
#include "toolkit/controls/unocontrolmodel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //=
    //==================================================================================================================
    typedef ::cppu::AggImplInheritanceHelper2   <   UnoControlBase
                                                ,   ::com::sun::star::awt::XAnimation
                                                ,   ::com::sun::star::container::XContainerListener
                                                >   AnimatedImagesControl_Base;

    class AnimatedImagesControl : public AnimatedImagesControl_Base
    {
    public:
                                    AnimatedImagesControl( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory );
        ::rtl::OUString             GetComponentServiceName();

        // XAnimation
        virtual void SAL_CALL startAnimation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stopAnimation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isAnimationRunning(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XControl
        sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& i_rModel ) throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& i_toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& i_parentPeer ) throw(::com::sun::star::uno::RuntimeException);


        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& i_event ) throw (::com::sun::star::uno::RuntimeException);
    };

    //==================================================================================================================
    //= AnimatedImagesControlModel
    //==================================================================================================================
    struct AnimatedImagesControlModel_Data;
    typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                                ,   ::com::sun::star::awt::XAnimatedImages
                                                >   AnimatedImagesControlModel_Base;
    class AnimatedImagesControlModel : public AnimatedImagesControlModel_Base
    {
    public:
                                        AnimatedImagesControlModel( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory );
                                        AnimatedImagesControlModel( const AnimatedImagesControlModel& i_copySource );

        virtual UnoControlModel*        Clone() const;

        // XPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XAnimatedImages
        virtual ::sal_Int32 SAL_CALL getStepTime() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setStepTime( ::sal_Int32 _steptime ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getAutoRepeat() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setAutoRepeat( ::sal_Bool _autorepeat ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getScaleMode() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setScaleMode( ::sal_Int16 _scalemode ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getImageSetCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getImageSet( ::sal_Int32 i_index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertImageSet( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& i_imageURLs ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL replaceImageSet( ::sal_Int32 i_index, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& i_imageURLs ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeImageSet( ::sal_Int32 i_index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAnimatedImages::XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& i_listener ) throw (::com::sun::star::uno::RuntimeException);

    protected:
                                        ~AnimatedImagesControlModel();

        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();
        void SAL_CALL                   setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);

    private:
        ::boost::scoped_ptr< AnimatedImagesControlModel_Data >
                m_pData;
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // TOOLKIT_ANIMATEDIMAGES_HXX
