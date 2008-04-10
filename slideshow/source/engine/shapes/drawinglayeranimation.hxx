/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawinglayeranimation.hxx,v $
 * $Revision: 1.3 $
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
#ifndef INCLUDED_DRAWINGLAYERANIMATION_HXX
#define INCLUDED_DRAWINGLAYERANIMATION_HXX

#include <sal/config.h>
#include <boost/shared_ptr.hpp>

namespace slideshow {
namespace internal {

class Activity;
struct SlideShowContext;
class DrawShape;

boost::shared_ptr<Activity> createDrawingLayerAnimActivity(
    SlideShowContext const&                 rContext,
    boost::shared_ptr<DrawShape> const&     pDrawShape );

} // namespace internal
} // namespace presentation

#endif // ! defined INCLUDED_DRAWINGLAYERANIMATION_HXX

