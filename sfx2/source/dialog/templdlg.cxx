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

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <vcl/menu.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/intlwrapper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>

#include "sfx2/sfxhelp.hxx"
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
#include "sfx2/tplpitem.hxx"
#include "sfx2/sfxresid.hxx"

#include "templdlg.hrc"
#include <sfx2/sfx.hrc>
#include "dialog.hrc"
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/module.hxx>
#include "sfx2/imgmgr.hxx"
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

static sal_uInt16 nLastItemId = USHRT_MAX;

// filter box has maximum 14 entries visible
#define MAX_FILTER_ENTRIES          14

//=========================================================================

TYPEINIT0(SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateDialog_Impl,SfxCommonTemplateDialog_Impl);

SFX_IMPL_DOCKINGWINDOW_WITHID(SfxTemplateDialogWrapper, SID_STYLE_DESIGNER)

//-------------------------------------------------------------------------

class SfxCommonTemplateDialog_Impl::DeletionWatcher : private boost::noncopyable
{
    typedef void (DeletionWatcher::* bool_type)();

public:
    explicit DeletionWatcher(SfxCommonTemplateDialog_Impl& rDialog)
        : m_pDialog(&rDialog)
    {
        m_pDialog->impl_setDeletionWatcher(this);
    }

    ~DeletionWatcher()
    {
        if (m_pDialog)
            m_pDialog->impl_setDeletionWatcher(0);
    }

    // Signal that the dialog was deleted
    void signal()
    {
        m_pDialog = 0;
    }

    // Return true if the dialog was deleted
    operator bool_type() const
    {
        return m_pDialog ? 0 : &DeletionWatcher::signal;
    }

private:
    SfxCommonTemplateDialog_Impl* m_pDialog;
};

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

    pImpl( new SfxTemplateDialog_Impl( pBind, this ) )

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

void DropListBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();

    sal_Bool bHitEmptySpace = ( NULL == GetEntry( rMEvt.GetPosPixel(), sal_True ) );
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


IMPL_LINK_NOARG(DropListBox_Impl, OnAsyncExecuteDrop)
{
    pDialog->ActionSelect( SID_STYLE_NEW_BY_EXAMPLE );
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

PopupMenu* SfxActionListBox::CreateContextMenu( void )
{

    if( !( GetSelectionCount() > 0 ) )
    {
        pDialog->EnableEdit( sal_False );
        pDialog->EnableDel( sal_False );
    }
    return pDialog->CreateContextMenu();
}

//-------------------------------------------------------------------------

SfxTemplateDialogWrapper::SfxTemplateDialogWrapper(Window *pParentWnd,
        sal_uInt16 nId,  SfxBindings *p, SfxChildWinInfo *pInfo) :
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



//===== SfxTemplatePanelControl ===============================================

SfxTemplatePanelControl::SfxTemplatePanelControl (
    SfxBindings* pBindings,
    Window* pParentWindow)
    : DockingWindow(pParentWindow, SfxResId(DLG_STYLE_DESIGNER) ),
      pImpl(new SfxTemplateDialog_Impl(pBindings, this)),
      mpBindings(pBindings)
{
    OSL_ASSERT(mpBindings!=NULL);

    pImpl->updateNonFamilyImages();

    SetStyle(GetStyle() & ~WB_DOCKABLE);
}




SfxTemplatePanelControl::~SfxTemplatePanelControl (void)
{
    delete pImpl;
}




ISfxTemplateCommon* SfxTemplatePanelControl::GetISfxTemplateCommon()
{
    return pImpl->GetISfxTemplateCommon();
}




void SfxTemplatePanelControl::SetParagraphFamily()
{
    // first select the paragraph family
    pImpl->FamilySelect( SFX_STYLE_FAMILY_PARA );
    // then select the automatic filter
    pImpl->SetAutomaticFilter();
}




void SfxTemplatePanelControl::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( ( DATACHANGED_SETTINGS == _rDCEvt.GetType() ) &&
         ( 0 != ( SETTINGS_STYLE & _rDCEvt.GetFlags() ) ) )
    {
        pImpl->updateFamilyImages();
        pImpl->updateNonFamilyImages();
    }

    DockingWindow::DataChanged( _rDCEvt );
}




void SfxTemplatePanelControl::Update()
{
    pImpl->Update();
}




void SfxTemplatePanelControl::Resize()
{
    if(pImpl)
        pImpl->Resize();
    DockingWindow::Resize();
}


void SfxTemplatePanelControl::FreeResource (void)
{
    DockingWindow::FreeResource();
}


SfxChildAlignment SfxTemplatePanelControl::CheckAlignment(SfxChildAlignment eActAlign,SfxChildAlignment eAlign)
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


void SfxTemplatePanelControl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxViewFrame *pFrame = mpBindings->GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Size aSize = pEditWin->GetSizePixel();
        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        Size aWinSize = GetSizePixel();
        aPoint.X() += aSize.Width() - aWinSize.Width() - 20;
        aPoint.Y() += aSize.Height() / 2 - aWinSize.Height() / 2;
        //      SetFloatingPos( aPoint );
    }

    DockingWindow::StateChanged( nStateChange );
}


//=========================================================================
typedef std::vector<OUString> ExpandedEntries_t;

/*  [Description]

    TreeListBox class for displaying the hierarchical view of the templates
*/

class StyleTreeListBox_Impl : public DropListBox_Impl
{
private:
    SvTreeListEntry*                    pCurEntry;
    Link                            aDoubleClickLink;
    Link                            aDropLink;
    String                          aParent;
    String                          aStyle;

protected:
    virtual void    Command( const CommandEvent& rMEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual sal_Bool    DoubleClickHdl();
    virtual long    ExpandingHdl();
    virtual void    ExpandedHdl();
    virtual sal_Bool    NotifyMoving(SvTreeListEntry*  pTarget,
                                     SvTreeListEntry*  pEntry,
                                     SvTreeListEntry*& rpNewParent,
                                     sal_uIntPtr&        rNewChildPos);
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle = 0);

    void            SetDoubleClickHdl(const Link &rLink) { aDoubleClickLink = rLink; }
    void            SetDropHdl(const Link &rLink) { aDropLink = rLink; }
    using SvTreeListBox::GetParent;
    const String&   GetParent() const { return aParent; }
    const String&   GetStyle() const { return aStyle; }
    void            MakeExpanded_Impl(ExpandedEntries_t& rEntries) const;

    virtual PopupMenu* CreateContextMenu( void );
};

//-------------------------------------------------------------------------


void StyleTreeListBox_Impl::MakeExpanded_Impl(ExpandedEntries_t& rEntries) const
{
    SvTreeListEntry *pEntry;
    for(pEntry=(SvTreeListEntry*)FirstVisible();pEntry;pEntry=(SvTreeListEntry*)NextVisible(pEntry))
    {
        if(IsExpanded(pEntry))
        {
            rEntries.push_back(GetEntryText(pEntry));
        }
    }
}

PopupMenu* StyleTreeListBox_Impl::CreateContextMenu()
{
    return pDialog->CreateContextMenu();
}

sal_Bool StyleTreeListBox_Impl::DoubleClickHdl()

/*  [Description]

    DoubleClick-Handler; calls the link.
    SV virtual method.
*/
{
    aDoubleClickLink.Call(this);
    return sal_False;
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

sal_Bool StyleTreeListBox_Impl::NotifyMoving(SvTreeListEntry*  pTarget,
                                         SvTreeListEntry*  pEntry,
                                         SvTreeListEntry*& rpNewParent,
                                         sal_uIntPtr& lPos)
/*  [Description]

    NotifyMoving Handler; This leads via a link on the event to the dialog.
    SV virtual method.
*/
{
    if(!pTarget || !pEntry)
        return sal_False;
    aParent = GetEntryText(pTarget);
    aStyle  = GetEntryText(pEntry);
    const sal_Bool bRet = (sal_Bool)aDropLink.Call(this);
    rpNewParent = pTarget;
    lPos=0;
    IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();
    for(SvTreeListEntry *pTmpEntry=FirstChild(pTarget);
        pTmpEntry && COMPARE_LESS==pCollator->compareString(
            GetEntryText(pTmpEntry),GetEntryText(pEntry));
        pTmpEntry=NextSibling(pTmpEntry),lPos++) ;

    return bRet? (sal_Bool)2: sal_False;
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
    return sal_True;
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
    SvTreeListEntry *pEntry = GetHdlEntry();
    if(!IsExpanded(pEntry) && pCurEntry != GetCurEntry())
        SelectAll( sal_False );
    pCurEntry = 0;
}

//-------------------------------------------------------------------------

StyleTreeListBox_Impl::StyleTreeListBox_Impl(
    SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle) :
    DropListBox_Impl(pParent->GetWindow(), nWinStyle, pParent),
    pCurEntry(0)

/*  [Description]

    Constructor StyleTreeListBox_Impl
*/
{
    EnableContextMenuHandling();
}

//-------------------------------------------------------------------------

class StyleTreeArr_Impl;

/*  [Description]

    Internal structure for the establishment of the hierarchical view
*/

struct StyleTree_Impl
{
    String aName;
    String aParent;
    StyleTreeArr_Impl *pChildren;
    sal_Bool bIsExpanded;
    sal_Bool HasParent() const { return aParent.Len() != 0; }

    StyleTree_Impl(const String &rName, const String &rParent):
        aName(rName), aParent(rParent), pChildren(0), bIsExpanded(0) {}
    ~StyleTree_Impl();
    void Put(StyleTree_Impl* pIns, sal_uIntPtr lPos=ULONG_MAX);
    sal_uIntPtr Count();
};

class StyleTreeArr_Impl : public std::vector<StyleTree_Impl*>
{
public:
    ~StyleTreeArr_Impl()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }

};

sal_uIntPtr StyleTree_Impl::Count()
{
    return pChildren ? pChildren->size() : 0L;
}

//-------------------------------------------------------------------------

StyleTree_Impl::~StyleTree_Impl()
{
    delete pChildren;
}

//-------------------------------------------------------------------------

void StyleTree_Impl::Put(StyleTree_Impl* pIns, sal_uIntPtr lPos)
{
    if ( !pChildren )
        pChildren = new StyleTreeArr_Impl;

    if ( ULONG_MAX == lPos )
        pChildren->push_back( pIns );
    else
        pChildren->insert( pChildren->begin() + (sal_uInt16)lPos, pIns );
}

//-------------------------------------------------------------------------

StyleTreeArr_Impl &MakeTree_Impl(StyleTreeArr_Impl &rArr)
{
    const sal_uInt16 nCount = rArr.size();

    comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLanguageTag().getLocale());

    // Arrange all under their Parents
    sal_uInt16 i;
    for(i = 0; i < nCount; ++i)
    {
        StyleTree_Impl* pEntry = rArr[i];
        if(pEntry->HasParent())
        {
            for(sal_uInt16 j = 0; j < nCount; ++j)
            {
                StyleTree_Impl* pCmp = rArr[j];
                if(pCmp->aName == pEntry->aParent)
                {
                    // Paste initial filter
                    sal_uInt16 nPos;
                    for( nPos = 0 ; nPos < pCmp->Count() &&
                             aSorter.compare((*pCmp->pChildren)[nPos]->aName, pEntry->aName) < 0 ; nPos++)
                    {};
                    pCmp->Put(pEntry,nPos);
                    break;
                }
            }
        }
    }

    for(i = 0; i < rArr.size(); )
    {
        if(rArr[i]->HasParent())
            rArr.erase(rArr.begin() + i);
        else
            ++i;
    }
    return rArr;
}

//-------------------------------------------------------------------------


inline sal_Bool IsExpanded_Impl( const ExpandedEntries_t& rEntries,
                             const OUString &rStr)
{
    for (size_t n = 0; n < rEntries.size(); ++n)
    {
        if (rEntries[n] == rStr)
            return sal_True;
    }
    return sal_False;
}



SvTreeListEntry* FillBox_Impl(SvTreeListBox *pBox,
                                 StyleTree_Impl* pEntry,
                                 const ExpandedEntries_t& rEntries,
                                 SvTreeListEntry* pParent = 0)
{
    SvTreeListEntry* pNewEntry = pBox->InsertEntry(pEntry->aName, pParent);
    const sal_uInt16 nCount = pEntry->pChildren ? pEntry->pChildren->size() : 0;
    for(sal_uInt16 i = 0; i < nCount; ++i)
        FillBox_Impl(pBox, (*pEntry->pChildren)[i], rEntries, pNewEntry);
    return pNewEntry;
}

//-------------------------------------------------------------------------
// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, Window* pW, bool ) :

    aISfxTemplateCommon     ( this ),
    pBindings               ( pB ),
    pWindow                 ( pW ),
    pModule                 ( NULL ),
    pTimer                  ( NULL ),
    m_pStyleFamiliesId      ( NULL ),
    pStyleSheetPool         ( NULL ),
    pTreeBox                ( NULL ),
    pCurObjShell            ( NULL ),
    xModuleManager          ( frame::ModuleManager::create(::comphelper::getProcessComponentContext()) ),
    m_pDeletionWatcher      ( NULL ),

    aFmtLb                  ( this, WB_BORDER | WB_TABSTOP | WB_SORT | WB_QUICK_SEARCH ),
    aFilterLb               ( pW, WB_BORDER | WB_DROPDOWN | WB_TABSTOP ),

    nActFamily              ( 0xffff ),
    nActFilter              ( 0 ),
    nAppFilter              ( 0 ),

    bDontUpdate             ( sal_False ),
    bIsWater                ( sal_False ),
    bEnabled                ( sal_True ),
    bUpdate                 ( sal_False ),
    bUpdateFamily           ( sal_False ),
    bCanEdit                ( sal_False ),
    bCanDel                 ( sal_False ),
    bCanNew                 ( sal_True ),
    bCanHide                ( sal_True ),
    bCanShow                ( sal_False ),
    bWaterDisabled          ( sal_False ),
    bNewByExampleDisabled   ( sal_False ),
    bUpdateByExampleDisabled( sal_False ),
    bTreeDrag               ( sal_True ),
    bHierarchical           ( sal_False ),
    m_bWantHierarchical     ( sal_False ),
    bBindingUpdate          ( sal_True )
{
    aFmtLb.SetAccessibleName(SfxResId(STR_STYLE_ELEMTLIST).toString());
    aFmtLb.SetHelpId( HID_TEMPLATE_FMT );
    aFilterLb.SetHelpId( HID_TEMPLATE_FILTER );
    aFmtLb.SetStyle( aFmtLb.GetStyle() | WB_SORT | WB_HIDESELECTION );
    Font aFont = aFmtLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFmtLb.SetFont( aFont );
}

//-------------------------------------------------------------------------

sal_uInt16 SfxCommonTemplateDialog_Impl::StyleNrToInfoOffset(sal_uInt16 nId)
{
    const SfxStyleFamilyItem *pItem = pStyleFamilies->at( nId );
    return SfxFamilyIdToNId(pItem->GetFamily())-1;
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableEdit(sal_Bool bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem( SID_STYLE_UPDATE_BY_EXAMPLE, bEnable);
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ReadResource()
{
    // Read global user resource
    for(sal_uInt16 i = 0; i < MAX_FAMILIES; ++i)
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

    nActFilter = pCurObjShell ? static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pCurObjShell ) ) : SFXSTYLEBIT_ALL;
    if ( pCurObjShell && SFXSTYLEBIT_ALL == nActFilter )
        nActFilter = pCurObjShell->GetAutoStyleFilterIndex();

    // Paste in the toolbox
    // reverse order, since always inserted at the head
    size_t nCount = pStyleFamilies->size();

    pBindings->ENTERREGISTRATIONS();

    size_t i;
    for(i = 0; i < nCount; ++i)
    {
        sal_uInt16 nSlot = 0;
        switch( (sal_uInt16)pStyleFamilies->at( i )->GetFamily() )
        {
            case SFX_STYLE_FAMILY_CHAR: nSlot = SID_STYLE_FAMILY1; break;
            case SFX_STYLE_FAMILY_PARA: nSlot = SID_STYLE_FAMILY2; break;
            case SFX_STYLE_FAMILY_FRAME:nSlot = SID_STYLE_FAMILY3; break;
            case SFX_STYLE_FAMILY_PAGE: nSlot = SID_STYLE_FAMILY4; break;
            case SFX_STYLE_FAMILY_PSEUDO: nSlot = SID_STYLE_FAMILY5; break;
            default: OSL_FAIL("unknown StyleFamily"); break;
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
        sal_uInt16 nId = SfxFamilyIdToNId( pItem->GetFamily() );
        InsertFamilyItem( nId, pItem );
    }

    LoadedFamilies();

    sal_uInt16 nStart = SID_STYLE_FAMILY1;
    sal_uInt16 nEnd = SID_STYLE_FAMILY4;

    for ( i = nStart; i <= nEnd; i++ )
        pBindings->Update(i);

    pModule = pCurObjShell ? pCurObjShell->GetModule() : NULL;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ClearResource()
{
    ClearFamilyList();
    impl_clear();
}

void SfxCommonTemplateDialog_Impl::impl_clear()
{
    DELETEX(pStyleFamilies);
    sal_uInt16 i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    pCurObjShell = NULL;

    DELETEZ( m_pStyleFamiliesId );
}

void SfxCommonTemplateDialog_Impl::impl_setDeletionWatcher(DeletionWatcher* pNewWatcher)
{
    m_pDeletionWatcher = pNewWatcher;
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
    aFmtLb.SetSelectionMode(MULTIPLE_SELECTION);


    aFilterLb.Show();
    if (!bHierarchical)
        aFmtLb.Show();
}

//-------------------------------------------------------------------------

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    String aEmpty;
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
    GetWindow()->Hide();
    impl_clear();
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = NULL;
    delete pTreeBox;
    delete pTimer;
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
}

//-------------------------------------------------------------------------

sal_uInt16 SfxCommonTemplateDialog_Impl::SfxFamilyIdToNId( SfxStyleFamily nFamily )
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
    sal_uInt16 nCount = aFilterLb.GetEntryCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        sal_uIntPtr nFlags = (sal_uIntPtr)aFilterLb.GetEntryData(i);
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
        sal_uInt16 nId = SfxFamilyIdToNId(pItem->GetFamily());
        if(nId == nActFamily)
            return pItem;
    }
    return 0;
}

SfxStyleSheetBase *SfxCommonTemplateDialog_Impl::GetSelectedStyle() const
{
    if (!IsInitialized() || !pStyleSheetPool || !HasSelectedStyle())
        return NULL;
    const String aTemplName( GetSelectedEntry() );
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    return pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );
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
    {
        bool bReadWrite = !(pStyle->GetMask() & SFXSTYLEBIT_READONLY);
        EnableEdit( bReadWrite );
        EnableHide( bReadWrite && !pStyle->IsHidden( ) && !pStyle->IsUsed( ) );
        EnableShow( bReadWrite && pStyle->IsHidden( ) );
    }
    else
    {
        EnableEdit( sal_False );
        EnableHide( sal_False );
        EnableShow( sal_False );
    }

    if ( pTreeBox )
    {
        if ( rStr.Len() )
        {
            SvTreeListEntry* pEntry = pTreeBox->First();
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
            pTreeBox->SelectAll( sal_False );
    }
    else
    {
        sal_Bool bSelect = ( rStr.Len() > 0 );
        if ( bSelect )
        {
            SvTreeListEntry* pEntry = (SvTreeListEntry*)aFmtLb.FirstVisible();
            while ( pEntry && aFmtLb.GetEntryText( pEntry ) != rStr )
                pEntry = (SvTreeListEntry*)aFmtLb.NextVisible( pEntry );
            if ( !pEntry )
                bSelect = sal_False;
            else
            {
                if (!aFmtLb.IsSelected(pEntry))
                {
                    aFmtLb.MakeVisible( pEntry );
                    aFmtLb.SelectAll(false);
                    aFmtLb.Select( pEntry );
                    bWaterDisabled = (aFmtLb.GetSelectionCount() <=1 ? sal_False : sal_True);
                    FmtSelectHdl( NULL );
                }
            }
        }

        if ( !bSelect )
        {
            aFmtLb.SelectAll( sal_False );
            EnableEdit(sal_False);
            EnableHide( sal_False );
            EnableShow( sal_False );
        }
    }
}

//-------------------------------------------------------------------------

String SfxCommonTemplateDialog_Impl::GetSelectedEntry() const
{
    String aRet;
    if ( pTreeBox )
    {
        SvTreeListEntry* pEntry = pTreeBox->FirstSelected();
        if ( pEntry )
            aRet = pTreeBox->GetEntryText( pEntry );
    }
    else
    {
        SvTreeListEntry* pEntry = aFmtLb.FirstSelected();
        if ( pEntry )
            aRet = aFmtLb.GetEntryText( pEntry );
    }
    return aRet;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::EnableTreeDrag( sal_Bool bEnable )
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
    OSL_ENSURE( pTreeBox, "FillTreeBox() without treebox");
    if(pStyleSheetPool && nActFamily != 0xffff)
    {
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        pStyleSheetPool->SetSearchMask(pItem->GetFamily(), SFXSTYLEBIT_ALL_VISIBLE);
        StyleTreeArr_Impl aArr;
        SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
        if(pStyle && pStyle->HasParentSupport() && bTreeDrag )
            pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
        else
            pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        while(pStyle)
        {
            StyleTree_Impl* pNew =
                new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
            aArr.push_back(pNew);
            pStyle = pStyleSheetPool->Next();
        }
        MakeTree_Impl(aArr);
        ExpandedEntries_t aEntries;
        if(pTreeBox)
            ((const StyleTreeListBox_Impl *)pTreeBox)->
                MakeExpanded_Impl( aEntries);
        pTreeBox->SetUpdateMode( sal_False );
        pTreeBox->Clear();
        const sal_uInt16 nCount = aArr.size();
        for(sal_uInt16 i = 0; i < nCount; ++i)
            FillBox_Impl(pTreeBox, aArr[i], aEntries);

        EnableItem(SID_STYLE_WATERCAN,sal_False);

        SfxTemplateItem* pState = pFamilyState[nActFamily-1];

        if ( nCount )
            pTreeBox->Expand( pTreeBox->First() );

        for ( SvTreeListEntry* pEntry = pTreeBox->First(); pEntry; pEntry = pTreeBox->Next( pEntry ) )
        {
            if ( IsExpanded_Impl( aEntries, pTreeBox->GetEntryText( pEntry ) ) )
                pTreeBox->Expand( pEntry );
        }

        pTreeBox->SetUpdateMode( sal_True );

        String aStyle;
        if(pState)  // Select current entry
            aStyle = pState->GetStyleName();
        SelectStyle(aStyle);
        EnableDelete();
    }
}

//-------------------------------------------------------------------------
sal_Bool SfxCommonTemplateDialog_Impl::HasSelectedStyle() const
{
    return pTreeBox? pTreeBox->FirstSelected() != 0:
            aFmtLb.GetSelectionCount() != 0;
}


//-------------------------------------------------------------------------

// internal: Refresh the display
// nFlags: what we should update.
void SfxCommonTemplateDialog_Impl::UpdateStyles_Impl(sal_uInt16 nFlags)
{
    OSL_ENSURE(nFlags, "nothing to do");
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
    sal_uInt16 nFilter     = pT ? pT->nFlags : 0;
    if(!nFilter)   // automatic
        nFilter = nAppFilter;

    OSL_ENSURE(pStyleSheetPool, "no StyleSheetPool");
    if(pStyleSheetPool)
    {
        pStyleSheetPool->SetSearchMask(eFam, nFilter);
        pItem = GetFamilyItem_Impl();
        if((nFlags & UPDATE_FAMILY) == UPDATE_FAMILY)   // Update view type list (Hierarchical, All, etc.
        {
            CheckItem(nActFamily, sal_True);    // check Button in Toolbox
            aFilterLb.SetUpdateMode(sal_False);
            aFilterLb.Clear();
            //insert hierarchical at the beginning
            sal_uInt16 nPos = aFilterLb.InsertEntry(SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString(), 0);
            aFilterLb.SetEntryData( nPos, (void*)(sal_uIntPtr)SFXSTYLEBIT_ALL );
            const SfxStyleFilter& rFilter = pItem->GetFilterList();
            for( size_t i = 0; i < rFilter.size(); ++i)
            {
                sal_uIntPtr nFilterFlags = rFilter[ i ]->nFlags;
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
                sal_uInt16 nFilterFlags = pActT ? pActT->nFlags : 0;
                pStyleSheetPool->SetSearchMask(eFam, nFilterFlags);
            }

            // if the tree view again, select family hierarchy
            if (pTreeBox || m_bWantHierarchical)
            {
                aFilterLb.SelectEntry(SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString());
                EnableHierarchical(true);
            }

            // show maximum 14 entries
            aFilterLb.SetDropDownLineCount( MAX_FILTER_ENTRIES );
            aFilterLb.SetUpdateMode(sal_True);
        }
        else
        {
            if (nActFilter < aFilterLb.GetEntryCount() - 1)
                aFilterLb.SelectEntryPos(nActFilter + 1);
            else
            {
                nActFilter = 0;
                aFilterLb.SelectEntryPos(1);
            }
        }

        if(nFlags & UPDATE_FAMILY_LIST)
        {
            EnableItem(SID_STYLE_WATERCAN,sal_False);

            SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
            SvTreeListEntry* pEntry = aFmtLb.First();
            std::vector<OUString> aStrings;

            comphelper::string::NaturalStringSorter aSorter(
                ::comphelper::getProcessComponentContext(),
                Application::GetSettings().GetLanguageTag().getLocale());

            while( pStyle )
            {
                //Bubblesort
                size_t nPos;
                for(nPos = aStrings.size(); nPos && aSorter.compare(aStrings[nPos-1], pStyle->GetName()) > 0; --nPos)
                {};
                aStrings.insert(aStrings.begin() + nPos, pStyle->GetName());
                pStyle = pStyleSheetPool->Next();
            }

            size_t nCount = aStrings.size();
            size_t nPos = 0;
            while(nPos < nCount && pEntry &&
                  aStrings[nPos] == OUString(aFmtLb.GetEntryText(pEntry)))
            {
                ++nPos;
                pEntry = aFmtLb.Next( pEntry );
            }

            if( nPos < nCount || pEntry )
            {
                // Fills the display box
                aFmtLb.SetUpdateMode(sal_False);
                aFmtLb.Clear();

                for(nPos = 0; nPos < nCount; ++nPos)
                    aFmtLb.InsertEntry(aStrings[nPos], 0, sal_False, nPos);

                aFmtLb.SetUpdateMode(true);
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
        //make sure the watercan is only activated when there is (only) one selection
        bWaterDisabled = (aFmtLb.GetSelectionCount() <=1 ? sal_False : sal_True);

    if(pItem && !bWaterDisabled)
    {
        CheckItem(SID_STYLE_WATERCAN, pItem->GetValue());
        EnableItem( SID_STYLE_WATERCAN, sal_True );
    }
    else
        if(!bWaterDisabled)
            EnableItem(SID_STYLE_WATERCAN, sal_True);
        else
            EnableItem(SID_STYLE_WATERCAN, sal_False);

// Ignore while in watercan mode statusupdates

    size_t nCount = pStyleFamilies->size();
    pBindings->EnterRegistrations();
    for(size_t n = 0; n < nCount; n++)
    {
        SfxControllerItem *pCItem=pBoundItems[n];
        sal_Bool bChecked = pItem && pItem->GetValue();
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

void SfxCommonTemplateDialog_Impl::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    DELETEZ(pFamilyState[nIdx]);
    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem(*pItem);
    bUpdate = sal_True;

    // If used templates (how the hell you find this out??)
    bUpdateFamily = sal_True;
}

//-------------------------------------------------------------------------
// Notice from SfxBindings that the update is completed. Pushes out the update
// of the display.

void SfxCommonTemplateDialog_Impl::Update_Impl()
{
    sal_Bool bDocChanged=sal_False;
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
            bDocChanged=sal_True;
        }
    }

    if (bUpdateFamily)
        UpdateFamily_Impl();

    sal_uInt16 i;
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
         CheckItem(nActFamily, sal_False);
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
         CheckItem( nActFamily, sal_True );
         nActFilter = static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pDocShell ) );
         if ( SFXSTYLEBIT_ALL == nActFilter )
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
         CheckItem( nActFamily, sal_True );
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
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
        bDontUpdate=sal_True;
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
        bDontUpdate=sal_False;
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
                    bUpdate = sal_False;
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
                        {
                            bool bReadWrite = !(pStyle->GetMask() & SFXSTYLEBIT_READONLY);
                            EnableEdit( bReadWrite );
                            EnableHide( bReadWrite && !pStyle->IsUsed( ) && !pStyle->IsHidden( ) );
                            EnableShow( bReadWrite && pStyle->IsHidden( ) );
                        }
                        else
                        {
                            EnableEdit(sal_False);
                            EnableHide(sal_False);
                            EnableShow(sal_False);
                        }
                    }
                }
                break;
            }

        // Necessary if switching between documents and in both documents
        // the same template is used. Do not immediately call Update_Impl,
        // for the case that one of the documents is an internal InPlaceObjekt!
          case SFX_HINT_DOCCHANGED:
            bUpdate = sal_True;
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

    sal_uIntPtr nId = rHint.ISA(SfxSimpleHint) ? ( (SfxSimpleHint&)rHint ).GetId() : 0;

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
                sal_uInt16 nEntry,  // Idx of the new Filters
                sal_Bool bForce )   // Force update, even if the new filter is
                                // equal to the current
{
    if( nEntry != nActFilter || bForce )
    {
        nActFilter = nEntry;
        SfxObjectShell *const pDocShell = SaveSelection();
        SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
        pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): 0;
        if ( pOldStyleSheetPool != pStyleSheetPool )
        {
            if ( pOldStyleSheetPool )
                EndListening(*pOldStyleSheetPool);
            if ( pStyleSheetPool )
                StartListening(*pStyleSheetPool);
        }

        UpdateStyles_Impl(UPDATE_FAMILY_LIST);
    }
}

//-------------------------------------------------------------------------

// Internal: Perform functions through the Dispatcher
sal_Bool SfxCommonTemplateDialog_Impl::Execute_Impl(
    sal_uInt16 nId, const String &rStr, const String& rRefStr, sal_uInt16 nFamily,
    sal_uInt16 nMask, sal_uInt16 *pIdx, const sal_uInt16* pModifier)
{
    SfxDispatcher &rDispatcher = *SFX_APP()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
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

    DeletionWatcher aDeleted(*this);
    sal_uInt16 nModi = pModifier ? *pModifier : 0;
    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
        pItems, nModi );

    // Dialog can be destroyed while in Execute() because started
    // subdialogs are not modal to it (#i97888#).
    if ( !pItem || aDeleted )
        return sal_False;

    if ( (nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId) && (aFmtLb.GetSelectionCount() <= 1) )
    {
        SfxUInt16Item *pFilterItem = PTR_CAST(SfxUInt16Item, pItem);
        OSL_ENSURE(pFilterItem, "SfxUINT16Item expected");
        sal_uInt16 nFilterFlags = pFilterItem->GetValue() & ~SFXSTYLEBIT_USERDEF;
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

    return sal_True;
}

//-------------------------------------------------------------------------

// Handler der Listbox der Filter
void SfxCommonTemplateDialog_Impl::EnableHierarchical(bool const bEnable)
{
    if (bEnable)
    {
        if (!bHierarchical)
        {
            // Turn on treeView
            bHierarchical=sal_True;
            m_bWantHierarchical = sal_True;
            SaveSelection(); // fdo#61429 store "hierarchical"
            const String aSelectEntry( GetSelectedEntry());
            aFmtLb.Hide();

            pTreeBox = new StyleTreeListBox_Impl(
                    this, WB_HASBUTTONS | WB_HASLINES |
                    WB_BORDER | WB_TABSTOP | WB_HASLINESATROOT |
                    WB_HASBUTTONSATROOT | WB_HIDESELECTION | WB_QUICK_SEARCH );
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
            FillTreeBox();
            SelectStyle(aSelectEntry);
            pTreeBox->SetAccessibleName(SfxResId(STR_STYLE_ELEMTLIST).toString());
            pTreeBox->Show();
        }
    }
    else
    {
        DELETEZ(pTreeBox);
        aFmtLb.Show();
        // If bHierarchical, then the family can have changed
        // minus one since hierarchical is inserted at the start
        m_bWantHierarchical = sal_False; // before FilterSelect
        FilterSelect(aFilterLb.GetSelectEntryPos() - 1, bHierarchical );
        bHierarchical=sal_False;
    }
}

IMPL_LINK( SfxCommonTemplateDialog_Impl, FilterSelectHdl, ListBox *, pBox )
{
    if (SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString() == pBox->GetSelectEntry())
    {
        EnableHierarchical(true);
    }
    else
    {
        EnableHierarchical(false);
    }

    return 0;
}

//-------------------------------------------------------------------------

// Select-Handler for the Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(sal_uInt16 nEntry)
{
    if( nEntry != nActFamily )
    {
        CheckItem( nActFamily, sal_False );
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

void SfxCommonTemplateDialog_Impl::ActionSelect(sal_uInt16 nEntry)
{
    String aEmpty;
    switch(nEntry)
    {
        case SID_STYLE_WATERCAN:
        {
            const sal_Bool bState = IsCheckedItem(nEntry);
            sal_Bool bCheck;
            SfxBoolItem aBool;
            // when a template is chosen.
            if(!bState && aFmtLb.GetSelectionCount())
            {
                const String aTemplName(
                    GetSelectedEntry());
                Execute_Impl(
                    SID_STYLE_WATERCAN, aTemplName, aEmpty,
                    (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
                bCheck = sal_True;
            }
            else
            {
                Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
                bCheck = sal_False;
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
                sal_uInt16 nFilter;
                if( pItem && nActFilter != SFXSTYLEBIT_ALL )
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
                                 (sal_uInt16)GetFamilyItem_Impl()->GetFamily(),
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
                    (sal_uInt16)GetFamilyItem_Impl()->GetFamily());
            break;
        }
        case SID_TEMPLATE_LOAD:
            SFX_APP()->GetDispatcher_Impl()->Execute(nEntry);
        break;
        default: OSL_FAIL("not implemented"); break;
    }
}

//-------------------------------------------------------------------------

static OUString getModuleIdentifier( const Reference< XModuleManager2 >& i_xModMgr, SfxObjectShell* i_pObjSh )
{
    OSL_ENSURE( i_xModMgr.is(), "getModuleIdentifier(): no XModuleManager" );
    OSL_ENSURE( i_pObjSh, "getModuleIdentifier(): no ObjectShell" );

    OUString sIdentifier;

    try
    {
        sIdentifier = i_xModMgr->identify( i_pObjSh->GetModel() );
    }
    catch ( ::com::sun::star::frame::UnknownModuleException& )
    {
        OSL_TRACE( "getModuleIdentifier(): unknown module" );
    }
    catch ( Exception& )
    {
        OSL_FAIL( "getModuleIdentifier(): exception of XModuleManager::identify()" );
    }

    return sIdentifier;
}

//-------------------------------------------------------------------------

sal_Int32 SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh )
{
    OSL_ENSURE( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    sal_Int32 nFilter = -1;

    Sequence< PropertyValue > lProps;
    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
    sal_Int32 nDefault = -1;
    nFilter = aFactoryProps.getUnpackedValueOrDefault( "ooSetupFactoryStyleFilter", nDefault );

    m_bWantHierarchical =
        (nFilter & SFXSTYLEBIT_HIERARCHY) ? sal_True : sal_False;
    nFilter &= ~SFXSTYLEBIT_HIERARCHY; // clear it

    return nFilter;
}

//-------------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::SaveFactoryStyleFilter( SfxObjectShell* i_pObjSh, sal_Int32 i_nFilter )
{
    OSL_ENSURE( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    Sequence< PropertyValue > lProps(1);
    lProps[0].Name = "ooSetupFactoryStyleFilter";
    lProps[0].Value = makeAny(
            i_nFilter | (m_bWantHierarchical ? SFXSTYLEBIT_HIERARCHY : 0));
    xModuleManager->replaceByName( getModuleIdentifier( xModuleManager, i_pObjSh ), makeAny( lProps ) );
}

SfxObjectShell* SfxCommonTemplateDialog_Impl::SaveSelection()
{
    SfxViewFrame *const pViewFrame(pBindings->GetDispatcher_Impl()->GetFrame());
    SfxObjectShell *const pDocShell(pViewFrame->GetObjectShell());
    if (pDocShell)
    {
        pDocShell->SetAutoStyleFilterIndex(nActFilter);
        SaveFactoryStyleFilter( pDocShell, nActFilter );
    }
    return pDocShell;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxCommonTemplateDialog_Impl, DropHdl, StyleTreeListBox_Impl *, pBox )
{
    bDontUpdate=sal_True;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    long ret= pStyleSheetPool->SetParent(eFam,pBox->GetStyle(), pBox->GetParent())? 1L: 0L;
    bDontUpdate=sal_False;
    return ret;
}

//-------------------------------------------------------------------------

// Handler for the New-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl(void *)
{
    String aEmpty;
    if ( nActFamily != 0xffff && (aFmtLb.GetSelectionCount() <= 1))
    {
        Window* pTmp;
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );

        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam=pItem->GetFamily();
        sal_uInt16 nMask;
        if( pItem && nActFilter != SFXSTYLEBIT_ALL )
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
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
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
        sal_uInt16 nFilter = nActFilter;
        String aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        Window* pTmp;
        //DefModalDialogParent set for modality of the following dialogs
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );
        if ( Execute_Impl( SID_STYLE_EDIT, aTemplName, String(),
                          (sal_uInt16)GetFamilyItem_Impl()->GetFamily(), 0, &nFilter ) )
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
        bool bUsedStyle = 0;     // one of the selected styles are used in the document?
        String aRet;

        std::vector<SvTreeListEntry*> aList;
        SvTreeListEntry* pEntry = aFmtLb.FirstSelected();
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

        String aMsg = SfxResId(STR_DELETE_STYLE_USED).toString();
        aMsg += SfxResId(STR_DELETE_STYLE).toString();

        while (pEntry)
        {
            aList.push_back( pEntry );
            // check the style is used or not
            if (pTreeBox)
                aRet = pTreeBox->GetEntryText( pEntry );
            else
                aRet = aFmtLb.GetEntryText( pEntry );

            const String aTemplName( aRet );

            SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );

            if ( pStyle->IsUsed() )  // pStyle is in use in the document?
            {
                if (bUsedStyle) // add a separator for the second and later styles
                    aMsg += ", ";
                aMsg += aTemplName;
                bUsedStyle = 1;
            }

            pEntry = aFmtLb.NextSelected( pEntry );
        }

        bool aApproved = 0;

        // we only want to show the dialog once and if we want to delete a style in use (UX-advice)
        if ( bUsedStyle )
        {
            #if defined UNX
                QueryBox aBox( SFX_APP()->GetTopWindow(), WB_YES_NO | WB_DEF_NO, aMsg );
            #else
                QueryBox aBox( GetWindow(), WB_YES_NO | WB_DEF_NO , aMsg );
            #endif
            aApproved = aBox.Execute() == RET_YES;
        }

        // if there are no used styles selected or the user approved the changes
        if ( !bUsedStyle || aApproved )
        {
            std::vector<SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();

            for (; it != itEnd; ++it)
            {
                if (pTreeBox)
                    aRet = pTreeBox->GetEntryText( *it );
                else
                    aRet = aFmtLb.GetEntryText( *it );

                const String aTemplName( aRet );
                PrepareDeleteAction();
                bDontUpdate = sal_True; // To prevent the Treelistbox to shut down while deleting
                Execute_Impl( SID_STYLE_DELETE, aTemplName,
                              String(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

                if ( pTreeBox )
                {
                    pTreeBox->RemoveParentKeepChildren( *it );
                    bDontUpdate = sal_False;
                }
            }
            bDontUpdate = sal_False; //if everything is deleted set bDontUpdate back to false
            UpdateStyles_Impl(UPDATE_FAMILY_LIST); //and force-update the list
        }
    }
}

void SfxCommonTemplateDialog_Impl::HideHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        const String aTemplName( GetSelectedEntry() );
        SfxStyleSheetBase* pStyle = GetSelectedStyle();
        if ( pStyle )
        {
            Execute_Impl( SID_STYLE_HIDE, aTemplName,
                          String(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
        }
    }
}

void SfxCommonTemplateDialog_Impl::ShowHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        const String aTemplName( GetSelectedEntry() );
        SfxStyleSheetBase* pStyle = GetSelectedStyle();
        if ( pStyle )
        {
            Execute_Impl( SID_STYLE_SHOW, aTemplName,
                          String(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
        }
    }
}

//-------------------------------------------------------------------------

void    SfxCommonTemplateDialog_Impl::EnableDelete()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        OSL_ENSURE(pStyleSheetPool, "No StyleSheetPool");
        const String aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        sal_uInt16 nFilter = 0;
        if(pItem->GetFilterList().size() > nActFilter)
            nFilter = pItem->GetFilterList()[ nActFilter ]->nFlags;
        if(!nFilter)    // automatic
            nFilter = nAppFilter;
        const SfxStyleSheetBase *pStyle =
            pStyleSheetPool->Find(aTemplName,eFam, pTreeBox? SFXSTYLEBIT_ALL : nFilter);

        OSL_ENSURE(pStyle, "Style not found");
        if(pStyle && pStyle->IsUserDefined())
        {
            EnableDel(sal_True);
        }
        else
        {
            EnableDel(sal_False);
        }
    }
    else
    {
        EnableDel(sal_False);
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
        sal_uInt16 nModifier = aFmtLb.GetModifier();
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), String(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
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
             0 != pFamilyState[nActFamily-1] && (aFmtLb.GetSelectionCount() <= 1) )
        {
            String aEmpty;
            Execute_Impl(SID_STYLE_WATERCAN,
                         aEmpty, aEmpty, 0);
            Execute_Impl(SID_STYLE_WATERCAN,
                         GetSelectedEntry(), aEmpty,
                         ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily());
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
        return sal_True;
    }

    switch(nLastItemId) {
    case ID_NEW: NewHdl(0); break;
    case ID_EDIT: EditHdl(0); break;
    case ID_DELETE: DeleteHdl(0); break;
    case ID_HIDE: HideHdl(0); break;
    case ID_SHOW: ShowHdl(0); break;
    default: return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SfxCommonTemplateDialog_Impl::ExecuteContextMenu_Impl( const Point& rPos, Window* pWin )
{
    // Bug# 94152: This part should never be called, because before this happens, the TreeListBox should captured this!
    OSL_FAIL( "+SfxCommonTemplateDialog_Impl::ExecuteContextMenu_Impl(): How could this happen? Please infirm developer ASAP!" );

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

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, sal_Bool bEnable)
{
    if( nId == SID_STYLE_NEW_BY_EXAMPLE )
    {
        bNewByExampleDisabled = (aFmtLb.GetSelectionCount() <=1 ? !bEnable : sal_True);
    }
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
        bUpdateByExampleDisabled = (aFmtLb.GetSelectionCount() <=1 ? !bEnable : sal_True);

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
        pBindings->Invalidate( SID_STYLE_NEW, sal_True, sal_False );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = sal_False;
    }
    PopupMenu* pMenu = new PopupMenu( SfxResId( MN_CONTEXT_TEMPLDLG ) );
    pMenu->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ) );
    pMenu->EnableItem( ID_EDIT, bCanEdit );
    pMenu->EnableItem( ID_DELETE, bCanDel );
    pMenu->EnableItem( ID_NEW, bCanNew );
    pMenu->EnableItem( ID_HIDE, bCanHide );
    pMenu->EnableItem( ID_SHOW, bCanShow );

    return pMenu;
}

// ------------------------------------------------------------------------


SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(
    SfxBindings* pB, SfxTemplateDialog* pDlgWindow ) :

    SfxCommonTemplateDialog_Impl( pB, pDlgWindow, true ),

    m_pFloat            ( pDlgWindow ),
    m_bZoomIn           ( sal_False ),
    m_aActionTbL        ( pDlgWindow, this ),
    m_aActionTbR        ( pDlgWindow, SfxResId( TB_ACTION ) )

{
    pDlgWindow->FreeResource();
    Initialize();
}

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(
    SfxBindings* pB, SfxTemplatePanelControl* pDlgWindow )
    : SfxCommonTemplateDialog_Impl( pB, pDlgWindow, true ),
      m_pFloat          ( pDlgWindow ),
      m_bZoomIn         ( sal_False ),
      m_aActionTbL        ( pDlgWindow, this ),
      m_aActionTbR      ( pDlgWindow, SfxResId( TB_ACTION ) )
{
    pDlgWindow->FreeResource();
    Initialize();
}

void SfxTemplateDialog_Impl::Initialize (void)
{
    SfxCommonTemplateDialog_Impl::Initialize();

    m_aActionTbL.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    m_aActionTbR.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_aActionTbR.SetDropdownClickHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRClick));
    m_aActionTbL.Show();
    m_aActionTbR.Show();
    Font aFont=aFilterLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFilterLb.SetFont( aFont );
    m_aActionTbL.SetHelpId( HID_TEMPLDLG_TOOLBOX_LEFT );
}

// ------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableFamilyItem( sal_uInt16 nId, sal_Bool bEnable )
{
    m_aActionTbL.EnableItem( nId, bEnable );
}

//-------------------------------------------------------------------------
// Insert element into dropdown filter "Frame Styles", "List Styles", etc.

void SfxTemplateDialog_Impl::InsertFamilyItem(sal_uInt16 nId,const SfxStyleFamilyItem *pItem)
{
    OString sHelpId;
    switch( (sal_uInt16) pItem->GetFamily() )
    {
        case SFX_STYLE_FAMILY_CHAR:     sHelpId = ".uno:CharStyle"; break;
        case SFX_STYLE_FAMILY_PARA:     sHelpId = ".uno:ParaStyle"; break;
        case SFX_STYLE_FAMILY_FRAME:    sHelpId = ".uno:FrameStyle"; break;
        case SFX_STYLE_FAMILY_PAGE:     sHelpId = ".uno:PageStyle"; break;
        case SFX_STYLE_FAMILY_PSEUDO:   sHelpId = ".uno:ListStyle"; break;
        default: OSL_FAIL("unknown StyleFamily"); break;
    }
    m_aActionTbL.InsertItem( nId, pItem->GetImage(), pItem->GetText(), 0, 0);
    m_aActionTbL.SetHelpId( nId, sHelpId );
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
        sal_uInt16 nId = SfxFamilyIdToNId( pItem->GetFamily() );
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
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, sal_True, sal_False);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, sal_True, sal_False);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, sal_True, sal_False);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, sal_True, sal_False );
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, sal_True, sal_False );
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
    SfxDockingWindow* pDockingWindow = dynamic_cast<SfxDockingWindow*>(m_pFloat);
    FloatingWindow *pF = pDockingWindow!=NULL ? pDockingWindow->GetFloatingWindow() : NULL;
    if ( pF )
    {
        m_bZoomIn = pF->IsRollUp();
        if ( m_bZoomIn )
            return;
    }

    if (m_pFloat == NULL)
        return;
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
    if (m_pFloat != NULL)
    {
        Size aSizeATL=m_pFloat->PixelToLogic(m_aActionTbL.CalcWindowSizePixel());
        Size aSizeATR=m_pFloat->PixelToLogic(m_aActionTbR.CalcWindowSizePixel());
        Size aMinSize=Size(
            aSizeATL.Width()+aSizeATR.Width()+
                2*SFX_TEMPLDLG_HFRAME + SFX_TEMPLDLG_MIDHSPACE,
            4*aSizeATL.Height()+2*SFX_TEMPLDLG_MIDVSPACE);
        return aMinSize;
    }
    else
        return Size(0,0);
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::Command( const CommandEvent& rCEvt )
{
    if (m_pFloat != NULL)
    {
        if(COMMAND_CONTEXTMENU  == rCEvt.GetCommand())
            ExecuteContextMenu_Impl( rCEvt.GetMousePosPixel(), m_pFloat );
        else
            m_pFloat->Command(rCEvt);
    }
}

//-------------------------------------------------------------------------

void SfxTemplateDialog_Impl::EnableItem(sal_uInt16 nMesId, sal_Bool bCheck)
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

void SfxTemplateDialog_Impl::CheckItem(sal_uInt16 nMesId, sal_Bool bCheck)
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

sal_Bool SfxTemplateDialog_Impl::IsCheckedItem(sal_uInt16 nMesId)
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
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )

//-------------------------------------------------------------------------
static OUString lcl_GetLabel(uno::Any& rAny)
{
    OUString sRet;
    uno::Sequence< beans::PropertyValue >aPropSeq;
    if ( rAny >>= aPropSeq )
    {
        for( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
        {
            if ( aPropSeq[i].Name == "Label" )
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
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry != SID_STYLE_NEW_BY_EXAMPLE ||
            TIB_DROPDOWN != (pBox->GetItemBits(nEntry)&TIB_DROPDOWN))
        ActionSelect(nEntry);
    return 0;
}
//-------------------------------------------------------------------------
IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxRClick, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry == SID_STYLE_NEW_BY_EXAMPLE &&
            TIB_DROPDOWN == (pBox->GetItemBits(nEntry)&TIB_DROPDOWN))
    {
        //create a popup menu in Writer
        boost::scoped_ptr<PopupMenu> pMenu(new PopupMenu);
        uno::Reference< container::XNameAccess > xNameAccess(
                frame::UICommandDescription::create(
                    ::comphelper::getProcessComponentContext()) );
        uno::Reference< container::XNameAccess > xUICommands;
        OUString sTextDoc("com.sun.star.text.TextDocument");
        if(xNameAccess->hasByName(sTextDoc))
        {
            uno::Any a = xNameAccess->getByName( sTextDoc );
            a >>= xUICommands;
        }
        if(!xUICommands.is())
            return 0;
        try
        {
            uno::Sequence< beans::PropertyValue > aPropSeq;
            uno::Any aCommand = xUICommands->getByName(".uno:StyleNewByExample");
            OUString sLabel = lcl_GetLabel( aCommand );
            pMenu->InsertItem( SID_STYLE_NEW_BY_EXAMPLE, sLabel );
            pMenu->SetHelpId(SID_STYLE_NEW_BY_EXAMPLE, HID_TEMPLDLG_NEWBYEXAMPLE);

            aCommand = xUICommands->getByName(".uno:StyleUpdateByExample");
            sLabel = lcl_GetLabel( aCommand );

            pMenu->InsertItem( SID_STYLE_UPDATE_BY_EXAMPLE, sLabel );
            pMenu->SetHelpId(SID_STYLE_UPDATE_BY_EXAMPLE, HID_TEMPLDLG_UPDATEBYEXAMPLE);

            aCommand = xUICommands->getByName(".uno:LoadStyles");
            sLabel = lcl_GetLabel( aCommand );
            pMenu->InsertItem( SID_TEMPLATE_LOAD, sLabel );
            pMenu->SetHelpId(SID_TEMPLATE_LOAD, ".uno:LoadStyles");

            pMenu->SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, MenuSelectHdl));
            pMenu->Execute( pBox,
                            pBox->GetItemRect(nEntry),
                            POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
        }
        catch(uno::Exception&)
        {
        }
        pBox->Invalidate();
    }
    return 0;
}
//-------------------------------------------------------------------------
IMPL_LINK( SfxTemplateDialog_Impl, MenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    ActionSelect(nMenuId);
    return 0;
}
//-------------------------------------------------------------------------


void SfxCommonTemplateDialog_Impl::SetFamily( sal_uInt16 nId )
{
    if ( nId != nActFamily )
    {
        if ( nActFamily != 0xFFFF )
            CheckItem( nActFamily, sal_False );
        nActFamily = nId;
        if ( nId != 0xFFFF )
            bUpdateFamily = sal_True;
    }
}

void SfxCommonTemplateDialog_Impl::UpdateFamily_Impl()
{
    bUpdateFamily = sal_False;

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
            StartListening(*pStyleSheetPool);
    }

    bWaterDisabled = sal_False;
    bCanNew = (aFmtLb.GetSelectionCount() <=1 ? sal_True : sal_False);
    bTreeDrag = sal_True;
    bUpdateByExampleDisabled = sal_False;

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
                      String(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
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
    sal_uInt16 nItemId = GetItemId( rEvt.maPosPixel );
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
