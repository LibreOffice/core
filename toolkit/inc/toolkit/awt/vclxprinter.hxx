/*************************************************************************
 *
 *  $RCSfile: vclxprinter.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_VCLXPRINTER_HXX_
#define _TOOLKIT_AWT_VCLXPRINTER_HXX_


#ifndef _COM_SUN_STAR_AWT_XPRINTERPROPERTYSET_HPP_
#include <com/sun/star/awt/XPrinterPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPRINTER_HPP_
#include <com/sun/star/awt/XPrinter.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPRINTERSERVER_HPP_
#include <com/sun/star/awt/XPrinterServer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XINFOPRINTER_HPP_
#include <com/sun/star/awt/XInfoPrinter.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
    ~VCLXPrinterPropertySet();

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
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

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
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

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
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XPrinterServer
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getPrinterNames(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > SAL_CALL createPrinter( const ::rtl::OUString& printerName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > SAL_CALL createInfoPrinter( const ::rtl::OUString& printerName ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_AWT_VCLXPRINTER_HXX_

