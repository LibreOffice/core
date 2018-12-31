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
#include <com/sun/star/awt/XPrinterServer.hpp>
#include <com/sun/star/awt/XInfoPrinter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/jobset.hxx>

namespace com { namespace sun { namespace star { namespace awt { class XPrinter; } } } }
namespace vcl { class OldStylePrintAdaptor; }
class Printer;

// relevant properties for the printer:
/*
     sal_Bool   Horizontal
     sal_uInt16 CopyCount;
     sal_Bool   Collate;
     String FormDescriptor;
     sal_uInt16 Orientation;    // PORTRAIT, LANDSCAPE
*/


//  class VCLXPrinterPropertySet


typedef ::cppu::WeakImplHelper <   css::awt::XPrinterPropertySet
                                >   VCLXPrinterPropertySet_Base;
class VCLXPrinterPropertySet    :public VCLXPrinterPropertySet_Base
                                ,public MutexAndBroadcastHelper
                                ,public ::cppu::OPropertySetHelper
{
protected:
    VclPtr<Printer>             mxPrinter;
    css::uno::Reference< css::awt::XDevice >  mxPrnDevice;

    sal_Int16                   mnOrientation;
    bool                        mbHorizontal;
public:
    VCLXPrinterPropertySet( const OUString& rPrinterName );
    virtual ~VCLXPrinterPropertySet() override;

    Printer*                    GetPrinter() const { return mxPrinter.get(); }
    css::uno::Reference< css::awt::XDevice > const &  GetDevice();

    // css::uno::XInterface
    DECLARE_XINTERFACE();

    // css::lang::XTypeProvider
    DECLARE_XTYPEPROVIDER();

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const css::uno::Any& aValue ) override { OPropertySetHelper::setPropertyValue( rPropertyName, aValue ); }
    css::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) override { return OPropertySetHelper::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { OPropertySetHelper::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { OPropertySetHelper::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { OPropertySetHelper::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { OPropertySetHelper::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // css::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) override;
    css::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) override;
    void SAL_CALL selectForm( const OUString& aFormDescription ) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) override;
    void SAL_CALL setBinarySetup( const css::uno::Sequence< sal_Int8 >& data ) override;
};


//  class VCLXPrinter


typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   css::awt::XPrinter
                                        >   VCLXPrinter_Base;
class VCLXPrinter:  public VCLXPrinter_Base
{
    std::shared_ptr<vcl::OldStylePrintAdaptor>    mxListener;
    JobSetup                                      maInitJobSetup;
public:
                    VCLXPrinter( const OUString& rPrinterName );
                    virtual ~VCLXPrinter() override;

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const css::uno::Any& aValue ) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    css::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // css::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    css::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const OUString& aFormDescription ) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    css::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const css::uno::Sequence< sal_Int8 >& data ) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // css::awt::XPrinter
    sal_Bool SAL_CALL start( const OUString& nJobName, sal_Int16 nCopies, sal_Bool nCollate ) override;
    void SAL_CALL end(  ) override;
    void SAL_CALL terminate(  ) override;
    css::uno::Reference< css::awt::XDevice > SAL_CALL startPage(  ) override;
    void SAL_CALL endPage(  ) override;
};


//  class VCLXInfoPrinter


typedef ::cppu::ImplInheritanceHelper  <   VCLXPrinterPropertySet
                                        ,   css::awt::XInfoPrinter
                                        >   VCLXInfoPrinter_Base;
class VCLXInfoPrinter:  public VCLXInfoPrinter_Base
{
public:
                        VCLXInfoPrinter( const OUString& rPrinterName );
                        virtual ~VCLXInfoPrinter() override;

    // css::beans::XPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const OUString& rPropertyName, const css::uno::Any& aValue ) override { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    css::uno::Any SAL_CALL getPropertyValue( const OUString& rPropertyName ) override { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const OUString& rPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // css::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) override { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    css::uno::Sequence< OUString > SAL_CALL getFormDescriptions(  ) override { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const OUString& aFormDescription ) override { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    css::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) override { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const css::uno::Sequence< sal_Int8 >& data ) override { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // css::awt::XInfoPrinter
    css::uno::Reference< css::awt::XDevice > SAL_CALL createDevice(  ) override;
};


//  class VCLXPrinterServer


typedef ::cppu::WeakImplHelper <   css::awt::XPrinterServer,
                                   css::lang::XServiceInfo
                                >   VCLXPrinterServer_Base;
class VCLXPrinterServer : public VCLXPrinterServer_Base
{
public:
    // css::awt::XPrinterServer
    css::uno::Sequence< OUString > SAL_CALL getPrinterNames(  ) override;
    css::uno::Reference< css::awt::XPrinter > SAL_CALL createPrinter( const OUString& printerName ) override;
    css::uno::Reference< css::awt::XInfoPrinter > SAL_CALL createInfoPrinter( const OUString& printerName ) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXPRINTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
