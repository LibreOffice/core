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


#ifndef _SWSYNCBTN_HXX
#define _SWSYNCBTN_HXX
#include <sfx2/childwin.hxx>

#ifdef _SYNCDLG
#include <sfx2/basedlgs.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwSyncBtnDlg : public SfxFloatingWindow
{
    PushButton              aSyncBtn;

    DECL_LINK( BtnHdl,      PushButton* pBtn = 0 );

public:
    SwSyncBtnDlg(SfxBindings*, SfxChildWindow*, Window *pParent);
    ~SwSyncBtnDlg();
};
#endif

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwSyncChildWin : public SfxChildWindow
{
public:
    SwSyncChildWin( Window* ,
                    sal_uInt16 nId,
                    SfxBindings*,
                    SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwSyncChildWin );
};


#endif

