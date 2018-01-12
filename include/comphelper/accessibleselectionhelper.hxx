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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLESELECTIONHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLESELECTIONHELPER_HXX

#include <comphelper/uno3.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <comphelper/comphelperdllapi.h>

#define ACCESSIBLE_SELECTION_CHILD_ALL  (sal_Int32(-1))
#define ACCESSIBLE_SELECTION_CHILD_SELF ((sal_Int32)-2)


namespace comphelper
{


    //= OCommonAccessibleSelection

    /** base class encapsulating common functionality for the helper classes implementing
        the XAccessibleSelection
    */
    class COMPHELPER_DLLPUBLIC OCommonAccessibleSelection
    {
    protected:

        OCommonAccessibleSelection();

        ~OCommonAccessibleSelection();

    protected:

        // access to context - still waiting to be overwritten
        /// @throws css::uno::RuntimeException
        virtual css::uno::Reference< css::accessibility::XAccessibleContext >
            implGetAccessibleContext() = 0;

        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        /// @throws css::uno::RuntimeException
        virtual bool
            implIsSelected( sal_Int32 nAccessibleChildIndex ) = 0;

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        /// @throws css::uno::RuntimeException
        virtual void
            implSelect( sal_Int32 nAccessibleChildIndex, bool bSelect ) = 0;

    protected:

        /** non-virtual versions of the methods which can be implemented using <method>implIsSelected</method> and <method>implSelect</method>

            @throws css::lang::IndexOutOfBoundsException
            @throws css::uno::RuntimeException
        */
        void selectAccessibleChild( sal_Int32 nChildIndex );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::uno::RuntimeException
        bool isAccessibleChildSelected( sal_Int32 nChildIndex );
        /// @throws css::uno::RuntimeException
        void clearAccessibleSelection(  );
        /// @throws css::uno::RuntimeException
        void selectAllAccessibleChildren(  );
        /// @throws css::uno::RuntimeException
        sal_Int32 getSelectedAccessibleChildCount(  );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex );
        /// @throws css::lang::IndexOutOfBoundsException
        /// @throws css::uno::RuntimeException
        void deselectAccessibleChild( sal_Int32 nSelectedChildIndex );
    };


    //= OAccessibleSelectionHelper


    typedef ::cppu::ImplHelper1< css::accessibility::XAccessibleSelection > OAccessibleSelectionHelper_Base;

    /** a helper class for implementing an AccessibleSelection which at the same time
        supports an XAccessibleSelection interface.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleSelectionHelper : public OAccessibleComponentHelper,
                                       public OCommonAccessibleSelection,
                                       public OAccessibleSelectionHelper_Base
    {
    protected:

        OAccessibleSelectionHelper();

        // return ourself here by default
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > implGetAccessibleContext() override;

    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleSelection - default implementations
        virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
        virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
        virtual void SAL_CALL clearAccessibleSelection(  ) override;
        virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
        virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
        virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLESELECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
