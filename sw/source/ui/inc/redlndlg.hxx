/*************************************************************************
 *
 *  $RCSfile: redlndlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:29:53 $
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
#ifndef _SWREDLNDLG_HXX
#define _SWREDLNDLG_HXX

#include "chldwrap.hxx"

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx> //CHINA001
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTDARR_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_CTREDLIN_HXX //autogen
#include <svx/ctredlin.hxx>
#endif
#ifndef _SVX_POSTATTR_HXX //autogen
#include <svx/postattr.hxx>
#endif
class SwChildWinWrapper;

//CHINA001 class SwRedlineAcceptDlg;
//add CHINA001 begin this part is extracted from sw\source\ui\misc\redlndlg.cxx
struct SwRedlineDataChild
{
    const SwRedlineData*        pChild;     // Verweis auf originale gestackte Daten
    const SwRedlineDataChild*   pNext;      // Verweis auf gestackte Daten
    SvLBoxEntry*                pTLBChild;  // zugehoeriger TreeListBox-Eintrag
};

struct SwRedlineDataParent
{
    const SwRedlineData*        pData;      // RedlineDataPtr
    const SwRedlineDataChild*   pNext;      // Verweis auf gestackte Daten
    SvLBoxEntry*                pTLBParent; // zugehoeriger TreeListBox-Eintrag
    String                      sComment;   // Redline-Kommentar

    inline BOOL operator==( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() == rObj.pData->GetSeqNo()); }
    inline BOOL operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

typedef SwRedlineDataParent* SwRedlineDataParentPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataParentArr, SwRedlineDataParentPtr, 10, 20)


SV_DECL_PTRARR_SORT(SwRedlineDataParentSortArr, SwRedlineDataParentPtr, 10, 20)


typedef SwRedlineDataChild* SwRedlineDataChildPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataChildArr, SwRedlineDataChildPtr, 4, 4)


typedef SvLBoxEntry* SvLBoxEntryPtr;
SV_DECL_PTRARR(SvLBoxEntryArr, SvLBoxEntryPtr, 100, 100)

class SwRedlineAcceptDlg
{
    Dialog*                 pParentDlg;
    SwRedlineDataParentArr  aRedlineParents;
    SwRedlineDataChildArr   aRedlineChilds;
    SwRedlineDataParentSortArr aUsedSeqNo;
    SvxAcceptChgCtr         aTabPagesCTRL;
    PopupMenu               aPopup;
    Timer                   aDeselectTimer;
    Timer                   aSelectTimer;
    String                  sInserted;
    String                  sDeleted;
    String                  sFormated;
    String                  sTableChgd;
    String                  sFmtCollSet;
    String                  sFilterAction;
    String                  sAutoFormat;
    Size                    aBorderSz;
    SvxTPView*              pTPView;
    SvxRedlinTable*         pTable;
    Link                    aOldSelectHdl;
    Link                    aOldDeselectHdl;
    BOOL                    bOnlyFormatedRedlines;
    BOOL                    bHasReadonlySel;
    BOOL                    bRedlnAutoFmt;

    // prevent update dialog data during longer operations (cf #102657#)
    bool                    bInhibitActivate;

    DECL_LINK( AcceptHdl,       void* );
    DECL_LINK( AcceptAllHdl,    void* );
    DECL_LINK( RejectHdl,       void* );
    DECL_LINK( RejectAllHdl,    void* );
    DECL_LINK( UndoHdl,         void* );
    DECL_LINK( DeselectHdl,     void* );
    DECL_LINK( SelectHdl,       void* );
    DECL_LINK( GotoHdl,         void* );
    DECL_LINK( CommandHdl,      void* );

    USHORT          CalcDiff(USHORT nStart, BOOL bChild);
    void            InsertChilds(SwRedlineDataParent *pParent, const SwRedline& rRedln, const USHORT nAutoFmt);
    void            InsertParents(USHORT nStart, USHORT nEnd = USHRT_MAX);
    void            RemoveParents(USHORT nStart, USHORT nEnd);
    void            InitAuthors();

    String          GetRedlineText(const SwRedline& rRedln, DateTime &rDateTime, USHORT nStack = 0);
    const String&   GetActionText(const SwRedline& rRedln, USHORT nStack = 0);
    USHORT          GetRedlinePos( const SvLBoxEntry& rEntry) const;

public:
    SwRedlineAcceptDlg(Dialog *pParent, BOOL bAutoFmt = FALSE);
    ~SwRedlineAcceptDlg();

    DECL_LINK( FilterChangedHdl, void *pDummy = 0 );

    inline SvxAcceptChgCtr* GetChgCtrl()        { return &aTabPagesCTRL; }
    inline BOOL     HasRedlineAutoFmt() const   { return bRedlnAutoFmt; }

    void            Init(USHORT nStart = 0);
    void            CallAcceptReject( BOOL bSelect, BOOL bAccept );

    void            Initialize(const String &rExtraData);
    void            FillInfo(String &rExtraData) const;

    virtual void    Activate();
    virtual void    Resize();
};
//add CHINA001 end

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwModelessRedlineAcceptDlg : public SfxModelessDialog
{
    SwRedlineAcceptDlg*     pImplDlg;
    SwChildWinWrapper*      pChildWin;

    virtual void    Resize();

public:
    SwModelessRedlineAcceptDlg(SfxBindings*, SwChildWinWrapper*, Window *pParent);
    ~SwModelessRedlineAcceptDlg();

    virtual void    Activate();
    virtual void    FillInfo(SfxChildWinInfo&) const;
    void            Initialize (SfxChildWinInfo* pInfo);
};

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

//CHINA001 class SwModalRedlineAcceptDlg : public SfxModalDialog
//CHINA001 {
//CHINA001 SwRedlineAcceptDlg*      pImplDlg;
//CHINA001
//CHINA001 virtual void Resize();
//CHINA001
//CHINA001 public:
//CHINA001 SwModalRedlineAcceptDlg(Window *pParent);
//CHINA001 ~SwModalRedlineAcceptDlg();
//CHINA001
//CHINA001 void         AcceptAll( BOOL bAccept );
//CHINA001 virtual void Activate();
//CHINA001 };

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwRedlineAcceptChild : public SwChildWinWrapper
{
public:
    SwRedlineAcceptChild(   Window* ,
                            USHORT nId,
                            SfxBindings*,
                            SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwRedlineAcceptChild );

    virtual BOOL    ReInitDlg(SwDocShell *pDocSh);
};


#endif


