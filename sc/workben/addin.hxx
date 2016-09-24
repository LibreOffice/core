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

#ifndef INCLUDED_SC_WORKBEN_ADDIN_HXX
#define INCLUDED_SC_WORKBEN_ADDIN_HXX

#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <stardiv/starcalc/test/XTestAddIn.hpp>

#include <cppuhelper/implbase.hxx>

css::uno::Reference<css::uno::XInterface> ScTestAddIn_CreateInstance(
    const css::uno::Reference<css::lang::XMultiServiceFactory>& );

class ScTestAddIn : public cppu::WeakImplHelper<
                        css::sheet::XAddIn,
                        stardiv::starcalc::test::XTestAddIn,
                        css::lang::XServiceName,
                        css::lang::XServiceInfo >
{
private:
    css::uno::Reference<css::sheet::XVolatileResult>  xAlphaResult;   //! Test
    css::uno::Reference<css::sheet::XVolatileResult>  xNumResult;     //! Test
    css::lang::Locale                                 aFuncLoc;

public:
                            ScTestAddIn();
    virtual                 ~ScTestAddIn();

//                          SMART_UNO_DECLARATION( ScTestAddIn, UsrObject );
//  friend Reflection *     ScTestAddIn_getReflection();
//  virtual BOOL            queryInterface( Uik, XInterfaceRef& );
//  virtual XIdlClassRef    getIdlClass();

    static UString          getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

                            // XAddIn
    virtual OUString SAL_CALL getProgrammaticFuntionName( const OUString& aDisplayName ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayFunctionName( const OUString& aProgrammaticName ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getFunctionDescription( const OUString& aProgrammaticName ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayArgumentName( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getArgumentDescription( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getProgrammaticCategoryName( const OUString& aProgrammaticFunctionName ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getDisplayCategoryName( const OUString& aProgrammaticFunctionName ) throw(css::uno::RuntimeException);

                            // XLocalizable
    virtual void SAL_CALL setLocale( const css::lang::Locale& eLocale ) throw(css::uno::RuntimeException);
    virtual css::lang::Locale SAL_CALL getLocale(  ) throw(css::uno::RuntimeException);

                            // XTestAddIn
    virtual sal_Int32 SAL_CALL countParams( const css::uno::Sequence< css::uno::Any >& aArgs ) throw(css::uno::RuntimeException);
    virtual double SAL_CALL addOne( double fValue ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL repeatStr( const OUString& aStr, sal_Int32 nCount ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getDateString( const css::uno::Reference< css::beans::XPropertySet >& xCaller, double fValue ) throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColorValue( const css::uno::Reference< css::table::XCellRange >& xRange ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL transpose( const css::uno::Sequence< css::uno::Sequence< double > >& aMatrix ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Sequence< sal_Int32 > > SAL_CALL transposeInt( const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& aMatrix ) throw(css::uno::RuntimeException);
    virtual css::uno::Reference< css::sheet::XVolatileResult > SAL_CALL callAsync( const OUString& aString ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL repeatMultiple( sal_Int32 nCount, const css::uno::Any& aFirst, const css::uno::Sequence< css::uno::Any >& aFollow ) throw(css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStrOrVal( sal_Int32 nFlag ) throw(css::uno::RuntimeException);

                            // XServiceName
    virtual OUString SAL_CALL getServiceName(  ) throw(css::uno::RuntimeException);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
