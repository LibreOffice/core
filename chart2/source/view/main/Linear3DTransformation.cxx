/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Linear3DTransformation.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:35:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "Linear3DTransformation.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

namespace chart
{

    Linear3DTransformation::Linear3DTransformation( const drawing::HomogenMatrix& rHomMatrix )
    : m_Matrix(rHomMatrix)
{}

Linear3DTransformation::~Linear3DTransformation()
{}

// ____ XTransformation ____
Sequence< double > SAL_CALL Linear3DTransformation::transform(
                        const Sequence< double >& rSourceValues )
    throw (RuntimeException,
           lang::IllegalArgumentException)
{
    /*
    ::basegfx::B3DPoint aSource( SequenceToB3DPoint( rSourceValues ) );
    ::basegfx::B3DPoint aTarget = m_Matrix*aSource;
    return B3DPointToSequence(aTarget);
    */
    Sequence< double > aNewVec(3);
    double fZwi;

    fZwi = m_Matrix.Line1.Column1 * rSourceValues[0]
         + m_Matrix.Line1.Column2 * rSourceValues[1]
         + m_Matrix.Line1.Column3 * rSourceValues[2]
         + m_Matrix.Line1.Column4;
    aNewVec[0] = fZwi;

    fZwi = m_Matrix.Line2.Column1 * rSourceValues[0]
         + m_Matrix.Line2.Column2 * rSourceValues[1]
         + m_Matrix.Line2.Column3 * rSourceValues[2]
         + m_Matrix.Line2.Column4;
    aNewVec[1] = fZwi;

    fZwi = m_Matrix.Line3.Column1 * rSourceValues[0]
         + m_Matrix.Line3.Column2 * rSourceValues[1]
         + m_Matrix.Line3.Column3 * rSourceValues[2]
         + m_Matrix.Line3.Column4;
    aNewVec[2] = fZwi;

    fZwi = m_Matrix.Line4.Column1 * rSourceValues[0]
         + m_Matrix.Line4.Column2 * rSourceValues[1]
         + m_Matrix.Line4.Column3 * rSourceValues[2]
         + m_Matrix.Line4.Column4;
    if(fZwi != 1.0 && fZwi != 0.0)
    {
        aNewVec[0] /= fZwi;
        aNewVec[1] /= fZwi;
        aNewVec[2] /= fZwi;
    }
    return aNewVec;
}

sal_Int32 SAL_CALL Linear3DTransformation::getSourceDimension()
    throw (RuntimeException)
{
    return 3;
}

sal_Int32 SAL_CALL Linear3DTransformation::getTargetDimension()
    throw (RuntimeException)
{
    return 3;
}


}  // namespace chart
