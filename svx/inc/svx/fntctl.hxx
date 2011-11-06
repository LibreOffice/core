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


#ifndef _SVX_FNTCTL_HXX
#define _SVX_FNTCTL_HXX

// include ---------------------------------------------------------------

#include <sfx2/mnuitem.hxx>
#include <svl/lstner.hxx>
#include "svx/svxdllapi.h"



class FontNameMenu;
class SfxBindings;

// class SvxFontMenuControl ----------------------------------------------

class SVX_DLLPUBLIC SvxFontMenuControl : public SfxMenuControl, public SfxListener
{
private:
    FontNameMenu*   pMenu;
    Menu&           rParent;

//#if 0 // _SOLAR__PRIVATE
    SVX_DLLPRIVATE void         FillMenu();
    DECL_LINK( MenuSelect, FontNameMenu * );
    SVX_DLLPRIVATE virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    SVX_DLLPRIVATE virtual void     Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
//#endif

public:
    SvxFontMenuControl( sal_uInt16 nId, Menu&, SfxBindings& );
    ~SvxFontMenuControl();

    virtual PopupMenu*  GetPopup() const;

    SFX_DECL_MENU_CONTROL();
};



#endif

