/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: windowstateguard.hxx,v $
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

#ifndef FORMS_WINDOWSTATEGUARD_HXX
#define FORMS_WINDOWSTATEGUARD_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
/** === end UNO includes === **/
#include <rtl/ref.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= WindowStateGuard
    //====================================================================
    class WindowStateGuard_Impl;

    /** a helper class which monitors certain states of an XWindow2, and ensures
        that they're consistent with respective properties at an XModel.

        For form controls, window states - such as "Enabled" - can be set by various
        means. You can set the respective control model property, you can directly manipulate
        the XWindow2, or the state can change implicitly due to VCL actions. In any case,
        we need to ensure that the state does not contradict the model property "too much".

        As an example, consider a form control which, according to its model's property, is disabled.
        Now when the parent VCL window of the control's VCL window is enabled, then the the control's
        window is enabled, too - which contradicts the model property.

        A WindowStateGuard helps you preventing such inconsistent states.

        The class is not threadsafe.
    */
    class WindowStateGuard
    {
    private:
        ::rtl::Reference< WindowStateGuard_Impl >   m_pImpl;

    public:
        WindowStateGuard();
        ~WindowStateGuard();

        void    attach(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2 >& _rxWindow,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel
        );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_WINDOWSTATEGUARD_HXX

