/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleTextHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:52:00 $
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
#ifndef CHART2_ACCESSIBLETEXTHELPER_HXX
#define CHART2_ACCESSIBLETEXTHELPER_HXX

#include "MutexContainer.hxx"

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif

// forward declaration of helper class from svx
namespace accessibility
{
class AccessibleTextHelper;
}

namespace chart
{

class DrawViewWrapper;

namespace impl
{
typedef ::cppu::WeakComponentImplHelper2<
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::accessibility::XAccessibleContext >
    AccessibleTextHelper_Base;
}

class AccessibleTextHelper :
        public MutexContainer,
        public impl::AccessibleTextHelper_Base
{
public:
    explicit AccessibleTextHelper( DrawViewWrapper * pDrawViewWrapper );
    virtual ~AccessibleTextHelper();

    // ____ XInitialization ____
    /** Must be called at least once for this helper class to work.

        mandatory parameter 0: type string. This is the CID that is used to find
            the corresponding drawing object that contains the text that should
            be handled by this helper class.
1
        mandatory parameter 1: type XAccessible.  Is used as EventSource for the
            ::accessibility::AccessibleTextHelper (svx)

        mandatory parameter 2: type awt::XWindow.  The Window that shows the
            text currently.
     */
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);

    // ____ XAccessibleContext ____
    virtual ::sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild(
        ::sal_Int32 i )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException,
               ::com::sun::star::uno::RuntimeException);

private:
    ::accessibility::AccessibleTextHelper * m_pTextHelper;
    DrawViewWrapper *                       m_pDrawViewWrapper;
};

} //  namespace chart

// CHART2_ACCESSIBLETEXTHELPER_HXX
#endif
