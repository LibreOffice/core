/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _ICCDLG_HXX
#define _ICCDLG_HXX

#include <vcl/tabpage.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <svtools/ivctrl.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <tools/string.hxx>
#include <vector>

#define CTRLS_OFFSET        3
#define RET_USER        100
#define RET_USER_CANCEL 101

// forward-declarations
struct IconChoicePageData;
class IconChoiceDialog;
class IconChoicePage;

// Create-Function
typedef IconChoicePage* (*CreatePage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef sal_uInt16*         (*GetPageRanges)(); // gives international Which-value

// position of iconchoicectrl
enum EIconChoicePos { PosLeft, PosRight, PosTop, PosBottom };

/// Data-structure for pages in dialog
struct IconChoicePageData
{
    sal_uInt16 nId;
    CreatePage fnCreatePage;    ///< pointer to the factory
    GetPageRanges fnGetRanges;  ///< pointer to the ranges-function
    IconChoicePage* pPage;      ///< the TabPage itself
    sal_Bool bOnDemand;         ///< Flag: ItemSet onDemand
    sal_Bool bRefresh;          ///< Flag: page has to be newly initialized

    // constructor
    IconChoicePageData( sal_uInt16 Id, CreatePage fnPage, GetPageRanges fnRanges, sal_Bool bDemand )
        : nId           ( Id ),
          fnCreatePage  ( fnPage ),
          fnGetRanges   ( fnRanges ),
          pPage         ( NULL ),
          bOnDemand     ( bDemand ),
          bRefresh      ( sal_False )
    {}
};

class IconChoicePage : public TabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private :
    const SfxItemSet*   pSet;
    String              aUserString;
    sal_Bool                bHasExchangeSupport;
    IconChoiceDialog*   pDialog;

    void                SetDialog( IconChoiceDialog* pNew ) { pDialog = pNew; }
    IconChoiceDialog*   GetDialog() const { return pDialog; }

    void                SetInputSet( const SfxItemSet* pNew ) { pSet = pNew; }

    void                ImplInitSettings();

protected :
    IconChoicePage( Window *pParent, const ResId &, const SfxItemSet &rAttrSet );

    sal_uInt16              GetSlot( sal_uInt16 nWhich ) const  { return pSet->GetPool()->GetSlotId( nWhich ); }
    sal_uInt16              GetWhich( sal_uInt16 nSlot ) const  { return pSet->GetPool()->GetWhich( nSlot ); }

public :
    virtual             ~IconChoicePage();

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual sal_Bool        FillItemSet( SfxItemSet& ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    sal_Bool                HasExchangeSupport() const              { return bHasExchangeSupport; }
    void                SetExchangeSupport( sal_Bool bNew = sal_True )  { bHasExchangeSupport = bNew; }

    enum {
        KEEP_PAGE = 0x0000, ///< error handling
        /** 2nd filling of an ItemSet for updating superior examples;
            this pointer can always be NULL!! */
        LEAVE_PAGE = 0x0001,
        /// refresh set and update other pages
        REFRESH_SET = 0x0002
    };

    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    void                SetUserData(const String& rString)  { aUserString = rString; }
    String              GetUserData() { return aUserString; }
    virtual void        FillUserData();
    virtual sal_Bool        IsReadOnly() const;
    virtual sal_Bool    QueryClose();

    void                StateChanged( StateChangedType nType );
    void                DataChanged( const DataChangedEvent& rDCEvt );
};

class IconChoiceDialog : public ModalDialog
{
private :
    friend class IconChoicePage;

    EIconChoicePos          meChoicePos;
    ::std::vector< IconChoicePageData* > maPageList;

    SvtIconChoiceCtrl       maIconCtrl;

    sal_uInt16                  mnCurrentPageId;

    // Buttons
    OKButton                aOKBtn;
    CancelButton            aCancelBtn;
    HelpButton              aHelpBtn;
    PushButton              aResetBtn;

    const SfxItemSet*       pSet;
    SfxItemSet*             pOutSet;
    SfxItemSet*             pExampleSet;
    sal_uInt16*                 pRanges;

    sal_uInt32              nResId;

    sal_Bool                    bHideResetBtn;
    sal_Bool                    bModal;
    sal_Bool                    bInOK;
    sal_Bool                    bModified;
    sal_Bool                    bItemsReset;

    DECL_LINK ( ChosePageHdl_Impl, void * );
    DECL_LINK(OkHdl, void *);
    DECL_LINK(ResetHdl, void *);
    DECL_LINK(CancelHdl, void *);

    IconChoicePageData*     GetPageData ( sal_uInt16 nId );
    void                    Start_Impl();
    sal_Bool                    OK_Impl();

    void                    SetPosSizeCtrls ( sal_Bool bInit = sal_False );
    void                    SetPosSizePages ( sal_uInt16 nId );

    void                    FocusOnIcon ( sal_uInt16 nId );

protected :
    void                    ShowPageImpl ( IconChoicePageData* pData );
    void                    HidePageImpl ( IconChoicePageData* pData );

    virtual void            PageCreated( sal_uInt16 nId, IconChoicePage& rPage );
    virtual SfxItemSet*     CreateInputItemSet( sal_uInt16 nId );
    inline SfxItemSet*      GetInputSetImpl() { return (SfxItemSet*)pSet; }
    inline IconChoicePage*  GetTabPage( sal_uInt16 nPageId )
                                { return ( GetPageData (nPageId)->pPage?GetPageData (nPageId)->pPage:NULL); }
    const SfxItemSet*       GetRefreshedSet();

    void                    ActivatePageImpl ();
    sal_Bool                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    short                   Ok();

public :

    // the IconChoiceCtrl's could also be set in the Ctor
    IconChoiceDialog ( Window* pParent, const ResId &rResId,
                       const EIconChoicePos ePos = PosLeft, const SfxItemSet * pItemSet = 0 );
    ~IconChoiceDialog ();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();

    // interface
    SvxIconChoiceCtrlEntry* AddTabPage(
        sal_uInt16 nId, const String& rIconText, const Image& rChoiceIcon,
        CreatePage pCreateFunc /* != NULL */, GetPageRanges pRangesFunc = NULL /* NULL allowed*/,
        sal_Bool bItemsOnDemand = sal_False, sal_uLong nPos = LIST_APPEND );

    void                SetCurPageId( sal_uInt16 nId ) { mnCurrentPageId = nId; FocusOnIcon( nId ); }
    sal_uInt16              GetCurPageId() const       { return mnCurrentPageId; }
    void                ShowPage( sal_uInt16 nId );

    /// gives via map converted local slots if applicable
    const sal_uInt16*   GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    const OKButton&     GetOKButton() const { return aOKBtn; }
    OKButton&           GetOKButton() { return aOKBtn; }
    const CancelButton& GetCancelButton() const { return aCancelBtn; }
    CancelButton&       GetCancelButton() { return aCancelBtn; }
    const HelpButton&   GetHelpButton() const { return aHelpBtn; }
    HelpButton&         GetHelpButton() { return aHelpBtn; }

    short               Execute();
    void                Start( sal_Bool bShow = sal_True );
    sal_Bool            QueryClose();

    const SfxItemSet*   GetExampleSet() const { return pExampleSet; }

    EIconChoicePos      SetCtrlPos   ( const EIconChoicePos& rPos );
};

#endif //_ICCDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
