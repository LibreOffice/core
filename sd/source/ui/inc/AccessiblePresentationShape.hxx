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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX

#include <svx/AccessibleShape.hxx>

namespace accessibility {

/** This class makes Impress shapes accessible.
*/
class AccessiblePresentationShape
    :   public AccessibleShape
{
public:
    //=====  internal  ========================================================
    AccessiblePresentationShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessiblePresentationShape (void);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  internal  ========================================================

    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName ()
        throw (::com::sun::star::uno::RuntimeException);

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);

private:
    /** Don't use the default constructor.  Use the public constructor that
        takes the original shape and the parent as arguments instead.
    */
    AccessiblePresentationShape (void);

    /// Don't use the constructor.  Not yet implemented.
    AccessiblePresentationShape (const AccessiblePresentationShape&);

    /// Don't use the assignment operator.  Not yet implemented.
    AccessiblePresentationShape& operator= (const AccessiblePresentationShape&);
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
