/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accselectionhelper.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:46:56 $
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
#ifndef _ACCSELECTIONHELPER_HXX_
#define _ACCSELECTIONHELPER_HXX_
class SwAccessibleContext;
class SwFEShell;
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

class SwAccessibleSelectionHelper
{
    /// the context on which this helper works
    SwAccessibleContext& rContext;


    /// get FE-Shell
    SwFEShell* GetFEShell();

    void throwIndexOutOfBoundsException()
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );

public:

    SwAccessibleSelectionHelper( SwAccessibleContext& rContext );
    ~SwAccessibleSelectionHelper();


    //=====  XAccessibleSelection  ============================================

    void selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    sal_Bool isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    void clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    void selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    sal_Int32 getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    // --> OD 2004-11-16 #111714# - index has to be treated as global child index.
    void deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
};


#endif

