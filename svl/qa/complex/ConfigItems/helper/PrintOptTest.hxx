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

#ifndef  SVTOOLS_PRINTOPTTEST_HXX
#define  SVTOOLS_PRINTOPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/printoptions.hxx>

namespace css = ::com::sun::star;

class PrintOptTest
{
public:

    PrintOptTest();
    ~PrintOptTest();

    void impl_checkPrint();

private: //members

    SvtPrinterOptions aPrintOpt;
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    css::uno::Reference< css::container::XNameAccess > m_xNode;

private: // methods
    sal_Bool    impl_IsReduceTransparency() const ;
    void        impl_SetReduceTransparency( sal_Bool bState ) ;

    sal_Int16   impl_GetReducedTransparencyMode() const ;
    void        impl_SetReducedTransparencyMode( sal_Int16 nMode ) ;

    sal_Bool    impl_IsReduceGradients() const ;
    void        impl_SetReduceGradients( sal_Bool bState ) ;

    sal_Int16   impl_GetReducedGradientMode() const ;
    void        impl_SetReducedGradientMode( sal_Int16 nMode ) ;

    sal_Int16   impl_GetReducedGradientStepCount() const ;
    void        impl_SetReducedGradientStepCount( sal_Int16 nStepCount );

    sal_Bool    impl_IsReduceBitmaps() const ;
    void        impl_SetReduceBitmaps( sal_Bool bState ) ;

    sal_Int16   impl_GetReducedBitmapMode() const ;
    void        impl_SetReducedBitmapMode( sal_Int16 nMode ) ;

    sal_Int16   impl_GetReducedBitmapResolution() const ;
    void        impl_SetReducedBitmapResolution( sal_Int16 nResolution ) ;

    sal_Bool    impl_IsReducedBitmapIncludesTransparency() const ;
    void        impl_SetReducedBitmapIncludesTransparency( sal_Bool bState ) ;

    sal_Bool    impl_IsConvertToGreyscales() const;
    void        impl_SetConvertToGreyscales( sal_Bool bState ) ;

};

#endif // #ifndef  SVTOOLS_PRINTOPTTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
