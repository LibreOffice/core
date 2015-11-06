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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCDOC_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCDOC_HXX

#include "acccontext.hxx"
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleGetAccFlowTo.hpp>
#include <accselectionhelper.hxx>

class VclSimpleEvent;

// base class for SwAccessibleDocument (in this same header file) and
// SwAccessiblePreview
class SwAccessibleDocumentBase : public SwAccessibleContext
{
    css::uno::Reference< css::accessibility::XAccessible> mxParent;

    VclPtr<vcl::Window> mpChildWin; // protected by solar mutext

    using SwAccessibleFrame::SetVisArea;

protected:
    virtual ~SwAccessibleDocumentBase();

public:
    SwAccessibleDocumentBase( SwAccessibleMap* pInitMap );

    void SetVisArea();

    void AddChild( vcl::Window *pWin, bool bFireEvent = true );
    void RemoveChild( vcl::Window *pWin );

    // XAccessibleContext

    // Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return the specified child or NULL if index is invalid.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
                css::lang::IndexOutOfBoundsException, std::exception) override;

    // Return a reference to the parent.
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription() throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint(
            const css::awt::Point& aPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(
                const css::awt::Point& aPoint )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getBounds()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocation()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;
};

/**
 * access to an accessible Writer document
 */
class SwAccessibleDocument : public SwAccessibleDocumentBase,
                             public css::accessibility::XAccessibleSelection,
                             public css::accessibility::XAccessibleExtendedAttributes,
                             public css::accessibility::XAccessibleGetAccFlowTo
{
    // Implementation for XAccessibleSelection interface
    SwAccessibleSelectionHelper maSelectionHelper;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets MULTISELECTABLE(1)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;

    virtual ~SwAccessibleDocument();

public:
    SwAccessibleDocument( SwAccessibleMap* pInitMap );

    DECL_LINK_TYPED( WindowChildEventListener, VclWindowEvent&, void );

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XInterface

    // XInterface is inherited through SwAcessibleContext and
    // XAccessibleSelection. These methods are needed to avoid
    // ambiguities.

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection

    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception) override;

    // index has to be treated as global child index.
    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL getExtendedAttributes()
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    // thread safe C++ interface

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( bool bRecursive = false ) override;

    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleGetAccFlowTo
    css::uno::Sequence< css::uno::Any >
        SAL_CALL getAccFlowTo(const css::uno::Any& rAny, sal_Int32 nType)
            throw (css::uno::RuntimeException,
                   std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
