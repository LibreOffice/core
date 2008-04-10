/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ExplicitCategoriesProvider.cxx,v $
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

#include "ExplicitCategoriesProvider.hxx"
#include "DiagramHelper.hxx"
#include "CommonConverters.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using rtl::OUString;

ExplicitCategoriesProvider::ExplicitCategoriesProvider( const Reference< chart2::XCoordinateSystem >& xCooSysModel )
    : m_bDirty(true)
    , m_xCooSysModel( xCooSysModel )
    , m_xCategories()
{
    if( xCooSysModel.is() )
    {
        uno::Reference< XAxis > xAxis( xCooSysModel->getAxisByDimension(0,0) );
        if( xAxis.is() )
            m_xCategories = xAxis->getScaleData().Categories;
    }
}

ExplicitCategoriesProvider::~ExplicitCategoriesProvider()
{
}

//XTextualDataSequence
Sequence< ::rtl::OUString > SAL_CALL ExplicitCategoriesProvider::getTextualData() throw( uno::RuntimeException)
{
    if( m_bDirty )
    {
        if( m_xCategories.is() )
            m_aExplicitCategories = DataSequenceToStringSequence(m_xCategories->getValues());
        if(!m_aExplicitCategories.getLength())
            m_aExplicitCategories = DiagramHelper::generateAutomaticCategories( uno::Reference< chart2::XCoordinateSystem >( m_xCooSysModel.get(), uno::UNO_QUERY ) );
        m_bDirty = false;
    }
    return m_aExplicitCategories;
}

// static
OUString ExplicitCategoriesProvider::getCategoryByIndex(
        const Reference< XCoordinateSystem >& xCooSysModel,
        sal_Int32 nIndex )
{
    if( xCooSysModel.is())
    {
        Reference< XTextualDataSequence > xTemp( new ExplicitCategoriesProvider( xCooSysModel ));
        if( xTemp.is())
        {
            Sequence< OUString > aCategories( xTemp->getTextualData());
            if( nIndex < aCategories.getLength())
                return aCategories[ nIndex ];
        }
    }
    return OUString();
}

//.............................................................................
} //namespace chart
//.............................................................................
