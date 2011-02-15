/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    TYPEINFO();

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
