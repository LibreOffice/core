/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleShapeTreeInfo.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:02:40 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "AccessibleShapeTreeInfo.hxx"

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
