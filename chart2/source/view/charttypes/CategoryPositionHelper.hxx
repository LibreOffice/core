/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CategoryPositionHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:15:54 $
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

#ifndef _CHART2_CATEGORYPOSITIONHELPER_HXX
#define _CHART2_CATEGORYPOSITIONHELPER_HXX

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class CategoryPositionHelper
{
public:
    CategoryPositionHelper( double fSeriesCount, double CategoryWidth = 1.0);
    CategoryPositionHelper( const CategoryPositionHelper& rSource );
    CategoryPositionHelper();
    virtual ~CategoryPositionHelper();

    double getSlotWidth() const;
    double getSlotPos( double fCategoryX, double fSeriesNumber ) const;

    //Distance between two neighboring bars in same category, seen relative to width of the bar
    void setInnerDistance( double fInnerDistance );

    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    void setOuterDistance( double fOuterDistance );

protected:
    double m_fSeriesCount;
    double m_fCategoryWidth;
    //Distance between two neighboring bars in same category, seen relative to width of the bar:
    double m_fInnerDistance; //[-1,1] m_fInnerDistance=1 --> distance == width; m_fInnerDistance=-1-->all rects are painted on the same position
    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    double m_fOuterDistance; //>=0 m_fOuterDistance=1 --> distance == width
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
