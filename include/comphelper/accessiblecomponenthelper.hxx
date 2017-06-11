/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX

#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{


    //= OCommonAccessibleComponent

    /** base class encapsulating common functionality for the helper classes implementing
        the XAccessibleComponent respectively XAccessibleExtendendComponent
    */
    class COMPHELPER_DLLPUBLIC OCommonAccessibleComponent : public OAccessibleContextHelper
    {
    protected:
        /// see the respective base class ctor for an extensive comment on this, please
        OCommonAccessibleComponent();
        virtual ~OCommonAccessibleComponent() override;

    protected:
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        ///
        /// @throws css::uno::RuntimeException
        virtual css::awt::Rectangle implGetBounds(  ) = 0;

    protected:
        /** non-virtual versions of the methods which can be implemented using <method>implGetBounds</method>
            note: getLocationOnScreen relies on a valid parent (XAccessibleContext::getParent()->getAccessibleContext()),
                 which itself implements XAccessibleComponent

            @throws css::uno::RuntimeException
        */
        bool SAL_CALL containsPoint( const css::awt::Point& aPoint );
        /// @throws css::uno::RuntimeException
        css::awt::Point SAL_CALL getLocation(  );
        /// @throws css::uno::RuntimeException
        css::awt::Point SAL_CALL getLocationOnScreen(  );
        /// @throws css::uno::RuntimeException
        css::awt::Size SAL_CALL getSize(  );
        /// @throws css::uno::RuntimeException
        css::awt::Rectangle SAL_CALL getBounds(  );
    };


    //= OAccessibleComponentHelper


    struct OAccessibleComponentHelper_Base :
        public ::cppu::ImplHelper1< css::accessibility::XAccessibleComponent >
    {
    protected:
        ~OAccessibleComponentHelper_Base() {}
    };

    /** a helper class for implementing an AccessibleContext which at the same time
        supports an XAccessibleComponent interface.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleComponentHelper
            :public OCommonAccessibleComponent
            ,public OAccessibleComponentHelper_Base
    {
    protected:
        OAccessibleComponentHelper();

    public:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent - default implementations
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
        virtual css::awt::Size SAL_CALL getSize(  ) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    };


    //= OAccessibleExtendedComponentHelper


    typedef ::cppu::ImplHelper1 <   css::accessibility::XAccessibleExtendedComponent
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

    public:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent - default implementations
        virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
        virtual css::awt::Size SAL_CALL getSize(  ) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLECOMPONENTHELPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
