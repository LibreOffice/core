/*************************************************************************
 *
 *  $RCSfile: cellbindinghandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:02:46 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX
#include "cellbindinghandler.hxx"
#endif

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef EXTENSIONS_PROPCTRLR_CELLBINDINGHELPER_HXX
#include "cellbindinghelper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_BINDING_XVALUEBINDING_HPP_
#include <com/sun/star/form/binding/XValueBinding.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::form::binding;

    //====================================================================
    //= CellBindingPropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    CellBindingPropertyHandler::CellBindingPropertyHandler( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XPropertySet >& _rxIntrospectee, const Reference< XModel >& _rxContextDocument,
            const Reference< XTypeConverter >& _rxTypeConverter  )
        :PropertyHandler( _rxIntrospectee, _rxTypeConverter )
    {
        if ( CellBindingHelper::isSpreadsheetDocument( _rxContextDocument ) )
            m_pHelper.reset( new CellBindingHelper( m_xIntrospectee, _rxContextDocument ) );
    }

    //--------------------------------------------------------------------
    CellBindingPropertyHandler::~CellBindingPropertyHandler( )
    {
    }

    //--------------------------------------------------------------------
    Any SAL_CALL CellBindingPropertyHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::getPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            Reference< XValueBinding > xBinding( m_pHelper->getCurrentBinding() );
            if ( !m_pHelper->isCellBinding( xBinding ) )
                xBinding.clear();

            aReturn <<= xBinding;
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            Reference< XListEntrySource > xSource( m_pHelper->getCurrentListSource() );
            if ( !m_pHelper->isCellRangeListSource( xSource ) )
                xSource.clear();

            aReturn <<= xSource;
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            Reference< XValueBinding > xBinding( m_pHelper->getCurrentBinding() );
            aReturn <<= (sal_Int16)( m_pHelper->isCellIntegerBinding( xBinding ) ? 1 : 0 );
        }
        break;

        default:
            DBG_ASSERT( _bLazy, "CellBindingPropertyHandler::getPropertyValue: cannot handle this!" );
            break;
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL CellBindingPropertyHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::setPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nPropId )
        {
        case PROPERTY_ID_BOUND_CELL:
        {
            Reference< XValueBinding > xBinding;
            _rValue >>= xBinding;
            m_pHelper->setBinding( xBinding );
        }
        break;

        case PROPERTY_ID_LIST_CELL_RANGE:
        {
            Reference< XListEntrySource > xSource;
            _rValue >>= xSource;
            m_pHelper->setListSource( xSource );
        }
        break;

        case PROPERTY_ID_CELL_EXCHANGE_TYPE:
        {
            Reference< XValueBinding > xBinding = m_pHelper->getCurrentBinding( );
            OSL_ENSURE( xBinding.is(), "CellBindingPropertyHandler::setPropertyValue: how this?" );
                // this property here should be disabled if there's no binding at our current document
            if ( !xBinding.is() )
                break;

            sal_Int16 nExchangeType = 0;
            _rValue >>= nExchangeType;

            sal_Bool bNeedIntegerBinding = ( nExchangeType == 1 );
            if ( (bool)bNeedIntegerBinding != m_pHelper->isCellIntegerBinding( xBinding ) )
            {
                CellAddress aAddress;
                if ( m_pHelper->getAddressFromCellBinding( xBinding, aAddress ) )
                {
                    xBinding = m_pHelper->createCellBindingFromAddress( aAddress, bNeedIntegerBinding );
                    m_pHelper->setBinding( xBinding );
                }
            }
        }
        break;

        default:
            DBG_ERROR( "CellBindingPropertyHandler::setPropertyValue: cannot handle this!" );
            break;
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL CellBindingPropertyHandler::getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::getPropertyValueFromStringRep: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;


        switch( _nPropId )
        {
            case PROPERTY_ID_LIST_CELL_RANGE:
                aReturn = makeAny( m_pHelper->createCellListSourceFromStringAddress( _rStringRep ) );
                break;

            case PROPERTY_ID_BOUND_CELL:
            {
                // if we have the possibility of an integer binding, then we must preserve
                // this property's value (e.g. if the current binding is an integer binding, then
                // the newly created one must be, too)
                bool bIntegerBinding = false;
                if ( m_pHelper->isCellIntegerBindingAllowed() )
                {
                    sal_Int16 nCurrentBindingType = 0;
                    getPropertyValue( PROPERTY_ID_CELL_EXCHANGE_TYPE ) >>= nCurrentBindingType;
                    bIntegerBinding = ( nCurrentBindingType != 0 );
                }
                aReturn = makeAny( m_pHelper->createCellBindingFromStringAddress( _rStringRep, bIntegerBinding ) );
            }
            break;

            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
            {
                // default string conversion works here
                StringRepresentation aConversionHelper( m_xTypeConverter );
                aReturn = aConversionHelper.getPropertyValueFromStringRep(
                    _rStringRep,
                    ::getCppuType( static_cast< sal_Int16* >( NULL ) ),
                    _nPropId,
                    m_pInfoService.get()
                );
            }
            break;

            default:
                DBG_ERROR( "CellBindingPropertyHandler::setPropertyValue: cannot handle this!" );
                break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL CellBindingPropertyHandler::getStringRepFromPropertyValue( PropertyId _nPropId, const Any& _rValue ) const
    {
        ::rtl::OUString sReturn;

        OSL_ENSURE( m_pHelper.get(), "CellBindingPropertyHandler::getStringRepFromPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return sReturn;

        switch ( _nPropId )
        {
            case PROPERTY_ID_BOUND_CELL:
            {
                Reference< XValueBinding > xBinding;
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                _rValue >>= xBinding;
                DBG_ASSERT( bSuccess, "CellBindingPropertyHandler::getPropertyValueFromStringRep: invalid value (1)!" );

                // the only value binding we support so far is linking to spreadsheet cells
                sReturn = m_pHelper->getStringAddressFromCellBinding( xBinding );
            }
            break;

            case PROPERTY_ID_LIST_CELL_RANGE:
            {
                Reference< XListEntrySource > xSource;
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                _rValue >>= xSource;
                DBG_ASSERT( bSuccess, "CellBindingPropertyHandler::getPropertyValueFromStringRep: invalid value (2)!" );

                // the only value binding we support so far is linking to spreadsheet cells
                sReturn = m_pHelper->getStringAddressFromCellListSource( xSource );
            }
            break;

            case PROPERTY_ID_CELL_EXCHANGE_TYPE:
            {
                // default string conversion works here
                StringRepresentation aConversionHelper( m_xTypeConverter );
                sReturn = aConversionHelper.getStringRepFromPropertyValue(
                            _rValue,
                            _nPropId,
                            m_pInfoService.get()
                          );
            }
            break;

            default:
                DBG_ERROR( "CellBindingPropertyHandler::getPropertyValueFromStringRep: cannot handle this!" );
                break;
        }

        return sReturn;
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL CellBindingPropertyHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        bool bAllowCellLinking      = m_pHelper.get() && m_pHelper->isCellBindingAllowed();
        bool bAllowCellIntLinking   = m_pHelper.get() && m_pHelper->isCellIntegerBindingAllowed();
        bool bAllowListCellRange    = m_pHelper.get() && m_pHelper->isListCellRangeAllowed();
        if ( bAllowCellLinking || bAllowListCellRange || bAllowCellIntLinking )
        {
            sal_Int32 nPos =  ( bAllowCellLinking    ? 1 : 0 )
                            + ( bAllowListCellRange  ? 1 : 0 )
                            + ( bAllowCellIntLinking ? 1 : 0 );
            aProperties.resize( nPos );

            if ( bAllowCellLinking )
            {
                aProperties[ --nPos ] = Property( PROPERTY_BOUND_CELL, PROPERTY_ID_BOUND_CELL,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
            if ( bAllowCellIntLinking )
            {
                aProperties[ --nPos ] = Property( PROPERTY_CELL_EXCHANGE_TYPE, PROPERTY_ID_CELL_EXCHANGE_TYPE,
                    ::getCppuType( static_cast< sal_Int16* >( NULL ) ), 0 );
            }
            if ( bAllowListCellRange )
            {
                aProperties[ --nPos ] = Property( PROPERTY_LIST_CELL_RANGE, PROPERTY_ID_LIST_CELL_RANGE,
                    ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), 0 );
            }
        }

        return aProperties;
    }

//........................................................................
}   // namespace pcr
//........................................................................
