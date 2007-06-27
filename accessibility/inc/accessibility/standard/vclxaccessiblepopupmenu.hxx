/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblepopupmenu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:27:21 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEPOPUPMENU_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEPOPUPMENU_HXX

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUCOMPONENT_HXX
#include <accessibility/standard/accessiblemenucomponent.hxx>
#endif


//  ----------------------------------------------------
//  class VCLXAccessiblePopupMenu
//  ----------------------------------------------------

class VCLXAccessiblePopupMenu : public OAccessibleMenuComponent
{
protected:
    virtual sal_Bool        IsFocused();

public:
    VCLXAccessiblePopupMenu( Menu* pMenu );
    virtual ~VCLXAccessiblePopupMenu();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEPOPUPMENU_HXX
