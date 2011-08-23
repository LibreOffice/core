/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <bf_sfx2/module.hxx>
#include <bf_sfx2/shell.hxx>
#include <bf_svtools/svstdarr.hxx>
class Graphic; 

namespace binfilter {

class GraphicFilter; 
class SfxItemSet; 

class SwWrtShell;
class SwCrsrShell;
class SwView;

class SwFlyFrmAttrMgr;

extern void DisableAllItems(SfxItemSet &);
struct DBTextStruct_Impl;
class SwBaseShell: public SfxShell
{
    SwView      &rView;

    // DragModus
    static USHORT nFrameMode;

    // Bug 75078 - if in GetState the asynch call of GetGraphic returns
    //				synch, the set the state directly into the itemset
    SfxItemSet* 		pGetStateSet;

    //Update-Timer fuer Graphic
    SvUShortsSort aGrfUpdateSlots;

    DECL_LINK( GraphicArrivedHdl, SwCrsrShell* );

protected:

    SwWrtShell& GetShell   ();

    SwView&     GetView() { return rView; }

    SwFlyFrmAttrMgr *pFrmMgr;

    DECL_STATIC_LINK( SwBaseShell, InsertDBTextHdl, DBTextStruct_Impl* );

    void SetGetStateSet( SfxItemSet* p )			{ pGetStateSet = p; }
    BOOL AddGrfUpdateSlot( USHORT nSlot )
                                { return aGrfUpdateSlots.Insert( nSlot ); }

public:
    SwBaseShell(SwView &rShell);
    virtual     ~SwBaseShell();
    SFX_DECL_INTERFACE(SW_BASESHELL);
    TYPEINFO();

    void        ExecDelete(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 void        ExecDelete(SfxRequest &);

    void        ExecClpbrd(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void        ExecClpbrd(SfxRequest &);
    void        StateClpbrd(SfxItemSet &);

    void        ExecUndo(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void        ExecUndo(SfxRequest &);
    void        StateUndo(SfxItemSet &);

    void        Execute(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        StateStyle(SfxItemSet &);

    void		ExecuteGallery(SfxRequest&){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		ExecuteGallery(SfxRequest&);
    void		GetGalleryState(SfxItemSet&){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		GetGalleryState(SfxItemSet&);

    void        ExecDlg(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void        ExecDlg(SfxRequest &);

    void        StateStatusLine(SfxItemSet &rSet);
    void        ExecTxtCtrl(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void        ExecTxtCtrl(SfxRequest& rReq);
    void        GetTxtFontCtrlState(SfxItemSet& rSet);
    void        GetTxtCtrlState(SfxItemSet& rSet);
    void 		GetBorderState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void 		GetBorderState(SfxItemSet &rSet);
    void        GetBckColState(SfxItemSet &rSet);

     void        ExecBckCol(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void        ExecBckCol(SfxRequest& rReq);
    void		SetWrapMode( USHORT nSlot );

    void		StateDisableItems(SfxItemSet &){DBG_BF_ASSERT(0, "STRIP");};//STRIP001 void		StateDisableItems(SfxItemSet &);

    void		EditRegionDialog(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP"); };//STRIP001 void		EditRegionDialog(SfxRequest& rReq);
    void		InsertRegionDialog(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP"); };//STRIP001 void		InsertRegionDialog(SfxRequest& rReq);

    void		ExecField(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		ExecField(SfxRequest& rReq);

    static void   _SetFrmMode( USHORT nMode )   { nFrameMode = nMode; }
    static USHORT  GetFrmMode()                 { return nFrameMode;  }

    //public fuer D&D

};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
