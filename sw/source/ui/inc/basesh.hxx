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


#ifndef _SWBASESH_HXX
#define _SWBASESH_HXX


#include <shellid.hxx>

#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include <svl/svstdarr.hxx>

#include <mdiexp.hxx>

class SwWrtShell;
class SwCrsrShell;
class SwView;
class SfxItemSet;
class Graphic;

struct DBTextStruct_Impl;
class SwBaseShell: public SfxShell
{
    SwView      &rView;

    // DragModus
    static FlyMode eFrameMode;

    // Bug 75078 - if in GetState the asynch call of GetGraphic returns
    //              synch, the set the state directly into the itemset
    SfxItemSet*         pGetStateSet;

    //Update-Timer fuer Graphic
    SvUShortsSort aGrfUpdateSlots;

    DECL_LINK( GraphicArrivedHdl, SwCrsrShell* );

protected:
    SwWrtShell&         GetShell();
    SwWrtShell*         GetShellPtr();

    inline SwView&      GetView()                       { return rView; }
    inline void         SetGetStateSet( SfxItemSet* p ) { pGetStateSet = p; }
    inline sal_Bool         AddGrfUpdateSlot( sal_uInt16 nSlot ){ return aGrfUpdateSlots.Insert( nSlot ); }

    DECL_STATIC_LINK(   SwBaseShell, InsertDBTextHdl, DBTextStruct_Impl* );

    void                InsertURLButton( const String& rURL, const String& rTarget, const String& rTxt );
    void                InsertTable( SfxRequest& _rRequest );

public:
    SwBaseShell(SwView &rShell);
    virtual     ~SwBaseShell();

    SFX_DECL_INTERFACE(SW_BASESHELL)

    void        ExecDelete(SfxRequest &);

    void        ExecClpbrd(SfxRequest &);
    void        StateClpbrd(SfxItemSet &);

    void        ExecUndo(SfxRequest &);
    void        StateUndo(SfxItemSet &);

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        StateStyle(SfxItemSet &);

    void        ExecuteGallery(SfxRequest&);
    void        GetGalleryState(SfxItemSet&);

    void        ExecDlg(SfxRequest &);

    void        StateStatusLine(SfxItemSet &rSet);
    void        ExecTxtCtrl(SfxRequest& rReq);
    void        GetTxtFontCtrlState(SfxItemSet& rSet);
    void        GetTxtCtrlState(SfxItemSet& rSet);
    void        GetBorderState(SfxItemSet &rSet);
    void        GetBckColState(SfxItemSet &rSet);

    void        ExecBckCol(SfxRequest& rReq);
    void        SetWrapMode( sal_uInt16 nSlot );

    void        StateDisableItems(SfxItemSet &);

    void        EditRegionDialog(SfxRequest& rReq);
    void        InsertRegionDialog(SfxRequest& rReq);

    void        ExecField(SfxRequest& rReq);

    static void    SetFrmMode( FlyMode eMode, SwWrtShell *pShell );  //Mit Update!
    static void   _SetFrmMode( FlyMode eMode )   { eFrameMode = eMode; }
    static FlyMode  GetFrmMode()                 { return eFrameMode;  }

};


#endif
