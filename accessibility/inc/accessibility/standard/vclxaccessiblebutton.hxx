/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblebutton.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:24:24 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBUTTON_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBUTTON_HXX

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_ACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif


//  ----------------------------------------------------
//  class VCLXAccessibleButton
//  ----------------------------------------------------

typedef ::cppu::ImplHelper2<
    ::com::sun::star::accessibility::XAccessibleAction,
    ::com::sun::star::accessibility::XAccessibleValue > VCLXAccessibleButton_BASE;

class VCLXAccessibleButton : public VCLXAccessibleTextComponent,
                             public VCLXAccessibleButton_BASE
{
protected:
    virtual ~VCLXAccessibleButton();

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

public:
    VCLXAccessibleButton( VCLXWindow* pVCLXindow );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleValue
    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCurrentValue( const ::com::sun::star::uno::Any& aNumber ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBUTTON_HXX

