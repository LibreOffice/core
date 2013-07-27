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

    explicit AquaA11yFocusListener();
    virtual ~AquaA11yFocusListener() {};
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

