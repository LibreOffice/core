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
#include <svtools/ivctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <vcl/layout.hxx>
#include <sfx2/tabdlg.hxx>
#include <vector>

// forward-declarations
struct IconChoicePageData;
class IconChoiceDialog;
class IconChoicePage;
class SfxItemPool;
class SfxItemSet;

// Create-Function
typedef VclPtr<IconChoicePage> (*CreatePage)(vcl::Window *pParent, IconChoiceDialog* pDlg, const SfxItemSet* pAttrSet);

/// Data-structure for pages in dialog
struct IconChoicePageData
{
    sal_uInt16 nId;
    CreatePage fnCreatePage;    ///< pointer to the factory
    VclPtr<IconChoicePage> pPage;      ///< the TabPage itself
    bool bRefresh;          ///< Flag: page has to be newly initialized

    // constructor
    IconChoicePageData( sal_uInt16 Id, CreatePage fnPage )
        : nId           ( Id ),
          fnCreatePage  ( fnPage ),
          pPage         ( nullptr ),
          bRefresh      ( false )
    {}
};

class IconChoicePage : public TabPage
{
private:
    const SfxItemSet*   pSet;
    OUString            aUserString;
    bool                bHasExchangeSupport;

    void                ImplInitSettings();

protected:
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    IconChoicePage( vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet* pItemSet );

public:
    virtual ~IconChoicePage() override;

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    bool                HasExchangeSupport() const { return bHasExchangeSupport; }
    void                SetExchangeSupport()       { bHasExchangeSupport = true; }

    virtual void        ActivatePage( const SfxItemSet& );
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet );
    const OUString&     GetUserData() { return aUserString; }
    virtual bool        QueryClose();

    void                StateChanged( StateChangedType nType ) override;
    void                DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class IconChoiceDialog : public ModalDialog
{
private:
    friend class IconChoicePage;

    std::vector< IconChoicePageData* > maPageList;

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

    DECL_LINK( ChosePageHdl_Impl, SvtIconChoiceCtrl*, void );
    DECL_LINK( OkHdl, Button*, void );
    DECL_LINK( ApplyHdl, Button*, void) ;
    DECL_LINK( ResetHdl, Button*, void) ;
    DECL_LINK( CancelHdl, Button*, void );

    IconChoicePageData*     GetPageData ( sal_uInt16 nId );
    void                    Start_Impl();
    bool                    OK_Impl();

    void                    FocusOnIcon ( sal_uInt16 nId );

protected:
    static void             ShowPageImpl ( IconChoicePageData* pData );
    static void             HidePageImpl ( IconChoicePageData* pData );

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage );
    IconChoicePage*  GetTabPage( sal_uInt16 nPageId )
                                { return ( GetPageData (nPageId)->pPage ? GetPageData (nPageId)->pPage.get() : nullptr); }

    void                    ActivatePageImpl ();
    void                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    void                    Ok();

public:

    // the IconChoiceCtrl's could also be set in the Ctor
    IconChoiceDialog ( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );
    virtual ~IconChoiceDialog () override;
    virtual void dispose() override;

    // interface
    SvxIconChoiceCtrlEntry* AddTabPage(
        sal_uInt16 nId, const OUString& rIconText, const Image& rChoiceIcon,
        CreatePage pCreateFunc /* != NULL */ );

    void                SetCurPageId( sal_uInt16 nId ) { mnCurrentPageId = nId; FocusOnIcon( nId ); }
    sal_uInt16          GetCurPageId() const       { return mnCurrentPageId; }
    void                ShowPage( sal_uInt16 nId );

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
};

#endif // INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
