/*************************************************************************
 *
 *  $RCSfile: cellvaluebinding.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:08:34 $
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

#ifndef SC_CELLVALUEBINDING_HXX
#include "cellvaluebinding.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEDATA_HPP_
#include <com/sun/star/sheet/XCellRangeData.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif

//.........................................................................
namespace calc
{
//.........................................................................

#define PROP_HANDLE_BOUND_CELL  1

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::table;
    using namespace ::com::sun::star::text;
    using namespace ::com::sun::star::sheet;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::drafts::com::sun::star::form;

    //=====================================================================
    //= OCellValueBinding
    //=====================================================================
    DBG_NAME( OCellValueBinding )
    //---------------------------------------------------------------------
#ifdef DBG_UTIL
    const char* OCellValueBinding::checkConsistency_static( const void* _pThis )
    {
        return static_cast< const OCellValueBinding* >( _pThis )->checkConsistency( );
    }

    const char* OCellValueBinding::checkConsistency( ) const
    {
        const char* pAssertion = NULL;
        if ( m_xCellText.is() && !m_xCell.is() )
            // there are places (e.g. getSupportedTypes) which rely on the fact
            // that m_xCellText.is() implies m_xCell.is()
            pAssertion = "cell references inconsistent!";

        // TODO: place any additional checks here to ensure consistency of this instance
        return pAssertion;
    }
#endif

    //---------------------------------------------------------------------
    OCellValueBinding::OCellValueBinding( const Reference< XSpreadsheetDocument >& _rxDocument, sal_Bool _bListPos )
        :OCellValueBinding_Base( m_aMutex )
        ,OCellValueBinding_PBase( OCellValueBinding_Base::rBHelper )
        ,m_bListPos( _bListPos )
        ,m_xDocument( _rxDocument )
        ,m_aModifyListeners( m_aMutex )
        ,m_bInitialized( sal_False )
    {
        DBG_CTOR( OCellValueBinding, checkConsistency_static );

        // register our property at the base class
        CellAddress aInitialPropValue;
        registerPropertyNoMember(
            ::rtl::OUString::createFromAscii( "BoundCell" ),
            PROP_HANDLE_BOUND_CELL,
            PropertyAttribute::BOUND | PropertyAttribute::READONLY,
            ::getCppuType( &aInitialPropValue ),
            &aInitialPropValue
        );

        // TODO: implement a ReadOnly property as required by the service,
        // which probably maps to the cell being locked
    }

    //---------------------------------------------------------------------
    OCellValueBinding::~OCellValueBinding( )
    {
        if ( !OCellValueBinding_Base::rBHelper.bDisposed )
        {
            acquire();  // prevent duplicate dtor
            dispose();
        }

        DBG_DTOR( OCellValueBinding, checkConsistency_static );
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OCellValueBinding, OCellValueBinding_Base, OCellValueBinding_PBase )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCellValueBinding, OCellValueBinding_Base, OCellValueBinding_PBase )

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::disposing()
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        Reference<XModifyBroadcaster> xBroadcaster( m_xCell, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeModifyListener( this );
        }

//        OCellValueBinding_Base::disposing();
        WeakAggComponentImplHelperBase::disposing();

        // TODO: clean up here whatever you need to clean up (e.g. deregister as XEventListener
        // for the cell)
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL OCellValueBinding::getPropertySetInfo(  ) throw(RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        return createPropertySetInfo( getInfoHelper() ) ;
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL OCellValueBinding::getInfoHelper()
    {
        return *OCellValueBinding_PABase::getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OCellValueBinding::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        DBG_ASSERT( _nHandle == PROP_HANDLE_BOUND_CELL, "OCellValueBinding::getFastPropertyValue: invalid handle!" );
            // we only have this one property ....

        _rValue.clear();
        Reference< XCellAddressable > xCellAddress( m_xCell, UNO_QUERY );
        if ( xCellAddress.is() )
            _rValue <<= xCellAddress->getCellAddress( );
    }

    //--------------------------------------------------------------------
    Sequence< Type > SAL_CALL OCellValueBinding::getSupportedValueTypes(  ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        checkDisposed( );
        checkInitialized( );

        sal_Int32 nCount = m_xCellText.is() ? 3 : m_xCell.is() ? 1 : 0;
        if ( m_bListPos )
            ++nCount;

        Sequence< Type > aTypes( nCount );
        if ( m_xCell.is() )
        {
            // an XCell can be used to set/get "double" values
            aTypes[0] = ::getCppuType( static_cast< double* >( NULL ) );
            if ( m_xCellText.is() )
            {
                // an XTextRange can be used to set/get "string" values
                aTypes[1] = ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) );
                // and additionally, we use it to handle booleans
                aTypes[2] = ::getCppuType( static_cast< sal_Bool* >( NULL ) );
            }

            // add sal_Int32 only if constructed as ListPositionCellBinding
            if ( m_bListPos )
                aTypes[nCount-1] = ::getCppuType( static_cast< sal_Int32* >( NULL ) );
        }

        return aTypes;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OCellValueBinding::supportsType( const Type& aType ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        checkDisposed( );
        checkInitialized( );

        // look up in our sequence
        Sequence< Type > aSupportedTypes( getSupportedValueTypes() );
        const Type* pTypes = aSupportedTypes.getConstArray();
        const Type* pTypesEnd = aSupportedTypes.getConstArray() + aSupportedTypes.getLength();
        while ( pTypes != pTypesEnd )
            if ( aType.equals( *pTypes++ ) )
                return sal_True;

        return sal_False;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OCellValueBinding::getValue( const Type& aType ) throw (IncompatibleTypesException, RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        checkDisposed( );
        checkInitialized( );
        checkValueType( aType );

        Any aReturn;
        switch ( aType.getTypeClass() )
        {
        case TypeClass_STRING:
            DBG_ASSERT( m_xCellText.is(), "OCellValueBinding::getValue: don't have a text!" );
            if ( m_xCellText.is() )
                aReturn <<= m_xCellText->getString();
            else
                aReturn <<= ::rtl::OUString();
            break;

        case TypeClass_BOOLEAN:
            DBG_ASSERT( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
            {
                // check if the cell has a numeric value (this might go into a helper function):

                sal_Bool bHasValue = sal_False;
                CellContentType eCellType = m_xCell->getType();
                if ( eCellType == CellContentType_VALUE )
                    bHasValue = sal_True;
                else if ( eCellType == CellContentType_FORMULA )
                {
                    // check if the formula result is a value
                    if ( m_xCell->getError() == 0 )
                    {
                        Reference<XPropertySet> xProp( m_xCell, UNO_QUERY );
                        if ( xProp.is() )
                        {
                            Any aTypeAny = xProp->getPropertyValue(
                                ::rtl::OUString::createFromAscii( "FormulaResultType" ) );
                            CellContentType eResultType;
                            if ( (aTypeAny >>= eResultType) && eResultType == CellContentType_VALUE )
                                bHasValue = sal_True;
                        }
                    }
                }

                if ( bHasValue )
                {
                    // 0 is "unchecked", any other value is "checked", regardless of number format
                    double nCellValue = m_xCell->getValue();
                    sal_Bool bBoolValue = ( nCellValue != 0.0 );
                    aReturn <<= bBoolValue;
                }
                // empty cells, text cells and text or error formula results: leave return value empty
            }
            break;

        case TypeClass_DOUBLE:
            DBG_ASSERT( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
                aReturn <<= m_xCell->getValue();
            else
                aReturn <<= (double)0;
            break;

        case TypeClass_LONG:
            DBG_ASSERT( m_xCell.is(), "OCellValueBinding::getValue: don't have a double value supplier!" );
            if ( m_xCell.is() )
            {
                // The list position value in the cell is 1-based.
                // We subtract 1 from any cell value (no special handling for 0 or negative values).

                sal_Int32 nValue = (sal_Int32) rtl::math::approxFloor( m_xCell->getValue() );
                --nValue;

                aReturn <<= nValue;
            }
            else
                aReturn <<= (sal_Int32)0;
            break;

        default:
            DBG_ERROR( "OCellValueBinding::getValue: unreachable code!" );
                // a type other than double and string should never have survived the checkValueType
                // above
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::setValue( const Any& aValue ) throw (IncompatibleTypesException, NoSupportException, RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );
        checkDisposed( );
        checkInitialized( );
        if ( aValue.hasValue() )
            checkValueType( aValue.getValueType() );

        switch ( aValue.getValueType().getTypeClass() )
        {
        case TypeClass_STRING:
            {
                DBG_ASSERT( m_xCellText.is(), "OCellValueBinding::setValue: don't have a text!" );

                ::rtl::OUString sText;
                aValue >>= sText;
                if ( m_xCellText.is() )
                    m_xCellText->setString( sText );
            }
            break;

        case TypeClass_BOOLEAN:
            {
                DBG_ASSERT( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                // boolean is stored as values 0 or 1
                // TODO: set the number format to boolean if no format is set?

                sal_Bool bValue( sal_False );
                aValue >>= bValue;
                double nCellValue = bValue ? 1.0 : 0.0;

                if ( m_xCell.is() )
                    m_xCell->setValue( nCellValue );

                setBooleanFormat();
            }
            break;

        case TypeClass_DOUBLE:
            {
                DBG_ASSERT( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                double nValue;
                aValue >>= nValue;
                if ( m_xCell.is() )
                    m_xCell->setValue( nValue );
            }
            break;

        case TypeClass_LONG:
            {
                DBG_ASSERT( m_xCell.is(), "OCellValueBinding::setValue: don't have a double value supplier!" );

                sal_Int32 nValue;
                aValue >>= nValue;      // list index from control layer (0-based)
                ++nValue;               // the list position value in the cell is 1-based
                if ( m_xCell.is() )
                    m_xCell->setValue( nValue );
            }
            break;

        case TypeClass_VOID:
            {
                // #N/A error value can only be set using XCellRangeData

                Reference<XCellRangeData> xData( m_xCell, UNO_QUERY );
                DBG_ASSERT( xData.is(), "OCellValueBinding::setValue: don't have XCellRangeData!" );
                if ( xData.is() )
                {
                    Sequence<Any> aInner(1);                            // one empty element
                    Sequence< Sequence<Any> > aOuter( &aInner, 1 );     // one row
                    xData->setDataArray( aOuter );
                }
            }
            break;

        default:
            DBG_ERROR( "OCellValueBinding::setValue: unreachable code!" );
                // a type other than double and string should never have survived the checkValueType
                // above
        }
    }
    //--------------------------------------------------------------------
    void OCellValueBinding::setBooleanFormat()
    {
        // set boolean number format if not already set

        ::rtl::OUString sPropName( ::rtl::OUString::createFromAscii( "NumberFormat" ) );
        Reference<XPropertySet> xCellProp( m_xCell, UNO_QUERY );
        Reference<XNumberFormatsSupplier> xSupplier( m_xDocument, UNO_QUERY );
        if ( xSupplier.is() && xCellProp.is() )
        {
            Reference<XNumberFormats> xFormats = xSupplier->getNumberFormats();
            Reference<XNumberFormatTypes> xTypes( xFormats, UNO_QUERY );
            if ( xTypes.is() )
            {
                Locale aLocale;
                sal_Bool bWasBoolean = sal_False;

                sal_Int32 nOldIndex = ::comphelper::getINT32( xCellProp->getPropertyValue( sPropName ) );
                Reference<XPropertySet> xOldFormat;
                try
                {
                    xOldFormat = xFormats->getByKey( nOldIndex );
                }
                catch ( Exception& )
                {
                    // non-existing format - can happen, use defaults
                }
                if ( xOldFormat.is() )
                {
                    // use the locale of the existing format
                    xOldFormat->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aLocale;

                    sal_Int16 nOldType = ::comphelper::getINT16(
                        xOldFormat->getPropertyValue( ::rtl::OUString::createFromAscii( "Type" ) ) );
                    if ( nOldType & NumberFormat::LOGICAL )
                        bWasBoolean = sal_True;
                }

                if ( !bWasBoolean )
                {
                    sal_Int32 nNewIndex = xTypes->getStandardFormat( NumberFormat::LOGICAL, aLocale );
                    xCellProp->setPropertyValue( sPropName, makeAny( nNewIndex ) );
                }
            }
        }
    }

    //--------------------------------------------------------------------
    void OCellValueBinding::checkDisposed( ) const SAL_THROW( ( DisposedException ) )
    {
        if ( OCellValueBinding_Base::rBHelper.bInDispose || OCellValueBinding_Base::rBHelper.bDisposed )
            throw DisposedException();
            // TODO: is it worth having an error message here?
    }

    //--------------------------------------------------------------------
    void OCellValueBinding::checkInitialized() SAL_THROW( ( RuntimeException ) )
    {
        if ( !m_bInitialized )
            throw RuntimeException();
            // TODO: error message
    }

    //--------------------------------------------------------------------
    void OCellValueBinding::checkValueType( const Type& _rType ) const SAL_THROW( ( IncompatibleTypesException ) )
    {
        OCellValueBinding* pNonConstThis = const_cast< OCellValueBinding* >( this );
        if ( !pNonConstThis->supportsType( _rType ) )
        {
            ::rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM( "The given type (" ) );
            sMessage += _rType.getTypeName();
            sMessage += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ") is not supported by this binding." ) );
                // TODO: localize this error message

            throw IncompatibleTypesException( sMessage, *pNonConstThis );
                // TODO: alternatively use a type converter service for this?
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OCellValueBinding::getImplementationName(  ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sheet.OCellValueBinding" ) );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OCellValueBinding::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        Sequence< ::rtl::OUString > aSupportedServices( getSupportedServiceNames() );
        const ::rtl::OUString* pLookup = aSupportedServices.getConstArray();
        const ::rtl::OUString* pLookupEnd = aSupportedServices.getConstArray() + aSupportedServices.getLength();
        while ( pLookup != pLookupEnd )
            if ( *pLookup++ == _rServiceName )
                return sal_True;

        return sal_False;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OCellValueBinding::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        Sequence< ::rtl::OUString > aServices( m_bListPos ? 3 : 2 );
        aServices[ 0 ] =  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.table.CellValueBinding" ) );
        aServices[ 1 ] =  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.form.ValueBinding" ) );
        if ( m_bListPos )
            aServices[ 2 ] =  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.table.ListPositionCellBinding" ) );
        return aServices;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::addModifyListener( const Reference< XModifyListener >& _rxListener ) throw (RuntimeException)
    {
       if ( _rxListener.is() )
           m_aModifyListeners.addInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::removeModifyListener( const Reference< XModifyListener >& _rxListener ) throw (RuntimeException)
    {
       if ( _rxListener.is() )
           m_aModifyListeners.removeInterface( _rxListener );
    }

    //--------------------------------------------------------------------
    void OCellValueBinding::notifyModified()
    {
        EventObject aEvent;
        aEvent.Source = *this;

        ::cppu::OInterfaceIteratorHelper aIter( m_aModifyListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
                reinterpret_cast< XModifyListener* >( aIter.next() )->modified( aEvent );
            }
            catch( const RuntimeException& )
            {
                // silent this
            }
            catch( const Exception& )
            {
                DBG_ERROR( "OCellValueBinding::notifyModified: caught a (non-runtime) exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::modified( const EventObject& aEvent ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        notifyModified();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::disposing( const EventObject& aEvent ) throw (RuntimeException)
    {
        DBG_CHKTHIS( OCellValueBinding, checkConsistency_static );

        Reference<XInterface> xCellInt( m_xCell, UNO_QUERY );
        if ( xCellInt == aEvent.Source )
        {
            // release references to cell object
            m_xCell.clear();
            m_xCellText.clear();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCellValueBinding::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
    {
        if ( m_bInitialized )
            throw Exception();
            // TODO: error message

        // get the cell address
        CellAddress aAddress;
        sal_Bool bFoundAddress = sal_False;

        const Any* pLoop = _rArguments.getConstArray();
        const Any* pLoopEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; ( pLoop != pLoopEnd ) && !bFoundAddress; ++pLoop )
        {
            NamedValue aValue;
            if ( *pLoop >>= aValue )
            {
                if ( aValue.Name.equalsAscii( "BoundCell" ) )
                {
                    if ( aValue.Value >>= aAddress )
                        bFoundAddress = sal_True;
                }
            }
        }

        if ( !bFoundAddress )
            // TODO: error message
            throw Exception();

        // get the cell object
        try
        {
            // first the sheets collection
            Reference< XIndexAccess > xSheets;
            if ( m_xDocument.is() )
                xSheets = xSheets.query( m_xDocument->getSheets( ) );
            DBG_ASSERT( xSheets.is(), "OCellValueBinding::initialize: could not retrieve the sheets!" );

            if ( xSheets.is() )
            {
                // the concrete sheet
                Reference< XCellRange > xSheet;
                xSheets->getByIndex( aAddress.Sheet ) >>= xSheet;
                DBG_ASSERT( xSheet.is(), "OCellValueBinding::initialize: NULL sheet, but no exception!" );

                // the concrete cell
                if ( xSheet.is() )
                {
                    m_xCell = xSheet->getCellByPosition( aAddress.Column, aAddress.Row );
                    Reference< XCellAddressable > xAddressAccess( m_xCell, UNO_QUERY );
                    DBG_ASSERT( xAddressAccess.is(), "OCellValueBinding::initialize: either NULL cell, or cell without address access!" );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_ERROR( "OCellValueBinding::initialize: caught an exception while retrieving the cell object!" );
        }

        if ( !m_xCell.is() )
            throw Exception();
            // TODO error message

        m_xCellText = m_xCellText.query( m_xCell );

        Reference<XModifyBroadcaster> xBroadcaster( m_xCell, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->addModifyListener( this );
        }

        // TODO: add as XEventListener to the cell, so we get notified when it dies,
        // and can dispose ourself then

        // TODO: somehow add as listener so we get notified when the address of the cell changes
        // We need to forward this as change in our BoundCell property to our property change listeners

        // TODO: be an XModifyBroadcaster, so that changes in our cell can be notified
        // to the BindableValue which is/will be bound to this instance.

        m_bInitialized = sal_True;
        // TODO: place your code here
    }


//.........................................................................
}   // namespace calc
//.........................................................................
