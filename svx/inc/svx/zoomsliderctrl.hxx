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



#ifndef _ZOOMSLIDER_STBCONTRL_HXX
#define _ZOOMSLIDER_STBCONTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>
#include "svx/svxdllapi.h"

// class SvxZoomSliderControl ----------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderControl : public SfxStatusBarControl
{
private:

    struct SvxZoomSliderControl_Impl;
    SvxZoomSliderControl_Impl* mpImpl;

    sal_uInt16 Offset2Zoom( long nOffset ) const;
    long Zoom2Offset( sal_uInt16 nZoom ) const;

public:

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomSliderControl( sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& _rStb );
    ~SvxZoomSliderControl();

    virtual void  StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void  Paint( const UserDrawEvent& rEvt );
    virtual sal_Bool  MouseButtonDown( const MouseEvent & );
    virtual sal_Bool  MouseMove( const MouseEvent & rEvt );
};

#endif

