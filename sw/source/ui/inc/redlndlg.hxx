/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_INC_REDLNDLG_HXX
#define INCLUDED_SW_SOURCE_UI_INC_REDLNDLG_HXX
#include "swdllapi.h"
#include "chldwrap.hxx"
#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <o3tl/sorted_vector.hxx>

class SwChildWinWrapper;

struct SwRedlineDataChild
{
    const SwRedlineData*        pChild;     // link to original stacked data
    const SwRedlineDataChild*   pNext;      // link to stacked data
    SvTreeListEntry*                pTLBChild;  // corresponding TreeListBox entry
};

struct SwRedlineDataParent
{
    const SwRedlineData*        pData;      // RedlineDataPtr
    const SwRedlineDataChild*   pNext;      // link to stacked data
    SvTreeListEntry*            pTLBParent; // corresponding TreeListBox entry
    OUString                    sComment;   // redline comment

    inline sal_Bool operator==( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() == rObj.pData->GetSeqNo()); }
    inline sal_Bool operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

typedef boost::ptr_vector<SwRedlineDataParent> SwRedlineDataParentArr;

class SwRedlineDataParentSortArr : public o3tl::sorted_vector<SwRedlineDataParent*, o3tl::less_ptr_to<SwRedlineDataParent> > {};


typedef SwRedlineDataChild* SwRedlineDataChildPtr;
typedef boost::ptr_vector<SwRedlineDataChild> SwRedlineDataChildArr;


typedef SvTreeListEntry* SvLBoxEntryPtr;
typedef std::vector<SvLBoxEntryPtr> SvLBoxEntryArr;

class SW_DLLPUBLIC SwRedlineAcceptDlg
{
    Dialog*                 pParentDlg;
    SwRedlineDataParentArr  aRedlineParents;
    SwRedlineDataChildArr   aRedlineChildren;
    SwRedlineDataParentSortArr aUsedSeqNo;
    SvxAcceptChgCtr         aTabPagesCTRL;
    PopupMenu               aPopup;
    Timer                   aDeselectTimer;
    Timer                   aSelectTimer;
    OUString                sInserted;
    OUString                sDeleted;
    OUString                sFormated;
    OUString                sTableChgd;
    OUString                sFmtCollSet;
    OUString                sFilterAction;
    OUString                sAutoFormat;
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
    SW_DLLPRIVATE void          InsertChildren(SwRedlineDataParent *pParent, const SwRangeRedline& rRedln, const sal_uInt16 nAutoFmt);
    SW_DLLPRIVATE void          InsertParents(sal_uInt16 nStart, sal_uInt16 nEnd = USHRT_MAX);
    SW_DLLPRIVATE void          RemoveParents(sal_uInt16 nStart, sal_uInt16 nEnd);
    SW_DLLPRIVATE void          InitAuthors();

    SW_DLLPRIVATE OUString      GetRedlineText(const SwRangeRedline& rRedln, DateTime &rDateTime, sal_uInt16 nStack = 0);
    SW_DLLPRIVATE OUString      GetActionText(const SwRangeRedline& rRedln, sal_uInt16 nStack = 0);
    SW_DLLPRIVATE sal_uInt16    GetRedlinePos( const SvTreeListEntry& rEntry) const;

public:
    SwRedlineAcceptDlg(Dialog *pParent, sal_Bool bAutoFmt = sal_False);
    virtual ~SwRedlineAcceptDlg();

    DECL_LINK( FilterChangedHdl, void *pDummy = 0 );

    inline SvxAcceptChgCtr* GetChgCtrl()        { return &aTabPagesCTRL; }
    inline sal_Bool     HasRedlineAutoFmt() const   { return bRedlnAutoFmt; }

    void            Init(sal_uInt16 nStart = 0);
    void            CallAcceptReject( sal_Bool bSelect, sal_Bool bAccept );

    void            Initialize(const OUString &rExtraData);
    void            FillInfo(OUString &rExtraData) const;

    virtual void    Activate();
};

class SwModelessRedlineAcceptDlg : public SfxModelessDialog
{
    SwRedlineAcceptDlg*     pImplDlg;
    SwChildWinWrapper*      pChildWin;

public:
    SwModelessRedlineAcceptDlg(SfxBindings*, SwChildWinWrapper*, Window *pParent);
    ~SwModelessRedlineAcceptDlg();

    virtual void    Activate();
    virtual void    FillInfo(SfxChildWinInfo&) const;
    void            Initialize (SfxChildWinInfo* pInfo);
};

class SwRedlineAcceptChild : public SwChildWinWrapper
{
public:
    SwRedlineAcceptChild(   Window* ,
                            sal_uInt16 nId,
                            SfxBindings*,
                            SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW_WITHID( SwRedlineAcceptChild );

    virtual sal_Bool    ReInitDlg(SwDocShell *pDocSh);
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
