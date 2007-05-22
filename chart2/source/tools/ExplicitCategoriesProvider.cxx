/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExplicitCategoriesProvider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:58:35 $
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
