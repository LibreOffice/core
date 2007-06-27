/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblemenuitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:27:01 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUITEM_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUITEM_HXX

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUITEMCOMPONENT_HXX
#include <accessibility/standard/accessiblemenuitemcomponent.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_ACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
#endif


//  ----------------------------------------------------
//  class VCLXAccessibleMenuItem
//  ----------------------------------------------------

typedef ::cppu::ImplHelper3<
    ::com::sun::star::accessibility::XAccessibleText,
    ::com::sun::star::accessibility::XAccessibleAction,
    ::com::sun::star::accessibility::XAccessibleValue > VCLXAccessibleMenuItem_BASE;

class VCLXAccessibleMenuItem :  public OAccessibleMenuItemComponent,
                                public ::comphelper::OCommonAccessibleText,
                                public VCLXAccessibleMenuItem_BASE
{
protected:
    virtual sal_Bool        IsFocused();
    virtual sal_Bool        IsSelected();
    virtual sal_Bool        IsChecked();

    virtual sal_Bool        IsHighlighted();

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleText
    virtual ::rtl::OUString                     implGetText();
    virtual ::com::sun::star::lang::Locale      implGetLocale();
    virtual void                                implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );

public:
    VCLXAccessibleMenuItem( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu = 0 );
    virtual ~VCLXAccessibleMenuItem();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCharacterCount() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSelectedText() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectionStart() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSelectionEnd() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleValue
    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCurrentValue( const ::com::sun::star::uno::Any& aNumber ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUITEM_HXX

