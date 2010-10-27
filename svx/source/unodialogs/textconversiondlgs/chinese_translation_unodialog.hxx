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

#ifndef _TEXTCONVERSIONDLGS_CHINESE_TRANSLATION_UNODIALOG_HXX
#define _TEXTCONVERSIONDLGS_CHINESE_TRANSLATION_UNODIALOG_HXX

#include <cppuhelper/component.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase5.hxx>

//.............................................................................
namespace textconversiondlgs
{
//.............................................................................

//-------------------------------------------------------------------------
/** This class provides the chinese translation dialog as an uno component.

It can be created via lang::XMultiComponentFactory::createInstanceWithContext
with servicename "com.sun.star.linguistic2.ChineseTranslationDialog"
or implemenation name "com.sun.star.comp.linguistic2.ChineseTranslationDialog"

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

class ChineseTranslation_UnoDialog : public ::cppu::WeakImplHelper5 <
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
    ChineseTranslation_UnoDialog( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~ChineseTranslation_UnoDialog();

    // lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    static ::rtl::OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext) throw(::com::sun::star::uno::Exception)
    {
        return (::cppu::OWeakObject *)new ChineseTranslation_UnoDialog( xContext );
    }

    // lang::XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // ui::dialogs::XExecutableDialog
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);

    // beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // lang::XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener ) throw (::com::sun::star::uno::RuntimeException);

private:
    //no default constructor
    ChineseTranslation_UnoDialog();

    void impl_DeleteDialog();

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext>    m_xCC;
    com::sun::star::uno::Reference<
        com::sun::star::awt::XWindow >               m_xParentWindow;

    ChineseTranslationDialog*     m_pDialog;

    sal_Bool m_bDisposed; ///Dispose call ready.
    sal_Bool m_bInDispose;///In dispose call
    osl::Mutex                      m_aContainerMutex;
    cppu::OInterfaceContainerHelper m_aDisposeEventListeners;
};

//.............................................................................
} //end namespace
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
