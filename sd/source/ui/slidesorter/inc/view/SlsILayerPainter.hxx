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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSILAYERPAINTER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSILAYERPAINTER_HXX

#include <sal/types.h>
#include <memory>

class OutputDevice;
class Rectangle;

namespace sd { namespace slidesorter { namespace view {

class ILayerInvalidator
{
public:
    virtual ~ILayerInvalidator() {}

    virtual void Invalidate (const Rectangle& rInvalidationBox) = 0;
};
typedef std::shared_ptr<ILayerInvalidator> SharedILayerInvalidator;

class ILayerPainter
{
public:
    virtual ~ILayerPainter() {}

    virtual void SetLayerInvalidator (
        const SharedILayerInvalidator& rpInvalidator) = 0;
    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea) = 0;
};
typedef std::shared_ptr<ILayerPainter> SharedILayerPainter;

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
