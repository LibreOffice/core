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

#ifndef INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX

#include <sal/config.h>

#include <memory>

#include <svx/hlnkitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/image.hxx>

#include "iconcdlg.hxx"

// hyperlink dialog
enum class HyperLinkPageType
{
    Internet,
    Mail,
    Document,
    NewDocument,
    NONE = USHRT_MAX
};

/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg;
class SvxHlinkCtrl : public SfxControllerItem
{
private:
    VclPtr<SvxHpLinkDlg> pParent;

    SfxStatusForwarder aRdOnlyForwarder;

public:
    SvxHlinkCtrl( sal_uInt16 nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg);
    virtual void dispose() override;

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                const SfxPoolItem* pState ) override;
};


/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg : public SfxModalDialog
{
private:
    friend class IconChoicePage;

    std::vector< std::unique_ptr<IconChoicePageData> > maPageList;

    VclPtr<SvtIconChoiceCtrl>       m_pIconCtrl;

    HyperLinkPageType               mnCurrentPageId;

    // Buttons
    VclPtr<OKButton>                m_pOKBtn;
    VclPtr<PushButton>              m_pApplyBtn;
    VclPtr<CancelButton>            m_pCancelBtn;
    VclPtr<HelpButton>              m_pHelpBtn;
    VclPtr<PushButton>              m_pResetBtn;

    VclPtr<VclVBox>                 m_pTabContainer;
    const SfxItemSet*       pSet;
    std::unique_ptr<SfxItemSet>     pOutSet;
    SfxItemSet*             pExampleSet;
    std::unique_ptr<sal_uInt16[]>   pRanges;

    DECL_LINK( ChosePageHdl_Impl, SvtIconChoiceCtrl*, void );
    DECL_LINK( OkHdl, Button*, void );
    DECL_LINK( ApplyHdl, Button*, void) ;
    DECL_LINK( ResetHdl, Button*, void) ;
    DECL_LINK( CancelHdl, Button*, void );

    IconChoicePageData*     GetPageData ( HyperLinkPageType nId );
    void                    Start_Impl();
    bool                    OK_Impl();

    void                    FocusOnIcon ( HyperLinkPageType nId );


    SvxHlinkCtrl        maCtrl;         ///< Controller
    SfxBindings*        mpBindings;
    std::unique_ptr<SfxItemSet> mpItemSet;

    bool            mbGrabFocus : 1;
    bool            mbIsHTMLDoc : 1;

    DECL_LINK (ClickOkHdl_Impl, Button *, void );
    DECL_LINK (ClickApplyHdl_Impl, Button *, void );
    DECL_LINK (ClickCloseHdl_Impl, Button *, void );

    static void             ShowPageImpl ( IconChoicePageData const * pData );
    static void             HidePageImpl ( IconChoicePageData const * pData );

    IconChoicePage*         GetTabPage( HyperLinkPageType nPageId )
                                { return ( GetPageData (nPageId)->pPage ? GetPageData (nPageId)->pPage.get() : nullptr); }

    void                    ActivatePageImpl ();
    void                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    void                    Ok();

    virtual bool            Close() override;
    virtual void            Move() override;
    void Apply();

public:
    SvxHpLinkDlg (vcl::Window* pParent, SfxBindings* pBindings );
    virtual ~SvxHpLinkDlg () override;
    virtual void dispose() override;

    // interface
    SvxIconChoiceCtrlEntry* AddTabPage(
        HyperLinkPageType nId, const OUString& rIconText, const Image& rChoiceIcon,
        CreatePage pCreateFunc /* != NULL */ );

    void                SetCurPageId( HyperLinkPageType nId ) { mnCurrentPageId = nId; FocusOnIcon( nId ); }
    HyperLinkPageType   GetCurPageId() const       { return mnCurrentPageId; }
    void                ShowPage( HyperLinkPageType nId );

    /// gives via map converted local slots if applicable
    const sal_uInt16*   GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );

    OKButton&           GetOKButton() { return *m_pOKBtn; }
    PushButton&         GetApplyButton() { return *m_pApplyBtn; }
    CancelButton&       GetCancelButton() { return *m_pCancelBtn; }

    short               Execute() override;
    void                Start();
    bool                QueryClose();

    void                SetCtrlStyle();

    void                PageCreated( HyperLinkPageType nId, IconChoicePage& rPage );

    void                SetPage( SvxHyperlinkItem const * pItem );
    void                SetReadOnlyMode( bool bReadOnly );
    bool             IsHTMLDoc() const { return mbIsHTMLDoc; }

    SfxDispatcher*   GetDispatcher() const { return mpBindings->GetDispatcher(); }
};


#endif // INCLUDED_CUI_SOURCE_INC_CUIHYPERDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
