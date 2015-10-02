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

#include "osx/a11yfocustracker.hxx"
#include "osx/a11yfactory.h"

#include "a11yfocuslistener.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

rtl::Reference< AquaA11yFocusListener > AquaA11yFocusListener::theListener;

rtl::Reference< AquaA11yFocusListener > const & AquaA11yFocusListener::get()
{
    if ( ! theListener.is() )
        theListener = new AquaA11yFocusListener();

    return theListener;
}

AquaA11yFocusListener::AquaA11yFocusListener() : m_focusedObject(nil)
{
}

id AquaA11yFocusListener::getFocusedUIElement()
{
    if ( nil == m_focusedObject ) {
        Reference< XAccessible > xAccessible( TheAquaA11yFocusTracker::get().getFocusedObject() );
#ifdef __OBJC__
        @try {
#else
        try {
#endif
            if( xAccessible.is() ) {
                Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
                if( xContext.is() )
                    m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
            }
        }
#ifdef __OBJC__
        @catch( ... )  {
#else
        catch ( ... ) {
#endif
            // intentionally do nothing ..
        }
    }

    return m_focusedObject;
}

void SAL_CALL
AquaA11yFocusListener::focusedObjectChanged(const Reference< XAccessible >& xAccessible)
{
    if ( nil != m_focusedObject ) {
        [ m_focusedObject release ];
        m_focusedObject = nil;
    }

#ifdef __OBJC__
    @try {
#else
    try {
#endif
        if( xAccessible.is() ) {
            Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
            if( xContext.is() )
            {
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
                NSAccessibilityPostNotification(m_focusedObject, NSAccessibilityFocusedUIElementChangedNotification);
            }
        }
    }
#ifdef __OBJC__
    @catch( ... ) {
#else
    catch( ... ) {
#endif
        // intentionally do nothing ..
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
