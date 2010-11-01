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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
