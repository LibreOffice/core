/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletextfield.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:29:04 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTCOMPONENT_HXX
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#endif

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

