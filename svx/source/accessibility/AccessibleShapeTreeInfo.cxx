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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/AccessibleShapeTreeInfo.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

AccessibleShapeTreeInfo::AccessibleShapeTreeInfo (
    const Reference<XAccessibleComponent>& rxDocumentWindow,
    const Reference<document::XEventBroadcaster>& rxModelBroadcaster)
    : mxDocumentWindow (rxDocumentWindow),
      mxModelBroadcaster (rxModelBroadcaster),
      mpView (NULL),
      mpWindow (NULL),
      mpViewForwarder (NULL)
{
    // Empty.
}




AccessibleShapeTreeInfo::AccessibleShapeTreeInfo (void)
    : mpView (NULL),
      mpWindow (NULL),
      mpViewForwarder (NULL)
{
    // Empty.
}




AccessibleShapeTreeInfo::AccessibleShapeTreeInfo (const AccessibleShapeTreeInfo& rInfo)
    : mxDocumentWindow (rInfo.mxDocumentWindow),
      mxModelBroadcaster (rInfo.mxModelBroadcaster),
      mpView (rInfo.mpView),
      mxController (rInfo.mxController),
      mpWindow (rInfo.mpWindow),
      mpViewForwarder (rInfo.mpViewForwarder)
{
    // Empty.
}




AccessibleShapeTreeInfo& AccessibleShapeTreeInfo::operator= (const AccessibleShapeTreeInfo& rInfo)
{
    if ( this != &rInfo )
    {
        mxDocumentWindow = rInfo.mxDocumentWindow;
        mxModelBroadcaster = rInfo.mxModelBroadcaster;
        mpView = rInfo.mpView;
        mxController = rInfo.mxController,
        mpWindow = rInfo.mpWindow;
        mpViewForwarder = rInfo.mpViewForwarder;
    }
    return *this;
}




AccessibleShapeTreeInfo::~AccessibleShapeTreeInfo (void)
{
    //empty
}




void AccessibleShapeTreeInfo::SetDocumentWindow (
    const Reference<XAccessibleComponent>& rxDocumentWindow)
{
    if (mxDocumentWindow != rxDocumentWindow)
        mxDocumentWindow = rxDocumentWindow;
}




uno::Reference<XAccessibleComponent>
    AccessibleShapeTreeInfo::GetDocumentWindow (void) const
{
    return mxDocumentWindow;
}




void AccessibleShapeTreeInfo::SetControllerBroadcaster (
    const uno::Reference<document::XEventBroadcaster>& rxControllerBroadcaster)
{
    mxModelBroadcaster = rxControllerBroadcaster;
}




uno::Reference<document::XEventBroadcaster>
    AccessibleShapeTreeInfo::GetControllerBroadcaster (void) const
{
    return mxModelBroadcaster;
}




void AccessibleShapeTreeInfo::SetModelBroadcaster (
    const Reference<document::XEventBroadcaster>& rxModelBroadcaster)
{
    mxModelBroadcaster = rxModelBroadcaster;
}




Reference<document::XEventBroadcaster>
        AccessibleShapeTreeInfo::GetModelBroadcaster (void) const
{
    return mxModelBroadcaster;
}




void AccessibleShapeTreeInfo::SetSdrView (SdrView* pView)
{
    mpView = pView;
}




SdrView* AccessibleShapeTreeInfo::GetSdrView (void) const
{
    return mpView;
}




void AccessibleShapeTreeInfo::SetController (
    const Reference<frame::XController>& rxController)
{
    mxController = rxController;
}




Reference<frame::XController>
    AccessibleShapeTreeInfo::GetController (void) const
{
    return mxController;
}




void AccessibleShapeTreeInfo::SetWindow (Window* pWindow)
{
    mpWindow = pWindow;
}




Window* AccessibleShapeTreeInfo::GetWindow (void) const
{
    return mpWindow;
}




void AccessibleShapeTreeInfo::SetViewForwarder (const IAccessibleViewForwarder* pViewForwarder)
{
    mpViewForwarder = pViewForwarder;
}




const IAccessibleViewForwarder* AccessibleShapeTreeInfo::GetViewForwarder (void) const
{
    return mpViewForwarder;
}


} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
