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

#ifndef INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_TRANSLATION_UNODIALOG_HXX
#define INCLUDED_SVX_SOURCE_UNODIALOGS_TEXTCONVERSIONDLGS_CHINESE_TRANSLATION_UNODIALOG_HXX

#include <cppuhelper/component.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/vclptr.hxx>


namespace textconversiondlgs
{



/** This class provides the chinese translation dialog as an uno component.

It can be created via lang::XMultiComponentFactory::createInstanceWithContext
with servicename "com.sun.star.linguistic2.ChineseTranslationDialog"
or implementation name "com.sun.star.comp.linguistic2.ChineseTranslationDialog"

It can be initialized via the XInitialization interface with the following single parameter:
PropertyValue-Parameter: Name="ParentWindow" Type="awt::XWindow".

It can be executed via the ui::dialogs::XExecutableDialog interface.

Made settings can be retrieved via beans::XPropertySet interface.
Following properties are available (read only and not bound):
1) Name="IsDirectionToSimplified" Type="sal_Bool"
2) Name="IsUseCharacterVariants" Type="sal_Bool"
3) Name="IsTranslateCommonTerms" Type="sal_Bool"

The dialog gets this information from the registry on execute and writes it back to the registry if ended with OK.
*/

class ChineseTranslationDialog;

class ChineseTranslation_UnoDialog : public ::cppu::WeakImplHelper <
                              ::com::sun::star::ui::dialogs::XExecutableDialog
                            , ::com::sun::star::lang::XInitialization
                            , ::com::sun::star::beans::XPropertySet
                            , ::com::sun::star::lang::XComponent
                            , ::com::sun::star::lang::XServiceInfo
                            >
                            //  ,public ::com::sun::star::uno::XWeak            // implemented by WeakImplHelper(optional interface)
                            //  ,public ::com::sun::star::uno::XInterface       // implemented by WeakImplHelper(optional interface)
                            //  ,public ::com::sun::star::lang::XTypeProvider   // implemented by WeakImplHelper
{
public:
    explicit ChineseTranslation_UnoDialog( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~ChineseTranslation_UnoDialog();

    // lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext) throw(::com::sun::star::uno::Exception)
    {
        return static_cast<cppu::OWeakObject *>(new ChineseTranslation_UnoDialog( xContext ));
    }

    // lang::XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ui::dialogs::XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // lang::XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    //no default constructor
    ChineseTranslation_UnoDialog();

    void impl_DeleteDialog();

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext>    m_xCC;
    com::sun::star::uno::Reference<
        com::sun::star::awt::XWindow >               m_xParentWindow;

    VclPtr<ChineseTranslationDialog>     m_pDialog;

    bool m_bDisposed; ///Dispose call ready.
    bool m_bInDispose;///In dispose call
    osl::Mutex                      m_aContainerMutex;
    cppu::OInterfaceContainerHelper m_aDisposeEventListeners;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
