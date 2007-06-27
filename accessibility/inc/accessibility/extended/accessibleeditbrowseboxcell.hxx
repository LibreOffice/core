/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibleeditbrowseboxcell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:19:26 $
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
#ifndef ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX
#define ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX

#ifndef ACCESSIBILITY_EXT_BROWSE_BOX_CELL_HXX
#include "accessiblebrowseboxcell.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEACTION_HPP_
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLERELATIONSET_HPP_
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_WRAPPER_HXX
#include <comphelper/accessiblewrapper.hxx>
#endif

namespace accessibility
{
    // =============================================================================
    // = EditBrowseBoxTableCell
    // =============================================================================
    class EditBrowseBoxTableCell    :public AccessibleBrowseBoxCell
                                    ,public ::comphelper::OAccessibleContextWrapperHelper
    {
    public:
        EditBrowseBoxTableCell(
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _xControlChild,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCell();

    protected:
        // XAccessibleComponent
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException) ;

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );

        // XInterface
        DECLARE_XINTERFACE( )
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        sal_Int16 SAL_CALL getAccessibleRole() throw ( ::com::sun::star::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XComponent/OComponentProxyAggregationHelper (needs to be disambiguated)
        virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

        // OAccessibleContextWrapperHelper();
        void notifyTranslatedEvent( const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

    private:
        EditBrowseBoxTableCell();                                           // never implemented
        EditBrowseBoxTableCell( const EditBrowseBoxTableCell& );            // never implemented
        EditBrowseBoxTableCell& operator=( const EditBrowseBoxTableCell& ); // never implemented
    };

    // =============================================================================
    // = EditBrowseBoxTableCell
    // =============================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::accessibility::XAccessible
                                                >   EditBrowseBoxTableCellAccess_Base;
    // XAccessible providing an EditBrowseBoxTableCell
    class EditBrowseBoxTableCellAccess
                        :public ::comphelper::OBaseMutex
                        ,public EditBrowseBoxTableCellAccess_Base
    {
    protected:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessibleContext >
                                            m_aContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                            m_xParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                            m_xControlAccessible;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                                            m_xFocusWindow;
        ::svt::IAccessibleTableProvider*    m_pBrowseBox;
        sal_Int32                           m_nRowPos;
        sal_uInt16                          m_nColPos;

    public:
        EditBrowseBoxTableCellAccess(
            const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible >& _rxParent,
            const ::com::sun::star::uno::Reference< com::sun::star::accessibility::XAccessible > _xControlAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
            ::svt::IAccessibleTableProvider& _rBrowseBox,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        );

    protected:
        virtual ~EditBrowseBoxTableCellAccess();

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent/OComponentHelper
        virtual void SAL_CALL disposing();

    private:
        EditBrowseBoxTableCellAccess();                                                 // never implemented
        EditBrowseBoxTableCellAccess( const EditBrowseBoxTableCellAccess& );            // never implemented
        EditBrowseBoxTableCellAccess& operator=( const EditBrowseBoxTableCellAccess& ); // never implemented
    };
}

#endif // ACCESSIBILITY_EXT_ACCESSIBILEEDITBROWSEBOXTABLECELL_HXX

