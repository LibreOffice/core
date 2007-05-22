/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BarPositionHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:15:13 $
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

#ifndef _CHART2_BARPOSITIONHELPER_HXX
#define _CHART2_BARPOSITIONHELPER_HXX

#include "PlottingPositionHelper.hxx"
#include "CategoryPositionHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class BarPositionHelper : public CategoryPositionHelper, public PlottingPositionHelper
{
public:
    BarPositionHelper( bool bSwapXAndY=true );
    BarPositionHelper( const BarPositionHelper& rSource );
    virtual ~BarPositionHelper();

    virtual PlottingPositionHelper* clone() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
                        getTransformationScaledLogicToScene() const;

    void                updateSeriesCount( double fSeriesCount ); /*only enter the size of x stacked series*/

    sal_Int32           getStartCategoryIndex() const {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nStart = static_cast<sal_Int32>(getLogicMinX() - 0.5);
                            if( nStart < 0 )
                                nStart = 0;
                            return nStart;
                        }
    sal_Int32           getEndCategoryIndex() const  {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nEnd = static_cast<sal_Int32>(getLogicMaxX() - 0.5);
                            if( nEnd < 0 )
                                nEnd = 0;
                            return nEnd;
                        }
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
