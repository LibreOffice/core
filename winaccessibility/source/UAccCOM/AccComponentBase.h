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

//////////////////////////////////////////////////////////////////////
// AccComponentBase.h: interface for the CAccComponentBase class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCCOMPONENTBASE_H__946BE230_1DCB_494B_ACF6_32A2E197DD2A__INCLUDED_)
#define AFX_ACCCOMPONENTBASE_H__946BE230_1DCB_494B_ACF6_32A2E197DD2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WNT

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

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

protected:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleComponent> pRXComp;

    inline com::sun::star::accessibility::XAccessibleComponent* GetXInterface()
    {
        return pRXComp.get();
    }
};

#endif // !defined(AFX_ACCCOMPONENTBASE_H__946BE230_1DCB_494B_ACF6_32A2E197DD2A__INCLUDED_)
