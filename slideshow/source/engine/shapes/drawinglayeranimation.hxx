/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawinglayeranimation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:51:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

