/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletoolboxitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:29:27 $
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
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_ACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATESET_HPP_
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// class VCLXAccessibleToolBoxItem ---------------------------------------------

class ToolBox;

typedef ::comphelper::OAccessibleTextHelper AccessibleTextHelper_BASE;
typedef ::cppu::ImplHelper4 < ::com::sun::star::accessibility::XAccessible,
                              ::com::sun::star::accessibility::XAccessibleAction,
                              ::com::sun::star::accessibility::XAccessibleValue,
                              ::com::sun::star::lang::XServiceInfo > VCLXAccessibleToolBoxItem_BASE;

class VCLExternalSolarLock;

class VCLXAccessibleToolBoxItem : public AccessibleTextHelper_BASE,
                                  public VCLXAccessibleToolBoxItem_BASE
{
private:
    ::rtl::OUString         m_sOldName;
    ToolBox*                m_pToolBox;
    VCLExternalSolarLock*   m_pExternalLock;
    sal_Int32               m_nIndexInParent;
    sal_Int16               m_nRole;
    USHORT                  m_nItemId;
    sal_Bool                m_bHasFocus;
    sal_Bool                m_bIsChecked;
    bool                    m_bIndeterminate;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >    m_xParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >    m_xChild;

public:
    inline sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
    inline void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

protected:
    virtual ~VCLXAccessibleToolBoxItem();

    virtual void SAL_CALL                   disposing();

    /// implements the calculation of the bounding rectangle
    virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

    // OCommonAccessibleText
    virtual ::rtl::OUString                             implGetText();
    virtual ::com::sun::star::lang::Locale              implGetLocale();
    virtual void                                        implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );

    ::rtl::OUString GetText( bool _bAsName );

public:
    VCLXAccessibleToolBoxItem( ToolBox* _pToolBox, sal_Int32 _nPos, const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent );

    void                SetFocus( sal_Bool _bFocus );
    inline sal_Bool     HasFocus() const { return m_bHasFocus; }
    void                SetChecked( sal_Bool _bCheck );
    inline sal_Bool     IsChecked() const { return m_bIsChecked; }
    void                SetIndeterminate( bool _bIndeterminate );
    inline bool         IsIndeterminate() const { return m_bIndeterminate; }
    inline void         ReleaseToolBox() { m_pToolBox = NULL; }
    void                NameChanged();
    void                SetChild( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xChild );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                        GetChild() const { return m_xChild; }
    void                NotifyChildEvent( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xChild, bool _bShow );

    void                ToggleEnableState();

    // XInterface
    DECLARE_XINTERFACE( )
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFontMetrics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& xFont ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);

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

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOXITEM_HXX

