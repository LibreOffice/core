/*************************************************************************
 *
 *  $RCSfile: basesh.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWBASESH_HXX
#define _SWBASESH_HXX

#ifndef _SHELLID_HXX
#include <shellid.hxx>
#endif

#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

class SwWrtShell;
class SwCrsrShell;
class SwView;
class GraphicFilter;
class SwFlyFrmAttrMgr;
class SfxItemSet;
class Graphic;

extern void DisableAllItems(SfxItemSet &);

class SwBaseShell: public SfxShell
{
    SwView      &rView;

    // DragModus
    static USHORT nFrameMode;

    // Bug 75078 - if in GetState the asynch call of GetGraphic returns
    //              synch, the set the state directly into the itemset
    SfxItemSet*         pGetStateSet;

    //Update-Timer fuer Graphic
    BOOL        bUpdateSID_IMap     :1;
    BOOL        bUpdateSID_IMapExec :1;
    BOOL        bUpdateSID_IContour :1;
    BOOL        bUpdateSID_IContourDlg :1;

    DECL_LINK( GraphicArrivedHdl, SwCrsrShell* );
    DECL_LINK( UpdatePercentHdl, GraphicFilter* );

protected:

    SwWrtShell& GetShell   ();
    SwWrtShell* GetShellPtr();

    SwView&     GetView() { return rView; }

    SwFlyFrmAttrMgr *pFrmMgr;

    DECL_STATIC_LINK( SwBaseShell, InsertDBTextHdl, String* );

public:
    SwBaseShell(SwView &rShell);
    virtual     ~SwBaseShell();
    SFX_DECL_INTERFACE(SW_BASESHELL);
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
    void        GetTxtCtrlState(SfxItemSet& rSet);
    void        GetBorderState(SfxItemSet &rSet);
    void        GetBckColState(SfxItemSet &rSet);

    void        ExecBckCol(SfxRequest& rReq);
    void        SetWrapMode( USHORT nSlot );

    void        ExecDB(SfxRequest &rReq);

    void        StateDisableItems(SfxItemSet &);

    void        EditRegionDialog(SfxRequest& rReq);
    void        InsertRegionDialog(SfxRequest& rReq);

    void        ExecField(SfxRequest& rReq);

    static void    SetFrmMode( USHORT nMode, SwWrtShell *pShell );  //Mit Update!
    static void   _SetFrmMode( USHORT nMode )   { nFrameMode = nMode; }
    static USHORT  GetFrmMode()                 { return nFrameMode;  }

    //public fuer D&D
    int     InsertGraphic( const String &rPath, const String &rFilter,
                            BOOL bLink = TRUE, GraphicFilter *pFlt = 0,
                            Graphic* pPreviewGrf = 0,
                            BOOL bRule = FALSE );

};


#endif
