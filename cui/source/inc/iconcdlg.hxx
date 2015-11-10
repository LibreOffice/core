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
#ifndef INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX

#include <rtl/ustring.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svtools/ivctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <vcl/layout.hxx>
#include <vector>

#define RET_USER        100
#define RET_USER_CANCEL 101

// forward-declarations
struct IconChoicePageData;
class IconChoiceDialog;
class IconChoicePage;

// Create-Function
typedef VclPtr<IconChoicePage> (*CreatePage)(vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet &rAttrSet);
typedef const sal_uInt16*      (*GetPageRanges)(); // gives international Which-value

/// Data-structure for pages in dialog
struct IconChoicePageData
{
    sal_uInt16 nId;
    CreatePage fnCreatePage;    ///< pointer to the factory
    GetPageRanges fnGetRanges;  ///< pointer to the ranges-function
    VclPtr<IconChoicePage> pPage;      ///< the TabPage itself
    bool bOnDemand;         ///< Flag: ItemSet onDemand
    bool bRefresh;          ///< Flag: page has to be newly initialized

    // constructor
    IconChoicePageData( sal_uInt16 Id, CreatePage fnPage, GetPageRanges fnRanges, bool bDemand )
        : nId           ( Id ),
          fnCreatePage  ( fnPage ),
          fnGetRanges   ( fnRanges ),
          pPage         ( nullptr ),
          bOnDemand     ( bDemand ),
          bRefresh      ( false )
    {}
};

class IconChoicePage : public TabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    const SfxItemSet*   pSet;
    OUString            aUserString;
    bool                bHasExchangeSupport;
    VclPtr<IconChoiceDialog>   pDialog;

    void                ImplInitSettings();

protected:
    IconChoicePage( vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet &rAttrSet );

    sal_uInt16          GetWhich( sal_uInt16 nSlot ) const  { return pSet->GetPool()->GetWhich( nSlot ); }

public:
    virtual ~IconChoicePage();
    virtual void        dispose() override;

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    bool                HasExchangeSupport() const              { return bHasExchangeSupport; }
    void                SetExchangeSupport( bool bNew = true )  { bHasExchangeSupport = bNew; }

    enum {
        KEEP_PAGE = 0x0000, ///< error handling
        /** 2nd filling of an ItemSet for updating superior examples;
            this pointer can always be NULL!! */
        LEAVE_PAGE = 0x0001,
        /// refresh set and update other pages
        REFRESH_SET = 0x0002
    };

    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet );
    OUString            GetUserData() { return aUserString; }
    virtual bool        QueryClose();

    void                StateChanged( StateChangedType nType ) override;
    void                DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class IconChoiceDialog : public ModalDialog
{
private:
    friend class IconChoicePage;

    ::std::vector< IconChoicePageData* > maPageList;

    VclPtr<SvtIconChoiceCtrl>       m_pIconCtrl;

    sal_uInt16                  mnCurrentPageId;

    // Buttons
    VclPtr<OKButton>                m_pOKBtn;
    VclPtr<PushButton>              m_pApplyBtn;
    VclPtr<CancelButton>            m_pCancelBtn;
    VclPtr<HelpButton>              m_pHelpBtn;
    VclPtr<PushButton>              m_pResetBtn;

    VclPtr<VclVBox>                 m_pTabContainer;
    const SfxItemSet*       pSet;
    SfxItemSet*             pOutSet;
    SfxItemSet*             pExampleSet;
    sal_uInt16*                 pRanges;

    bool                    bHideResetBtn;
    bool                    bModal;
    bool                    bInOK;
    bool                    bItemsReset;

    DECL_LINK_TYPED( ChosePageHdl_Impl, SvtIconChoiceCtrl*, void );
    DECL_LINK_TYPED( OkHdl, Button*, void );
    DECL_LINK_TYPED( ApplyHdl, Button*, void) ;
    DECL_LINK_TYPED( ResetHdl, Button*, void) ;
    DECL_LINK_TYPED( CancelHdl, Button*, void );

    IconChoicePageData*     GetPageData ( sal_uInt16 nId );
    void                    Start_Impl();
    bool                    OK_Impl();

    void                    FocusOnIcon ( sal_uInt16 nId );

protected:
    static void             ShowPageImpl ( IconChoicePageData* pData );
    static void             HidePageImpl ( IconChoicePageData* pData );

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage );
    static SfxItemSet*      CreateInputItemSet( sal_uInt16 nId );
    inline IconChoicePage*  GetTabPage( sal_uInt16 nPageId )
                                { return ( GetPageData (nPageId)->pPage ? GetPageData (nPageId)->pPage.get() : nullptr); }
    static void             RefreshInputSet();

    void                    ActivatePageImpl ();
    bool                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    short                   Ok();

public:

    // the IconChoiceCtrl's could also be set in the Ctor
    IconChoiceDialog ( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription,
                       const SfxItemSet * pItemSet = nullptr );
    virtual ~IconChoiceDialog ();
    virtual void dispose() override;

    // interface
    SvxIconChoiceCtrlEntry* AddTabPage(
        sal_uInt16 nId, const OUString& rIconText, const Image& rChoiceIcon,
        CreatePage pCreateFunc /* != NULL */, GetPageRanges pRangesFunc = nullptr /* NULL allowed*/,
        bool bItemsOnDemand = false, sal_uLong nPos = TREELIST_APPEND );

    void                SetCurPageId( sal_uInt16 nId ) { mnCurrentPageId = nId; FocusOnIcon( nId ); }
    sal_uInt16          GetCurPageId() const       { return mnCurrentPageId; }
    void                ShowPage( sal_uInt16 nId );

    /// gives via map converted local slots if applicable
    const sal_uInt16*   GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );

    const OKButton&     GetOKButton() const { return *m_pOKBtn; }
    OKButton&           GetOKButton() { return *m_pOKBtn; }
    const PushButton&   GetApplyButton() const { return *m_pApplyBtn; }
    PushButton&         GetApplyButton() { return *m_pApplyBtn; }
    const CancelButton& GetCancelButton() const { return *m_pCancelBtn; }
    CancelButton&       GetCancelButton() { return *m_pCancelBtn; }

    short               Execute() override;
    void                Start( bool bShow = true );
    bool                QueryClose();

    void                SetCtrlStyle();
};

#endif // INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
