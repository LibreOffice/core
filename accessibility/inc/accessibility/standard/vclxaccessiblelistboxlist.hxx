/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOXLIST_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOXLIST_HXX

#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#ifndef _COMPHELPER_BROADCASTHELPER_HXX
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessibleSelection
                            >   VCLXAccessibleListBoxList_BASE;


class ListBox;
/** This class extends the list of the <type>VCLXAccessibleList</type> class
    about selection.
*/
class VCLXAccessibleListBoxList :
    public VCLXAccessibleList,
    public VCLXAccessibleListBoxList_BASE
{
public:
    VCLXAccessibleListBoxList (VCLXWindow* pVCLXindow, BoxType aBoxType,
                               const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::accessibility::XAccessible >& _xParent);

    // XInterface
    DECLARE_XINTERFACE( )

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return list box list specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleSelection
    void        SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    sal_Bool    SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void        SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
    void        SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
    sal_Int32   SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    void        SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    /** Create the specified child and insert it into the list of children.
        Sets the child's states.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        CreateChild (sal_Int32 i);


protected:
    virtual ~VCLXAccessibleListBoxList (void);

    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);

private:
    sal_Int32   m_nLastTopEntry;
    sal_uInt16      m_nLastSelectedPos;
    bool        m_bDisableProcessEvent;

    void UpdateEntryRange_Impl (void);
    using VCLXAccessibleList::UpdateSelection_Impl;
    void UpdateSelection_Impl (void);
    using VCLXAccessibleList::checkEntrySelected;
    sal_Bool checkEntrySelected(ListBox* _pListBox,
                            sal_uInt16 _nPos,
                            ::com::sun::star::uno::Any& _rNewValue,
                            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxNewAcc);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX

