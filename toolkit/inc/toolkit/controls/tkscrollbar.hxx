/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tkscrollbar.hxx,v $
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
#ifndef TOOLKIT_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX
#define TOOLKIT_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#include <com/sun/star/awt/AdjustmentType.hpp>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    class UnoControlScrollBarModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

    public:
                            UnoControlScrollBarModel();
                            UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoControlScrollBarModel( *this ); }

        // ::com::sun::star::beans::XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::io::XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        DECLIMPL_SERVICEINFO_DERIVED( UnoControlScrollBarModel, UnoControlModel, szServiceName2_UnoControlScrollBarModel )
    };

    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    class UnoScrollBarControl : public UnoControlBase,
                                public ::com::sun::star::awt::XAdjustmentListener,
                                public ::com::sun::star::awt::XScrollBar
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoScrollBarControl();
        ::rtl::OUString             GetComponentServiceName();

        ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlBase::queryInterface(rType); }
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
        void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XTypeProvider
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::awt::XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::awt::XScrollBar
        void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getMaximum(  ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getLineIncrement(  ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getBlockIncrement(  ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getVisibleSize(  ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getOrientation(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XServiceInfo
        DECLIMPL_SERVICEINFO_DERIVED( UnoScrollBarControl, UnoControlBase, szServiceName2_UnoControlScrollBar )
    };



//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX
