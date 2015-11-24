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

// AccComponentBase.h: interface for the CAccComponentBase class.

#ifndef INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCCOMPONENTBASE_H
#define INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCCOMPONENTBASE_H

#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include "UNOXWrapper.h"

class ATL_NO_VTABLE CAccComponentBase : public CUNOXWrapper
{
public:
    CAccComponentBase();
    virtual ~CAccComponentBase();
    // IAccessibleComponent
public:
    // IAccessibleComponent

    // Returns the location of the upper left corner of the object's bounding
    // box relative to the parent.
    STDMETHOD(get_locationInParent)(long *x, long *y);

    // Returns the location of the upper left corner of the object's bounding
    // box in screen.
    STDMETHOD(get_locationOnScreen)(long *x, long *y);

    // Grabs the focus to this object.
    STDMETHOD(grabFocus)(boolean * success);

    // Returns the foreground color of this object.
    STDMETHOD(get_foreground)(IA2Color * foreground);

    // Returns the background color of this object.
    STDMETHOD(get_background)(IA2Color * background);

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

protected:

    css::uno::Reference<css::accessibility::XAccessibleComponent> pRXComp;

    inline css::accessibility::XAccessibleComponent* GetXInterface()
    {
        return pRXComp.get();
    }
};

#endif // INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCCOMPONENTBASE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
