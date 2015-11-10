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


#include <svx/AccessibleShapeTreeInfo.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

AccessibleShapeTreeInfo::AccessibleShapeTreeInfo()
    : mpView (nullptr),
      mpWindow (nullptr),
      mpViewForwarder (nullptr)
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




AccessibleShapeTreeInfo::~AccessibleShapeTreeInfo()
{
    SolarMutexGuard g;
    mpWindow.reset();
}




void AccessibleShapeTreeInfo::SetDocumentWindow (
    const Reference<XAccessibleComponent>& rxDocumentWindow)
{
    if (mxDocumentWindow != rxDocumentWindow)
        mxDocumentWindow = rxDocumentWindow;
}

void AccessibleShapeTreeInfo::SetModelBroadcaster (
    const Reference<document::XEventBroadcaster>& rxModelBroadcaster)
{
    mxModelBroadcaster = rxModelBroadcaster;
}








void AccessibleShapeTreeInfo::SetSdrView (SdrView* pView)
{
    mpView = pView;
}








void AccessibleShapeTreeInfo::SetController (
    const Reference<frame::XController>& rxController)
{
    mxController = rxController;
}








void AccessibleShapeTreeInfo::SetWindow (vcl::Window* pWindow)
{
    mpWindow = pWindow;
}








void AccessibleShapeTreeInfo::SetViewForwarder (const IAccessibleViewForwarder* pViewForwarder)
{
    mpViewForwarder = pViewForwarder;
}






} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
