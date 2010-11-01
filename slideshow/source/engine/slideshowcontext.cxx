/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
