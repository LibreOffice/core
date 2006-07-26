/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generateevent.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:35:39 $
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

#ifndef INCLUDED_SLIDESHOW_GENERATEEVENT_HXX
#define INCLUDED_SLIDESHOW_GENERATEEVENT_HXX

#include "slideshowcontext.hxx"
#include "delayevent.hxx"
#include "com/sun/star/uno/Any.hxx"

namespace presentation {
namespace internal {

/** Create an event for the given description, calling the given functor.

    @param rEventDescription
    Directly from API

    @param rFunctor
    Functor to call when event fires.

    @param rContext
    Context struct, to provide event queue

    @param nAdditionalDelay
    Additional delay, gets added on top of timeout.
*/
EventSharedPtr generateEvent(
    ::com::sun::star::uno::Any const& rEventDescription,
    Delay::FunctorT const& rFunctor,
    SlideShowContext const& rContext,
    double nAdditionalDelay );

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_GENERATEEVENT_HXX */

