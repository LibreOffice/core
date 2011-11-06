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


#ifndef _VIEWLAYOUTCTRL_HXX
#define _VIEWLAYOUTCTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>

// class SwViewLayoutControl ----------------------------------------------

class SwViewLayoutControl : public SfxStatusBarControl
{
private:

    struct SwViewLayoutControl_Impl;
    SwViewLayoutControl_Impl* mpImpl;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SwViewLayoutControl( sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb );
    ~SwViewLayoutControl();

    virtual void  StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void  Paint( const UserDrawEvent& rEvt );
    virtual sal_Bool  MouseButtonDown( const MouseEvent & );
};

#endif

