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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <vcl/menu.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

#include "sfxhelp.hxx"
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/templdlg.hxx>
#include "templdgi.hxx"
#include "tplcitem.hxx"
#include "sfxtypes.hxx"
#include <sfx2/styfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/newstyle.hxx>
#include "tplpitem.hxx"
#include "sfxresid.hxx"

#include "templdlg.hrc"
#include <sfx2/sfx.hrc>
#include "dialog.hrc"
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "docvor.hxx"
#include <sfx2/doctempl.hxx>
#include <sfx2/module.hxx>
#include "imgmgr.hxx"
#include "helpid.hrc"
#include "appdata.hxx"
#include <sfx2/viewfrm.hxx>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;

//=========================================================================
// Window is now created dynamically. So here margins, etc.

#define SFX_TEMPLDLG_HFRAME         3
#define SFX_TEMPLDLG_VTOPFRAME      3

#define SFX_TEMPLDLG_VBOTFRAME      3
#define SFX_TEMPLDLG_MIDHSPACE      3
#define SFX_TEMPLDLG_MIDVSPACE      3
#define SFX_TEMPLDLG_FILTERHEIGHT   100

static USHORT nLastItemId = USHRT_MAX;

// filter box has maximum 12 entries visible
#define MAX_FILTER_ENTRIES          12

// Special constant to save hierarchical mode
// We've to use this hack since this is not the
// index in the filter listbox that is saved but the
// index in the filters list and hierarchical is not
// in it
#define HIERARCHICAL_FILTER_INDEX   0xfffe  // 0xffff is reserved

//=========================================================================

TYPEINIT0(SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateDialog_Impl,SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateCatalog_Impl,SfxCommonTemplateDialog_Impl);

SFX_IMPL_DOCKINGWINDOW(SfxTemplateDialogWrapper, SID_STYLE_DESIGNER)

//-------------------------------------------------------------------------

// Re-direct functions

SfxTemplateDialog::SfxTemplateDialog
(
    SfxBindings *pBind,
    SfxChildWindow *pCW,
    Window *pParent
)

/*  [Description]
    Designer class.
*/
    : SfxDockingWindow( pBind, pCW, pParent, SfxResId(DLG_STYLE_DESIGNER) ),

    pImpl( new SfxTemplateDialog_Impl( pParent, pBind, this ) )

{
    pImpl->updateNonFamilyImages();
}

//-------------------------------------------------------------------------

SfxTemplateDialog::~SfxTemplateDialog()
{
    delete pImpl;
}

ISfxTemplateCommon* SfxTemplateDialog::GetISfxTemplateCommon()
{
    return pImpl->GetISfxTemplateCommon();
}

void SfxTemplateDialog::SetParagraphFamily()
{
    // first select the paragraph family
    pImpl->FamilySelect( SFX_STYLE_FAMILY_PARA );
    // then select the automatic filter
    pImpl->SetAutomaticFilter();
}

// ------------------------------------------------------------------------

void SfxTemplateDialog::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( ( DATACHANGED_SETTINGS == _rDCEvt.GetType() ) &&
         ( 0 != ( SETTINGS_STYLE & _rDCEvt.GetFlags() ) ) )
    {
        pImpl->updateFamilyImages();
        pImpl->updateNonFamilyImages();
    }

    SfxDockingWindow::DataChanged( _rDCEvt );
}

//-------------------------------------------------------------------------

void SfxTemplateDialog::Update()
{
    pImpl->Update();
}

//-------------------------------------------------------------------------

void SfxTemplateDialog::Resize()
{
    if(pImpl)
        pImpl->Resize();
    SfxDockingWindow::Resize();
}


//-------------------------------------------------------------------------

SfxChildAlignment SfxTemplateDialog::CheckAlignment(SfxChildAlignment eActAlign,SfxChildAlignment eAlign)
{
    switch (eAlign)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            return eActAlign;

        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            return eAlign;

        default:
            return eAlign;
    }
}

//-------------------------------------------------------------------------

SfxTemplateCatalog::SfxTemplateCatalog(Window *pParent, SfxBindings *pBindings)
 : SfxModalDialog(pParent,SfxResId(RID_STYLECATALOG))
{
    pImpl = new SfxTemplateCatalog_Impl(pParent, pBindings, this);
}

//-------------------------------------------------------------------------

SfxTemplateCatalog::~SfxTemplateCatalog()
{
    delete pImpl;
}

//-------------------------------------------------------------------------

void DropListBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();

    BOOL bHitEmptySpace = ( NULL == GetEntry( rMEvt.GetPosPixel(), TRUE ) );
    if( bHitEmptySpace && ( rMEvt.GetClicks() == 2 ) && rMEvt.IsMod1() )
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

sal_Int8 DropListBox_Impl::AcceptDrop( const AcceptDropEvent& rEvt )

/*  [Description: ]
    Drop is enabled as long as it is allowed to create a new style by example, i.e. to
    create a style out of the current selection.
*/

{
    if ( IsDropFormatSupported( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
    {
        // special case: page styles are allowed to create new styles by example
        // but not allowed to be created by drag and drop
        if( pDialog->nActFamily == SfxCommonTemplateDialog_Impl::SfxFamilyIdToNId( SFX_STYLE_FAMILY_PAGE ) ||
                pDialog->bNewByExampleDisabled )
            return DND_ACTION_NONE;
        else
            return DND_ACTION_COPY;
    }
    return SvTreeListBox::AcceptDrop( rEvt );
}

//-------------------------------------------------------------------------

sal_Int8 DropListBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    SfxObjectShell* pDocShell = pDialog->GetObjectShell();
    TransferableDataHelper aHelper( rEvt.maDropEvent.Transferable );
    sal_uInt32 nFormatCount = aHelper.GetFormatCount();
    if ( pDocShell )
    {
        sal_Bool bFormatFound = sal_False;

        for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
        {
            SotFormatStringId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if ( aHelper.GetTransferableObjectDescriptor( nId, aDesc ) )
            {
                if ( aDesc.maClassName == pDocShell->GetFactory().GetClassId() )
                {
                    PostUserEvent( LINK( this, DropListBox_Impl, OnAsyncExecuteDrop ), 0 );

                    bFormatFound = sal_True;
                    nRet =  rEvt.mnAction;
                    break;
                }
            }
        }

        if ( !bFormatFound )
            return SvTreeListBox::ExecuteDrop( rEvt );
    }

    return nRet;
}


IMPL_LINK( DropListBox_Impl, OnAsyncExecuteDrop, SvLBoxEntry*, EMPTYARG )
{
    pDialog->ActionSelect( SID_STYLE_NEW_BY_EXAMPLE );
    return 0;
}


IMPL_LINK( DropListBox_Impl, OnAsyncExecuteError, void*, NOTINTERESTEDIN )
{
    (void)NOTINTERESTEDIN; // unused
    ErrorHandler::HandleError( ERRCODE_IO_WRONGFORMAT );

    return 0;
}


long DropListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nRet = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyCode&  rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if(!rKeyCode.GetModifier())
        {
            if( pDialog->bCanDel && KEY_DELETE == rKeyCode.GetCode())
            {
                pDialog->DeleteHdl( NULL );
                nRet =  1;
            }
            else if( KEY_RETURN == rKeyCode.GetCode())
            {
                GetDoubleClickHdl().Call(this);
                nRet = 1;
            }
        }
    }
    if(!nRet)
        nRet = SvTreeListBox::Notify( rNEvt );
    return nRet;
}


//-------------------------------------------------------------------------


SfxActionListBox::SfxActionListBox
(
    SfxCommonTemplateDialog_Impl* pParent,
    WinBits nWinBits
)

/*  [Description]

    ListBox class that starts a PopupMenu (designer specific) in the
    command handler.
*/

:       DropListBox_Impl(pParent->GetWindow(), nWinBits, pParent)

{
    EnableContextMenuHandling();
}

//-------------------------------------------------------------------------

SfxActionListBox::SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent,
                                    const ResId &rResId) :
    DropListBox_Impl(pParent->GetWindow(), rResId, pParent)
{
    EnableContextMenuHandling();
}

//-------------------------------------------------------------------------

PopupMenu* SfxActionListBox::CreateContextMenu( void )
{

    if( !( GetSelectionCount() > 0 ) )
    {
        pDialog->EnableEdit( FALSE );
        pDialog->EnableDel( FALSE );
    }
    return pDialog->CreateContextMenu();
}

//-------------------------------------------------------------------------

SfxTemplateDialogWrapper::SfxTemplateDialogWrapper(Window *pParentWnd,
        USHORT nId,  SfxBindings *p, SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWnd, nId)
{
    SfxTemplateDialog *pWin = new SfxTemplateDialog(p, this, pParentWnd);
    pWindow = pWin;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWin->Initialize( pInfo );
    pWin->SetMinOutputSizePixel(pWin->pImpl->GetMinOutputSizePixel());
}

void SfxTemplateDialogWrapper::SetParagraphFamily()
{
    // forward to SfxTemplateDialog, because SfxTemplateDialog isn't exported
    static_cast< SfxTemplateDialog* >( GetWindow() )->SetParagraphFamily();
}

//=========================================================================
SV_DECL_PTRARR_DEL(ExpandedEntries, StringPtr,16,8)
SV_IMPL_PTRARR(ExpandedEntries, StringPtr)

/*  [Description]

    TreeListBox class for displaying the hierarchical view of the templates
*/

class StyleTreeListBox_Impl : public DropListBox_Impl
{
private:
    SvLBoxEntry*                    pCurEntry;
    SfxCommonTemplateDialog_Impl*   pCommon;
    Link                            aDoubleClickLink;
    Link                            aDropLink;
    String                          aParent;
    String                          aStyle;

protected:
    virtual void    Command( const CommandEvent& rMEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual BOOL    DoubleClickHdl();
    virtual long    ExpandingHdl();
    virtual void    ExpandedHdl();
    virtual BOOL    NotifyMoving(SvLBoxEntry*  pTarget,
                                     SvLBoxEntry*  pEntry,
                                     SvLBoxEntry*& rpNewParent,
                                     ULONG&        rNewChildPos);
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle = 0);

    void            SetDoubleClickHdl(const Link &rLink) { aDoubleClickLink = rLink; }
    void            SetDropHdl(const Link &rLink) { aDropLink = rLink; }
        using SvLBox::GetParent;
    const String&   GetParent() const { return aParent; }
    const String&   GetStyle() const { return aStyle; }
    void            MakeExpanded_Impl(ExpandedEntries& rEntries) const;

    virtual PopupMenu* CreateContextMenu( void );
};

//-------------------------------------------------------------------------


void StyleTreeListBox_Impl::MakeExpanded_Impl(ExpandedEntries& rEntries) const
{
    SvLBoxEntry *pEntry;
    USHORT nCount=0;
    for(pEntry=(SvLBoxEntry*)FirstVisible();pEntry;pEntry=(SvLBoxEntry*)NextVisible(pEntry))
    {
        if(IsExpanded(pEntry))
        {
            StringPtr pString=new String(GetEntryText(pEntry));
            rEntries.Insert(pString,nCount++);
        }
    }
}

PopupMenu* StyleTreeListBox_Impl::CreateContextMenu()
{
    return pDialog->CreateContextMenu();
}

BOOL StyleTreeListBox_Impl::DoubleClickHdl()

/*  [Description]

    DoubleClick-Handler; calls the link.
    SV virtual method.
*/
{
    aDoubleClickLink.Call(this);
    return FALSE;
}

//-------------------------------------------------------------------------

void StyleTreeListBox_Impl::Command( const CommandEvent& rCEvt )

/*  [Description]

    Command Handler; this executes a PopupMenu (designer-specific)
    SV virtual method.
*/
{
    SvTreeListBox::Command(rCEvt);
}

//-------------------------------------------------------------------------

long StyleTreeListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    // handle <RETURN> as double click

    long nRet = 0;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyCode&  rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if ( !rKeyCode.GetModifier() && KEY_RETURN == rKeyCode.GetCode() )
        {
            aDoubleClickLink.Call( this );
            nRet = 1;
        }
    }

    if ( !nRet )
        nRet = DropListBox_Impl::Notify( rNEvt );

    return nRet;
}

//-------------------------------------------------------------------------

BOOL StyleTreeListBox_Impl::NotifyMoving(SvLBoxEntry*  pTarget,
                                         SvLBoxEntry*  pEntry,
                                         SvLBoxEntry*& rpNewParent,
                                         ULONG& lPos)
/*  [Description]

    NotifyMoving Handler; This leads via a link on the event to the dialog.
    SV virtual method.
*/
{
    if(!pTarget || !pEntry)
        return FALSE;
    aParent = GetEntryText(pTarget);
    aStyle  = GetEntryText(pEntry);
    const BOOL bRet = (BOOL)aDropLink.Call(this);
    rpNewParent = pTarget;
    lPos=0;
    IntlWrapper aIntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();
    for(SvLBoxEntry *pTmpEntry=FirstChild(pTarget);
        pTmpEntry && COMPARE_LESS==pCollator->compareString(
            GetEntryText(pTmpEntry),GetEntryText(pEntry));
        pTmpEntry=NextSibling(pTmpEntry),lPos++) ;

    return bRet? (BOOL)2: FALSE;
}

//-------------------------------------------------------------------------

long  StyleTreeListBox_Impl::ExpandingHdl()

/*  [Description]

    ExpandingHdl Handler; the current entry is noticed.
    SV virtual method.

    [Cross-reference]
    <StyleTreeListBox_Impl::ExpandedHdl()>
*/
{
    pCurEntry = GetCurEntry();
    return TRUE;
}

//-------------------------------------------------------------------------

void  StyleTreeListBox_Impl::ExpandedHdl()

/*  [Description]

    ExpandedHdl Handler;
    SV virtual method.

    [Cross-reference]
    <StyleTreeListBox_Impl::ExpandingHdl()>
*/

{
    SvLBoxEntry *pEntry = GetHdlEntry();
    if(!IsExpanded(pEntry) && pCurEntry != GetCurEntry())
        SelectAll( FALSE );
    pCurEntry = 0;
}

//-------------------------------------------------------------------------

StyleTreeListBox_Impl::StyleTreeListBox_Impl(
    SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle) :
    DropListBox_Impl(pParent->GetWindow(), nWinStyle, pParent),
    pCurEntry(0),
    pCommon(pParent)

/*  [Description]

    Constructor StyleTreeListBox_Impl
*/
{
    EnableContextMenuHandling();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

class StyleTreeArr_Impl;


/*  [Description]

    Internal structure for the establishment of the hierarchical view
*/

struct StyleTree_Impl
{
    String aName;
    String aParent;
    StyleTreeArr_Impl *pChilds;
    BOOL bIsExpanded;
    BOOL HasParent() const { return aParent.Len() != 0; }

    StyleTree_Impl(const String &rName, const String &rParent):
        aName(rName), aParent(rParent), pChilds(0), bIsExpanded(0) {}
    ~StyleTree_Impl();
    void Put(StyleTree_Impl* pIns, ULONG lPos=ULONG_MAX);
    ULONG Count();
};

typedef StyleTree_Impl* StyleTree_ImplPtr;
SV_DECL_PTRARR_DEL(StyleTreeArr_Impl, StyleTree_ImplPtr, 16, 8)
SV_IMPL_PTRARR(StyleTreeArr_Impl, StyleTree_ImplPtr)


ULONG StyleTree_Impl::Count()
{
    return pChilds ? pChilds->Count() : 0L;
}

//-------------------------------------------------------------------------

StyleTree_Impl::~StyleTree_Impl()
{
    delete pChilds;
}

//-------------------------------------------------------------------------

void StyleTree_Impl::Put(StyleTree_Impl* pIns, ULONG lPos)
{
    if ( !pChilds )
        pChilds = new StyleTreeArr_Impl;

    if ( ULONG_MAX == lPos )
        lPos = pChilds->Count();
    pChilds->Insert( pIns, (USHORT)lPos );
}

//-------------------------------------------------------------------------

StyleTreeArr_Impl &MakeTree_Impl(StyleTreeArr_Impl &rArr)
{
    const USHORT nCount = rArr.Count();

    comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLocale());

    // Arrange all under their Parents
    USHORT i;
    for(i = 0; i < nCount; ++i)
    {
        StyleTree_ImplPtr pEntry = rArr[i];
        if(pEntry->HasParent())
        {
            for(USHORT j = 0; j < nCount; ++j)
            {
                StyleTree_ImplPtr pCmp = rArr[j];
                if(pCmp->aName == pEntry->aParent)
                {
                    // Paste initial filter
                    USHORT nPos;
                    for( nPos = 0 ; nPos < pCmp->Count() &&
                        aSorter.compare((*pCmp->pChilds)[nPos]->aName, pEntry->aName) < 0 ; nPos++);
                    pCmp->Put(pEntry,nPos);
                    break;
                }
            }
        }
    }

    for(i = 0; i < rArr.Count(); )
    {
        if(rArr[i]->HasParent())
            rArr.Remove(i);
        else
            ++i;
    }
    return rArr;
}

//-------------------------------------------------------------------------


inline BOOL IsExpanded_Impl( const ExpandedEntries& rEntries,
                             const String &rStr)
{
    USHORT nCount=rEntries.Count();
    for(USHORT n=0;n<nCount;n++)
        if(*rEntries[n]==rStr)
            return TRUE;
    return FALSE;
}



SvLBoxEntry* FillBox_Impl(SvTreeListBox *pBox,
                                 StyleTree_ImplPtr pEntry,
                                 const ExpandedEntries& rEntries,
                                 SvLBoxEntry* pParent = 0)
{
    SvLBoxEntry* pNewEntry = pBox->InsertEntry(pEntry->aName, pParent);
    const USHORT nCount = pEntry->pChilds? pEntry->pChilds->Count(): 0;
    for(USHORT i = 0; i < nCount; ++i)
        FillBox_Impl(pBox, (*pEntry->pChilds)[i], rEntries, pNewEntry);
    return pNewEntry;
}

//-------------------------------------------------------------------------
// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, SfxDockingWindow* pW ) :

    aISfxTemplateCommon     ( this ),
    pBindings               ( pB ),
    pWindow                 ( pW ),
    pModule                 ( NULL ),
    pTimer                  ( NULL ),
    m_pStyleFamiliesId      ( NULL ),
    pStyleSheetPool         ( NULL ),
    pTreeBox                ( NULL ),
    pCurObjShell            ( NULL ),
    xModuleManager          ( ::comphelper::getProcessServiceFactory()->createInstance(
                                DEFINE_CONST_UNICODE("com.sun.star.frame.ModuleManager") ), UNO_QUERY ),
    pbDeleted               ( NULL ),

    aFmtLb                  ( this, WB_BORDER | WB_TABSTOP ),
    aFilterLb               ( pW, WB_BORDER | WB_DROPDOWN | WB_TABSTOP ),

    nActFamily              ( 0xffff ),
    nActFilter              ( 0 ),
    nAppFilter              ( 0 ),

    bDontUpdate             ( FALSE ),
    bIsWater                ( FALSE ),
    bEnabled                ( TRUE ),
    bUpdate                 ( FALSE ),
    bUpdateFamily           ( FALSE ),
    bCanEdit                ( FALSE ),
    bCanDel                 ( FALSE ),
    bCanNew                 ( TRUE ),
    bWaterDisabled          ( FALSE ),
    bNewByExampleDisabled   ( FALSE ),
    bUpdateByExampleDisabled( FALSE ),
    bTreeDrag               ( TRUE ),
    bHierarchical           ( FALSE ),
    bBindingUpdate          ( TRUE )
{
    aFmtLb.SetHelpId( HID_TEMPLATE_FMT );
    aFilterLb.SetHelpId( HID_TEMPLATE_FILTER );
    aFmtLb.SetWindowBits( WB_HIDESELECTION );
    Font aFont = aFmtLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFmtLb.SetFont( aFont );
}

//-------------------------------------------------------------------------

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, ModalDialog* pW ) :

    aISfxTemplateCommon     ( this ),
    pBindings               ( pB ),
    pWindow                 ( pW ),
    pModule                 ( NULL ),
    pTimer                  ( NULL ),
    pStyleSheetPool         ( NULL ),
    pTreeBox                ( NULL ),
    pCurObjShell            ( NULL ),
    pbDeleted               ( NULL ),

    aFmtLb                  ( this, SfxResId( BT_VLIST ) ),
    aFilterLb               ( pW, SfxResId( BT_FLIST ) ),

    nActFamily              ( 0xffff ),
    nActFilter              ( 0 ),
    nAppFilter              ( 0 ),

    bDontUpdate             ( FALSE ),
    bIsWater                ( FALSE ),
    bEnabled                ( TRUE ),
    bUpdate                 ( FALSE ),
    bUpdateFamily           ( FALSE ),
    bCanEdit                ( FALSE ),
    bCanDel                 ( FALSE ),
    bCanNew                 ( TRUE ),
    bWaterDisabled          ( FALSE ),
    bNewByExampleDisabled   ( FALSE ),
    bUpdateByExampleDisabled( FALSE ),
    bTreeDrag               ( TRUE ),
    bHierarchical           ( FALSE ),
    bBindingUpdate          ( TRUE )

{
    aFmtLb.SetWindowBits( WB_SORT );
}

//-------------------------------------------------------------------------

USHORT SfxCommonTemplateDialog_Impl::StyleNrToInfoOffset(USHORT nId)
{
    const SfxStyleFamilyItem *pItem = pStyleFamilies->at( nId );
    return SfxFamilyIdToNId(pItem->GetFamily())-1;
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableEdit(BOOL bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem( SID_STYLE_UPDATE_BY_EXAMPLE, bEnable);
}

//-------------------------------------------------------------------------


USHORT SfxCommonTemplateDialog_Impl::InfoOffsetToStyleNr(USHORT nId)
{
    for ( size_t i = 0; i < pStyleFamilies->size(); i++ )
        if ( SfxFamilyIdToNId(pStyleFamilies->at( i )->GetFamily()) == nId+1 )
            return i;
    OSL_FAIL("Style Nummer not found");
    return 0;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ReadResource()
{
    // Read global user resource
    for(USHORT i = 0; i < MAX_FAMILIES; ++i)
        pFamilyState[i] = 0;

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    ResMgr* pMgr = pCurObjShell ? pCurObjShell->GetResMgr() : NULL;
    ResId aFamId( DLG_STYLE_DESIGNER, *pMgr );
    aFamId.SetRT(RSC_SFX_STYLE_FAMILIES);
    m_pStyleFamiliesId = new ResId( aFamId.GetId(), *pMgr );
    m_pStyleFamiliesId->SetRT(RSC_SFX_STYLE_FAMILIES);
    if( !pMgr || !pMgr->IsAvailable( aFamId ) )
        pStyleFamilies = new SfxStyleFamilies;
    else
        pStyleFamilies = new SfxStyleFamilies( aFamId );

    nActFilter = pCurObjShell ? static_cast< USHORT >( LoadFactoryStyleFilter( pCurObjShell ) ) : 0xFFFF;
    if ( pCurObjShell && 0xFFFF == nActFilter )
        nActFilter = pCurObjShell->GetAutoStyleFilterIndex();

    // Paste in the toolbox
    // reverse order, since always inserted at the head
    size_t nCount = pStyleFamilies->size();

    pBindings->ENTERREGISTRATIONS();

    size_t i;
    for(i = 0; i < nCount; ++i)
    {
        USHORT nSlot = 0;
        switch( (USHORT)pStyleFamilies->at( i )->GetFamily() )
        {
            case SFX_STYLE_FAMILY_CHAR: nSlot = SID_STYLE_FAMILY1; break;
            case SFX_STYLE_FAMILY_PARA: nSlot = SID_STYLE_FAMILY2; break;
            case SFX_STYLE_FAMILY_FRAME:nSlot = SID_STYLE_FAMILY3; break;
            case SFX_STYLE_FAMILY_PAGE: nSlot = SID_STYLE_FAMILY4; break;
            case SFX_STYLE_FAMILY_PSEUDO: nSlot = SID_STYLE_FAMILY5; break;
            default: OSL_FAIL("unbekannte StyleFamily"); break;
        }
        pBoundItems[i] =
            new SfxTemplateControllerItem(nSlot, *this, *pBindings);
    }
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_WATERCAN, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_NEW_BY_EXAMPLE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_UPDATE_BY_EXAMPLE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_NEW, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_DRAGHIERARCHIE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_EDIT, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_DELETE, *this, *pBindings);
    pBoundItems[i++] = new SfxTemplateControllerItem(
        SID_STYLE_FAMILY, *this, *pBindings);
    pBindings->LEAVEREGISTRATIONS();

    for(; i < COUNT_BOUND_FUNC; ++i)
        pBoundItems[i] = 0;

    StartListening(*pBindings);

// Insert in the reverse order of occurrence in the Style Families. This is for
// the toolbar of the designer. The list box of the catalog respects the
// correct order by itself.

// Sequences: the order of Resource = the order of Toolbar for example list box.
// Order of ascending SIDs: Low SIDs are displayed first when templates of
// several families are active.

    // in the Writer the UpdateStyleByExample Toolbox button is removed and
    // the NewStyle button gets a PopupMenu
    if(nCount > 4)
        ReplaceUpdateButtonByMenu();

    for( ; nCount--; )
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->at( nCount );
        USHORT nId = SfxFamilyIdToNId( pItem->GetFamily() );
        InsertFamilyItem( nId, pItem );
    }

    LoadedFamilies();

    USHORT nStart = SID_STYLE_FAMILY1;
    USHORT nEnd = SID_STYLE_FAMILY4;

    for ( i = nStart; i <= nEnd; i++ )
        pBindings->Update(i);

    pModule = pCurObjShell ? pCurObjShell->GetModule() : NULL;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ClearResource()
{
    ClearFamilyList();
    DELETEX(pStyleFamilies);
    USHORT i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    pCurObjShell = NULL;

    DELETEZ( m_pStyleFamiliesId );
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::Initialize()
{
    // Read global user resource
    ReadResource();
    pBindings->Invalidate( SID_STYLE_FAMILY );
    pBindings->Update( SID_STYLE_FAMILY );
    Update_Impl();

    aFilterLb.SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FilterSelectHdl ) );
    aFmtLb.SetDoubleClickHdl( LINK( this, SfxCommonTemplateDialog_Impl, ApplyHdl ) );
    aFmtLb.SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FmtSelectHdl ) );

    aFilterLb.Show();
    aFmtLb.Show();
}

//-------------------------------------------------------------------------

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    String aEmpty;
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
    GetWindow()->Hide();
    DELETEX(pStyleFamilies);
    DELETEZ( m_pStyleFamiliesId );
    USHORT i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = NULL;
    delete pTreeBox;
    delete pTimer;
    if ( pbDeleted )
    {
        pbDeleted->bDead = true;
        pbDeleted = NULL;
    }
}

//-------------------------------------------------------------------------

USHORT SfxCommonTemplateDialog_Impl::SfxFamilyIdToNId( SfxStyleFamily nFamily )
{
    switch ( nFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:   return 1;
        case SFX_STYLE_FAMILY_PARA:   return 2;
        case SFX_STYLE_FAMILY_FRAME:  return 3;
        case SFX_STYLE_FAMILY_PAGE:   return 4;
        case SFX_STYLE_FAMILY_PSEUDO: return 5;
        default:                      return 0;
    }
}

void SfxCommonTemplateDialog_Impl::SetAutomaticFilter()
{
    USHORT nCount = aFilterLb.GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        ULONG nFlags = (ULONG)aFilterLb.GetEntryData(i);
        if ( SFXSTYLEBIT_AUTO == nFlags )
        {
            // automatic entry found -> select it
            aFilterLb.SelectEntryPos(i);
            // then call the handler to filter the styles
            FilterSelect( i - 1 );
            break;
        }
    }
}

//-------------------------------------------------------------------------
// Helper function: Access to the current family item
const SfxStyleFamilyItem *SfxCommonTemplateDialog_Impl::GetFamilyItem_Impl() const
{
    const size_t nCount = pStyleFamilies->size();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->at( i );
        USHORT nId = SfxFamilyIdToNId(pItem->GetFamily());
        if(nId == nActFamily)
            return pItem;
    }
    return 0;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::SelectStyle(const String &rStr)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if ( !pItem )
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( rStr, eFam, SFXSTYLEBIT_ALL );
    if( pStyle )
        EnableEdit( !(pStyle->GetMask() & SFXSTYLEBIT_READONLY) );
    else
        EnableEdit(FALSE);

    if ( pTreeBox )
    {
        if ( rStr.Len() )
        {
            SvLBoxEntry* pEntry = pTreeBox->First();
            while ( pEntry )
            {
                if ( pTreeBox->GetEntryText( pEntry ) == rStr )
                {
                    pTreeBox->MakeVisible( pEntry );
                    pTreeBox->Select( pEntry );
                    return;
                }
                pEntry = pTreeBox->Next( pEntry );
            }
        }
        else
            pTreeBox->SelectAll( FALSE );
    }
    else
    {
        BOOL bSelect = ( rStr.Len() > 0 );
        if ( bSelect )
        {
            SvLBoxEntry* pEntry = (SvLBoxEntry*)aFmtLb.FirstVisible();
            while ( pEntry && aFmtLb.GetEntryText( pEntry ) != rStr )
                pEntry = (SvLBoxEntry*)aFmtLb.NextVisible( pEntry );
            if ( !pEntry )
                bSelect = FALSE;
            else
            {
                aFmtLb.MakeVisible( pEntry );
                aFmtLb.Select( pEntry );
                bWaterDisabled = !HasSelectedStyle();
                FmtSelectHdl( NULL );
            }
        }

        if ( !bSelect )
        {
            aFmtLb.SelectAll( FALSE );
            EnableEdit(FALSE);
        }
    }
}

//-------------------------------------------------------------------------

String SfxCommonTemplateDialog_Impl::GetSelectedEntry() const
{
    String aRet;
    if ( pTreeBox )
    {
        SvLBoxEntry* pEntry = pTreeBox->FirstSelected();
        if ( pEntry )
            aRet = pTreeBox->GetEntryText( pEntry );
    }
    else
    {
        SvLBoxEntry* pEntry = aFmtLb.FirstSelected();
        if ( pEntry )
            aRet = aFmtLb.GetEntryText( pEntry );
    }
    return aRet;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::EnableTreeDrag( BOOL bEnable )
{
    if ( pStyleSheetPool )
    {
        SfxStyleSheetBase* pStyle = pStyleSheetPool->First();
        if ( pTreeBox )
        {
            if ( pStyle && pStyle->HasParentSupport() && bEnable )
                pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
            else
                pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        }
    }
    bTreeDrag = bEnable;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::FillTreeBox()
{
    DBG_ASSERT( pTreeBox, "FillTreeBox() without treebox");
    if(pStyleSheetPool && nActFamily != 0xffff)
    {
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        pStyleSheetPool->SetSearchMask(pItem->GetFamily(), SFXSTYLEBIT_ALL);
        StyleTreeArr_Impl aArr;
        SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
        if(pStyle && pStyle->HasParentSupport() && bTreeDrag )
            pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
        else
            pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        while(pStyle)
        {
            StyleTree_ImplPtr pNew =
                new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
            aArr.Insert(pNew, aArr.Count());
            pStyle = pStyleSheetPool->Next();
        }
        MakeTree_Impl(aArr);
        ExpandedEntries aEntries;
        if(pTreeBox)
            ((const StyleTreeListBox_Impl *)pTreeBox)->
                MakeExpanded_Impl( aEntries);
        pTreeBox->SetUpdateMode( FALSE );
        pTreeBox->Clear();
        const USHORT nCount = aArr.Count();
        for(USHORT i = 0; i < nCount; ++i)
            FillBox_Impl(pTreeBox, aArr[i], aEntries);
;
        EnableItem(SID_STYLE_WATERCAN,FALSE);

        SfxTemplateItem* pState = pFamilyState[nActFamily-1];

        if ( nCount )
            pTreeBox->Expand( pTreeBox->First() );

        for ( SvLBoxEntry* pEntry = pTreeBox->First(); pEntry; pEntry = pTreeBox->Next( pEntry ) )
        {
            if ( IsExpanded_Impl( aEntries, pTreeBox->GetEntryText( pEntry ) ) )
                pTreeBox->Expand( pEntry );
        }

        pTreeBox->SetUpdateMode( TRUE );

        String aStyle;
        if(pState)  // Select current entry
            aStyle = pState->GetStyleName();
        SelectStyle(aStyle);
        EnableDelete();
    }
}

//-------------------------------------------------------------------------
BOOL SfxCommonTemplateDialog_Impl::HasSelectedStyle() const
{
    return pTreeBox? pTreeBox->FirstSelected() != 0:
            aFmtLb.GetSelectionCount() != 0;
}


//-------------------------------------------------------------------------

// internal: Refresh the display
// nFlags: what we should update.
void SfxCommonTemplateDialog_Impl::UpdateStyles_Impl(USHORT nFlags)
{
    DBG_ASSERT(nFlags, "nothing to do");
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    if (!pItem)
    {
        // Is the case for the template catalog
        SfxTemplateItem **ppItem = pFamilyState;
        const size_t nFamilyCount = pStyleFamilies->size();
        size_t n;
        for( n = 0; n < nFamilyCount; n++ )
            if( ppItem[ StyleNrToInfoOffset(n) ] ) break;
        if ( n == nFamilyCount )
            // It happens sometimes, God knows why
            return;
        ppItem += StyleNrToInfoOffset(n);
        nAppFilter = (*ppItem)->GetValue();
        FamilySelect(  StyleNrToInfoOffset(n)+1 );
        pItem = GetFamilyItem_Impl();
    }

    const SfxStyleFamily eFam = pItem->GetFamily();

    SfxFilterTupel* pT = ( nActFilter < pItem->GetFilterList().size() ? pItem->GetFilterList()[nActFilter] : NULL );
    USHORT nFilter     = pT ? pT->nFlags : 0;
    if(!nFilter)   // automatic
        nFilter = nAppFilter;

    DBG_ASSERT(pStyleSheetPool, "no StyleSheetPool");
    if(pStyleSheetPool)
    {
        pStyleSheetPool->SetSearchMask(eFam, nFilter);
        pItem = GetFamilyItem_Impl();
        if((nFlags & UPDATE_FAMILY) == UPDATE_FAMILY)   // Update view type list (Hierarchical, All, etc.
        {
            CheckItem(nActFamily, TRUE);    // check Button in Toolbox
            aFilterLb.SetUpdateMode(FALSE);
            aFilterLb.Clear();
            //insert hierarchical at the beginning
            USHORT nPos = aFilterLb.InsertEntry(String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)), 0);
            aFilterLb.SetEntryData( nPos, (void*)(ULONG)SFXSTYLEBIT_ALL );
            const SfxStyleFilter& rFilter = pItem->GetFilterList();
            for( size_t i = 0; i < rFilter.size(); ++i)
            {
                ULONG nFilterFlags = rFilter[ i ]->nFlags;
                nPos = aFilterLb.InsertEntry( rFilter[ i ]->aName );
                aFilterLb.SetEntryData( nPos, (void*)nFilterFlags );
            }
            if(nActFilter < aFilterLb.GetEntryCount() - 1)
                aFilterLb.SelectEntryPos(nActFilter + 1);
            else
            {
                nActFilter = 0;
                aFilterLb.SelectEntryPos(1);
                SfxFilterTupel* pActT = ( nActFilter < rFilter.size() ) ? rFilter[ nActFilter ] : NULL;
                USHORT nFilterFlags = pActT ? pActT->nFlags : 0;
                pStyleSheetPool->SetSearchMask(eFam, nFilterFlags);
            }

            // if the tree view again, select family hierarchy
            if(pTreeBox)
                aFilterLb.SelectEntry(String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)));

            // show maximum 12 entries
            aFilterLb.SetDropDownLineCount( MAX_FILTER_ENTRIES );
            aFilterLb.SetUpdateMode(TRUE);
        }
        else
        {
            if( nActFilter < aFilterLb.GetEntryCount() - 1)
                aFilterLb.SelectEntryPos(nActFilter + 1);
            else
            {
                nActFilter = 0;
                aFilterLb.SelectEntryPos(1);
            }
        }

        if(nFlags & UPDATE_FAMILY_LIST)
        {
            EnableItem(SID_STYLE_WATERCAN,FALSE);

            SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
            SvLBoxEntry* pEntry = aFmtLb.First();
            SvStringsDtor aStrings;

            comphelper::string::NaturalStringSorter aSorter(
                ::comphelper::getProcessComponentContext(),
                Application::GetSettings().GetLocale());

            while( pStyle )
            {
                //Bubblesort
                USHORT nPos;
                for( nPos = aStrings.Count() ; nPos &&
                    aSorter.compare(*(aStrings[nPos-1]), pStyle->GetName()) > 0 ; nPos--);
                aStrings.Insert( new String( pStyle->GetName() ), nPos );
                pStyle = pStyleSheetPool->Next();
            }


            USHORT nCount = aStrings.Count();
            USHORT nPos = 0;
            while( nPos < nCount && pEntry &&
                   *aStrings[ nPos ] == aFmtLb.GetEntryText( pEntry ) )
            {
                nPos++;
                pEntry = aFmtLb.Next( pEntry );
            }

            if( nPos < nCount || pEntry )
            {
                // Fills the display box
                aFmtLb.SetUpdateMode(FALSE);
                aFmtLb.Clear();

                nPos = 0;
                for(nPos = 0 ;  nPos < nCount ; ++nPos )
                {
                    aFmtLb.InsertEntry( *aStrings.GetObject( nPos ), 0, FALSE, nPos);
                }
                aFmtLb.SetUpdateMode(TRUE);
            }
            // Selects the current style if any
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            String aStyle;
            if(pState)
                aStyle = pState->GetStyleName();
            SelectStyle(aStyle);
            EnableDelete();
        }
    }
}

//-------------------------------------------------------------------------

// Updated display: Watering the house
void SfxCommonTemplateDialog_Impl::SetWaterCanState(const SfxBoolItem *pItem)
{
    bWaterDisabled =  pItem == 0;

    if(!bWaterDisabled)
        bWaterDisabled = !HasSelectedStyle();

    if(pItem && !bWaterDisabled)
    {
        CheckItem(SID_STYLE_WATERCAN, pItem->GetValue());
        EnableItem( SID_STYLE_WATERCAN, TRUE );
    }
    else
        if(!bWaterDisabled)
            EnableItem(SID_STYLE_WATERCAN, TRUE);
        else
            EnableItem(SID_STYLE_WATERCAN, FALSE);

// Ignore while in watercan mode statusupdates

    size_t nCount = pStyleFamilies->size();
    pBindings->EnterRegistrations();
    for(size_t n = 0; n < nCount; n++)
    {
        SfxControllerItem *pCItem=pBoundItems[n];
        BOOL bChecked = pItem && pItem->GetValue();
        if( pCItem->IsBound() == bChecked )
        {
            if( !bChecked )
                pCItem->ReBind();
            else
                pCItem->UnBind();
        }
    }
    pBindings->LeaveRegistrations();
}

//-------------------------------------------------------------------------

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>

void SfxCommonTemplateDialog_Impl::SetFamilyState( USHORT nSlotId, const SfxTemplateItem* pItem )
{
    USHORT nIdx = nSlotId - SID_STYLE_FAMILY_START;
    DELETEZ(pFamilyState[nIdx]);
    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem(*pItem);
    bUpdate = TRUE;

    // If used templates (how the hell you find this out??)
    bUpdateFamily = TRUE;
}

//-------------------------------------------------------------------------
// Notice from SfxBindings that the update is completed. Pushes out the update
// of the display.

void SfxCommonTemplateDialog_Impl::Update_Impl()
{
    BOOL bDocChanged=FALSE;
    SfxStyleSheetBasePool* pNewPool = NULL;
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
    if( pDocShell )
        pNewPool = pDocShell->GetStyleSheetPool();

    if ( pNewPool != pStyleSheetPool && pDocShell )
    {
        SfxModule* pNewModule = pDocShell->GetModule();
        if( pNewModule && pNewModule != pModule )
        {
            ClearResource();
            ReadResource();
        }
        if ( pStyleSheetPool )
        {
            EndListening(*pStyleSheetPool);
            pStyleSheetPool = 0;
        }

        if ( pNewPool )
        {
            StartListening(*pNewPool);
            pStyleSheetPool = pNewPool;
            bDocChanged=TRUE;
        }
    }

    if (bUpdateFamily)
        UpdateFamily_Impl();

    USHORT i;
    for(i = 0; i < MAX_FAMILIES; ++i)
        if(pFamilyState[i])
            break;
    if(i == MAX_FAMILIES || !pNewPool)
        // nothing is allowed
        return;

     SfxTemplateItem *pItem = 0;
     // current region not within the allowed region or default
     if(nActFamily == 0xffff || 0 == (pItem = pFamilyState[nActFamily-1] ) )
     {
         CheckItem(nActFamily, FALSE);
         SfxTemplateItem **ppItem = pFamilyState;
         const size_t nFamilyCount = pStyleFamilies->size();
         size_t n;
         for( n = 0; n < nFamilyCount; n++ )
             if( ppItem[ StyleNrToInfoOffset(n) ] ) break;
         ppItem+=StyleNrToInfoOffset(n);

         nAppFilter = (*ppItem)->GetValue();
         FamilySelect(  StyleNrToInfoOffset(n)+1 );

         pItem = *ppItem;
     }
     else if( bDocChanged )
     {
         // other DocShell -> all new
         CheckItem( nActFamily, TRUE );
         nActFilter = static_cast< USHORT >( LoadFactoryStyleFilter( pDocShell ) );
         if ( 0xFFFF == nActFilter )
            nActFilter = pDocShell->GetAutoStyleFilterIndex();

         nAppFilter = pItem->GetValue();
         if(!pTreeBox)
         {
             UpdateStyles_Impl(UPDATE_FAMILY_LIST);
         }
         else
             FillTreeBox();
     }
     else
     {
         // other filters for automatic
         CheckItem( nActFamily, TRUE );
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
#if OSL_DEBUG_LEVEL > 1
         SfxFilterTupel *pT;
         pT = pStyleItem->GetFilterList()[ nActFilter ];
#endif
         if (  0 == pStyleItem->GetFilterList()[ nActFilter ]->nFlags
            && nAppFilter != pItem->GetValue())
         {
             nAppFilter = pItem->GetValue();
             if(!pTreeBox)
                 UpdateStyles_Impl(UPDATE_FAMILY_LIST);
             else
                 FillTreeBox();
         }
         else
             nAppFilter = pItem->GetValue();
     }
     const String aStyle(pItem->GetStyleName());
     SelectStyle(aStyle);
     EnableDelete();
     EnableNew( bCanNew );
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, TimeOut, Timer *, pTim )
{
    (void)pTim; // unused
    if(!bDontUpdate)
    {
        bDontUpdate=TRUE;
        if(!pTreeBox)
            UpdateStyles_Impl(UPDATE_FAMILY_LIST);
        else
        {
            FillTreeBox();
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            if(pState)
            {
                const String aStyle(pState->GetStyleName());
                SelectStyle(aStyle);
                EnableDelete();
            }
        }
        bDontUpdate=FALSE;
        DELETEZ(pTimer);
    }
    else
        pTimer->Start();
    return 0;
}


//-------------------------------------------------------------------------
void SfxCommonTemplateDialog_Impl::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    // tap update
    if(rHint.Type() == TYPE(SfxSimpleHint))
    {
        switch(((SfxSimpleHint&) rHint ).GetId())
        {
          case SFX_HINT_UPDATEDONE:
            {
                SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
                SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
                if (
                    bUpdate &&
                    (
                     !IsCheckedItem(SID_STYLE_WATERCAN) ||
                     (pDocShell && pDocShell->GetStyleSheetPool() != pStyleSheetPool)
                    )
                   )
                {
                    bUpdate = FALSE;
                    Update_Impl();
                }
                else if ( bUpdateFamily )
                {
                    UpdateFamily_Impl();
                }

                if( pStyleSheetPool )
                {
                    String aStr = GetSelectedEntry();
                    if( aStr.Len() && pStyleSheetPool )
                    {
                        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                        if( !pItem ) break;
                        const SfxStyleFamily eFam = pItem->GetFamily();
                        SfxStyleSheetBase *pStyle =
                            pStyleSheetPool->Find(
                                aStr, eFam, SFXSTYLEBIT_ALL );
                        if( pStyle )
                            EnableEdit(
                                !(pStyle->GetMask() & SFXSTYLEBIT_READONLY) );
                        else
                            EnableEdit(FALSE);
                    }
                }
                break;
            }

        // Necessary if switching between documents and in both documents
        // the same template is used. Do not immediately call Update_Impl,
        // for the case that one of the documents is an internal InPlaceObjekt!
          case SFX_HINT_DOCCHANGED:
            bUpdate = TRUE;
            break;
          case SFX_HINT_DYING:
          {
            EndListening(*pStyleSheetPool);
            pStyleSheetPool=0;
            break;
          }
        }
    }

    // Do not set timer when the stylesheet pool is in the box, because it is
    // possible that a new one is registered after the timer is up -
    // works bad in UpdateStyles_Impl ()!

    ULONG nId = rHint.ISA(SfxSimpleHint) ? ( (SfxSimpleHint&)rHint ).GetId() : 0;

    if(!bDontUpdate && nId != SFX_HINT_DYING &&
       (rHint.Type() == TYPE(SfxStyleSheetPoolHint)||
       rHint.Type() == TYPE(SfxStyleSheetHint) ||
       rHint.Type() == TYPE( SfxStyleSheetHintExtended )))
    {
        if(!pTimer)
        {
            pTimer=new Timer;
            pTimer->SetTimeout(500);
            pTimer->SetTimeoutHdl(LINK(this,SfxCommonTemplateDialog_Impl,TimeOut));
        }
        pTimer->Start();

    }
}


//-------------------------------------------------------------------------

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void SfxCommonTemplateDialog_Impl::FilterSelect(
                USHORT nEntry,  // Idx of the new Filters
                BOOL bForce )   // Force update, even if the new filter is
                                // equal to the current
{
    if( nEntry != nActFilter || bForce )
    {
        nActFilter = nEntry;
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
        if (pDocShell)
        {
            pDocShell->SetAutoStyleFilterIndex(nActFilter);
            SaveFactoryStyleFilter( pDocShell, nActFilter );
        }

        SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
        pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): 0;
        if ( pOldStyleSheetPool != pStyleSheetPool )
        {
            if ( pOldStyleSheetPool )
                EndListening(*pOldStyleSheetPool);
            if ( pStyleSheetPool )
                StartListening(*pOldStyleSheetPool);
        }

        UpdateStyles_Impl(UPDATE_FAMILY_LIST);
    }
}

//-------------------------------------------------------------------------

// Internal: Perform functions through the Dispatcher
BOOL SfxCommonTemplateDialog_Impl::Execute_Impl(
    USHORT nId, const String &rStr, const String& rRefStr, USHORT nFamily,
    USHORT nMask, USHORT *pIdx, const USHORT* pModifier)
{
    SfxDispatcher &rDispatcher = *SFX_APP()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    USHORT nCount = 0;
    if( rStr.Len() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask )
        pItems[ nCount++ ] = &aMask;
    if(SID_STYLE_UPDATE_BY_EXAMPLE == nId)
    {
        // Special solution for Numbering update in Writer
        const String aTemplName(GetSelectedEntry());
        aUpdName.SetValue(aTemplName);
        pItems[ nCount++ ] = &aUpdName;
    }
    if ( rRefStr.Len() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = 0;

    Deleted aDeleted;
    pbDeleted = &aDeleted;
    USHORT nModi = pModifier ? *pModifier : 0;
    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
        pItems, nModi );

    // FIXME: Dialog can be destroyed while in Execute() check stack variable for dtor flag!
    if ( !pItem || aDeleted() )
        return FALSE;

    if ( nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId )
    {
        SfxUInt16Item *pFilterItem = PTR_CAST(SfxUInt16Item, pItem);
        DBG_ASSERT(pFilterItem, "SfxUINT16Item expected");
        USHORT nFilterFlags = pFilterItem->GetValue() & ~SFXSTYLEBIT_USERDEF;
        if(!nFilterFlags)       // User Template?
            nFilterFlags = pFilterItem->GetValue();
        const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
        const size_t nFilterCount = pFamilyItem->GetFilterList().size();

        for ( size_t i = 0; i < nFilterCount; ++i )
        {
            const SfxFilterTupel *pTupel = pFamilyItem->GetFilterList()[ i ];

            if ( ( pTupel->nFlags & nFilterFlags ) == nFilterFlags && pIdx )
                *pIdx = i;
        }
    }

    // Reset destroyed flag otherwise we use the pointer in the dtor
    // where the local stack object is already destroyed. This would
    // overwrite objects on the stack!! See #i100110
    pbDeleted = NULL;
    return TRUE;
}

//-------------------------------------------------------------------------

// Handler der Listbox der Filter
IMPL_LINK( SfxCommonTemplateDialog_Impl, FilterSelectHdl, ListBox *, pBox )
{
    if ( pBox->GetSelectEntry() ==  String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)) )
    {
        if ( !bHierarchical )
        {
            // Turn on treeView
            bHierarchical=TRUE;
            const String aSelectEntry( GetSelectedEntry());
            aFmtLb.Hide();

            pTreeBox = new StyleTreeListBox_Impl(
                    this, WB_HASBUTTONS | WB_HASLINES |
                    WB_BORDER | WB_TABSTOP | WB_HASLINESATROOT |
                    WB_HASBUTTONSATROOT | WB_HIDESELECTION );
            pTreeBox->SetFont( aFmtLb.GetFont() );

            pTreeBox->SetPosSizePixel(aFmtLb.GetPosPixel(), aFmtLb.GetSizePixel());
            pTreeBox->SetNodeDefaultImages();
            pTreeBox->SetSelectHdl(
                LINK(this, SfxCommonTemplateDialog_Impl, FmtSelectHdl));
            ((StyleTreeListBox_Impl*)pTreeBox)->
                SetDoubleClickHdl(
                    LINK(this, SfxCommonTemplateDialog_Impl,  ApplyHdl));
            ((StyleTreeListBox_Impl*)pTreeBox)->
                SetDropHdl(LINK(this, SfxCommonTemplateDialog_Impl,  DropHdl));
            pTreeBox->SetIndent(10);
            SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
            SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
            if (pDocShell)
                SaveFactoryStyleFilter( pDocShell, HIERARCHICAL_FILTER_INDEX );
            FillTreeBox();
            SelectStyle(aSelectEntry);
            pTreeBox->Show();
        }
    }

    else
    {
        DELETEZ(pTreeBox);
        aFmtLb.Show();
        //                              aFilterLb.Enable();
        // If bHierarchical, then the family can have changed
        // minus one since hierarchical is inserted at the start
        FilterSelect(pBox->GetSelectEntryPos() - 1, bHierarchical );
        bHierarchical=FALSE;
    }

    return 0;
}

//-------------------------------------------------------------------------

// Select-Handler for the Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(USHORT nEntry)
{
    if( nEntry != nActFamily )
    {
        CheckItem( nActFamily, FALSE );
        nActFamily = nEntry;
        SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
        SfxUInt16Item aItem( SID_STYLE_FAMILY, nEntry );
        pDispat->Execute( SID_STYLE_FAMILY, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
        pBindings->Invalidate( SID_STYLE_FAMILY );
        pBindings->Update( SID_STYLE_FAMILY );
        UpdateFamily_Impl();
    }
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ActionSelect(USHORT nEntry)
{
    String aEmpty;
    switch(nEntry)
    {
        case SID_STYLE_WATERCAN:
        {
            const BOOL bState = IsCheckedItem(nEntry);
            BOOL bCheck;
            SfxBoolItem aBool;
            // when a template is chosen.
            if(!bState && aFmtLb.GetSelectionCount())
            {
                const String aTemplName(
                    GetSelectedEntry());
                Execute_Impl(
                    SID_STYLE_WATERCAN, aTemplName, aEmpty,
                    (USHORT)GetFamilyItem_Impl()->GetFamily() );
                bCheck = TRUE;
            }
            else
            {
                Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
                bCheck = FALSE;
            }
            CheckItem(nEntry, bCheck);
            aBool.SetValue(bCheck);
            SetWaterCanState(&aBool);
            break;
        }
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if(pStyleSheetPool && nActFamily != 0xffff)
            {
                const SfxStyleFamily eFam=GetFamilyItem_Impl()->GetFamily();
                const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                USHORT nFilter;
                if( pItem && nActFilter != 0xffff )
                {
                    nFilter = pItem->GetFilterList()[ nActFilter ]->nFlags;
                    if(!nFilter)    // automatisch
                        nFilter = nAppFilter;
                }
                else
                    nFilter=pStyleSheetPool->GetSearchMask();
                pStyleSheetPool->SetSearchMask( eFam, SFXSTYLEBIT_USERDEF );

                SfxNewStyleDlg *pDlg = new SfxNewStyleDlg(pWindow, *pStyleSheetPool);
                    // why? : FloatingWindow must not be parent of a modal dialog
                if(RET_OK == pDlg->Execute())
                {
                    pStyleSheetPool->SetSearchMask(eFam, nFilter);
                    const String aTemplName(pDlg->GetName());
                    Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE,
                                 aTemplName, aEmpty,
                                 (USHORT)GetFamilyItem_Impl()->GetFamily(),
                                 nFilter);
                }
                pStyleSheetPool->SetSearchMask( eFam, nFilter );
                delete pDlg;
            }
            break;
        }
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            Execute_Impl(SID_STYLE_UPDATE_BY_EXAMPLE,
                    aEmpty, aEmpty,
                    (USHORT)GetFamilyItem_Impl()->GetFamily());
            break;
        }
        case SID_TEMPLATE_LOAD:
            SFX_APP()->GetDispatcher_Impl()->Execute(nEntry);
        break;
        default: OSL_FAIL("not implemented"); break;
    }
}

//-------------------------------------------------------------------------

static rtl::OUString getModuleIdentifier( const Reference< XModuleManager >& i_xModMgr, SfxObjectShell* i_pObjSh )
{
    DBG_ASSERT( i_xModMgr.is(), "getModuleIdentifier(): no XModuleManager" );
    DBG_ASSERT( i_pObjSh, "getModuleIdentifier(): no ObjectShell" );

    ::rtl::OUString sIdentifier;

    try
    {
        sIdentifier = i_xModMgr->identify( i_pObjSh->GetModel() );
    }
    catch ( ::com::sun::star::frame::UnknownModuleException& )
    {
        DBG_WARNING( "getModuleIdentifier(): unknown module" );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getModuleIdentifier(): exception of XModuleManager::identify()" );
    }

    return sIdentifier;
}

//-------------------------------------------------------------------------

sal_Int32 SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh )
{
    DBG_ASSERT( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    sal_Int32 nFilter = -1;

    Sequence< PropertyValue > lProps;
    Reference< ::com::sun::star::container::XNameAccess > xContainer( xModuleManager, UNO_QUERY );
    if ( xContainer.is() )
    {
        ::comphelper::SequenceAsHashMap aFactoryProps(
            xContainer->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
        sal_Int32 nDefault = -1;
        nFilter = aFactoryProps.getUnpackedValueOrDefault( DEFINE_CONST_UNICODE("ooSetupFactoryStyleFilter"), nDefault );
    }

    return nFilter;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::SaveFactoryStyleFilter( SfxObjectShell* i_pObjSh, sal_Int32 i_nFilter )
{
    DBG_ASSERT( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    Reference< ::com::sun::star::container::XNameReplace > xContainer( xModuleManager, UNO_QUERY );
    if ( xContainer.is() )
    {
        Sequence< PropertyValue > lProps(1);
        lProps[0].Name = DEFINE_CONST_UNICODE("ooSetupFactoryStyleFilter");
        lProps[0].Value = makeAny( i_nFilter );;
        xContainer->replaceByName( getModuleIdentifier( xModuleManager, i_pObjSh ), makeAny( lProps ) );
    }
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, DropHdl, StyleTreeListBox_Impl *, pBox )
{
    bDontUpdate=TRUE;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    long ret= pStyleSheetPool->SetParent(eFam,pBox->GetStyle(), pBox->GetParent())? 1L: 0L;
    bDontUpdate=FALSE;
    return ret;
}

//-------------------------------------------------------------------------

// Handler for the New-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl(void *)
{
    String aEmpty;
    if ( nActFamily != 0xffff )
    {
        Window* pTmp;
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );

        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam=pItem->GetFamily();
        USHORT nMask;
        if( pItem && nActFilter != 0xffff )
        {
            nMask = pItem->GetFilterList()[ nActFilter ]->nFlags;
            if(!nMask)    // automatic
                nMask = nAppFilter;
        }
        else
            nMask=pStyleSheetPool->GetSearchMask();

        pStyleSheetPool->SetSearchMask(eFam,nMask);

        Execute_Impl(SID_STYLE_NEW,
                     aEmpty, GetSelectedEntry(),
                     ( USHORT )GetFamilyItem_Impl()->GetFamily(),
                     nMask);

        Application::SetDefDialogParent( pTmp );
    }
}

//-------------------------------------------------------------------------

// Handler for the edit-Buttons
void SfxCommonTemplateDialog_Impl::EditHdl(void *)
{
    if(IsInitialized() && HasSelectedStyle())
    {
        USHORT nFilter = nActFilter;
        String aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        pStyleSheetPool->Find(aTemplName,eFam,SFXSTYLEBIT_ALL);  // -Wall required??
        Window* pTmp;
        //DefModalDialogParent set for modality of the following dialogs
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );
        if ( Execute_Impl( SID_STYLE_EDIT, aTemplName, String(),
                          (USHORT)GetFamilyItem_Impl()->GetFamily(), 0, &nFilter ) )
        {
        }
        Application::SetDefDialogParent( pTmp );
    }
}

//-------------------------------------------------------------------------

// Handler for the Delete-Buttons
void SfxCommonTemplateDialog_Impl::DeleteHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        const String aTemplName( GetSelectedEntry() );
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        SfxStyleSheetBase* pStyle =
            pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );
        if ( pStyle )
        {
            String aMsg;
            if ( pStyle->IsUsed() )
                aMsg = String( SfxResId( STR_DELETE_STYLE_USED ) );
            aMsg += String ( SfxResId( STR_DELETE_STYLE ) );
            aMsg.SearchAndReplaceAscii( "$1", aTemplName );
#if defined UNX
            QueryBox aBox( SFX_APP()->GetTopWindow(), WB_YES_NO | WB_DEF_NO, aMsg );
#else
            QueryBox aBox( GetWindow(), WB_YES_NO | WB_DEF_NO , aMsg );
#endif
            if ( RET_YES == aBox.Execute() )
            {
                PrepareDeleteAction();

                if ( pTreeBox ) // To prevent the Treelistbox to shut down while
                                // deleting.
                {
                    bDontUpdate = TRUE;
                }
                Execute_Impl( SID_STYLE_DELETE, aTemplName,
                              String(), (USHORT)GetFamilyItem_Impl()->GetFamily() );

                if ( pTreeBox )
                {
                    pTreeBox->RemoveParentKeepChilds( pTreeBox->FirstSelected() );
                    bDontUpdate = FALSE;
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void    SfxCommonTemplateDialog_Impl::EnableDelete()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        DBG_ASSERT(pStyleSheetPool, "No StyleSheetPool");
        const String aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        USHORT nFilter = 0;
        if(pItem->GetFilterList().size() > nActFilter)
            nFilter = pItem->GetFilterList()[ nActFilter ]->nFlags;
        if(!nFilter)    // automatic
            nFilter = nAppFilter;
        const SfxStyleSheetBase *pStyle =
            pStyleSheetPool->Find(aTemplName,eFam, pTreeBox? SFXSTYLEBIT_ALL: nFilter);

        DBG_ASSERT(pStyle, "Style ot found");
        if(pStyle && pStyle->IsUserDefined())
        {
            EnableDel(TRUE);
        }
        else
        {
            EnableDel(FALSE);
        }
    }
    else
    {
        EnableDel(FALSE);
    }
}

//-------------------------------------------------------------------------
// After selecting a focused item if possible again on the app window
void    SfxCommonTemplateDialog_Impl::ResetFocus()
{
    if(ISA(SfxTemplateDialog_Impl))
    {
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell *pVu = pViewFrame->GetViewShell();
        Window *pAppWin = pVu ? pVu->GetWindow(): 0;
        if(pAppWin)
            pAppWin->GrabFocus();
    }
}

//-------------------------------------------------------------------------

// Doppelclick on a style sheet in the ListBox is applied.
IMPL_LINK( SfxCommonTemplateDialog_Impl, ApplyHdl, Control *, pControl )
{
    (void)pControl; //unused
    // only if that region is allowed
    if ( IsInitialized() && 0 != pFamilyState[nActFamily-1] &&
         GetSelectedEntry().Len() )
    {
        USHORT nModifier = aFmtLb.GetModifier();
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), String(),
                     ( USHORT )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
        if(ISA(SfxTemplateCatalog_Impl))
            ((SfxTemplateCatalog_Impl*) this)->pReal->EndDialog(RET_OK);
    }
    ResetFocus();
    return 0;
}

//-------------------------------------------------------------------------

// Selection of a template during the Watercan-Status
IMPL_LINK( SfxCommonTemplateDialog_Impl, FmtSelectHdl, SvTreeListBox *, pListBox )
{
    // Trigger Help PI, if this is permitted of call handlers and field
    if( !pListBox || pListBox->IsSelected( pListBox->GetHdlEntry() ) )
    {
        // Only when the watercan is on
        if ( IsInitialized() &&
             IsCheckedItem(SID_STYLE_WATERCAN) &&
             // only if that region is allowed
             0 != pFamilyState[nActFamily-1] )
        {
            String aEmpty;
            Execute_Impl(SID_STYLE_WATERCAN,
                         aEmpty, aEmpty, 0);
            Execute_Impl(SID_STYLE_WATERCAN,
                         GetSelectedEntry(), aEmpty,
                         ( USHORT )GetFamilyItem_Impl()->GetFamily());
        }
        EnableItem(SID_STYLE_WATERCAN, !bWaterDisabled);
        EnableDelete();
    }
    if( pListBox )
        SelectStyle( pListBox->GetEntryText( pListBox->GetHdlEntry() ));

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, MenuSelectHdl, Menu *, pMenu )
{
    if( pMenu )
    {
        nLastItemId = pMenu->GetCurItemId();
        Application::PostUserEvent(
            LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ), 0 );
        return TRUE;
    }

    switch(nLastItemId) {
    case ID_NEW: NewHdl(0); break;
    case ID_EDIT: EditHdl(0); break;
    case ID_DELETE: DeleteHdl(0); break;
    default: return FALSE;
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ExecuteContextMenu_Impl( const Point& rPos, Window* pWin )
{
    // Bug# 94152: This part should never be called, because before this happens, the TreeListBox should captured this!
    DBG_ASSERT( FALSE, "+SfxCommonTemplateDialog_Impl::ExecuteContextMenu_Impl(): How could this happen? Please infirm developer ASAP!" );

    PopupMenu* pMenu = CreateContextMenu();
    pMenu->Execute( pWin, rPos );
    delete pMenu;
}

// -----------------------------------------------------------------------

SfxStyleFamily SfxCommonTemplateDialog_Impl::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SFX_STYLE_FAMILY_PARA;
    else
        return pFamilyItem->GetFamily();
}

// -----------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(USHORT nId, BOOL bEnable)
{
    if( nId == SID_STYLE_NEW_BY_EXAMPLE )
        bNewByExampleDisabled = !bEnable;
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
        bUpdateByExampleDisabled = !bEnable;
    EnableItem(nId, bEnable);
}

void SfxCommonTemplateDialog_Impl::PrepareDeleteAction()
{
}

// -----------------------------------------------------------------------

PopupMenu* SfxCommonTemplateDialog_Impl::CreateContextMenu( void )
{
    if ( bBindingUpdate )
    {
        pBindings->Invalidate( SID_STYLE_NEW, TRUE, FALSE );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = FALSE;
    }
    PopupMenu* pMenu = new PopupMenu( SfxResId( MN_CONTEXT_TEMPLDLG ) );
    pMenu->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ) );
    pMenu->EnableItem( ID_EDIT, bCanEdit );
    pMenu->EnableItem( ID_DELETE, bCanDel );
    pMenu->EnableItem( ID_NEW, bCanNew );

    return pMenu;
}

// ------------------------------------------------------------------------

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(
    Window* /*pParent*/, SfxBindings* pB, SfxTemplateDialog* pDlgWindow ) :

    SfxCommonTemplateDialog_Impl( pB, pDlgWindow ),

    m_pFloat            ( pDlgWindow ),
    m_bZoomIn           ( FALSE ),
    m_aActionTbL        ( pDlgWindow, this ),
    m_aActionTbR        ( pDlgWindow, SfxResId( TB_ACTION ) )

{
    pDlgWindow->FreeResource();
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    USHORT nSavedFilter = static_cast< USHORT >( LoadFactoryStyleFilter( pCurObjShell ) );
    Initialize();

    m_aActionTbL.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    m_aActionTbR.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_aActionTbR.SetDropdownClickHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRClick));
    m_aActionTbL.Show();
    m_aActionTbR.Show();
    Font aFont=aFilterLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFilterLb.SetFont( aFont );
    m_aActionTbL.SetHelpId( HID_TEMPLDLG_TOOLBOX_LEFT );
    if( nSavedFilter == HIERARCHICAL_FILTER_INDEX )
    {
        bHierarchical = FALSE; // Force content refresh
        aFilterLb.SelectEntry(String(SfxResId(STR_STYLE_FILTER_HIERARCHICAL)));
        FilterSelectHdl(&aFilterLb);
    }
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableFamilyItem( USHORT nId, BOOL bEnable )
{
    m_aActionTbL.EnableItem( nId, bEnable );
}

//-------------------------------------------------------------------------
// Insert element into dropdown filter "Frame Styles", "List Styles", etc.

void SfxTemplateDialog_Impl::InsertFamilyItem(USHORT nId,const SfxStyleFamilyItem *pItem)
{
    USHORT nHelpId = 0;
    switch( (USHORT) pItem->GetFamily() )
    {
        case SFX_STYLE_FAMILY_CHAR: nHelpId = SID_STYLE_FAMILY1; break;
        case SFX_STYLE_FAMILY_PARA: nHelpId = SID_STYLE_FAMILY2; break;
        case SFX_STYLE_FAMILY_FRAME:nHelpId = SID_STYLE_FAMILY3; break;
        case SFX_STYLE_FAMILY_PAGE: nHelpId = SID_STYLE_FAMILY4; break;
        case SFX_STYLE_FAMILY_PSEUDO: nHelpId = SID_STYLE_FAMILY5; break;
        default: OSL_FAIL("unknown StyleFamily"); break;
    }
    m_aActionTbL.InsertItem( nId, pItem->GetImage(), pItem->GetText(), 0, 0);
    m_aActionTbL.SetHelpId( nId, nHelpId );
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    m_aActionTbR.HideItem(SID_STYLE_UPDATE_BY_EXAMPLE);
    m_aActionTbR.SetItemBits( SID_STYLE_NEW_BY_EXAMPLE,
            TIB_DROPDOWNONLY|m_aActionTbR.GetItemBits( SID_STYLE_NEW_BY_EXAMPLE ));
}

// ------------------------------------------------------------------------
void SfxTemplateDialog_Impl::updateFamilyImages()
{
    if ( !m_pStyleFamiliesId )
        // we do not have a resource id to load the new images from
        return;

    // let the families collection update the images
    pStyleFamilies->updateImages( *m_pStyleFamiliesId );

    // and set the new images on our toolbox
    size_t nLoop = pStyleFamilies->size();
    for( ; nLoop--; )
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->at( nLoop );
        USHORT nId = SfxFamilyIdToNId( pItem->GetFamily() );
        m_aActionTbL.SetItemImage( nId, pItem->GetImage() );
    }
}

// ------------------------------------------------------------------------
void SfxTemplateDialog_Impl::updateNonFamilyImages()
{
    m_aActionTbR.SetImageList( ImageList( SfxResId( DLG_STYLE_DESIGNER ) ) );
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::ClearFamilyList()
{
    m_aActionTbL.Clear();
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::InvalidateBindings()
{
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, TRUE, FALSE);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, TRUE, FALSE);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, TRUE, FALSE);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, TRUE, FALSE );
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, TRUE, FALSE );
    pBindings->Update( SID_STYLE_DRAGHIERARCHIE );
}

//-------------------------------------------------------------------------

SfxTemplateDialog_Impl::~SfxTemplateDialog_Impl()
{
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::LoadedFamilies()
{
    updateFamilyImages();
    Resize();
}

//-------------------------------------------------------------------------

// Overloaded Resize-Handler ( StarView )
// The size of the Listboxen is adjusted
void SfxTemplateDialog_Impl::Resize()
{
    FloatingWindow *pF = m_pFloat->GetFloatingWindow();
    if ( pF )
    {
        m_bZoomIn = pF->IsRollUp();
        if ( m_bZoomIn )
            return;
    }

    Size aDlgSize=m_pFloat->PixelToLogic(m_pFloat->GetOutputSizePixel());
    Size aSizeATL=m_pFloat->PixelToLogic(m_aActionTbL.CalcWindowSizePixel());
    Size aSizeATR=m_pFloat->PixelToLogic(m_aActionTbR.CalcWindowSizePixel());
    Size aMinSize = GetMinOutputSizePixel();

    long nListHeight = m_pFloat->PixelToLogic( aFilterLb.GetSizePixel() ).Height();
    long nWidth = aDlgSize.Width()- 2 * SFX_TEMPLDLG_HFRAME;

    m_aActionTbL.SetPosSizePixel(m_pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME,SFX_TEMPLDLG_VTOPFRAME)),
                                 m_pFloat->LogicToPixel(aSizeATL));

    // only change the position of the right toolbox, when the window is wide
    // enough
    Point aPosATR(aDlgSize.Width()-SFX_TEMPLDLG_HFRAME-aSizeATR.Width(),SFX_TEMPLDLG_VTOPFRAME);
    if(aDlgSize.Width() >= aMinSize.Width())
        m_aActionTbR.SetPosPixel(m_pFloat->LogicToPixel(aPosATR));
    else
        m_aActionTbR.SetPosPixel( m_pFloat->LogicToPixel(
            Point( SFX_TEMPLDLG_HFRAME + aSizeATL.Width() + SFX_TEMPLDLG_MIDHSPACE,
                   SFX_TEMPLDLG_VTOPFRAME ) ) );

    m_aActionTbR.SetSizePixel(m_pFloat->LogicToPixel(aSizeATR));

    Point aFilterPos(
        m_pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME,
            aDlgSize.Height()-SFX_TEMPLDLG_VBOTFRAME-nListHeight)) );

    Size aFilterSize(
        m_pFloat->LogicToPixel(Size(nWidth,SFX_TEMPLDLG_FILTERHEIGHT)) );

    Point aFmtPos(
        m_pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME, SFX_TEMPLDLG_VTOPFRAME +
                            SFX_TEMPLDLG_MIDVSPACE+aSizeATL.Height())) );
    Size aFmtSize(
        m_pFloat->LogicToPixel(Size(nWidth,
                    aDlgSize.Height() - SFX_TEMPLDLG_VBOTFRAME -
                    SFX_TEMPLDLG_VTOPFRAME - 2*SFX_TEMPLDLG_MIDVSPACE-
                    nListHeight-aSizeATL.Height())) );

    // only change the position of the listbox, when the window is high enough
    if(aDlgSize.Height() >= aMinSize.Height())
    {
        aFilterLb.SetPosPixel(aFilterPos);
        aFmtLb.SetPosPixel( aFmtPos );
        if(pTreeBox)
            pTreeBox->SetPosPixel(aFmtPos);
    }
    else
        aFmtSize.Height() += aFilterSize.Height();

    aFilterLb.SetSizePixel(aFilterSize);
    aFmtLb.SetSizePixel( aFmtSize );
    if(pTreeBox)
        pTreeBox->SetSizePixel(aFmtSize);
}

// -----------------------------------------------------------------------


Size SfxTemplateDialog_Impl::GetMinOutputSizePixel()
{
    Size aSizeATL=m_pFloat->PixelToLogic(m_aActionTbL.CalcWindowSizePixel());
    Size aSizeATR=m_pFloat->PixelToLogic(m_aActionTbR.CalcWindowSizePixel());
    Size aMinSize=Size(
        aSizeATL.Width()+aSizeATR.Width()+
        2*SFX_TEMPLDLG_HFRAME + SFX_TEMPLDLG_MIDHSPACE,
        4*aSizeATL.Height()+2*SFX_TEMPLDLG_MIDVSPACE);
    return aMinSize;
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::Command( const CommandEvent& rCEvt )
{
    if(COMMAND_CONTEXTMENU  == rCEvt.GetCommand())
        ExecuteContextMenu_Impl( rCEvt.GetMousePosPixel(), m_pFloat );
    else
        m_pFloat->Command(rCEvt);
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableItem(USHORT nMesId, BOOL bCheck)
{
    String aEmpty;
    switch(nMesId)
    {
      case SID_STYLE_WATERCAN :
          if(!bCheck && IsCheckedItem(SID_STYLE_WATERCAN))
            Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
      case SID_STYLE_NEW_BY_EXAMPLE:
      case SID_STYLE_UPDATE_BY_EXAMPLE:
        m_aActionTbR.EnableItem(nMesId,bCheck);
        break;
    }
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::CheckItem(USHORT nMesId, BOOL bCheck)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            bIsWater=bCheck;
            m_aActionTbR.CheckItem(SID_STYLE_WATERCAN,bCheck);
            break;
        default:
            m_aActionTbL.CheckItem(nMesId,bCheck); break;
    }
}

//-------------------------------------------------------------------------

BOOL SfxTemplateDialog_Impl::IsCheckedItem(USHORT nMesId)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            return m_aActionTbR.GetItemState(SID_STYLE_WATERCAN)==STATE_CHECK;
        default:
            return m_aActionTbL.GetItemState(nMesId)==STATE_CHECK;
    }
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )
{
    const USHORT nEntry = pBox->GetCurItemId();
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )

//-------------------------------------------------------------------------
::rtl::OUString lcl_GetLabel(uno::Any& rAny)
{
    ::rtl::OUString sRet;
    uno::Sequence< beans::PropertyValue >aPropSeq;
    if ( rAny >>= aPropSeq )
    {
        for( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
        {
            if ( aPropSeq[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Label" ) ))
            {
                aPropSeq[i].Value >>= sRet;
                break;
            }
        }
    }
    return sRet;
}
//-------------------------------------------------------------------------

IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxRSelect, ToolBox *, pBox )
{
    const USHORT nEntry = pBox->GetCurItemId();
    if(nEntry != SID_STYLE_NEW_BY_EXAMPLE ||
            TIB_DROPDOWN != (pBox->GetItemBits(nEntry)&TIB_DROPDOWN))
        ActionSelect(nEntry);
    return 0;
}
//-------------------------------------------------------------------------
IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxRClick, ToolBox *, pBox )
{
    const USHORT nEntry = pBox->GetCurItemId();
    if(nEntry == SID_STYLE_NEW_BY_EXAMPLE &&
            TIB_DROPDOWN == (pBox->GetItemBits(nEntry)&TIB_DROPDOWN))
    {
        //create a popup menu in Writer
        PopupMenu *pMenu = new PopupMenu;
        uno::Reference< container::XNameAccess > xNameAccess(
                    ::comphelper::getProcessServiceFactory()->
                    createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.frame.UICommandDescription")) ), uno::UNO_QUERY );
        uno::Reference< container::XNameAccess > xUICommands;
        if ( xNameAccess.is() )
        {
            rtl::OUString sTextDoc(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument"));
            if(xNameAccess->hasByName(sTextDoc))
            {
                uno::Any a = xNameAccess->getByName( sTextDoc );
                a >>= xUICommands;
            }
        }
        if(!xUICommands.is())
            return 0;
        try
        {
            uno::Sequence< beans::PropertyValue > aPropSeq;
            uno::Any aCommand = xUICommands->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:StyleNewByExample")));
            ::rtl::OUString sLabel = lcl_GetLabel( aCommand );
            pMenu->InsertItem( SID_STYLE_NEW_BY_EXAMPLE, sLabel );
            pMenu->SetHelpId(SID_STYLE_NEW_BY_EXAMPLE, HID_TEMPLDLG_NEWBYEXAMPLE);

            aCommand = xUICommands->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:StyleUpdateByExample")));
            sLabel = lcl_GetLabel( aCommand );

            pMenu->InsertItem( SID_STYLE_UPDATE_BY_EXAMPLE, sLabel );
            pMenu->SetHelpId(SID_STYLE_UPDATE_BY_EXAMPLE, HID_TEMPLDLG_UPDATEBYEXAMPLE);

            aCommand = xUICommands->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:LoadStyles")));
            sLabel = lcl_GetLabel( aCommand );
            pMenu->InsertItem( SID_TEMPLATE_LOAD, sLabel );
            pMenu->SetHelpId(SID_TEMPLATE_LOAD, SID_TEMPLATE_LOAD);

            pMenu->SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, MenuSelectHdl));
            pMenu->Execute( pBox,
                            pBox->GetItemRect(nEntry),
                            POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
        }
        catch(uno::Exception&)
        {
        }
        delete pMenu;
        pBox->Invalidate();
    }
    return 0;
}
//-------------------------------------------------------------------------
IMPL_LINK( SfxTemplateDialog_Impl, MenuSelectHdl, Menu*, pMenu)
{
    USHORT nMenuId = pMenu->GetCurItemId();
    ActionSelect(nMenuId);
    return 0;
}
//-------------------------------------------------------------------------

SfxTemplateCatalog_Impl::SfxTemplateCatalog_Impl( Window* /*pParent*/, SfxBindings* pB,
                                                  SfxTemplateCatalog* pTmpWindow ) :

    SfxCommonTemplateDialog_Impl( pB, pTmpWindow ),

    aFamList    ( pTmpWindow, SfxResId( BT_TOOL ) ),
    aOkBtn      ( pTmpWindow, SfxResId( BT_OK ) ),
    aCancelBtn  ( pTmpWindow, SfxResId( BT_CANCEL ) ),
    aNewBtn     ( pTmpWindow, SfxResId( BT_NEW ) ),
    aChangeBtn  ( pTmpWindow, SfxResId( BT_EDIT ) ),
    aDelBtn     ( pTmpWindow, SfxResId( BT_DEL ) ),
    aOrgBtn     ( pTmpWindow, SfxResId( BT_ORG ) ),
    aHelpBtn    ( pTmpWindow, SfxResId( BT_HELP ) ),
    pReal       ( pTmpWindow ),
    aHelper     ( pTmpWindow )

{
    aNewBtn.Disable();
    aDelBtn.Disable();
    aChangeBtn.Disable();

    SFX_APP()->Get_Impl()->pTemplateCommon = GetISfxTemplateCommon();
    pTmpWindow->FreeResource();

    Initialize();

    aFamList.SetSelectHdl(  LINK( this, SfxTemplateCatalog_Impl, FamListSelect ) );
    aOkBtn.SetClickHdl(     LINK( this, SfxTemplateCatalog_Impl, OkHdl ) );
    aCancelBtn.SetClickHdl( LINK( this, SfxTemplateCatalog_Impl, CancelHdl ) );
    aNewBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, NewHdl ) );
    aDelBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, DelHdl ) );
    aChangeBtn.SetClickHdl( LINK( this, SfxTemplateCatalog_Impl, ChangeHdl ) );
    aOrgBtn.SetClickHdl(    LINK( this, SfxTemplateCatalog_Impl, OrgHdl ) );
}

//-------------------------------------------------------------------------

SfxTemplateCatalog_Impl::~SfxTemplateCatalog_Impl()
{
    SFX_APP()->Get_Impl()->pTemplateCommon = 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, OkHdl, Button *, pButton )
{
    (void)pButton; //unused
    ApplyHdl( NULL );
    pReal->EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, OkHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, CancelHdl, Button *, pButton )
{
    (void)pButton; //unused
    pReal->EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, CancelHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, NewHdl, Button *, pButton )
{
    (void)pButton; //unused
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxCommonTemplateDialog_Impl::NewHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, NewHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, ChangeHdl, Button *, pButton )
{
    (void)pButton; //unused
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxCommonTemplateDialog_Impl::EditHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, ChangeHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, DelHdl, Button *, pButton )
{
    (void)pButton; //unused
    SfxCommonTemplateDialog_Impl::DeleteHdl( NULL );
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, DelHdl, Button *, pButton )

//-------------------------------------------------------------------------

IMPL_LINK( SfxTemplateCatalog_Impl, OrgHdl, Button *, pButton )
{
    (void)pButton; //unused
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
    SfxDocumentTemplates aTemplates;
    aTemplates.Construct();
    SfxTemplateOrganizeDlg* pDlg = new SfxTemplateOrganizeDlg( pReal, &aTemplates );
    const short nRet = pDlg->Execute();
    delete pDlg;
    if ( RET_OK == nRet )
        Update_Impl();
    else if ( RET_EDIT_STYLE == nRet )
        pReal->EndDialog( RET_CANCEL );
    return 0;
}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableEdit( BOOL bEnable )
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    aChangeBtn.Enable( bEnable );
}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableDel( BOOL bEnable )
{
    SfxCommonTemplateDialog_Impl::EnableDel( bEnable );
    aDelBtn.Enable( bEnable );
}

void SfxTemplateCatalog_Impl::EnableNew(BOOL bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableNew( bEnable );
    aNewBtn.Enable( bEnable );
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxTemplateCatalog_Impl, FamListSelect, ListBox *, pList )
{
    const USHORT nEntry = aFamIds[pList->GetSelectEntryPos()];
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateCatalog_Impl, FamListSelect, ListBox *, pList )

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::EnableItem( USHORT nMesId, BOOL bCheck )
{
    if ( nMesId == SID_STYLE_WATERCAN )
        aOkBtn.Enable( bCheck );
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return;
}

//-------------------------------------------------------------------------

void SfxTemplateCatalog_Impl::CheckItem(USHORT nMesId, BOOL /*bCheck*/)
{
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return;
    USHORT i;
    for ( i = 0; i < aFamIds.Count() && aFamIds[i] != nMesId; i++ ) ;
    aFamList.SelectEntryPos(i);
}

//-------------------------------------------------------------------------

BOOL SfxTemplateCatalog_Impl::IsCheckedItem(USHORT nMesId)
{
    if ( nMesId > SFX_STYLE_FAMILY_PSEUDO || nMesId < SFX_STYLE_FAMILY_CHAR )
        return FALSE;
    USHORT i;
    for ( i = 0; i < aFamIds.Count() && aFamIds[i] != nMesId; i++ )
        ;
    return aFamList.IsEntrySelected( String::CreateFromInt32(i) );
}

//-------------------------------------------------------------------------
// The list has only to master the disabling, since during his life time no
// changes in selection can be made,
void SfxTemplateCatalog_Impl::EnableFamilyItem( USHORT nId, BOOL bEnable )
{
    if ( !bEnable )
        for ( USHORT nPos = aFamIds.Count(); nPos--; )
            if ( aFamIds[ nPos ] == nId )
            {
                aFamIds.Remove( nPos );
                aFamList.RemoveEntry( nPos );
            }
}

void SfxTemplateCatalog_Impl::InsertFamilyItem( USHORT nId, const SfxStyleFamilyItem* pItem )
{
    if ( nId > SFX_STYLE_FAMILY_PSEUDO || nId < SFX_STYLE_FAMILY_CHAR )
        return;
    aFamList.InsertEntry( pItem->GetText(), 0 );
    aFamIds.Insert( nId, 0 );
}

void SfxTemplateCatalog_Impl::ClearFamilyList()
{
    aFamList.Clear();
    aFamIds.Remove( 0, aFamIds.Count() );
}

void SfxTemplateCatalog_Impl::PrepareDeleteAction()
{
    aDelBtn.Disable();
    aCancelBtn.SetText( String( SfxResId( STR_CLOSE ) ) );
}


void SfxCommonTemplateDialog_Impl::SetFamily( USHORT nId )
{
    if ( nId != nActFamily )
    {
        if ( nActFamily != 0xFFFF )
            CheckItem( nActFamily, FALSE );
        nActFamily = nId;
        if ( nId != 0xFFFF )
            bUpdateFamily = TRUE;
    }
}

void SfxCommonTemplateDialog_Impl::UpdateFamily_Impl()
{
    bUpdateFamily = FALSE;

    SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
    SfxViewFrame *pViewFrame = pDispat->GetFrame();
    SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): 0;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            StartListening(*pOldStyleSheetPool);
    }

    bWaterDisabled = FALSE;
    bCanNew = TRUE;
    bTreeDrag = TRUE;
    bUpdateByExampleDisabled = FALSE;

    if ( pStyleSheetPool )
    {
        if(!pTreeBox)
            UpdateStyles_Impl(UPDATE_FAMILY | UPDATE_FAMILY_LIST);
        else
        {
            UpdateStyles_Impl(UPDATE_FAMILY);
            FillTreeBox();
        }
    }

    InvalidateBindings();

    if ( IsCheckedItem( SID_STYLE_WATERCAN ) &&
         // only if that area is allowed
         0 != pFamilyState[ nActFamily - 1 ] )
        Execute_Impl( SID_STYLE_APPLY, GetSelectedEntry(),
                      String(), (USHORT)GetFamilyItem_Impl()->GetFamily() );
}
void SfxCommonTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    //does nothing
}

void SfxTemplateDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxViewFrame *pFrame = GetBindings().GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Size aSize = pEditWin->GetSizePixel();
        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        Size aWinSize = GetSizePixel();
        aPoint.X() += aSize.Width() - aWinSize.Width() - 20;
        aPoint.Y() += aSize.Height() / 2 - aWinSize.Height() / 2;
        SetFloatingPos( aPoint );
    }

    SfxDockingWindow::StateChanged( nStateChange );
}

DropToolBox_Impl::DropToolBox_Impl(Window* pParent, SfxTemplateDialog_Impl* pTemplateDialog) :
    ToolBox(pParent),
    DropTargetHelper(this),
    rParent(*pTemplateDialog)
{
}

DropToolBox_Impl::~DropToolBox_Impl()
{
}

sal_Int8    DropToolBox_Impl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nReturn = DND_ACTION_NONE;
    USHORT nItemId = GetItemId( rEvt.maPosPixel );
    if(USHRT_MAX != nItemId && !IsItemChecked( nItemId ))
    {
        SetCurItemId(nItemId);
        GetSelectHdl().Call(this);
    }
    // special case: page styles are allowed to create new styles by example
    // but not allowed to be created by drag and drop
    if ( nItemId != SfxCommonTemplateDialog_Impl::SfxFamilyIdToNId( SFX_STYLE_FAMILY_PAGE )&&
        IsDropFormatSupported( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) &&
        !rParent.bNewByExampleDisabled )
    {
        nReturn = DND_ACTION_COPY;
    }
    return nReturn;
}

sal_Int8    DropToolBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
     return rParent.aFmtLb.ExecuteDrop(rEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
