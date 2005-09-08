/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Linear3DTransformation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:43:24 $
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
#ifndef _CHART_LINEAR3DTRANSFORMATION_HXX
#define _CHART_LINEAR3DTRANSFORMATION_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XTRANSFORMATION_HPP_
#include <com/sun/star/chart2/XTransformation.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

namespace chart
{

class Linear3DTransformation : public ::cppu::WeakImplHelper1<
    ::com::sun::star::chart2::XTransformation
    >
{
public:
    Linear3DTransformation( const ::com::sun::star::drawing::HomogenMatrix& rHomMatrix );
    virtual ~Linear3DTransformation();

    // ____ XTransformation ____
    /// @see ::com::sun::star::chart2::XTransformation
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL transform(
        const ::com::sun::star::uno::Sequence< double >& rSourceValues )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::drawing::HomogenMatrix    m_Matrix;
};

}  // namespace chart

// _CHART_LINEAR3DTRANSFORMATION_HXX
#endif

