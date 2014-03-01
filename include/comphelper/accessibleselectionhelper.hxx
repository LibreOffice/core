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

#define ACCESSIBLE_SELECTION_CHILD_ALL  ((sal_Int32)-1)
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
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
            implGetAccessibleContext()
            throw ( ::com::sun::star::uno::RuntimeException ) = 0;

        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual bool
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
        bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
        sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };


    //= OAccessibleSelectionHelper


    typedef ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleSelection > OAccessibleSelectionHelper_Base;

    /** a helper class for implementing an AccessibleSelection which at the same time
        supports an XAccessibleSelection interface.
    */
    class COMPHELPER_DLLPUBLIC OAccessibleSelectionHelper : public OAccessibleComponentHelper,
                                       public OCommonAccessibleSelection,
                                       public OAccessibleSelectionHelper_Base
    {
    protected:

        /// see the respective base class ctor for an extensive comment on this, please
        OAccessibleSelectionHelper( IMutex* _pExternalLock );

        // return ourself here by default
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > implGetAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException );

    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleSelection - default implementations
        virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLESELECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
