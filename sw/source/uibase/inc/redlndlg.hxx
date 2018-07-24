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
#include <swdllapi.h>
#include "chldwrap.hxx"
#include <redline.hxx>
#include <tools/datetime.hxx>
#include <svl/eitem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
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

    bool operator< ( const SwRedlineDataParent& rObj ) const
                        { return (pData && pData->GetSeqNo() <  rObj.pData->GetSeqNo()); }
};

class SwRedlineDataParentSortArr : public o3tl::sorted_vector<SwRedlineDataParent*, o3tl::less_ptr_to<SwRedlineDataParent> > {};

typedef std::vector<std::unique_ptr<SwRedlineDataChild>> SwRedlineDataChildArr;

class SW_DLLPUBLIC SwRedlineAcceptDlg final
{
    VclPtr<vcl::Window>     m_pParentDlg;
    std::vector<std::unique_ptr<SwRedlineDataParent>> m_RedlineParents;
    SwRedlineDataChildArr   m_RedlineChildren;
    SwRedlineDataParentSortArr m_aUsedSeqNo;
    VclPtr<SvxAcceptChgCtr>    m_aTabPagesCTRL;
    VclPtr<PopupMenu>       m_xPopup;
    Timer                   m_aDeselectTimer;
    Timer                   m_aSelectTimer;
    OUString                m_sInserted;
    OUString                m_sDeleted;
    OUString                m_sFormated;
    OUString                m_sTableChgd;
    OUString                m_sFormatCollSet;
    OUString                m_sFilterAction;
    OUString                m_sAutoFormat;
    VclPtr<SvxTPView>       m_pTPView;
    VclPtr<SvxRedlinTable>  m_pTable; // PB 2006/02/02 #i48648 now SvHeaderTabListBox
    Link<SvTreeListBox*,void> m_aOldSelectHdl;
    Link<SvTreeListBox*,void> m_aOldDeselectHdl;
    bool                    m_bOnlyFormatedRedlines;
    bool                    m_bRedlnAutoFormat;

    // prevent update dialog data during longer operations (cf #102657#)
    bool                    m_bInhibitActivate;

    Image                   m_aInserted;
    Image                   m_aDeleted;
    Image                   m_aFormated;
    Image                   m_aTableChgd;
    Image                   m_aFormatCollSet;

    DECL_DLLPRIVATE_LINK( AcceptHdl,     SvxTPView*, void );
    DECL_DLLPRIVATE_LINK( AcceptAllHdl,  SvxTPView*, void );
    DECL_DLLPRIVATE_LINK( RejectHdl,     SvxTPView*, void );
    DECL_DLLPRIVATE_LINK( RejectAllHdl,  SvxTPView*, void );
    DECL_DLLPRIVATE_LINK( UndoHdl,       SvxTPView*, void );
    DECL_DLLPRIVATE_LINK( DeselectHdl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK( SelectHdl,   SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK( SelectTimerHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( GotoHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( CommandHdl, SvSimpleTable*, void );

    SAL_DLLPRIVATE SwRedlineTable::size_type CalcDiff(SwRedlineTable::size_type nStart, bool bChild);
    SAL_DLLPRIVATE void          InsertChildren(SwRedlineDataParent *pParent, const SwRangeRedline& rRedln, const sal_uInt16 nAutoFormat);
    SAL_DLLPRIVATE void          InsertParents(SwRedlineTable::size_type nStart, SwRedlineTable::size_type nEnd = SwRedlineTable::npos);
    SAL_DLLPRIVATE void          RemoveParents(SwRedlineTable::size_type nStart, SwRedlineTable::size_type nEnd);
    SAL_DLLPRIVATE void          InitAuthors();

    SAL_DLLPRIVATE static OUString GetRedlineText(const SwRangeRedline& rRedln, DateTime &rDateTime, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE Image         GetActionImage(const SwRangeRedline& rRedln, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE OUString      GetActionText(const SwRangeRedline& rRedln, sal_uInt16 nStack = 0);
    SAL_DLLPRIVATE static SwRedlineTable::size_type GetRedlinePos( const SvTreeListEntry& rEntry);

    SwRedlineAcceptDlg(SwRedlineAcceptDlg const&) = delete;
    SwRedlineAcceptDlg& operator=(SwRedlineAcceptDlg const&) = delete;

public:
    SwRedlineAcceptDlg(vcl::Window *pParent, VclBuilderContainer *pBuilder, vcl::Window *pContentArea, bool bAutoFormat = false);
    ~SwRedlineAcceptDlg();

    DECL_LINK( FilterChangedHdl, SvxTPFilter*, void );

    SvxAcceptChgCtr& GetChgCtrl()        { return *m_aTabPagesCTRL.get(); }
    bool     HasRedlineAutoFormat() const   { return m_bRedlnAutoFormat; }

    void            Init(SwRedlineTable::size_type nStart = 0);
    void            CallAcceptReject( bool bSelect, bool bAccept );

    void            Initialize(const OUString &rExtraData);
    void            FillInfo(OUString &rExtraData) const;

    void            Activate();
};

class SwModelessRedlineAcceptDlg : public SfxModelessDialog
{
    std::unique_ptr<SwRedlineAcceptDlg> pImplDlg;
    SwChildWinWrapper*      pChildWin;

public:
    SwModelessRedlineAcceptDlg(SfxBindings*, SwChildWinWrapper*, vcl::Window *pParent);
    virtual ~SwModelessRedlineAcceptDlg() override;
    virtual void dispose() override;

    virtual void    Activate() override;
    virtual void    FillInfo(SfxChildWinInfo&) const override;
    void            Initialize (SfxChildWinInfo const * pInfo);
};

class SwRedlineAcceptChild : public SwChildWinWrapper
{
public:
    SwRedlineAcceptChild(   vcl::Window* ,
                            sal_uInt16 nId,
                            SfxBindings*,
                            SfxChildWinInfo const *  );

    SFX_DECL_CHILDWINDOW_WITHID( SwRedlineAcceptChild );

    virtual bool    ReInitDlg(SwDocShell *pDocSh) override;
};

/// Redline (Manage Changes) panel for the sidebar.
class SwRedlineAcceptPanel : public PanelLayout, public SfxListener
{
    std::unique_ptr<SwRedlineAcceptDlg> mpImplDlg;
public:
    SwRedlineAcceptPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual ~SwRedlineAcceptPanel() override;
    virtual void dispose() override;

    /// We need to be a SfxListener to be able to update the list of changes when we get SfxHintId::DocChanged.
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
