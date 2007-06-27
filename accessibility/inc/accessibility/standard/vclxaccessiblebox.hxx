/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblebox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:24:02 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBOX_HXX

#include <map>

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEEDIT_HXX
#include <accessibility/standard/vclxaccessibleedit.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEKEYBINDING_HPP_
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif


typedef ::cppu::ImplHelper2<
    ::com::sun::star::accessibility::XAccessible,
    ::com::sun::star::accessibility::XAccessibleAction
    > VCLXAccessibleBox_BASE;


/** Base class for list- and combo boxes.  This class manages the box'
    children.  The classed derived from this one have only to implement the
    <member>IsValid</member> method and return the corrent implementation name.
*/
class VCLXAccessibleBox
    : public VCLXAccessibleComponent,
      public VCLXAccessibleBox_BASE
{
public:
    enum BoxType {COMBOBOX, LISTBOX};

    /** The constructor is initialized with the box type whitch may be
        either <const>COMBOBOX</const> or <const>LISTBOX</const> and a flag
        indicating whether the box is a drop down box.
    */
    VCLXAccessibleBox (VCLXWindow* pVCLXindow, BoxType aType, bool bIsDropDownBox);

    /** The index that is passed to this method is returned on following
        calls to <member>getAccessibleIndexInParent</member>.
    */
    void SetIndexInParent (sal_Int32 _nIndex);

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XInterface
    DECLARE_XINTERFACE()



    // XAccessible

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
            getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext

    /** Each object has one or two children: an optional text field and the
        actual list.  The text field is not provided for non drop down list
        boxes.
    */
    sal_Int32 SAL_CALL getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);
    /** For drop down list boxes the text field is a not editable
        <type>VCLXAccessibleTextField</type>, for combo boxes it is an
        editable <type>VLCAccessibleEdit</type>.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 i)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    /** The role is always <const
        scope="com::sun::star::accessibility">AccessibleRole::COMBO_BOX</const>.
    */
    sal_Int16 SAL_CALL getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException);
    /** The index returned as index in parent is always the one set with the
        <member>SetIndexInParent()</member> method.
    */
    sal_Int32 SAL_CALL getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleAction

    /** There is one action for drop down boxes and none for others.
    */
    virtual sal_Int32 SAL_CALL getAccessibleActionCount (void)
        throw (::com::sun::star::uno::RuntimeException);
    /** The action for drop down boxes lets the user toggle the visibility of the
        popup menu.
    */
    virtual sal_Bool SAL_CALL doAccessibleAction (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);
    /** The returned string is assoicated with resource
        <const>RID_STR_ACC_ACTION_TOGGLEPOPUP</const>.
    */
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException);
    /** No keybinding returned so far.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
            getAccessibleActionKeyBinding( sal_Int32 nIndex )
                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                       ::com::sun::star::uno::RuntimeException);

    // XComponent

    /** This method is called from the implementation helper during an
        XComponent::dispose() call.
    */
    virtual void SAL_CALL disposing (void);


protected:
    /** Specifies whether the box is a combo box or a list box.  List boxes
        have multi selection.
    */
    BoxType m_aBoxType;

    /// Specifies whether the box is a drop down box and thus has an action.
    bool m_bIsDropDownBox;

    /// The child that represents the text field if there is one.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>
        m_xText;

    /// The child that contains the items of this box.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>
        m_xList;

    /** This flag specifies whether an object has a text field as child
        regardless of whether that child being currently instantiated or
        not.
    */
    bool m_bHasTextChild;

    /** This flag specifies whether an object has a list as child regardless
        of whether that child being currently instantiated or not.  This
        flag is always true in the current implementation because the list
        child is just another wrapper arround this object and thus has the
        same life time.
    */
    bool m_bHasListChild;

    virtual ~VCLXAccessibleBox (void);

    /** Returns </true> when the object is valid.
    */
    virtual bool IsValid (void) const = 0;

    virtual void ProcessWindowChildEvent (const VclWindowEvent& rVclWindowEvent);
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);


private:
    /// Index in parent.  This is settable from the outside.
    sal_Int32 m_nIndexInParent;
};

#endif

