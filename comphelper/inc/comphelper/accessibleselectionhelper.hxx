/*************************************************************************
 *
 *  $RCSfile: accessibleselectionhelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:25:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCESSIBLE_SELECTION_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_SELECTION_HELPER_HXX

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#include <comphelper/accessiblecomponenthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif

#define ACCESSIBLE_SELECTION_CHILD_ALL  ((sal_Int32)-1)
#define ACCESSIBLE_SELECTION_CHILD_SELF ((sal_Int32)-2)

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OCommonAccessibleSelection
    //=====================================================================
    /** base class encapsulating common functionality for the helper classes implementing
        the XAccessibleSelection
    */
    class OCommonAccessibleSelection
    {
    protected:

        OCommonAccessibleSelection();

    protected:

        // access to context - still waiting to be overwritten
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            implGetAccessibleContext()
            throw ( ::com::sun::star::uno::RuntimeException ) = 0;

        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual sal_Bool
            implIsSelected( sal_Int32 nAccessibleChildIndex )
            throw (::com::sun::star::uno::RuntimeException) = 0;

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual void
            implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) = 0;

    protected:

        /** non-virtual versions of the methods which can be implemented using <method>implIsSelected</method> and <method>implSelect</method>
        */
        void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL selectAllAccessible(  ) throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };

    //=====================================================================
    //= OAccessibleSelectionHelper
    //=====================================================================

    typedef ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleSelection > OAccessibleSelectionHelper_Base;

    /** a helper class for implementing an AccessibleSelection which at the same time
        supports an XAccessibleSelection interface.
    */
    class OAccessibleSelectionHelper : public OAccessibleComponentHelper,
                                       public OCommonAccessibleSelection,
                                       public OAccessibleSelectionHelper_Base
    {
    protected:

        OAccessibleSelectionHelper( );

        /// see the respective base class ctor for an extensive comment on this, please
        OAccessibleSelectionHelper( IMutex* _pExternalLock );

        // return ourself here by default
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > implGetAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException );

    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleSelection - default implementations
        virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL selectAllAccessible(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_SELECTION_HELPER_HXX

