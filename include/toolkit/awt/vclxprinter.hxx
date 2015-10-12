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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXPRINTER_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXPRINTER_HXX


#include <com/sun/star/awt/XPrinterPropertySet.hpp>
#include <com/sun/star/awt/XPrinter.hpp>
#include <com/sun/star/awt/XPrinterServer.hpp>
#include <com/sun/star/awt/XInfoPrinter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include <vcl/oldprintadaptor.hxx>

// relevant properties for the printer:
/*
     sal_Bool   Horizontal
     sal_uInt16 CopyCount;
     sal_Bool   Collate;
     String FormDescriptor;
     sal_uInt16 Orientation;    // PORTRAIT, LANDSCAPE
*/


//  class VCLXPrinterPropertySet


typedef ::cppu::WeakImplHelper <   ::com::sun::star::awt::XPrinterPropertySet
                                >   VCLXPrinterPropertySet_Base;
class VCLXPrinterPropertySet    :public VCLXPrinterPropertySet_Base
                                ,public MutexAndBroadcastHelper
                                ,public ::cppu::OPropertySetHelper
{
protected:
    VclPtr<Printer>             mxPrinter;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  mxPrnDevice;

    sal_Int16                   mnOrientation;
    bool                        mbHorizontal;
public:
    VCLXPrinterPropertySet( const OUString& rPrinterName );
    virtual ~VCLXPrinterPropertySet();

    Printer*                    GetPrinter() const { return mxPrinter.get(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  GetDevice();

    // ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE();

    // ::com::sun::star::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER();

    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { OPropertySetHelper::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { return OPropertySetHelper::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { OPropertySetHelper::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { OPropertySetHelper::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { OPropertySetHelper::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { OPropertySetHelper::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException) override;
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) override;
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectForm( const OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};


//  class VCLXPrinter


typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   ::com::sun::star::awt::XPrinter
                                        >   VCLXPrinter_Base;
class VCLXPrinter:  public VCLXPrinter_Base
{
    std::shared_ptr<vcl::OldStylePrintAdaptor>    mxListener;
    JobSetup                                      maInitJobSetup;
public:
                    VCLXPrinter( const OUString& rPrinterName );
                    virtual ~VCLXPrinter();

    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // ::com::sun::star::awt::XPrinter
    sal_Bool SAL_CALL start( const OUString& nJobName, sal_Int16 nCopies, sal_Bool nCollate ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL end(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL terminate(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > SAL_CALL startPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL endPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};


//  class VCLXInfoPrinter


typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   ::com::sun::star::awt::XInfoPrinter
                                        >   VCLXInfoPrinter_Base;
class VCLXInfoPrinter:  public VCLXInfoPrinter_Base
{
public:
                        VCLXInfoPrinter( const OUString& rPrinterName );
                        virtual ~VCLXInfoPrinter();

    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // ::com::sun::star::awt::XInfoPrinter
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > SAL_CALL createDevice(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};


//  class VCLXPrinterServer


typedef ::cppu::WeakImplHelper <   ::com::sun::star::awt::XPrinterServer,
                                   css::lang::XServiceInfo
                                >   VCLXPrinterServer_Base;
class VCLXPrinterServer : public VCLXPrinterServer_Base
{
public:
    // ::com::sun::star::awt::XPrinterServer
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getPrinterNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > SAL_CALL createPrinter( const OUString& printerName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > SAL_CALL createInfoPrinter( const OUString& printerName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXPRINTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
