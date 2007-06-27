/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletabbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:20:24 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBAR_HXX_
#define ACCESSIBILITY_EXT_ACCESSIBLETABBAR_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX
#include "accessibility/extended/accessibletabbarbase.hxx"
#endif

#include <vector>

namespace utl {
class AccessibleStateSetHelper;
}

//.........................................................................
namespace accessibility
{
//.........................................................................

    //  ----------------------------------------------------
    //  class AccessibleTabBar
    //  ----------------------------------------------------

    typedef ::cppu::ImplHelper2<
        ::com::sun::star::accessibility::XAccessible,
        ::com::sun::star::lang::XServiceInfo > AccessibleTabBar_BASE;

    class AccessibleTabBar :    public AccessibleTabBarBase,
                                public AccessibleTabBar_BASE
    {
    private:
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > AccessibleChildren;

        AccessibleChildren      m_aAccessibleChildren;

    protected:
        virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
        virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

        // OCommonAccessibleComponent
        virtual ::com::sun::star::awt::Rectangle SAL_CALL   implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL   disposing();

    public:
        AccessibleTabBar( TabBar* pTabBar );
        ~AccessibleTabBar();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

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
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

        // XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleExtendedComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace accessibility
//.........................................................................

#endif // ACCESSIBILITY_EXT_ACCESSIBLETABBAR_HXX_

