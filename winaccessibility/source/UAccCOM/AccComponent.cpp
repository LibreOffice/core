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

/**
 * AccComponent.cpp : Implementation of CUAccCOMApp and DLL registration.
 */
#include "stdafx.h"
#include "UAccCOM2.h"
#include "AccComponent.h"

/**
 * Returns the location of the upper left corner of the object's bounding
 * box relative to the parent.
 *
 * @param    Location    the upper left corner of the object's bounding box.
 */
STDMETHODIMP CAccComponent::get_locationInParent(long *x, long *y)
{

    return CAccComponentBase::get_locationInParent(x,y);
}

/**
 * Returns the foreground color of this object.
 *
 * @param    Color    the color of foreground.
 */
STDMETHODIMP CAccComponent::get_foreground(IA2Color * foreground)
{

    return CAccComponentBase::get_foreground(foreground);
}

/**
 * Returns the background color of this object.
 *
 * @param    Color    the color of background.
 */
STDMETHODIMP CAccComponent::get_background(IA2Color * background)
{

    return CAccComponentBase::get_background(background);
}

/**
 * Overide of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccComponent::put_XInterface(long pXInterface)
{

    return CAccComponentBase::put_XInterface(pXInterface);
}
