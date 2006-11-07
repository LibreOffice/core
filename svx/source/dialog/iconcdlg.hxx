/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iconcdlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:50:45 $
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
#ifndef _ICCDLG_HXX
#define _ICCDLG_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _ICNVW_HXX
#include <svtools/ivctrl.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#define CTRLS_OFFSET        3
#define BUTTON_DISTANCE     8
#define RET_USER        100
#define RET_USER_CANCEL 101

// forward-declarations
struct IconChoicePageData;
class IconChoiceDialog;
class IconChoicePage;

// Create-Function
typedef IconChoicePage* (*CreatePage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef USHORT*         (*GetPageRanges)(); // liefert internationale Which-Wert

// page-list
DECLARE_LIST( IconChoicePageList, IconChoicePageData * )

// position of iconchoicectrl
enum EIconChoicePos { PosLeft, PosRight, PosTop, PosBottom };

/**********************************************************************
|
| Data-structure for pages in dialog
|
\**********************************************************************/

struct IconChoicePageData
{
    USHORT nId;                  // Die ID
    CreatePage fnCreatePage;     // Pointer auf die Factory
    GetPageRanges fnGetRanges;// Pointer auf die Ranges-Funktion
    IconChoicePage* pPage;       // die TabPage selber
    BOOL bOnDemand;              // Flag: ItemSet onDemand
    BOOL bRefresh;               // Flag: Seite mu\s neu initialisiert werden

    // Konstruktor
    IconChoicePageData( USHORT Id, CreatePage fnPage, GetPageRanges fnRanges, BOOL bDemand )
        : nId           ( Id ),
          fnCreatePage  ( fnPage ),
          fnGetRanges   ( fnRanges ),
          pPage         ( NULL ),
          bOnDemand     ( bDemand ),
          bRefresh      ( FALSE )
    {}
};

/**********************************************************************
|
| IconChoicePage
|
\**********************************************************************/

class IconChoicePage : public TabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private :
    const SfxItemSet*   pSet;
    String              aUserString;
    BOOL                bHasExchangeSupport;
    IconChoiceDialog*   pDialog;
    BOOL                bStandard;

    void                SetDialog( IconChoiceDialog* pNew ) { pDialog = pNew; }
    IconChoiceDialog*   GetDialog() const { return pDialog; }

    void                SetInputSet( const SfxItemSet* pNew ) { pSet = pNew; }

    void                ImplInitSettings();

protected :
    IconChoicePage( Window *pParent, const ResId &, const SfxItemSet &rAttrSet );

    USHORT              GetSlot( USHORT nWhich ) const  { return pSet->GetPool()->GetSlotId( nWhich ); }
    USHORT              GetWhich( USHORT nSlot ) const  { return pSet->GetPool()->GetWhich( nSlot ); }
    const SfxPoolItem*  GetOldItem( const SfxItemSet& rSet, USHORT nSlot );
    const SfxPoolItem*  GetExchangeItem( const SfxItemSet& rSet, USHORT nSlot );

public :
    virtual             ~IconChoicePage();

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual BOOL        FillItemSet( SfxItemSet& ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    BOOL                HasExchangeSupport() const              { return bHasExchangeSupport; }
    void                SetExchangeSupport( BOOL bNew = TRUE )  { bHasExchangeSupport = bNew; }

    enum {
        KEEP_PAGE = 0x0000,     // Fehlerbehandlung; Seite nicht wechseln
            // 2. F"ullen eines ItemSets f"ur die Aktualilsierung
            // "ubergeordneter Beispiele; dieser Pointer kann immer
            // NULL sein!!
        LEAVE_PAGE = 0x0001,
            // Set aktualisieren und andere Page aktualisieren
        REFRESH_SET = 0x0002
    };

    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    void                SetUserData(const String& rString)  { aUserString = rString; }
    String              GetUserData() { return aUserString; }
    virtual void        FillUserData();
    virtual BOOL        IsReadOnly() const;
    virtual sal_Bool    QueryClose();

    static const SfxPoolItem* GetItem( const SfxItemSet& rSet, USHORT nSlot );

    void                StateChanged( StateChangedType nType );
    void                DataChanged( const DataChangedEvent& rDCEvt );
};

/**********************************************************************
|
| IconChoiceDialog
|
\**********************************************************************/

class IconChoiceDialog : public ModalDialog
{
private :
    friend class IconChoicePage;

    EIconChoicePos          meChoicePos;    // Position des IconChoiceCtrl's
    IconChoicePageList      maPageList;     // Liste von PageData-Pointer

    SvtIconChoiceCtrl       maIconCtrl;     // DAS IconChoice-Control

    USHORT                  mnCurrentPageId;    // Id der aktuell sichtbaren Page

    // Buttons
    OKButton                aOKBtn;
    CancelButton            aCancelBtn;
    HelpButton              aHelpBtn;
    PushButton              aResetBtn;

    const SfxItemSet*       pSet;
    SfxItemSet*             pOutSet;
    SfxItemSet*             pExampleSet;
    USHORT*                 pRanges;

    sal_uInt32              nResId;

    BOOL                    bHideResetBtn;
    BOOL                    bModal;
    BOOL                    bInOK;
    BOOL                    bModified;
    BOOL                    bItemsReset;

    DECL_LINK ( ChosePageHdl_Impl, void * );
    DECL_LINK ( OkHdl,             Button * );
    DECL_LINK ( ResetHdl,          Button * );
    DECL_LINK ( CancelHdl,         Button * );

    IconChoicePageData*     GetPageData ( USHORT nId );
    void                    Start_Impl();
    BOOL                    OK_Impl();

    void                    SetPosSizeCtrls ( BOOL bInit = FALSE );
    void                    SetPosSizePages ( USHORT nId );

    void                    FocusOnIcon ( USHORT nId );

protected :
    void                    ShowPageImpl ( IconChoicePageData* pData );
    void                    HidePageImpl ( IconChoicePageData* pData );

    virtual SfxItemSet*     CreateInputItemSet( USHORT nId );
    inline SfxItemSet*      GetInputSetImpl() { return (SfxItemSet*)pSet; }
    inline IconChoicePage*  GetTabPage( USHORT nPageId )
                                { return ( GetPageData (nPageId)->pPage?GetPageData (nPageId)->pPage:NULL); }
    const SfxItemSet*       GetRefreshedSet();

    void                    ActivatePageImpl ();
    BOOL                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    short                   Ok();
    BOOL                    IsInOK() const;

public :

    // im Ctor könnte auch die Position des IconChoiceCtrl's gesetzt werden.

    IconChoiceDialog ( Window* pParent, const ResId &rResId,
                       const EIconChoicePos ePos = PosLeft, const SfxItemSet * pItemSet = 0 );
    ~IconChoiceDialog ();

    ///////////////////////////////////////////////////////////////////////////////////
    // virtuelle Methoden
    //
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();

    ///////////////////////////////////////////////////////////////////////////////////
    // Schnittstelle
    //

    SvxIconChoiceCtrlEntry* AddTabPage(
        USHORT nId, const String& rIconText, const Image& rChoiceIcon,
        CreatePage pCreateFunc /* != NULL */, GetPageRanges pRangesFunc = NULL /* NULL allowed*/,
        BOOL bItemsOnDemand = FALSE, ULONG nPos = LIST_APPEND );

    SvxIconChoiceCtrlEntry* AddTabPage(
        USHORT nId, const String& rIconText, const Image& rChoiceIcon, const Image& rChoiceIconHC,
        CreatePage pCreateFunc /* != NULL */, GetPageRanges pRangesFunc = NULL /* NULL allowed*/,
        BOOL bItemsOnDemand = FALSE, ULONG nPos = LIST_APPEND );

    void                RemoveTabPage( USHORT nId );

    void                SetCurPageId( USHORT nId ) { mnCurrentPageId = nId; FocusOnIcon( nId ); }
    USHORT              GetCurPageId() const       { return mnCurrentPageId; }
    void                ShowPage( USHORT nId );

                        // liefert ggf. per Map konvertierte lokale Slots
    const USHORT*       GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }
    const SfxItemSet*   GetOutputItemSet( USHORT nId );
    int                 FillOutputItemSet();

    const OKButton&     GetOKButton() const { return aOKBtn; }
    OKButton&           GetOKButton() { return aOKBtn; }
    const CancelButton& GetCancelButton() const { return aCancelBtn; }
    CancelButton&       GetCancelButton() { return aCancelBtn; }
    const HelpButton&   GetHelpButton() const { return aHelpBtn; }
    HelpButton&         GetHelpButton() { return aHelpBtn; }

    void                RemoveResetButton();

    short               Execute();
    void                Start( BOOL bShow = TRUE );
    sal_Bool            QueryClose();

    const SfxItemSet*   GetExampleSet() const { return pExampleSet; }

    void                SetCtrlColor ( const Color& rColor );
    EIconChoicePos      SetCtrlPos   ( const EIconChoicePos& rPos );

    void                CreateIconTextAutoMnemonics( void );
};

#endif //_ICCDLG_HXX

