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

#ifndef SC_ADDIN_HXX
#define SC_ADDIN_HXX

#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <stardiv/starcalc/test/XTestAddIn.hpp>

#include <cppuhelper/implbase4.hxx> // helper for implementations


com::sun::star::uno::Reference<com::sun::star::uno::XInterface> ScTestAddIn_CreateInstance(
    const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& );


class ScTestAddIn : public cppu::WeakImplHelper4<
                        com::sun::star::sheet::XAddIn,
                        stardiv::starcalc::test::XTestAddIn,
                        com::sun::star::lang::XServiceName,
                        com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult>  xAlphaResult;   //! Test
    com::sun::star::uno::Reference<com::sun::star::sheet::XVolatileResult>  xNumResult;     //! Test
    com::sun::star::lang::Locale                                            aFuncLoc;

public:
                            ScTestAddIn();
    virtual                 ~ScTestAddIn();

//                          SMART_UNO_DECLARATION( ScTestAddIn, UsrObject );
//  friend Reflection *     ScTestAddIn_getReflection();
//  virtual BOOL            queryInterface( Uik, XInterfaceRef& );
//  virtual XIdlClassRef    getIdlClass(void);

    static UString          getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static();

                            // XAddIn
    virtual OUString SAL_CALL getProgrammaticFuntionName( const OUString& aDisplayName ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayFunctionName( const OUString& aProgrammaticName ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getFunctionDescription( const OUString& aProgrammaticName ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayArgumentName( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getArgumentDescription( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getProgrammaticCategoryName( const OUString& aProgrammaticFunctionName ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayCategoryName( const OUString& aProgrammaticFunctionName ) throw(::com::sun::star::uno::RuntimeException);

                            // XLocalizable
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& eLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XTestAddIn
    virtual sal_Int32 SAL_CALL countParams( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL addOne( double fValue ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL repeatStr( const OUString& aStr, sal_Int32 nCount ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getDateString( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCaller, double fValue ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColorValue( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >& xRange ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL transpose( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aMatrix ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > > SAL_CALL transposeInt( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& aMatrix ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XVolatileResult > SAL_CALL callAsync( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL repeatMultiple( sal_Int32 nCount, const ::com::sun::star::uno::Any& aFirst, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aFollow ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getStrOrVal( sal_Int32 nFlag ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceName
    virtual OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
