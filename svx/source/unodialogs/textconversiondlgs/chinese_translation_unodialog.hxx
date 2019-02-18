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
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
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
                              css::ui::dialogs::XExecutableDialog
                            , css::lang::XInitialization
                            , css::beans::XPropertySet
                            , css::lang::XComponent
                            , css::lang::XServiceInfo
                            >
{
public:
    ChineseTranslation_UnoDialog();
    virtual ~ChineseTranslation_UnoDialog() override;

    // lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// @throws css::uno::Exception
    static css::uno::Reference< css::uno::XInterface >
            create( css::uno::Reference< css::uno::XComponentContext > const & )
    {
        return static_cast<cppu::OWeakObject *>( new ChineseTranslation_UnoDialog );
    }

    // lang::XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // ui::dialogs::XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
    virtual sal_Int16 SAL_CALL execute(  ) override;

    // beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // lang::XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;

private:

    void impl_DeleteDialog();

private:
    css::uno::Reference<
        css::awt::XWindow >              m_xParentWindow;

    std::unique_ptr<ChineseTranslationDialog> m_xDialog;

    bool m_bDisposed; ///Dispose call ready.
    bool m_bInDispose;///In dispose call
    osl::Mutex                      m_aContainerMutex;
    comphelper::OInterfaceContainerHelper2 m_aDisposeEventListeners;
};


} //end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
