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


#include "gridcontrol.hxx"
#include "grideventforwarder.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/awt/grid/DefaultGridDataModel.hpp>
#include <com/sun/star/awt/grid/SortableGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <tools/color.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

namespace toolkit
{
//======================================================================================================================
//= UnoGridModel
//======================================================================================================================
namespace
{
    Reference< XGridDataModel > lcl_getDefaultDataModel_throw( ::comphelper::ComponentContext const & i_context )
    {
        Reference< XMutableGridDataModel > const xDelegatorModel( DefaultGridDataModel::create( i_context.getUNOContext() ), UNO_QUERY_THROW );
        Reference< XGridDataModel > const xDataModel( SortableGridDataModel::create( i_context.getUNOContext(), xDelegatorModel ), UNO_QUERY_THROW );
        return xDataModel;
    }

    Reference< XGridColumnModel > lcl_getDefaultColumnModel_throw( ::comphelper::ComponentContext const & i_context )
    {
        Reference< XGridColumnModel > const xColumnModel( i_context.createComponent( "com.sun.star.awt.grid.DefaultGridColumnModel" ), UNO_QUERY_THROW );
        return xColumnModel;
    }
}

//----------------------------------------------------------------------------------------------------------------------
UnoGridModel::UnoGridModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
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
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_GRID_SHOWROWHEADER );
    ImplRegisterProperty( BASEPROPERTY_ROW_HEADER_WIDTH );
    ImplRegisterProperty( BASEPROPERTY_GRID_SHOWCOLUMNHEADER );
    ImplRegisterProperty( BASEPROPERTY_COLUMN_HEADER_HEIGHT );
    ImplRegisterProperty( BASEPROPERTY_ROW_HEIGHT );
    ImplRegisterProperty( BASEPROPERTY_GRID_DATAMODEL, makeAny( lcl_getDefaultDataModel_throw( maContext ) ) );
    ImplRegisterProperty( BASEPROPERTY_GRID_COLUMNMODEL, makeAny( lcl_getDefaultColumnModel_throw( maContext ) ) );
    ImplRegisterProperty( BASEPROPERTY_GRID_SELECTIONMODE );
    ImplRegisterProperty( BASEPROPERTY_FONTRELIEF );
    ImplRegisterProperty( BASEPROPERTY_FONTEMPHASISMARK );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_TEXTLINECOLOR );
    ImplRegisterProperty( BASEPROPERTY_USE_GRID_LINES );
    ImplRegisterProperty( BASEPROPERTY_GRID_LINE_COLOR );
    ImplRegisterProperty( BASEPROPERTY_GRID_HEADER_BACKGROUND );
    ImplRegisterProperty( BASEPROPERTY_GRID_HEADER_TEXT_COLOR );
    ImplRegisterProperty( BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS );
    ImplRegisterProperty( BASEPROPERTY_VERTICALALIGN );
}

//----------------------------------------------------------------------------------------------------------------------
UnoGridModel::UnoGridModel( const UnoGridModel& rModel )
    :UnoControlModel( rModel )
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        Reference< XGridDataModel > xDataModel;
        // clone the data model
        const Reference< XFastPropertySet > xCloneSource( &const_cast< UnoGridModel& >( rModel ) );
        try
        {
            const Reference< XCloneable > xCloneable( xCloneSource->getFastPropertyValue( BASEPROPERTY_GRID_DATAMODEL ), UNO_QUERY_THROW );
            xDataModel.set( xCloneable->createClone(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        if ( !xDataModel.is() )
            xDataModel = lcl_getDefaultDataModel_throw( maContext );
        UnoControlModel::setFastPropertyValue_NoBroadcast( BASEPROPERTY_GRID_DATAMODEL, makeAny( xDataModel ) );
            // do *not* use setFastPropertyValue here: The UnoControlModel ctor did a simple copy of all property values,
            // so before this call here, we share our data model with the own of the clone source. setFastPropertyValue,
            // then, disposes the old data model - which means the data model which in fact belongs to the clone source.
            // so, call the UnoControlModel's impl-method for setting the value.

        // clone the column model
        Reference< XGridColumnModel > xColumnModel;
        try
        {
            const Reference< XCloneable > xCloneable( xCloneSource->getFastPropertyValue( BASEPROPERTY_GRID_COLUMNMODEL ), UNO_QUERY_THROW );
            xColumnModel.set( xCloneable->createClone(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        if ( !xColumnModel.is() )
            xColumnModel = lcl_getDefaultColumnModel_throw( maContext );
        UnoControlModel::setFastPropertyValue_NoBroadcast( BASEPROPERTY_GRID_COLUMNMODEL, makeAny( xColumnModel ) );
            // same comment as above: do not use our own setPropertyValue here.
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//----------------------------------------------------------------------------------------------------------------------
UnoControlModel* UnoGridModel::Clone() const
{
    return new UnoGridModel( *this );
}

//----------------------------------------------------------------------------------------------------------------------
namespace
{
    void lcl_dispose_nothrow( const Any& i_component )
    {
        try
        {
            const Reference< XComponent > xComponent( i_component, UNO_QUERY_THROW );
            xComponent->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridModel::dispose(  ) throw(RuntimeException)
{
    lcl_dispose_nothrow( getFastPropertyValue( BASEPROPERTY_GRID_COLUMNMODEL ) );
    lcl_dispose_nothrow( getFastPropertyValue( BASEPROPERTY_GRID_DATAMODEL ) );

    UnoControlModel::dispose();
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    Any aOldSubModel;
    if ( ( nHandle == BASEPROPERTY_GRID_COLUMNMODEL ) || ( nHandle == BASEPROPERTY_GRID_DATAMODEL ) )
    {
        aOldSubModel = getFastPropertyValue( nHandle );
        if ( aOldSubModel == rValue )
        {
            OSL_ENSURE( false, "UnoGridModel::setFastPropertyValue_NoBroadcast: setting the same value, again!" );
                // shouldn't this have been caught by convertFastPropertyValue?
            aOldSubModel.clear();
        }
    }

    UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

    if ( aOldSubModel.hasValue() )
        lcl_dispose_nothrow( aOldSubModel );
}

//----------------------------------------------------------------------------------------------------------------------
OUString UnoGridModel::getServiceName() throw(RuntimeException)
{
    return OUString::createFromAscii( szServiceName_GridControlModel );
}

//----------------------------------------------------------------------------------------------------------------------
Any UnoGridModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_GridControl ) );
        case BASEPROPERTY_GRID_SELECTIONMODE:
            return uno::makeAny( SelectionType(1) );
        case BASEPROPERTY_GRID_SHOWROWHEADER:
        case BASEPROPERTY_USE_GRID_LINES:
            return uno::makeAny( (sal_Bool)sal_False );
        case BASEPROPERTY_ROW_HEADER_WIDTH:
            return uno::makeAny( sal_Int32( 10 ) );
        case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
            return uno::makeAny( (sal_Bool)sal_True );
        case BASEPROPERTY_COLUMN_HEADER_HEIGHT:
        case BASEPROPERTY_ROW_HEIGHT:
        case BASEPROPERTY_GRID_HEADER_BACKGROUND:
        case BASEPROPERTY_GRID_HEADER_TEXT_COLOR:
        case BASEPROPERTY_GRID_LINE_COLOR:
        case BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS:
            return Any();
        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
    }

}

//----------------------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------------------
// XMultiPropertySet
Reference< XPropertySetInfo > UnoGridModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//======================================================================================================================
//= UnoGridControl
//======================================================================================================================
UnoGridControl::UnoGridControl( const Reference< XMultiServiceFactory >& i_factory )
    :UnoGridControl_Base( i_factory )
    ,m_aSelectionListeners( *this )
    ,m_pEventForwarder( new GridEventForwarder( *this ) )
{
}

//----------------------------------------------------------------------------------------------------------------------
UnoGridControl::~UnoGridControl()
{
}

//----------------------------------------------------------------------------------------------------------------------
OUString UnoGridControl::GetComponentServiceName()
{
    return OUString("Grid");
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::dispose(  ) throw(RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    m_aSelectionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    const Reference< XGridRowSelection > xGrid( getPeer(), UNO_QUERY_THROW );
    xGrid->addSelectionListener( &m_aSelectionListeners );
}

//----------------------------------------------------------------------------------------------------------------------
namespace
{
    void lcl_setEventForwarding( const Reference< XControlModel >& i_gridControlModel, const ::boost::scoped_ptr< GridEventForwarder >& i_listener,
        bool const i_add )
    {
        const Reference< XPropertySet > xModelProps( i_gridControlModel, UNO_QUERY );
        if ( !xModelProps.is() )
            return;

        try
        {
            Reference< XContainer > const xColModel(
                xModelProps->getPropertyValue( ::rtl::OUString( "ColumnModel" ) ),
                UNO_QUERY_THROW );
            if ( i_add )
                xColModel->addContainerListener( i_listener.get() );
            else
                xColModel->removeContainerListener( i_listener.get() );

            Reference< XGridDataModel > const xDataModel(
                xModelProps->getPropertyValue( ::rtl::OUString( "GridDataModel" ) ),
                UNO_QUERY_THROW
            );
            Reference< XMutableGridDataModel > const xMutableDataModel( xDataModel, UNO_QUERY );
            if ( xMutableDataModel.is() )
            {
                if ( i_add )
                    xMutableDataModel->addGridDataListener( i_listener.get() );
                else
                    xMutableDataModel->removeGridDataListener( i_listener.get() );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL UnoGridControl::setModel( const Reference< XControlModel >& i_model ) throw(RuntimeException)
{
    lcl_setEventForwarding( getModel(), m_pEventForwarder, false );
    if ( !UnoGridControl_Base::setModel( i_model ) )
        return sal_False;
    lcl_setEventForwarding( getModel(), m_pEventForwarder, true );
    return sal_True;
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Int32 UnoGridControl::getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl > const xGrid ( getPeer(), UNO_QUERY_THROW );
    return xGrid->getRowAtPoint( x, y );
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Int32 UnoGridControl::getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridControl > const xGrid ( getPeer(), UNO_QUERY_THROW );
    return xGrid->getColumnAtPoint( x, y );
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Int32 SAL_CALL UnoGridControl::getCurrentColumn(  ) throw (RuntimeException)
{
    Reference< XGridControl > const xGrid ( getPeer(), UNO_QUERY_THROW );
    return xGrid->getCurrentColumn();
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Int32 SAL_CALL UnoGridControl::getCurrentRow(  ) throw (RuntimeException)
{
    Reference< XGridControl > const xGrid ( getPeer(), UNO_QUERY_THROW );
    return xGrid->getCurrentRow();
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::selectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException )
{
    Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->selectRow( i_rowIndex );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::selectAllRows() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->selectAllRows();
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::deselectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException )
{
    Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->deselectRow( i_rowIndex );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::deselectAllRows() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->deselectAllRows();
}

//----------------------------------------------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL UnoGridControl::getSelectedRows() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->getSelectedRows();
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Bool SAL_CALL UnoGridControl::hasSelectedRows() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->hasSelectedRows();
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Bool SAL_CALL UnoGridControl::isRowSelected(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XGridRowSelection >( getPeer(), UNO_QUERY_THROW )->isRowSelected( index );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.addInterface( listener );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoGridControl::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.removeInterface( listener );
}

}//namespace toolkit

Reference< XInterface > SAL_CALL GridControl_CreateInstance( const Reference< XMultiServiceFactory >& i_factory )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoGridControl( i_factory ) );
}

Reference< XInterface > SAL_CALL GridControlModel_CreateInstance( const Reference< XMultiServiceFactory >& i_factory )
{
    return Reference < XInterface >( ( ::cppu::OWeakObject* ) new ::toolkit::UnoGridModel( i_factory ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
