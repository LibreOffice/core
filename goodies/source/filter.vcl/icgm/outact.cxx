/*************************************************************************
 *
 *  $RCSfile: outact.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#include <outact.hxx>
#include <vcl/gradient.hxx>

using namespace ::com::sun::star;

// ---------------------------------------------------------------

CGMOutAct::CGMOutAct( CGM& rCGM )
{
    mpCGM = &rCGM;
    mnCurrentPage = mnGroupActCount = mnGroupLevel = 0;
    mpGroupLevel = new sal_uInt32[ CGM_OUTACT_MAX_GROUP_LEVEL ];
    mpPoints = (Point*)new sal_Int8[ 0x2000 * sizeof( Point ) ];
    mpFlags = new BYTE[ 0x2000 ];

    mnIndex = 0;
    mpGradient = NULL;
};

// ---------------------------------------------------------------

CGMOutAct::~CGMOutAct()
{
    delete mpPoints;
    delete mpFlags;
    delete mpGroupLevel;

    if ( mpGradient )
        delete mpGradient;
};

// ---------------------------------------------------------------

void CGMOutAct::BeginFigure()
{
    if ( mnIndex )
        EndFigure();

    BeginGroup();
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::CloseRegion()
{
    if ( mnIndex > 2 )
    {
        NewRegion();
        DrawPolyPolygon( maPolyPolygon );
        maPolyPolygon.Clear();
    }
}

// ---------------------------------------------------------------

void CGMOutAct::NewRegion()
{
    if ( mnIndex > 2 )
    {
        Polygon aPolygon( mnIndex, mpPoints, mpFlags );
        maPolyPolygon.Insert( aPolygon );
    }
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::EndFigure()
{
    NewRegion();
    DrawPolyPolygon( maPolyPolygon );
    maPolyPolygon.Clear();
    EndGroup();
    mnIndex = 0;
}

// ---------------------------------------------------------------

void CGMOutAct::RegPolyLine( Polygon& rPolygon, sal_Bool bReverse )
{
    sal_uInt32 nPoints = rPolygon.GetSize();
    if ( nPoints )
    {
        if ( bReverse )
        {
            for ( sal_uInt32 i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( nPoints - i - 1 );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( nPoints - i - 1 );
            }
        }
        else
        {
            for ( sal_uInt32 i = 0; i <  nPoints; i++ )
            {
                mpPoints[ mnIndex + i ] = rPolygon.GetPoint( i );
                mpFlags[ mnIndex + i ] = (sal_Int8)rPolygon.GetFlags( i );
            }
        }
        mnIndex += nPoints;
    }
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientOffset( long nHorzOfs, long nVertOfs, sal_uInt32 nType )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->XOffset = ( (sal_uInt16)nHorzOfs & 0x7f );
    mpGradient->YOffset = ( (sal_uInt16)nHorzOfs & 0x7f );
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientEdge( long nEdge )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientAngle( long nAngle )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->Angle = nAngle;
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientDescriptor( sal_uInt32 nColorFrom, sal_uInt32 nColorTo )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    mpGradient->StartColor = nColorFrom;
    mpGradient->EndColor = nColorTo;
}

// ---------------------------------------------------------------

void CGMOutAct::SetGradientStyle( sal_uInt32 nStyle, double fRatio )
{
    if ( !mpGradient )
        mpGradient = new awt::Gradient;
    switch ( nStyle )
    {
        case 0xff :
        {
            mpGradient->Style = awt::GradientStyle_AXIAL;
        }
        break;
        case 4 :
        {
            mpGradient->Style = awt::GradientStyle_RADIAL;          // CONICAL
        }
        break;
        case 3 :
        {
            mpGradient->Style = awt::GradientStyle_RECT;
        }
        break;
        case 2 :
        {
            mpGradient->Style = awt::GradientStyle_ELLIPTICAL;
        }
        break;
        default :
        {
            mpGradient->Style = awt::GradientStyle_LINEAR;
        }
    }
}


