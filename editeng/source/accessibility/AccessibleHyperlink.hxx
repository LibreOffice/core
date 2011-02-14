/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleEditableTextPara.hxx,v $
 * $Revision: 1.22 $
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

#ifndef _SVX_ACCESSIBLE_HYPERLINK_HXX
#define _SVX_ACCESSIBLE_HYPERLINK_HXX

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>

#include <tools/solar.h>

class SvxFieldItem;
class SvxAccessibleTextAdapter;

namespace accessibility
{

    class AccessibleHyperlink : public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleHyperlink >
    {
    private:

        SvxAccessibleTextAdapter& rTA;
        SvxFieldItem* pFld;
        sal_uInt16 nPara, nRealIdx;  // EE values
        sal_Int32 nStartIdx, nEndIdx;   // translated values
        ::rtl::OUString aDescription;

    public:
        AccessibleHyperlink( SvxAccessibleTextAdapter& r, SvxFieldItem* p, sal_uInt16 nP, sal_uInt16 nR, sal_Int32 nStt, sal_Int32 nEnd, const ::rtl::OUString& rD );
        ~AccessibleHyperlink();

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleHyperlink
        virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionAnchor( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionObject( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getStartIndex() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getEndIndex() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isValid() throw (::com::sun::star::uno::RuntimeException);
    };

} // end of namespace accessibility

#endif

