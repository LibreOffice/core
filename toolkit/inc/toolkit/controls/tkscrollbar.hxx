/*************************************************************************
 *
 *  $RCSfile: tkscrollbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-12-11 11:54:48 $
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
#define TOOLKIT_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

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

#ifndef _COM_SUN_STAR_AWT_XSCROLLBAR_HPP_
#include <com/sun/star/awt/XScrollBar.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XADJUSTMENTLISTENER_HPP_
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_ADJUSTMENTTYPE_HPP_
#include <com/sun/star/awt/AdjustmentType.hpp>
#endif

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
