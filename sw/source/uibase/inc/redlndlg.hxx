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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_REDLNDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_REDLNDLG_HXX
#include "swdllapi.h"
#include "chldwrap.hxx"
#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <o3tl/sorted_vector.hxx>

#include <memory>
#include <vector>

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

    inline bool operator==( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() == rObj.pData->GetSeqNo()); }
    inline bool operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

class SwRedlineDataParentSortArr : public o3tl::sorted_vector<SwRedlineDataParent*, o3tl::less_ptr_to<SwRedlineDataParent> > {};

typedef boost::ptr_vector<SwRedlineDataChild> SwRedlineDataChildArr;

class SW_DLLPUBLIC SwRedlineAcceptDlg
{
    VclPtr<vcl::Window>     pParentDlg;
    std::vector<std::unique_ptr<SwRedlineDataParent>> m_RedlineParents;
    SwRedlineDataChildArr   aRedlineChildren;
    SwRedlineDataParentSortArr aUsedSeqNo;
    VclPtr<SvxAcceptChgCtr>    aTabPagesCTRL;
    PopupMenu               aPopup;
    Timer                   aDeselectTimer;
    Timer                   aSelectTimer;
    OUString                sInserted;
    OUString                sDeleted;
    OUString                sFormated;
    OUString                sTableChgd;
    OUString                sFormatCollSet;
    OUString                sFilterAction;
    OUString                sAutoFormat;
    VclPtr<SvxTPView>       pTPView;
    VclPtr<SvxRedlinTable>  pTable; // PB 2006/02/02 #i48648 now SvHeaderTabListBox
    Link<SvTreeListBox*,void> aOldSelectHdl;
    Link<SvTreeListBox*,void> aOldDeselectHdl;
    bool                    bOnlyFormatedRedlines;
    bool                    bHasReadonlySel;
    bool                    bRedlnAutoFormat;

    // prevent update dialog data during longer operations (cf #102657#)
    bool                    bInhibitActivate;

    DECL_DLLPRIVATE_LINK_TYPED( AcceptHdl,     SvxTPView*, void );
    DECL_DLLPRIVATE_LINK_TYPED( AcceptAllHdl,  SvxTPView*, void );
    DECL_DLLPRIVATE_LINK_TYPED( RejectHdl,     SvxTPView*, void );
    DECL_DLLPRIVATE_LINK_TYPED( RejectAllHdl,  SvxTPView*, void );
    DECL_DLLPRIVATE_LINK_TYPED( UndoHdl,       SvxTPView*, void );
    DECL_DLLPRIVATE_LINK_TYPED( DeselectHdl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK_TYPED( SelectHdl,   SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK_TYPED( SelectTimerHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK_TYPED( GotoHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK_TYPED( CommandHdl, SvSimpleTable*, void );

    SAL_DLLPRIVATE sal_uInt16    CalcDiff(sal_uInt16 nStart, bool bChild);
    SAL_DLLPRIVATE void          InsertChildren(SwRedlineDataParent *pParent, const SwRangeRedline& rRedln, const sal_uInt16 nAutoFormat);
    SAL_DLLPRIVATE void          InsertParents(sal_uInt16 nStart, sal_uInt16 nEnd = USHRT_MAX);
    SAL_DLLPRIVATE void          RemoveParents(sal_uInt16 nStart, sal_uInt16 nEnd);
    SAL_DLLPRIVATE void          InitAuthors();

    SAL_DLLPRIVATE OUString      GetRedlineText(const SwRangeRedline& rRedln, DateTime &rDateTime, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE Image         GetActionImage(const SwRangeRedline& rRedln, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE OUString      GetActionText(const SwRangeRedline& rRedln, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE sal_uInt16    GetRedlinePos( const SvTreeListEntry& rEntry) const;

public:
    SwRedlineAcceptDlg(vcl::Window *pParent, VclBuilderContainer *pBuilder, vcl::Window *pContentArea, bool bAutoFormat = false);
    virtual ~SwRedlineAcceptDlg();

    DECL_LINK_TYPED( FilterChangedHdl, SvxTPFilter*, void );

    inline SvxAcceptChgCtr& GetChgCtrl()        { return *aTabPagesCTRL.get(); }
    inline bool     HasRedlineAutoFormat() const   { return bRedlnAutoFormat; }

    void            Init(sal_uInt16 nStart = 0);
    void            CallAcceptReject( bool bSelect, bool bAccept );

    void            Initialize(const OUString &rExtraData);
    void            FillInfo(OUString &rExtraData) const;

    void            Activate();
};

class SwModelessRedlineAcceptDlg : public SfxModelessDialog
{
    SwRedlineAcceptDlg*     pImplDlg;
    SwChildWinWrapper*      pChildWin;

public:
    SwModelessRedlineAcceptDlg(SfxBindings*, SwChildWinWrapper*, vcl::Window *pParent);
    virtual ~SwModelessRedlineAcceptDlg();
    virtual void dispose() SAL_OVERRIDE;

    virtual void    Activate() SAL_OVERRIDE;
    virtual void    FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;
    void            Initialize (SfxChildWinInfo* pInfo);
};

class SwRedlineAcceptChild : public SwChildWinWrapper
{
public:
    SwRedlineAcceptChild(   vcl::Window* ,
                            sal_uInt16 nId,
                            SfxBindings*,
                            SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW_WITHID( SwRedlineAcceptChild );

    virtual bool    ReInitDlg(SwDocShell *pDocSh) SAL_OVERRIDE;
};

/// Redline (Manage Changes) panel for the sidebar.
class SwRedlineAcceptPanel : public PanelLayout, public SfxListener
{
    SwRedlineAcceptDlg* mpImplDlg;
public:
    SwRedlineAcceptPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual ~SwRedlineAcceptPanel();
    virtual void dispose() SAL_OVERRIDE;

    /// We need to be a SfxListener to be able to update the list of changes when we get SFX_HINT_DOCCHANGED.
    using Control::Notify;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
