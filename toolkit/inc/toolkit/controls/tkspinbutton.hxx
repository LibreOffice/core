/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkspinbutton.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:51:18 $
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
#ifndef TOOLKIT_CONTROLS_TKSPINBUTTON_HXX
#define TOOLKIT_CONTROLS_TKSPINBUTTON_HXX

#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif
#ifndef _TOOLKIT_AWT_UNOCONTROLBASE_HXX_
#include <toolkit/controls/unocontrolbase.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XSPINVALUE_HPP_
#include <com/sun/star/awt/XSpinValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XADJUSTMENTLISTENER_HPP_
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= UnoSpinButtonModel
    //====================================================================
    class UnoSpinButtonModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

    public:
                            UnoSpinButtonModel();
                            UnoSpinButtonModel( const UnoSpinButtonModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoSpinButtonModel( *this ); }

        // XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= UnoSpinButtonControl
    //====================================================================

    typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XAdjustmentListener
                                ,   ::com::sun::star::awt::XSpinValue
                                >   UnoSpinButtonControl_Base;

    class UnoSpinButtonControl :public UnoControlBase
                               ,public UnoSpinButtonControl_Base
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoSpinButtonControl();
        ::rtl::OUString             GetComponentServiceName();

        DECLARE_UNO3_AGG_DEFAULTS( UnoSpinButtonControl, UnoControlBase );
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

        // XSpinValue
        virtual void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( sal_Int32 value ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getValue(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMinimum( sal_Int32 minValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMaximum( sal_Int32 maxValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMinimum(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMaximum(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSpinIncrement( sal_Int32 spinIncrement ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSpinIncrement(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getOrientation(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };



//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_CONTROLS_TKSPINBUTTON_HXX
