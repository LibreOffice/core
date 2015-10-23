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

#ifndef INCLUDED_SVX_ACCESSIBLEOLESHAPE_HXX
#define INCLUDED_SVX_ACCESSIBLEOLESHAPE_HXX

#include <svx/AccessibleShape.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <svx/svxdllapi.h>

namespace accessibility {

/** @descr
        This class makes OLE objects accessible.  With respect to its
        base class AccessibleShape it supports the additional
        XAccessibleAction interface.
*/
class SVX_DLLPUBLIC AccessibleOLEShape
    :   public AccessibleShape,
        public css::accessibility::XAccessibleAction
{
public:
    //=====  internal  ========================================================
    AccessibleOLEShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleOLEShape();

    //=====  XAccessibleAction  ===============================================

    sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL doAccessibleAction (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException,
            css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getAccessibleActionDescription (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException,
            css::uno::RuntimeException, std::exception) override;

    css::uno::Reference<
        css::accessibility::XAccessibleKeyBinding> SAL_CALL getAccessibleActionKeyBinding (
            sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException,
            css::uno::RuntimeException, std::exception) override;

    //=====  XInterface  ======================================================

    virtual css::uno::Any SAL_CALL
        queryInterface (const css::uno::Type & rType)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        acquire()
        throw () override;

    virtual void SAL_CALL
        release()
        throw () override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type> SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
// ====== XAccessibleExtendedAttributes =====================================
    virtual css::uno::Any SAL_CALL getExtendedAttributes() throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override ;
protected:
    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName ()
        throw (css::uno::RuntimeException) override;

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription ()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    AccessibleOLEShape (const AccessibleOLEShape&) = delete;

    AccessibleOLEShape& operator= (const AccessibleOLEShape&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
