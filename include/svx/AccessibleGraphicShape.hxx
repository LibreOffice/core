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

#ifndef INCLUDED_SVX_ACCESSIBLEGRAPHICSHAPE_HXX
#define INCLUDED_SVX_ACCESSIBLEGRAPHICSHAPE_HXX

#include <svx/AccessibleShape.hxx>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <svx/svxdllapi.h>

namespace accessibility {

/** @descr
        This class makes graphic shapes accessible.  With respect to its
        base class AccessibleShape it supports the additional
        XAccessibleImage interface.
*/
class SVX_DLLPUBLIC AccessibleGraphicShape
    :   public AccessibleShape,
        public ::com::sun::star::accessibility::XAccessibleImage
{
public:
    //=====  internal  ========================================================
    AccessibleGraphicShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);

    virtual ~AccessibleGraphicShape();

    //=====  XAccessibleImage  ================================================

    OUString SAL_CALL getAccessibleImageDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Int32 SAL_CALL getAccessibleImageHeight()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Int32 SAL_CALL getAccessibleImageWidth()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //=====  XInterface  ======================================================

    virtual com::sun::star::uno::Any SAL_CALL
        queryInterface (const com::sun::star::uno::Type & rType)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL
        acquire()
        throw () SAL_OVERRIDE;

    virtual void SAL_CALL
        release()
        throw () SAL_OVERRIDE;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
/// Return this object's role.
    virtual sal_Int16 SAL_CALL getAccessibleRole() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
protected:
    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName ()
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    AccessibleGraphicShape (const AccessibleGraphicShape&) = delete;

    AccessibleGraphicShape& operator= (const AccessibleGraphicShape&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
