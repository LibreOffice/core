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



#ifndef __SD_SLIDELAYOUTCONTROLLER_HXX_
#define __SD_SLIDELAYOUTCONTROLLER_HXX_

#include <svtools/popupwindowcontroller.hxx>

namespace sd
{

class SlideLayoutController : public svt::PopupWindowController
{
public:
    SlideLayoutController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager, const rtl::OUString& sCommandURL, bool bInsertPage );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    using svt::PopupWindowController::createPopupWindow;
private:
    bool mbInsertPage;
};

}

#endif // __SD_SLIDELAYOUTCONTROLLER_HXX_
