/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PrintOptTest.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:09:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef  SVTOOLS_PRINTOPTTEST_HXX
#define  SVTOOLS_PRINTOPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <svtools/printoptions.hxx>

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
