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


#ifndef _SFX_STYLEDLG_HXX
#define _SFX_STYLEDLG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/tabdlg.hxx>

class SfxStyleSheetBase;

class SFX2_DLLPUBLIC SfxStyleDialog: public SfxTabDialog
{
private:
    SfxStyleSheetBase*          pStyle;
//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( CancelHdl, Button * );
//#endif
protected:
    virtual const SfxItemSet*   GetRefreshedSet();

public:
#define ID_TABPAGE_MANAGESTYLES 1
    SfxStyleDialog( Window* pParent, const ResId& rResId, SfxStyleSheetBase&,
                    sal_Bool bFreeRes = sal_True, const String* pUserBtnTxt = 0 );

    ~SfxStyleDialog();

    SfxStyleSheetBase&          GetStyleSheet() { return *pStyle; }
    const SfxStyleSheetBase&    GetStyleSheet() const { return *pStyle; }

    virtual short               Ok();
};

#endif

