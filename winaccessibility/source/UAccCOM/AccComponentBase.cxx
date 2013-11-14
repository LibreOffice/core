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

#include "stdafx.h"
#include "AccComponentBase.h"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include "MAccessible.h"

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAccComponentBase::CAccComponentBase()
{}

CAccComponentBase::~CAccComponentBase()
{}


/**
 * Returns the location of the upper left corner of the object's bounding
 * box relative to the parent.
 *
 * @param    Location    the upper left corner of the object's bounding box.
 */
STDMETHODIMP CAccComponentBase::get_locationInParent(long *x, long *y)
{

    CHECK_ENABLE_INF

    try
    {
        if (x == NULL || y == NULL)
            return E_INVALIDARG;
        // #CHECK XInterface#
        if(!pRXComp.is())
            return E_FAIL;

        const ::com::sun::star::awt::Point& pt = GetXInterface()->getLocation();
        *x = pt.X;
        *y = pt.Y;
        return S_OK;
    }
    catch(...)
    {
        return E_FAIL;
    }
}

/**
 * Returns the location of the upper left corner of the object's bounding
 * box in screen.
 *
 * @param    Location    the upper left corner of the object's bounding
 *                       box in screen coordinates.
 */
STDMETHODIMP CAccComponentBase::get_locationOnScreen(long *x, long *y)
{

    CHECK_ENABLE_INF

    try
    {
        if (x == NULL || y == NULL)
            return E_INVALIDARG;
        // #CHECK XInterface#
        if(!pRXComp.is())
            return E_FAIL;

        const ::com::sun::star::awt::Point& pt = GetXInterface()->getLocationOnScreen();
        *x = pt.X;
        *y = pt.Y;
        return S_OK;

    }
    catch(...)
    {
        return E_FAIL;
    }
}

/**
 * Grabs the focus to this object.
 *
 * @param    success    the boolean result to be returned.
 */
STDMETHODIMP CAccComponentBase::grabFocus(boolean * success)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    if (success == NULL)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXComp.is())
    {
        return E_FAIL;
    }
    GetXInterface()->grabFocus();
    *success = TRUE;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Returns the foreground color of this object.
 *
 * @param    Color    the color of foreground.
 */
STDMETHODIMP CAccComponentBase::get_foreground(IA2Color * foreground)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    if (foreground == NULL)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXComp.is())
    {
        return E_FAIL;
    }
    *foreground = (long)GetXInterface()->getForeground();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Returns the background color of this object.
 *
 * @param    Color    the color of background.
 */
STDMETHODIMP CAccComponentBase::get_background(IA2Color * background)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    if (background == NULL)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXComp.is())
    {
        return E_FAIL;
    }
    *background = (long)GetXInterface()->getBackground();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Overide of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccComponentBase::put_XInterface(long pXInterface)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == NULL)
        return E_FAIL;
    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleComponent> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXComp = NULL;
    else
        pRXComp = pRXI.get();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}
