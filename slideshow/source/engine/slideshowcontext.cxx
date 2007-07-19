/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshowcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-07-19 14:48:28 $
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
#include "precompiled_slideshow.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "slideshowcontext.hxx"
#include "subsettableshapemanager.hxx"
#include "screenupdater.hxx"
#include "eventqueue.hxx"
#include "activitiesqueue.hxx"
#include "usereventqueue.hxx"
#include "eventmultiplexer.hxx"
#include "unoviewcontainer.hxx"


using namespace ::com::sun::star;

namespace slideshow
{
namespace internal
{

SlideShowContext::SlideShowContext( SubsettableShapeManagerSharedPtr& rSubsettableShapeManager,
                                    EventQueue&                       rEventQueue,
                                    EventMultiplexer&                 rEventMultiplexer,
                                    ScreenUpdater&                    rScreenUpdater,
                                    ActivitiesQueue&                  rActivitiesQueue,
                                    UserEventQueue&                   rUserEventQueue,
                                    CursorManager&                    rCursorManager,
                                    const UnoViewContainer&           rViewContainer,
                                    const uno::Reference<
                                          uno::XComponentContext>&    rComponentContext ) :
        mpSubsettableShapeManager( rSubsettableShapeManager ),
        mrEventQueue( rEventQueue ),
        mrEventMultiplexer( rEventMultiplexer ),
        mrScreenUpdater( rScreenUpdater ),
        mrActivitiesQueue( rActivitiesQueue ),
        mrUserEventQueue( rUserEventQueue ),
        mrCursorManager( rCursorManager ),
        mrViewContainer( rViewContainer ),
        mxComponentContext( rComponentContext )
    {}

void SlideShowContext::dispose()
{
    mxComponentContext.clear();
}

} // namespace internal
} // namespace slideshow
