/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxprinter.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _TOOLKIT_AWT_VCLXPRINTER_HXX_
#define _TOOLKIT_AWT_VCLXPRINTER_HXX_


#include <com/sun/star/awt/XPrinterPropertySet.hpp>
#include <com/sun/star/awt/XPrinter.hpp>
#include <com/sun/star/awt/XPrinterServer.hpp>
#include <com/sun/star/awt/XInfoPrinter.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <toolkit/helper/mutexandbroadcasthelper.hxx>
#include <cppuhelper/propshlp.hxx>

class Printer;
class String;


// Fuer den Drucker relevante Properties:
/*
     sal_Bool   Horizontal
     sal_uInt16 CopyCount;
     sal_Bool   Collate;
     String FormDescriptor;
     sal_uInt16 Orientation;    // PORTRAIT, LANDSCAPE
*/

//  ----------------------------------------------------
//  class VCLXPrinterPropertySet
//  ----------------------------------------------------

class VCLXPrinterPropertySet :  public ::com::sun::star::awt::XPrinterPropertySet,
                                public ::com::sun::star::lang::XTypeProvider,
                                public MutexAndBroadcastHelper,
                                public ::cppu::OPropertySetHelper
{
private:
    Printer*                    mpPrinter;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  mxPrnDevice;

    sal_Int16                   mnOrientation;
    sal_Bool                    mbHorizontal;

protected:

public:
    VCLXPrinterPropertySet( const String& rPrinterName );
    virtual ~VCLXPrinterPropertySet();

    Printer*                    GetPrinter() const { return mpPrinter; }
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  GetDevice();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { OPropertySetHelper::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { return OPropertySetHelper::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { OPropertySetHelper::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { OPropertySetHelper::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { OPropertySetHelper::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { OPropertySetHelper::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::cppu::OPropertySetHelper
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);
    using cppu::OPropertySetHelper::getFastPropertyValue;
    void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL selectForm( const ::rtl::OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXPrinter
//  ----------------------------------------------------

class VCLXPrinter:  public ::com::sun::star::awt::XPrinter,
                    public VCLXPrinterPropertySet,
                    public ::cppu::OWeakObject
{
public:
                    VCLXPrinter( const String& rPrinterName );
                    ~VCLXPrinter();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const ::rtl::OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // ::com::sun::star::awt::XPrinter
    sal_Bool SAL_CALL start( const ::rtl::OUString& nJobName, sal_Int16 nCopies, sal_Bool nCollate ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL end(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL terminate(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > SAL_CALL startPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL endPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXInfoPrinter
//  ----------------------------------------------------

class VCLXInfoPrinter:  public ::com::sun::star::awt::XInfoPrinter,
                        public VCLXPrinterPropertySet,
                        public ::cppu::OWeakObject
{
public:
                        VCLXInfoPrinter( const String& rPrinterName );
                        ~VCLXInfoPrinter();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getPropertySetInfo(); }
    void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setPropertyValue( rPropertyName, aValue ); }
    ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getPropertyValue( rPropertyName ); }
    void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::addPropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::removePropertyChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::addVetoableChangeListener( rPropertyName, rxListener ); }
    void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& rxListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::removeVetoableChangeListener( rPropertyName, rxListener ); }

    // ::com::sun::star::awt::XPrinterPropertySet
    void SAL_CALL setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setHorizontal( bHorizontal ); }
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getFormDescriptions(); }
    void SAL_CALL selectForm( const ::rtl::OUString& aFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::selectForm( aFormDescription ); }
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException) { return VCLXPrinterPropertySet::getBinarySetup(); }
    void SAL_CALL setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) { VCLXPrinterPropertySet::setBinarySetup( data ); }

    // ::com::sun::star::awt::XInfoPrinter
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > SAL_CALL createDevice(  ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXPrinterServer
//  ----------------------------------------------------

class VCLXPrinterServer :   public ::com::sun::star::awt::XPrinterServer,
                            public  ::cppu::OWeakObject
{
public:

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XPrinterServer
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getPrinterNames(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > SAL_CALL createPrinter( const ::rtl::OUString& printerName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > SAL_CALL createInfoPrinter( const ::rtl::OUString& printerName ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_AWT_VCLXPRINTER_HXX_

