/*************************************************************************
 *
 *  $RCSfile: snakewipe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:08:49 $
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

#if ! defined INCLUDED_SLIDESHOW_SNAKEWIPE_HXX
#define INCLUDED_SLIDESHOW_SNAKEWIPE_HXX

#include "parametricpolypolygon.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/point/b2dpoint.hxx"


namespace presentation {
namespace internal {

/// Generates a snake wipe:
class SnakeWipe : public ParametricPolyPolygon
{
public:
    SnakeWipe( sal_Int32 nElements, bool diagonal, bool flipOnYAxis );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );

protected:
    // topLeftHorizontal:
    ::basegfx::B2DPolyPolygon calcSnake( double t ) const;
    // topLeftDiagonal:
    ::basegfx::B2DPolyPolygon calcHalfDiagonalSnake( double t, bool in ) const;

    const sal_Int32 m_sqrtElements;
    const double m_elementEdge;
    const bool m_diagonal;
    const bool m_flipOnYAxis;
};

/// Generates a parallel snakes wipe:
class ParallelSnakesWipe : public SnakeWipe
{
public:
    ParallelSnakesWipe( sal_Int32 nElements,
                        bool diagonal, bool flipOnYAxis, bool opposite )
        : SnakeWipe( nElements, diagonal, flipOnYAxis ),
          m_opposite( opposite )
        {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    const bool m_opposite;
};

}
}

#endif /* INCLUDED_SLIDESHOW_SNAKEWIPE_HXX */
