/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasUpdateRequester.cxx,v $
 *
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

#include "precompiled_sd.hxx"

#include "CanvasUpdateRequester.hxx"
#include <vcl/svapp.hxx>
#include <com/sun/star/lang/XComponent.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace presenter {

//===== CanvasUpdateRequester::Deleter ========================================

class CanvasUpdateRequester::Deleter
{
public:
    void operator() (CanvasUpdateRequester* pObject) { delete pObject; }
};




//===== CanvasUpdateRequester =================================================

CanvasUpdateRequester::RequesterMap CanvasUpdateRequester::maRequesterMap;

::boost::shared_ptr<CanvasUpdateRequester> CanvasUpdateRequester::Instance (
    const Reference<rendering::XSpriteCanvas>& rxSharedCanvas)
{
    RequesterMap::const_iterator iRequester;
    for (iRequester=maRequesterMap.begin(); iRequester!=maRequesterMap.end(); ++iRequester)
    {
        if (iRequester->first == rxSharedCanvas)
            return iRequester->second;
    }

    // No requester for the given canvas found.  Create a new one.
    ::boost::shared_ptr<CanvasUpdateRequester> pRequester (
        new CanvasUpdateRequester(rxSharedCanvas), Deleter());
    maRequesterMap.push_back(RequesterMap::value_type(rxSharedCanvas,pRequester));
    return pRequester;
}




CanvasUpdateRequester::CanvasUpdateRequester (
    const Reference<rendering::XSpriteCanvas>& rxCanvas)
    : mxCanvas(rxCanvas),
      mnUserEventId(0),
      mbUpdateFlag(sal_False)
{
    Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
    if (xComponent.is())
    {
        //xComponent->addEventListener(this);
    }
}




CanvasUpdateRequester::~CanvasUpdateRequester (void)
{
    if (mnUserEventId != 0)
        Application::RemoveUserEvent(mnUserEventId);
}




void CanvasUpdateRequester::RequestUpdate (const sal_Bool bUpdateAll)
{
    if (mnUserEventId == 0)
    {
        mbUpdateFlag = bUpdateAll;
        mnUserEventId = Application::PostUserEvent(LINK(this, CanvasUpdateRequester, Callback));
    }
    else
    {
        mbUpdateFlag |= bUpdateAll;
    }
}



IMPL_LINK(CanvasUpdateRequester, Callback, void*, EMPTYARG)
{
    mnUserEventId = 0;
    if (mxCanvas.is())
    {
        mxCanvas->updateScreen(mbUpdateFlag);
        mbUpdateFlag = sal_False;
    }
    return 0;
}


} } // end of namespace ::sd::presenter
