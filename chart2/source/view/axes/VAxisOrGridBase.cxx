/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VAxisOrGridBase.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "VAxisOrGridBase.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "TickmarkHelper.hxx"

// header for define DBG_ASSERT
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VAxisOrGridBase::VAxisOrGridBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
            : PlotterBase( nDimensionCount )
            , m_nDimensionIndex( nDimensionIndex )
{
}

VAxisOrGridBase::~VAxisOrGridBase()
{
}

void SAL_CALL VAxisOrGridBase::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
            throw (uno::RuntimeException)
{
    m_aScale = rScale;
    m_aIncrement = rIncrement;
}

void VAxisOrGridBase::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix )
{
    m_aMatrixScreenToScene = HomogenMatrixToB3DHomMatrix(rMatrix);
    PlotterBase::setTransformationSceneToScreen( rMatrix);
}

TickmarkHelper* VAxisOrGridBase::createTickmarkHelper()
{
    TickmarkHelper* pRet=NULL;
    if( 2==m_nDimension )
    {
        pRet = new TickmarkHelper( m_aScale, m_aIncrement );
    }
    else
    {
        pRet = new TickmarkHelper_3D( m_aScale, m_aIncrement );
    }
    return pRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
