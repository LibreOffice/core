/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
