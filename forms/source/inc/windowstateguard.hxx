/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: windowstateguard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 13:18:58 $
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

#ifndef FORMS_WINDOWSTATEGUARD_HXX
#define FORMS_WINDOWSTATEGUARD_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

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

