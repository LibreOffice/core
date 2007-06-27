/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibledropdownlistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:25:19 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBOX_HXX
#include <accessibility/standard/vclxaccessiblebox.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX
#include <cppuhelper/weakref.hxx>
#endif


/** The accessible drop down combobox has two children.  The first is the
    text field represented by an object of the
    <type>VCLXAccessibleTextField</type> class which can not be edited.  The
    second is the list containing all items and is represented by an object
    of the <type>VCLXAccessibleListBoxList</type> class which does support
    selection.
*/
class VCLXAccessibleDropDownListBox : public VCLXAccessibleBox
{
public:
    VCLXAccessibleDropDownListBox (VCLXWindow* pVCLXindow);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return drop down list box specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ~VCLXAccessibleDropDownListBox (void);

    virtual bool IsValid (void) const;
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);

};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX

