/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessibletextfield.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX

#include <accessibility/standard/vclxaccessibletextcomponent.hxx>

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

typedef ::cppu::ImplHelper1<
    ::com::sun::star::accessibility::XAccessible
    > VCLXAccessible_BASE;


/** This class represents non editable text fields.  The object passed to
    the constructor is expected to be a list (a <type>ListBox</type> to be
    more specific).  From this allways the selected item is token to be made
    accessible by this class.  When the selected item changes then also the
    exported text changes.
*/
class VCLXAccessibleTextField :
    public VCLXAccessibleTextComponent,
    public VCLXAccessible_BASE
{
public:
    VCLXAccessibleTextField (VCLXWindow* pVCLXindow,
                             const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::accessibility::XAccessible >& _xParent);

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XAccessible
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    sal_Int32 SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 i)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleParent(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return text field specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ~VCLXAccessibleTextField (void);

    /** With this method the text of the currently selected item is made
        available to the <type>VCLXAccessibleTextComponent</type> base class.
    */
    ::rtl::OUString implGetText (void);

private:
    /** We need to save the accessible parent to return it in <type>getAccessibleParent()</type>,
        because this method of the base class returns the wrong parent.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  m_xParent;
};

#endif

