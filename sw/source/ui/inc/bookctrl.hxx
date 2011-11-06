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


#ifndef _BOOKCTRL_HXX
#define _BOOKCTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>
#include <tools/string.hxx>

// class BookmarkToolBoxControl -------------------------------------------

class SwBookmarkControl : public SfxStatusBarControl
{
    virtual void    Command( const CommandEvent& rCEvt );

public:
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Paint( const UserDrawEvent& rEvt );

    SFX_DECL_STATUSBAR_CONTROL();

    SwBookmarkControl( sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb );
    ~SwBookmarkControl();

private:
    String  sPageNumber;
};


#endif

