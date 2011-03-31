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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <salhelper/refobj.hxx>

#include "aqua/aqua11yfocustracker.hxx"
#include "aqua/aqua11yfactory.h"

#include "aqua11yfocuslistener.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;


rtl::Reference< AquaA11yFocusListener > AquaA11yFocusListener::theListener;

//------------------------------------------------------------------------------

rtl::Reference< AquaA11yFocusListener > AquaA11yFocusListener::get()
{
    if ( ! theListener.is() )
        theListener = new AquaA11yFocusListener();

    return theListener;
}

//------------------------------------------------------------------------------

AquaA11yFocusListener::AquaA11yFocusListener() : m_focusedObject(nil)
{
}

//------------------------------------------------------------------------------

id AquaA11yFocusListener::getFocusedUIElement()
{
    if ( nil == m_focusedObject ) {
        Reference< XAccessible > xAccessible( AquaA11yFocusTracker::get().getFocusedObject() );
        try {
            if( xAccessible.is() ) {
                Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
                if( xContext.is() )
                    m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
            }
        } catch( RuntimeException )  {
            // intentionally do nothing ..
        }
    }

    return m_focusedObject;
}

//------------------------------------------------------------------------------

void SAL_CALL
AquaA11yFocusListener::focusedObjectChanged(const Reference< XAccessible >& xAccessible)
{
    if ( nil != m_focusedObject ) {
        [ m_focusedObject release ];
        m_focusedObject = nil;
    }

    try {
        if( xAccessible.is() ) {
            Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
            if( xContext.is() )
            {
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
                NSAccessibilityPostNotification(m_focusedObject, NSAccessibilityFocusedUIElementChangedNotification);
            }
        }
    } catch( RuntimeException ) {
        // intentionally do nothing ..
    }
}

//------------------------------------------------------------------------------

oslInterlockedCount SAL_CALL
AquaA11yFocusListener::acquire() SAL_THROW(())
{
    return ReferenceObject::acquire();
}

//------------------------------------------------------------------------------

oslInterlockedCount SAL_CALL
AquaA11yFocusListener::release() SAL_THROW(())
{
    return ReferenceObject::release();
}

