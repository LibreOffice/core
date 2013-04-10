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


#ifndef _SWTMPDLG_HXX
#define _SWTMPDLG_HXX

#include <sfx2/styledlg.hxx>

class SfxItemSet;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung:    Der Tabdialog Traeger der TabPages
 --------------------------------------------------------------------*/

class SwTemplateDlg: public SfxStyleDialog
{

    sal_uInt16      nType;
    sal_uInt16      nHtmlMode;
    SwWrtShell*     pWrtShell;
    sal_Bool        bNewStyle;

    DECL_LINK( NumOptionsHdl, PushButton* );

public:
    // @param nSlot
    // Identifies optional Slot by which the creation of the Template (Style) dialog is triggered.
    // Currently used, if nRegion == SFX_STYLE_FAMILY_PAGE in order to activate certain dialog pane
    SwTemplateDlg(  Window*             pParent,
                    SfxStyleSheetBase&  rBase,
                    sal_uInt16          nRegion,
                    const sal_uInt16    nSlot = 0,
                    SwWrtShell*         pActShell = 0,
                    sal_Bool            bNew = sal_False );

    ~SwTemplateDlg();
    const SfxItemSet* GetRefreshedSet();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual short Ok();
};


#endif

