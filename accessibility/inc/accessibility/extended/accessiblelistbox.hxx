/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblelistbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 14:12:43 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLELISTBOX_HXX_
#define ACCESSIBILITY_EXT_ACCESSIBLELISTBOX_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _VCL_VCLEVENT_HXX
#include <vcl/vclevent.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif


// class AccessibleListBox -----------------------------------------------

class SvTreeListBox;

//........................................................................
namespace accessibility
{
//........................................................................

    typedef ::cppu::ImplHelper2<  ::com::sun::star::accessibility::XAccessible
                                , ::com::sun::star::accessibility::XAccessibleSelection> AccessibleListBox_BASE;

    /** the class OAccessibleListBoxEntry represents the base class for an accessible object of a listbox entry
    */
    class AccessibleListBox :public AccessibleListBox_BASE
                            ,public VCLXAccessibleComponent
    {
    protected:

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xParent;

    protected:
        virtual ~AccessibleListBox();

        // OComponentHelper overridables
        /** this function is called upon disposing the component */
        virtual void SAL_CALL   disposing();

        // VCLXAccessibleComponent
        virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
        virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );
        virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

        SvTreeListBox*  getListBox() const;

    public:
        /** OAccessibleBase needs a valid view
            @param  _rListBox
                is the box for which we implement an accessible object
            @param  _xParent
                is our parent accessible object
        */
        AccessibleListBox( SvTreeListBox& _rListBox,
                           const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent );

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XInterface
        DECLARE_XINTERFACE()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw(com::sun::star::uno::RuntimeException);
        static ::rtl::OUString getImplementationName_Static(void) throw(com::sun::star::uno::RuntimeException);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleSelection
        void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };

//........................................................................
}// namespace accessibility
//........................................................................

#endif // ACCESSIBILITY_EXT_ACCESSIBLELISTBOX_HXX_

