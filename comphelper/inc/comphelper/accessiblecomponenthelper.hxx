/*************************************************************************
 *
 *  $RCSfile: accessiblecomponenthelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:25:04 $
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

#ifndef COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEXTENDEDCOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#include <comphelper/accessiblecontexthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OCommonAccessibleComponent
    //=====================================================================
    /** base class encapsulating common functionality for the helper classes implementing
        the XAccessibleComponent respectively XAccessibleExtendendComponent
    */
    class OCommonAccessibleComponent : public OAccessibleContextHelper
    {
    protected:
        OCommonAccessibleComponent();
        /// see the respective base class ctor for an extensive comment on this, please
        OCommonAccessibleComponent( IMutex* _pExternalLock );
        ~OCommonAccessibleComponent();

    protected:
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

    protected:
        /** non-virtual versions of the methods which can be implemented using <method>implGetBounds</method>
            note: getLocationOnScreen relies on a valid parent (XAccessibleContext::getParent()->getAccessibleContext()),
                 which itself implements XAccessibleComponent
        */
        sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    //=====================================================================
    //= OAccessibleComponentHelper
    //=====================================================================

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessibleComponent
                                >   OAccessibleComponentHelper_Base;

    /** a helper class for implementing an AccessibleContext which at the same time
        supports an XAccessibleComponent interface.
    */
    class OAccessibleComponentHelper
            :public OCommonAccessibleComponent
            ,public OAccessibleComponentHelper_Base
    {
    protected:
        OAccessibleComponentHelper( );
        /// see the respective base class ctor for an extensive comment on this, please
        OAccessibleComponentHelper( IMutex* _pExternalLock );

    public:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent - default implementations
        virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    //=====================================================================
    //= OAccessibleExtendedComponentHelper
    //=====================================================================

    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessibleExtendedComponent
                                >   OAccessibleExtendedComponentHelper_Base;

    /** a helper class for implementing an AccessibleContext which at the same time
        supports an XAccessibleExtendedComponent interface.
    */
    class OAccessibleExtendedComponentHelper
            :public OCommonAccessibleComponent
            ,public OAccessibleExtendedComponentHelper_Base
    {
    protected:
        OAccessibleExtendedComponentHelper( );
        /// see the respective base class ctor for an extensive comment on this, please
        OAccessibleExtendedComponentHelper( IMutex* _pExternalLock );

    public:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent - default implementations
        virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_COMPONENT_HELPER_HXX


