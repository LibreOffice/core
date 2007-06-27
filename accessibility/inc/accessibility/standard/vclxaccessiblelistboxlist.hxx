/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblelistboxlist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:26:18 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOXLIST_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOXLIST_HXX

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELIST_HXX
#include <accessibility/standard/vclxaccessiblelist.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
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
    USHORT      m_nLastSelectedPos;
    bool        m_bDisableProcessEvent;

    void UpdateEntryRange_Impl (void);
    using VCLXAccessibleList::UpdateSelection_Impl;
    void UpdateSelection_Impl (void);
    using VCLXAccessibleList::checkEntrySelected;
    BOOL checkEntrySelected(ListBox* _pListBox,
                            USHORT _nPos,
                            ::com::sun::star::uno::Any& _rNewValue,
                            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxNewAcc);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOX_HXX

