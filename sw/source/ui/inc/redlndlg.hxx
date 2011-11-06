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


#ifndef _SWREDLNDLG_HXX
#define _SWREDLNDLG_HXX
#include "swdllapi.h"
#include "chldwrap.hxx"
#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SVSTDARR_HXX
#include <svl/svstdarr.hxx>
#endif
#include <svl/eitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>

class SwChildWinWrapper;

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

    inline sal_Bool operator==( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() == rObj.pData->GetSeqNo()); }
    inline sal_Bool operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

typedef SwRedlineDataParent* SwRedlineDataParentPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataParentArr, SwRedlineDataParentPtr, 10, 20)


SV_DECL_PTRARR_SORT(SwRedlineDataParentSortArr, SwRedlineDataParentPtr, 10, 20)


typedef SwRedlineDataChild* SwRedlineDataChildPtr;
SV_DECL_PTRARR_DEL(SwRedlineDataChildArr, SwRedlineDataChildPtr, 4, 4)


typedef SvLBoxEntry* SvLBoxEntryPtr;
SV_DECL_PTRARR(SvLBoxEntryArr, SvLBoxEntryPtr, 100, 100)

class SW_DLLPUBLIC SwRedlineAcceptDlg
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
    SvxRedlinTable*         pTable; // PB 2006/02/02 #i48648 now SvHeaderTabListBox
    Link                    aOldSelectHdl;
    Link                    aOldDeselectHdl;
    sal_Bool                    bOnlyFormatedRedlines;
    sal_Bool                    bHasReadonlySel;
    sal_Bool                    bRedlnAutoFmt;

    // prevent update dialog data during longer operations (cf #102657#)
    bool                    bInhibitActivate;

    SW_DLLPRIVATE DECL_LINK( AcceptHdl,     void* );
    SW_DLLPRIVATE DECL_LINK( AcceptAllHdl,  void* );
    SW_DLLPRIVATE DECL_LINK( RejectHdl,     void* );
    SW_DLLPRIVATE DECL_LINK( RejectAllHdl,  void* );
    SW_DLLPRIVATE DECL_LINK( UndoHdl,           void* );
    SW_DLLPRIVATE DECL_LINK( DeselectHdl,       void* );
    SW_DLLPRIVATE DECL_LINK( SelectHdl,     void* );
    SW_DLLPRIVATE DECL_LINK( GotoHdl,           void* );
    SW_DLLPRIVATE DECL_LINK( CommandHdl,        void* );

    SW_DLLPRIVATE sal_uInt16            CalcDiff(sal_uInt16 nStart, sal_Bool bChild);
    SW_DLLPRIVATE void          InsertChilds(SwRedlineDataParent *pParent, const SwRedline& rRedln, const sal_uInt16 nAutoFmt);
    SW_DLLPRIVATE void          InsertParents(sal_uInt16 nStart, sal_uInt16 nEnd = USHRT_MAX);
    SW_DLLPRIVATE void          RemoveParents(sal_uInt16 nStart, sal_uInt16 nEnd);
    SW_DLLPRIVATE void          InitAuthors();

    SW_DLLPRIVATE String            GetRedlineText(const SwRedline& rRedln, DateTime &rDateTime, sal_uInt16 nStack = 0);
    SW_DLLPRIVATE const String& GetActionText(const SwRedline& rRedln, sal_uInt16 nStack = 0);
    SW_DLLPRIVATE sal_uInt16            GetRedlinePos( const SvLBoxEntry& rEntry) const;

public:
    SwRedlineAcceptDlg(Dialog *pParent, sal_Bool bAutoFmt = sal_False);
    virtual ~SwRedlineAcceptDlg();

    DECL_LINK( FilterChangedHdl, void *pDummy = 0 );

    inline SvxAcceptChgCtr* GetChgCtrl()        { return &aTabPagesCTRL; }
    inline sal_Bool     HasRedlineAutoFmt() const   { return bRedlnAutoFmt; }

    void            Init(sal_uInt16 nStart = 0);
    void            CallAcceptReject( sal_Bool bSelect, sal_Bool bAccept );

    void            Initialize(const String &rExtraData);
    void            FillInfo(String &rExtraData) const;

    virtual void    Activate();
    virtual void    Resize();
};

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

class SwRedlineAcceptChild : public SwChildWinWrapper
{
public:
    SwRedlineAcceptChild(   Window* ,
                            sal_uInt16 nId,
                            SfxBindings*,
                            SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwRedlineAcceptChild );

    virtual sal_Bool    ReInitDlg(SwDocShell *pDocSh);
};


#endif


