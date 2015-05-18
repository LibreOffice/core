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

#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <tools/wintypes.hxx>
#include "rtattributes.hxx"
#include "attributedispatcher.hxx"

#include <map>


namespace frm
{

    class ORichTextFeatureDispatcher;

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextControl_Base;
    class ORichTextControl  :public UnoEditControl
                            ,public ORichTextControl_Base
    {
    public:
        ORichTextControl();

    protected:
        virtual ~ORichTextControl();

    public:
        // XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ORichTextControl, UnoEditControl )
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XControl
        virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rParent ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // UnoControl
        virtual bool   requiresNewPeer( const OUString& _rPropertyName ) const SAL_OVERRIDE;
    };

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextPeer_Base;
    class ORichTextPeer :public VCLXWindow
                        ,public ORichTextPeer_Base
                        ,public ITextSelectionListener
    {
    private:
        typedef rtl::Reference<ORichTextFeatureDispatcher> SingleAttributeDispatcher;
        typedef ::std::map< SfxSlotId, SingleAttributeDispatcher >                                                      AttributeDispatchers;
        AttributeDispatchers                                                                                            m_aDispatchers;

    public:
        /** factory method
            @return
                a new ORichTextPeer instance, which has been acquired once!
        */
        static ORichTextPeer* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >&         _rxModel,
            vcl::Window* _pParentWindow,
            WinBits _nStyle
        );

        // XInterface
        DECLARE_XINTERFACE( )

    protected:
        ORichTextPeer();
        virtual ~ORichTextPeer();

        // XView
        void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XVclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE;

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        virtual void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ITextSelectionListener
        virtual void    onSelectionChanged( const ESelection& _rSelection ) SAL_OVERRIDE;

    private:
        SingleAttributeDispatcher implCreateDispatcher( SfxSlotId _nSlotId, const ::com::sun::star::util::URL& _rURL );
    };


}


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
