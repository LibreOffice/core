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

/**
 * AccComponent.cpp : Implementation of CUAccCOMApp and DLL registration.
 */
#include "stdafx.h"
#include "AccComponent.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <UAccCOM.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

/**
 * Returns the location of the upper left corner of the object's bounding
 * box relative to the parent.
 *
 * @param    Location    the upper left corner of the object's bounding box.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccComponent::get_locationInParent(long* x, long* y)
{
    return CAccComponentBase::get_locationInParent(x, y);
}

/**
 * Returns the foreground color of this object.
 *
 * @param    Color    the color of foreground.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccComponent::get_foreground(IA2Color* foreground)
{
    return CAccComponentBase::get_foreground(foreground);
}

/**
 * Returns the background color of this object.
 *
 * @param    Color    the color of background.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccComponent::get_background(IA2Color* background)
{
    return CAccComponentBase::get_background(background);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
