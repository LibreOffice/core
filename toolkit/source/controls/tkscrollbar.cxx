/*************************************************************************
 *
 *  $RCSfile: tkscrollbar.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:56:17 $
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

#ifndef TOOLKIT_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX
#include "toolkit/controls/tkscrollbar.hxx"
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include "toolkit/helper/property.hxx"
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include "toolkit/helper/unopropertyarrayhelper.hxx"
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

    using namespace ::com::sun::star;

    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoControlScrollBarModel::UnoControlScrollBarModel()
    {
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BLOCKINCREMENT );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_ENABLED );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_LINEINCREMENT );
        ImplRegisterProperty( BASEPROPERTY_LIVE_SCROLL );
        ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_REPEAT_DELAY );
        ImplRegisterProperty( BASEPROPERTY_SCROLLVALUE );
        ImplRegisterProperty( BASEPROPERTY_SCROLLVALUE_MAX );
        ImplRegisterProperty( BASEPROPERTY_SCROLLVALUE_MIN );
        ImplRegisterProperty( BASEPROPERTY_SYMBOL_COLOR );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
        ImplRegisterProperty( BASEPROPERTY_VISIBLESIZE );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoControlScrollBarModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBarModel );
    }

    //--------------------------------------------------------------------
    uno::Any UnoControlScrollBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_LIVE_SCROLL:
            return uno::makeAny( (sal_Bool)sal_False );
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBar ) );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoControlScrollBarModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    uno::Reference< beans::XPropertySetInfo > UnoControlScrollBarModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
    {
        static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    UnoScrollBarControl::UnoScrollBarControl()
        : maAdjustmentListeners( *this )
    {
    }

    ::rtl::OUString UnoScrollBarControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "ScrollBar" );
    }

    // ::com::sun::star::uno::XInterface
    uno::Any UnoScrollBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                            SAL_STATIC_CAST( awt::XAdjustmentListener*, this ),
                                            SAL_STATIC_CAST( awt::XScrollBar*, this ) );
        return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
    }

    // ::com::sun::star::lang::XTypeProvider
    IMPL_XTYPEPROVIDER_START( UnoScrollBarControl )
        getCppuType( ( uno::Reference< awt::XAdjustmentListener>* ) NULL ),
        getCppuType( ( uno::Reference< awt::XScrollBar>* ) NULL ),
        UnoControlBase::getTypes()
    IMPL_XTYPEPROVIDER_END

    void UnoScrollBarControl::dispose() throw(uno::RuntimeException)
    {
        lang::EventObject aEvt;
        aEvt.Source = (::cppu::OWeakObject*)this;
        maAdjustmentListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }

    void UnoScrollBarControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        uno::Reference < awt::XScrollBar >  xScrollBar( getPeer(), uno::UNO_QUERY );
        xScrollBar->addAdjustmentListener( this );
    }

    // ::com::sun::star::awt::XAdjustmentListener
    void UnoScrollBarControl::adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
    {
        switch ( rEvent.Type )
        {
            case ::com::sun::star::awt::AdjustmentType_ADJUST_LINE:
            case ::com::sun::star::awt::AdjustmentType_ADJUST_PAGE:
            case ::com::sun::star::awt::AdjustmentType_ADJUST_ABS:
            {
                uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );

                if ( xScrollBar.is() )
                {
                    uno::Any aAny;
                    aAny <<= xScrollBar->getValue();
                    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_False );
                }
            }
            break;
            default:
            {
                DBG_ERROR( "UnoScrollBarControl::adjustmentValueChanged - unknown Type" );

            }
        }

        if ( maAdjustmentListeners.getLength() )
            maAdjustmentListeners.adjustmentValueChanged( rEvent );
    }

    // ::com::sun::star::awt::XScrollBar
    void UnoScrollBarControl::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
    {
        maAdjustmentListeners.addInterface( l );
    }

    void UnoScrollBarControl::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
    {
        maAdjustmentListeners.removeInterface( l );
    }

    void UnoScrollBarControl::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), uno::makeAny( n ), sal_True );
    }

    void UnoScrollBarControl::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException)
    {
        uno::Any aAny;
        aAny <<= nValue;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_True );
        aAny <<= nVisible;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), aAny, sal_True );
        aAny <<= nMax;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), aAny, sal_True );
    }

    sal_Int32 UnoScrollBarControl::getValue() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getValue();
        }
        return n;
    }

    void UnoScrollBarControl::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getMaximum() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getMaximum();
        }
        return n;
    }

    void UnoScrollBarControl::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINEINCREMENT ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getLineIncrement() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getLineIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_BLOCKINCREMENT ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getBlockIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getVisibleSize() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getVisibleSize();
        }
        return n;
    }

    void UnoScrollBarControl::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getOrientation() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getOrientation();
        }
        return n;
    }



//........................................................................
}  // namespace toolkit
//........................................................................

