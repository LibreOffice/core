/*************************************************************************
 *
 *  $RCSfile: datefunc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:45 $
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

//------------------------------------------------------------------
//
// date functions add in
//
//------------------------------------------------------------------

#ifndef DATEFUNC_HXX
#define DATEFUNC_HXX

#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XDateFunctions.hpp>

#include <cppuhelper/implbase4.hxx> // helper for implementations


com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL DateFunctionAddIn_CreateInstance(
    const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& );


class DateFunctionAddIn : public cppu::WeakImplHelper4<
                            com::sun::star::sheet::XAddIn,
                            com::sun::star::sheet::addin::XDateFunctions,
                            com::sun::star::lang::XServiceName,
                            com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::lang::Locale    aFuncLoc;

public:
                            DateFunctionAddIn();
    virtual                 ~DateFunctionAddIn();

    static ::rtl::OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

                            // XAddIn
    virtual ::rtl::OUString SAL_CALL getProgrammaticFuntionName( const ::rtl::OUString& aDisplayName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayFunctionName( const ::rtl::OUString& aProgrammaticName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFunctionDescription( const ::rtl::OUString& aProgrammaticName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayArgumentName( const ::rtl::OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getArgumentDescription( const ::rtl::OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getProgrammaticCategoryName( const ::rtl::OUString& aProgrammaticFunctionName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDisplayCategoryName( const ::rtl::OUString& aProgrammaticFunctionName ) throw(::com::sun::star::uno::RuntimeException);

                            // XLocalizable
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& eLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceName
    virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    //  methods from own interfaces start here

                            // XDateFunctions
    virtual sal_Int32 SAL_CALL getDiffWeeks( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nEndDate, sal_Int32 nStartDate, sal_Int32 nMode ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDiffMonths( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nEndDate, sal_Int32 nStartDate, sal_Int32 nMode ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDiffYears( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nEndDate, sal_Int32 nStartDate, sal_Int32 nMode ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL isLeapYear( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDaysInMonth( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDaysInYear( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getWeeksInYear( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions, sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException);
};

//------------------------------------------------------------------

#endif

