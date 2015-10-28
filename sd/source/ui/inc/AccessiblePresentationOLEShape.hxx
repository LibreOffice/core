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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEPRESENTATIONOLESHAPE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEPRESENTATIONOLESHAPE_HXX

#include <svx/AccessibleOLEShape.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
namespace accessibility {

/** This class makes Impress shapes accessible.
*/
class AccessiblePresentationOLEShape
    :   public AccessibleOLEShape
{
public:
    //=====  internal  ========================================================
    AccessiblePresentationOLEShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessiblePresentationOLEShape();

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  internal  ========================================================

    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName ()
        throw (css::uno::RuntimeException) override;

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription ()
        throw (css::uno::RuntimeException, std::exception) override;
    /// Return this object's role.
    virtual sal_Int16 SAL_CALL getAccessibleRole () throw (css::uno::RuntimeException, std::exception) override;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
