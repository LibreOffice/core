/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CanvasUpdateRequester.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:05:54 $
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
