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

#ifndef SD_SLIDESORTER_VIEW_LAYER_PAINTER_HXX
#define SD_SLIDESORTER_VIEW_LAYER_PAINTER_HXX

#include <boost/shared_ptr.hpp>
#include <sal/types.h>

class OutputDevice;
class Rectangle;

namespace sd { namespace slidesorter { namespace view {

class ILayerInvalidator
{
public:
    virtual void Invalidate (const Rectangle& rInvalidationBox) = 0;
};
typedef ::boost::shared_ptr<ILayerInvalidator> SharedILayerInvalidator;

class ILayerPainter
{
public:
    virtual void SetLayerInvalidator (
        const SharedILayerInvalidator& rpInvalidator) = 0;
    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea) = 0;
};
typedef ::boost::shared_ptr<ILayerPainter> SharedILayerPainter;


} } } // end of namespace ::sd::slidesorter::view

#endif
