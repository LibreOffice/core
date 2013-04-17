/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <salhelper/refobj.hxx>

#include "aqua/aqua11yfocustracker.hxx"
#include "aqua/aqua11yfactory.h"

#include "aqua11yfocuslistener.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;


static rtl::Reference< AquaA11yFocusListener > theListener;

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

