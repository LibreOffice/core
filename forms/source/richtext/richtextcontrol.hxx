/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/awt/vclxwindow.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/frame/XDispatchProvider.hpp>
/** === end UNO includes === **/
#include <comphelper/uno3.hxx>
#include <comphelper/implementationreference.hxx>
#include <cppuhelper/implbase1.hxx>
#include <tools/wintypes.hxx>
#include "rtattributes.hxx"
#include "attributedispatcher.hxx"

#include <map>

//.........................................................................
namespace frm
{
//.........................................................................

    class ORichTextFeatureDispatcher;
    class RichTextControl;

    //==================================================================
    // ORichTextControl
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextControl_Base;
    class ORichTextControl  :public UnoEditControl
                            ,public ORichTextControl_Base
    {
    public:
        ORichTextControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

    protected:
        ~ORichTextControl();

    public:
        // XServiceInfo - static version
        static  ::rtl::OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ORichTextControl, UnoEditControl );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw ( ::com::sun::star::uno::RuntimeException );

        // XControl
        virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rParent ) throw( ::com::sun::star::uno::RuntimeException );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

        // UnoControl
        virtual sal_Bool   requiresNewPeer( const ::rtl::OUString& _rPropertyName ) const;
    };

    //==================================================================
    // ORichTextPeer
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextPeer_Base;
    class ORichTextPeer :public VCLXWindow
                        ,public ORichTextPeer_Base
                        ,public ITextSelectionListener
    {
    private:
        typedef ::comphelper::ImplementationReference< ORichTextFeatureDispatcher, ::com::sun::star::frame::XDispatch > SingleAttributeDispatcher;
        typedef ::std::map< SfxSlotId, SingleAttributeDispatcher >                                                      AttributeDispatchers;
        AttributeDispatchers                                                                                            m_aDispatchers;

    public:
        /** factory method
            @return
                a new ORichTextPeer instance, which has been aquired once!
        */
        static ORichTextPeer* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >&         _rxModel,
            Window* _pParentWindow,
            WinBits _nStyle
        );

        // XInterface
        DECLARE_XINTERFACE( )

    protected:
        ORichTextPeer();
        ~ORichTextPeer();

        // XView
        void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);

        // XVclWindowPeer
        virtual void SAL_CALL setProperty( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        virtual void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

        // ITextSelectionListener
        virtual void    onSelectionChanged( const ESelection& _rSelection );

    private:
        SingleAttributeDispatcher implCreateDispatcher( SfxSlotId _nSlotId, const ::com::sun::star::util::URL& _rURL );
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
