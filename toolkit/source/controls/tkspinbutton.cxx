/*************************************************************************
 *
 *  $RCSfile: tkspinbutton.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:56:32 $
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

#ifndef TOOLKIT_CONTROLS_TKSPINBUTTON_HXX
#include "toolkit/controls/tkspinbutton.hxx"
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include "toolkit/helper/property.hxx"
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif


#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= UnoSpinButtonModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoSpinButtonModel::UnoSpinButtonModel()
    {
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_ENABLED );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_REPEAT );
        ImplRegisterProperty( BASEPROPERTY_REPEAT_DELAY );
        ImplRegisterProperty( BASEPROPERTY_SYMBOL_COLOR );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE_MIN );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE_MAX );
        ImplRegisterProperty( BASEPROPERTY_SPININCREMENT );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSpinButtonModel::getServiceName( ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonModel );
    }

    //--------------------------------------------------------------------
    Any UnoSpinButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonControl ) );

        case BASEPROPERTY_BORDER:
            return makeAny( (sal_Int16) 0 );

        case BASEPROPERTY_REPEAT:
            return makeAny( (sal_Bool)sal_True );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoSpinButtonModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoSpinButtonModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSpinButtonModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSpinButtonModel" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSpinButtonModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonModel );
        return aServices;
    }

    //====================================================================
    //= UnoSpinButtonControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoSpinButtonControl::UnoSpinButtonControl()
        :maAdjustmentListeners( *this )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSpinButtonControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "SpinButton" );
    }

    //--------------------------------------------------------------------
    Any UnoSpinButtonControl::queryAggregation( const Type & rType ) throw(RuntimeException)
    {
        Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoSpinButtonControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoSpinButtonControl, UnoControlBase, UnoSpinButtonControl_Base )

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::dispose() throw(RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        if ( maAdjustmentListeners.getLength() )
        {
            Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
            if ( xSpinnable.is() )
                xSpinnable->removeAdjustmentListener( this );

            EventObject aDisposeEvent;
            aDisposeEvent.Source = *this;

            aGuard.clear();
            maAdjustmentListeners.disposeAndClear( aDisposeEvent );
        }

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSpinButtonControl::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSpinButtonControl" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSpinButtonControl::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        Reference < XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            xSpinnable->addAdjustmentListener( this );
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::adjustmentValueChanged( const AdjustmentEvent& rEvent ) throw(RuntimeException)
    {
        switch ( rEvent.Type )
        {
            case AdjustmentType_ADJUST_LINE:
            case AdjustmentType_ADJUST_PAGE:
            case AdjustmentType_ADJUST_ABS:
                ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( rEvent.Value ), sal_False );
                break;
            default:
                DBG_ERROR( "UnoSpinButtonControl::adjustmentValueChanged - unknown Type" );
        }

        if ( maAdjustmentListeners.getLength() )
        {
            AdjustmentEvent aEvent( rEvent );
            aEvent.Source = *this;
            maAdjustmentListeners.adjustmentValueChanged( aEvent );
        }
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::addAdjustmentListener( const Reference< XAdjustmentListener > & listener ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maAdjustmentListeners.addInterface( listener );
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::removeAdjustmentListener( const Reference< XAdjustmentListener > & listener ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maAdjustmentListeners.removeInterface( listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setValue( sal_Int32 value ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( value ), sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MIN ), makeAny( minValue ), sal_True );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MAX ), makeAny( maxValue ), sal_True );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( currentValue ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nValue = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nValue = xSpinnable->getValue();

        return nValue;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setMinimum( sal_Int32 minValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MIN ), makeAny( minValue ), sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setMaximum( sal_Int32 maxValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MAX ), makeAny( maxValue ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getMinimum(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nMin = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nMin = xSpinnable->getMinimum();

        return nMin;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getMaximum(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nMax = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nMax = xSpinnable->getMaximum();

        return nMax;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setSpinIncrement( sal_Int32 spinIncrement ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPININCREMENT ), makeAny( spinIncrement ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getSpinIncrement(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nIncrement = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nIncrement = xSpinnable->getSpinIncrement();

        return nIncrement;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setOrientation( sal_Int32 orientation ) throw (NoSupportException, RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), makeAny( orientation ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getOrientation(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nOrientation = ScrollBarOrientation::HORIZONTAL;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nOrientation = xSpinnable->getOrientation();

        return nOrientation;
    }

//........................................................................
}  // namespace toolkit
//........................................................................

