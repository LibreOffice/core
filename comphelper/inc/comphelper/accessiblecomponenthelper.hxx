/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblecomponenthelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:24:35 $
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
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
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
    class COMPHELPER_DLLPUBLIC OCommonAccessibleComponent : public OAccessibleContextHelper
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

    struct OAccessibleComponentHelper_Base :
        public ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleComponent >
    {};

    /** a helper class for implementing an AccessibleContext which at the same time
        supports an XAccessibleComponent interface.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleComponentHelper
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
    class COMPHELPER_DLLPUBLIC OAccessibleExtendedComponentHelper
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


