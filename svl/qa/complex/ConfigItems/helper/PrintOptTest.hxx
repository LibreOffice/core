/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
