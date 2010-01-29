/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gridcontrol.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_toolkit.hxx"

#include <gridcontrol.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/ScrollBarMode.hpp>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

namespace toolkit
{
//  ----------------------------------------------------
//  class UnoGridModel
//  ----------------------------------------------------
UnoGridModel::UnoGridModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FILLCOLOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE ); // resizeable
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
    ImplRegisterProperty( BASEPROPERTY_GRID_SHOWROWHEADER );
    ImplRegisterProperty( BASEPROPERTY_GRID_SHOWCOLUMNHEADER );
    ImplRegisterProperty( BASEPROPERTY_GRID_DATAMODEL );
    ImplRegisterProperty( BASEPROPERTY_GRID_COLUMNMODEL );
    ImplRegisterProperty( BASEPROPERTY_GRID_SELECTIONMODE );

}

UnoGridModel::UnoGridModel( const UnoGridModel& rModel )
: UnoControlModel( rModel )
{
}

UnoControlModel* UnoGridModel::Clone() const
{
    return new UnoGridModel( *this );
}

OUString UnoGridModel::getServiceName() throw(RuntimeException)
{
    return OUString::createFromAscii( szServiceName_GridControlModel );
}

Any UnoGridModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_GridControl ) );
        case BASEPROPERTY_GRID_SELECTIONMODE:
            return uno::makeAny( SelectionType(1) );
        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
    }

}

::cppu::IPropertyArrayHelper& UnoGridModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoGridModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoGridControl
//  ----------------------------------------------------
UnoGridControl::UnoGridControl()
: mSelectionMode(SelectionType(1))
{
}

OUString UnoGridControl::GetComponentServiceName()
{
    return OUString::createFromAscii( "Grid" );
}

void SAL_CALL UnoGridControl::dispose(  ) throw(RuntimeException)
{
    UnoControl::dispose();
}

void UnoGridControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    Reference< XGridControl >  xGrid( getPeer(), UNO_QUERY_THROW );

    Reference<XGridDataListener> xListener ( getPeer(), UNO_QUERY_THROW );
    Reference<XPropertySet> xPropSet ( getModel(), UNO_QUERY_THROW );

    Reference<XGridDataModel> xGridDataModel ( xPropSet->getPropertyValue(OUString::createFromAscii( "GridDataModel" )), UNO_QUERY_THROW );
    xGridDataModel->addDataListener(xListener);
}


// -------------------------------------------------------------------
// XGridControl
// -------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > SAL_CALL UnoGridControl::getColumnModel() throw (::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet> xPropSet ( getModel(), UNO_QUERY_THROW );
    Reference<XGridColumnModel> xGridColumnModel ( xPropSet->getPropertyValue(OUString::createFromAscii( "ColumnModel" )), UNO_QUERY_THROW );

    return  xGridColumnModel;
}

void SAL_CALL UnoGridControl::setColumnModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > & model) throw (::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet> xPropSet ( getModel(), UNO_QUERY_THROW );
    xPropSet->setPropertyValue(OUString::createFromAscii( "ColumnModel" ), Any (model));
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > SAL_CALL UnoGridControl::getDataModel() throw (::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet> xPropSet ( getModel(), UNO_QUERY_THROW );
    Reference<XGridDataModel> xGridDataModel ( xPropSet->getPropertyValue(OUString::createFromAscii( "GridDataModel" )), UNO_QUERY_THROW );

    return xGridDataModel;
}

void SAL_CALL UnoGridControl::setDataModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > & model) throw (::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet> xPropSet ( getModel(), UNO_QUERY_THROW );
    xPropSet->setPropertyValue(OUString::createFromAscii( "GridDataModel" ), Any(model));
}

::sal_Int32 UnoGridControl::getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->getItemIndexAtPoint( x, y );
}

/*
void SAL_CALL UnoGridControl::addMouseListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
     Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->addMouseListener( listener );
}

void SAL_CALL UnoGridControl::removeMouseListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->removeMouseListener( listener );
}
*/
// -------------------------------------------------------------------
// XGridSelection
// -------------------------------------------------------------------

::sal_Int32 SAL_CALL UnoGridControl::getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->getMinSelectionIndex();
}

::sal_Int32 SAL_CALL UnoGridControl::getMaxSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->getMaxSelectionIndex();
}

void SAL_CALL UnoGridControl::insertIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->insertIndexIntervall( start, length);
}

void SAL_CALL UnoGridControl::removeIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->removeIndexIntervall( start, length );
}

::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL UnoGridControl::getSelection() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->getSelection();
}

::sal_Bool SAL_CALL UnoGridControl::isSelectionEmpty() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->isSelectionEmpty();
}

::sal_Bool SAL_CALL UnoGridControl::isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->isSelectedIndex( index );
}

void SAL_CALL UnoGridControl::selectRow(::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->selectRow( y );
}

void SAL_CALL UnoGridControl::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->addSelectionListener( listener );
}

void SAL_CALL UnoGridControl::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl >( getPeer(), UNO_QUERY_THROW )->removeSelectionListener( listener );
}
}

Reference< XInterface > SAL_CALL GridControl_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoGridControl );
}

Reference< XInterface > SAL_CALL GridControlModel_CreateInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoGridModel );
}
