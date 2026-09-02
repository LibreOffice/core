/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once


#include <com/sun/star/awt/XPrinterPropertySet.hpp>
#include <com/sun/star/awt/XPrinterServer2.hpp>
#include <com/sun/star/awt/XInfoPrinter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/jobset.hxx>

#include <memory>

namespace com::sun::star::awt { class XPrinter; }
namespace vcl { class OldStylePrintAdaptor; }
class Printer;
class VCLXDevice;

// relevant properties for the printer:
/*
     bool   Horizontal
     sal_uInt16 CopyCount;
     bool   Collate;
     String FormDescriptor;
     sal_uInt16 Orientation;    // PORTRAIT, LANDSCAPE
*/




typedef ::cppu::WeakImplHelper <   css::awt::XPrinterPropertySet
                                >   VCLXPrinterPropertySet_Base;
class VCLXPrinterPropertySet    :public VCLXPrinterPropertySet_Base
                                ,public comphelper::OMutexAndBroadcastHelper
                                ,public ::cppu::OPropertySetHelper
{
protected:
    VclPtr<Printer>             mxPrinter;
    rtl::Reference< VCLXDevice >  mxPrnDevice;

    sal_Int16                   mnOrientation;
    bool                        mbHorizontal;
public:
    VCLXPrinterPropertySet( const OUString& rPrinterName );
    virtual ~VCLXPrinterPropertySet() override;

    Printer*                    GetPrinter() const { return mxPrinter.get(); }
    rtl::Reference< VCLXDevice > const &  GetDevice();

    // css::uno::XInterface
    DECLARE_XINTERFACE();

    // css::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER();

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    void setPropertyValue( const OUString& rPropertyName, const cpo::uno::Any& aValue ) override { OPropertySetHelper::setPropertyValue( rPropertyName, aValue ); }
    cpo::uno::Any getPropertyValue( const OUString& rPropertyName ) override { return OPropertySetHelper::getPropertyValue( rPropertyName ); }
    void addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { OPropertySetHelper::addPropertyChangeListener( rPropertyName, rxListener ); }
    void removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { OPropertySetHelper::removePropertyChangeListener( rPropertyName, rxListener ); }
    void addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { OPropertySetHelper::addVetoableChangeListener( rPropertyName, rxListener ); }
    void removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { OPropertySetHelper::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    bool convertFastPropertyValue( cpo::uno::Any & rConvertedValue, cpo::uno::Any & rOldValue, sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
    void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void getFastPropertyValue( cpo::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // css::awt::XPrinterPropertySet
    void setHorizontal( bool bHorizontal ) override;
    cpo::uno::Sequence< OUString > getFormDescriptions(  ) override;
    void selectForm( const OUString& aFormDescription ) override;
    cpo::uno::Sequence< sal_Int8 > getBinarySetup(  ) override;
    void setBinarySetup( const cpo::uno::Sequence< sal_Int8 >& data ) override;
};




typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   css::awt::XPrinter
                                        >   VCLXPrinter_Base;
class VCLXPrinter final : public VCLXPrinter_Base
{
    std::shared_ptr<vcl::OldStylePrintAdaptor>    mxListener;
    JobSetup                                      maInitJobSetup;
public:
                    VCLXPrinter( const OUString& rPrinterName );
                    virtual ~VCLXPrinter() override;

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void setPropertyValue( const OUString& rPropertyName, const cpo::uno::Any& aValue ) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    cpo::uno::Any getPropertyValue( const OUString& rPropertyName ) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // css::awt::XPrinterPropertySet
    void setHorizontal( bool bHorizontal ) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    cpo::uno::Sequence< OUString > getFormDescriptions(  ) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void selectForm( const OUString& aFormDescription ) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    cpo::uno::Sequence< sal_Int8 > getBinarySetup(  ) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void setBinarySetup( const cpo::uno::Sequence< sal_Int8 >& data ) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // css::awt::XPrinter
    bool start( const OUString& nJobName, sal_Int16 nCopies, bool nCollate ) override;
    void end(  ) override;
    void terminate(  ) override;
    css::uno::Reference< css::awt::XDevice > startPage(  ) override;
    void endPage(  ) override;
};




typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   css::awt::XInfoPrinter
                                        >   VCLXInfoPrinter_Base;
class VCLXInfoPrinter final : public VCLXInfoPrinter_Base
{
public:
                        VCLXInfoPrinter( const OUString& rPrinterName );
                        virtual ~VCLXInfoPrinter() override;

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void setPropertyValue( const OUString& rPropertyName, const cpo::uno::Any& aValue ) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    cpo::uno::Any getPropertyValue( const OUString& rPropertyName ) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // css::awt::XPrinterPropertySet
    void setHorizontal( bool bHorizontal ) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    cpo::uno::Sequence< OUString > getFormDescriptions(  ) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void selectForm( const OUString& aFormDescription ) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    cpo::uno::Sequence< sal_Int8 > getBinarySetup(  ) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void setBinarySetup( const cpo::uno::Sequence< sal_Int8 >& data ) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // css::awt::XInfoPrinter
    css::uno::Reference< css::awt::XDevice > createDevice(  ) override;
};




typedef ::cppu::WeakImplHelper <   css::awt::XPrinterServer2,
                                   css::lang::XServiceInfo
                                >   VCLXPrinterServer_Base;
class VCLXPrinterServer final : public VCLXPrinterServer_Base
{
public:
    // css::awt::XPrinterServer2
    cpo::uno::Sequence< OUString > getPrinterNames(  ) override;
    OUString getDefaultPrinterName() override;
    css::uno::Reference< css::awt::XPrinter > createPrinter( const OUString& printerName ) override;
    css::uno::Reference< css::awt::XInfoPrinter > createInfoPrinter( const OUString& printerName ) override;

    OUString getImplementationName() override;

    bool supportsService(OUString const & ServiceName) override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
