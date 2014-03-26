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

#ifndef SVTOOLS_COMMONPICKER_HXX
#define SVTOOLS_COMMONPICKER_HXX

#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <tools/link.hxx>

class SvtFileDialog;
class Window;


namespace svt
{


    typedef ::cppu::WeakComponentImplHelper5    <   ::com::sun::star::ui::dialogs::XControlAccess
                                                ,   ::com::sun::star::ui::dialogs::XControlInformation
                                                ,   ::com::sun::star::lang::XEventListener
                                                ,   ::com::sun::star::util::XCancellable
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCommonPicker_Base;
    /** implements common functionality for the 2 UNO picker components
    */
    class OCommonPicker
                    :public ::comphelper::OBaseMutex
                    ,public OCommonPicker_Base
                    ,public ::comphelper::OPropertyContainer
                    ,public ::comphelper::OPropertyArrayUsageHelper< OCommonPicker >
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

        // <properties>
        OUString                                                     m_sHelpURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xWindow;
        // </properties>

        SvtFileDialog*      m_pDlg;
        sal_uInt32          m_nCancelEvent;
        sal_Bool            m_bExecuting;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xDialogParent;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  m_xWindowListenerAdapter;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  m_xParentListenerAdapter;

    protected:
        OUString     m_aTitle;
        OUString     m_aDisplayDirectory;

    protected:
        inline  SvtFileDialog*  getDialog() { return m_pDlg; }

        inline const    ::cppu::OBroadcastHelper&   GetBroadcastHelper() const  { return OCommonPicker_Base::rBHelper; }
        inline          ::cppu::OBroadcastHelper&   GetBroadcastHelper()        { return OCommonPicker_Base::rBHelper; }

    public:
        OCommonPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        virtual ~OCommonPicker();

        // overridables

        // will be called with locked SolarMutex
        virtual SvtFileDialog*  implCreateDialog( Window* _pParent ) = 0;
        virtual sal_Int16       implExecutePicker( ) = 0;
            // do NOT override XExecutableDialog::execute! We need to do some stuff there ourself ...

    protected:

        // disambiguate XInterface

        DECLARE_XINTERFACE( )


        // disambiguate XTypeProvider

        DECLARE_XTYPEPROVIDER( )


        // ComponentHelper/XComponent

        virtual void SAL_CALL disposing() SAL_OVERRIDE;


        // XEventListner

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // property set related methods


        // XPropertySet pure methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // OPropertySetHelper pure methods
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        // OPropertyArrayUsageHelper pure methods
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

        // OPropertySetHelper overridden methods
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;


        // XExecutableDialog functions

        virtual void SAL_CALL           setTitle( const OUString& _rTitle ) throw( ::com::sun::star::uno::RuntimeException, std::exception );
        virtual sal_Int16 SAL_CALL      execute() throw( ::com::sun::star::uno::RuntimeException, std::exception );


        // XControlAccess functions

        virtual void SAL_CALL setControlProperty( const OUString& aControlName, const OUString& aControlProperty, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getControlProperty( const OUString& aControlName, const OUString& aControlProperty ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XControlInformation functions

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedControls(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isControlSupported( const OUString& aControlName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedControlProperties( const OUString& aControlName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isControlPropertySupported( const OUString& aControlName, const OUString& aControlProperty ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XCancellable functions

        virtual void SAL_CALL cancel(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


        // XInitialization functions


        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


        // misc

        void checkAlive() const SAL_THROW( (::com::sun::star::lang::DisposedException) );

        void prepareDialog();

    protected:
                sal_Bool    createPicker();

        /** handle a single argument from the XInitialization::initialize method

            @return <TRUE/> if the argument could be handled
        */
        virtual sal_Bool    implHandleInitializationArgument(
                                const OUString& _rName,
                                const ::com::sun::star::uno::Any& _rValue
                            )
                            SAL_THROW( ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) );

    private:
        void        stopWindowListening();

        DECL_LINK( OnCancelPicker, void* );
    };

}   // namespace svt


#endif // SVTOOLS_COMMONPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
