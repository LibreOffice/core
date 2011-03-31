/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _AQUA11YFOCUSLISTENER_HXX_
#define _AQUA11YFOCUSLISTENER_HXX_

#include <salhelper/refobj.hxx>

#include "aqua/keyboardfocuslistener.hxx"
#include "aqua/aquavcltypes.h"

// #include <com/sun/star/accessibility/XAccessibleContext.hpp>

class AquaA11yFocusListener :
    public KeyboardFocusListener,
    public salhelper::ReferenceObject
{
    id m_focusedObject;

    static rtl::Reference< AquaA11yFocusListener > theListener;

    AquaA11yFocusListener::AquaA11yFocusListener();
    virtual AquaA11yFocusListener::~AquaA11yFocusListener() {};
public:

    static rtl::Reference< AquaA11yFocusListener > get();

    id getFocusedUIElement();

    // KeyboardFocusListener
    virtual void SAL_CALL focusedObjectChanged(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& xAccessible);

    // rtl::IReference
    virtual oslInterlockedCount SAL_CALL acquire() SAL_THROW(());
    virtual oslInterlockedCount SAL_CALL release() SAL_THROW(());
};

#endif // _AQUA11YFOCUSLISTENER_HXX_