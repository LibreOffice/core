/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: changedb.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:03:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHANGEDB_HXX
#define _CHANGEDB_HXX


#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif
#ifndef _DBTREE_HXX
#include "dbtree.hxx"
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

class SwFldMgr;
class SwView;
class SwWrtShell;
struct SwDBData;

/*--------------------------------------------------------------------
     Beschreibung: Datenbank an Feldern austauschen
 --------------------------------------------------------------------*/

class SwChangeDBDlg: public SvxStandardDialog
{
    FixedLine       aDBListFL;
    FixedText       aUsedDBFT;
    FixedText       aAvailDBFT;
    SvTreeListBox   aUsedDBTLB;
    SwDBTreeList    aAvailDBTLB;
    PushButton      aAddDBPB;
    FixedInfo       aDescFT;
    FixedText       aDocDBTextFT;
    FixedText       aDocDBNameFT;
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
//  PushButton      aChangeBT;

    ImageList       aImageList;
    ImageList       aImageListHC;

    SwWrtShell      *pSh;
    SwFldMgr        *pMgr;

    DECL_LINK(TreeSelectHdl, SvTreeListBox* pBox = 0);
    DECL_LINK(ButtonHdl, Button* pBtn);
    DECL_LINK(AddDBHdl, PushButton*);

    virtual void    Apply();
    void            UpdateFlds();
    void            FillDBPopup();
    SvLBoxEntry*    Insert(const String& rDBName);
    void            ShowDBName(const SwDBData& rDBData);

public:
    SwChangeDBDlg(SwView& rVw);
    ~SwChangeDBDlg();
};

#endif
