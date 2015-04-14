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
#include <vcl/settings.hxx>
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
#include <com/sun/star/frame/theUICommandDescription.hpp>

#include <sfx2/sfxhelp.hxx>
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
#include <sfx2/tplpitem.hxx>
#include <sfx2/sfxresid.hxx>

#include "templdlg.hrc"
#include <sfx2/sfx.hrc>
#include "dialog.hrc"
#include "arrdecl.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/module.hxx>
#include <sfx2/imgmgr.hxx>
#include "helpid.hrc"
#include "appdata.hxx"
#include <sfx2/viewfrm.hxx>

#include <comphelper/string.hxx>

using namespace css;
using namespace css::beans;
using namespace css::frame;
using namespace css::uno;

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


TYPEINIT0(SfxCommonTemplateDialog_Impl);
TYPEINIT1(SfxTemplateDialog_Impl,SfxCommonTemplateDialog_Impl);

class SfxCommonTemplateDialog_Impl::DeletionWatcher : private boost::noncopyable
{
    typedef void (DeletionWatcher::* bool_type)();

public:
    explicit DeletionWatcher(SfxCommonTemplateDialog_Impl& rDialog)
        : m_pDialog(&rDialog)
        , m_pPrevious(m_pDialog->impl_setDeletionWatcher(this))
    {
    }

    ~DeletionWatcher()
    {
        if (m_pDialog)
            m_pDialog->impl_setDeletionWatcher(m_pPrevious);
    }

    // Signal that the dialog was deleted
    void signal()
    {
        m_pDialog = 0;
        if (m_pPrevious)
            m_pPrevious->signal();
    }

    // Return true if the dialog was deleted
    operator bool_type() const
    {
        return m_pDialog ? 0 : &DeletionWatcher::signal;
    }

private:
    SfxCommonTemplateDialog_Impl* m_pDialog;
    DeletionWatcher *const m_pPrevious; /// let's add more epicycles!
};

void DropListBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();

    bool bHitEmptySpace = ( NULL == GetEntry( rMEvt.GetPosPixel(), true ) );
    if( bHitEmptySpace && ( rMEvt.GetClicks() == 2 ) && rMEvt.IsMod1() )
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

/** Drop is enabled as long as it is allowed to create a new style by example, i.e. to
    create a style out of the current selection.
*/
sal_Int8 DropListBox_Impl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if ( IsDropFormatSupported( SotClipboardFormatId::OBJECTDESCRIPTOR ) )
    {
        // special case: page styles are allowed to create new styles by example
        // but not allowed to be created by drag and drop
        if (pDialog->GetActualFamily() == SFX_STYLE_FAMILY_PAGE ||
                pDialog->bNewByExampleDisabled)
            return DND_ACTION_NONE;
        else
            return DND_ACTION_COPY;
    }
    return SvTreeListBox::AcceptDrop( rEvt );
}

sal_Int8 DropListBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    SfxObjectShell* pDocShell = pDialog->GetObjectShell();
    TransferableDataHelper aHelper( rEvt.maDropEvent.Transferable );
    sal_uInt32 nFormatCount = aHelper.GetFormatCount();
    if ( pDocShell )
    {
        bool bFormatFound = false;

        for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
        {
            SotClipboardFormatId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if ( aHelper.GetTransferableObjectDescriptor( nId, aDesc ) )
            {
                if ( aDesc.maClassName == pDocShell->GetFactory().GetClassId() )
                {
                    PostUserEvent( LINK( this, DropListBox_Impl, OnAsyncExecuteDrop ), 0 );

                    bFormatFound = true;
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

bool DropListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool nRet = false;
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if(!rKeyCode.GetModifier())
        {
            if( pDialog->bCanDel && KEY_DELETE == rKeyCode.GetCode())
            {
                pDialog->DeleteHdl( NULL );
                nRet =  true;
            }
            else if( KEY_RETURN == rKeyCode.GetCode())
            {
                GetDoubleClickHdl().Call(this);
                nRet = true;
            }
        }
    }
    if(!nRet)
        nRet = SvTreeListBox::Notify( rNEvt );
    return nRet;
}

/** ListBox class that starts a PopupMenu (designer specific) in the
    command handler.
*/
SfxActionListBox::SfxActionListBox(SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits)
    : DropListBox_Impl(pParent->GetWindow(), nWinBits, pParent)
{
    EnableContextMenuHandling();
}

PopupMenu* SfxActionListBox::CreateContextMenu()
{

    if( !( GetSelectionCount() > 0 ) )
    {
        pDialog->EnableEdit( false );
        pDialog->EnableDel( false );
    }
    return pDialog->CreateContextMenu();
}

SfxTemplatePanelControl::SfxTemplatePanelControl(SfxBindings* pBindings, vcl::Window* pParentWindow)
    : DockingWindow(pParentWindow, SfxResId(DLG_STYLE_DESIGNER))
    , pImpl(new SfxTemplateDialog_Impl(pBindings, this))
    , mpBindings(pBindings)
{
    OSL_ASSERT(mpBindings!=NULL);

    pImpl->updateNonFamilyImages();

    SetStyle(GetStyle() & ~WB_DOCKABLE);
}

SfxTemplatePanelControl::~SfxTemplatePanelControl()
{
}

void SfxTemplatePanelControl::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( ( DataChangedEventType::SETTINGS == _rDCEvt.GetType() ) &&
         ( AllSettingsFlags::STYLE & _rDCEvt.GetFlags() ) )
    {
        pImpl->updateFamilyImages();
        pImpl->updateNonFamilyImages();
    }

    DockingWindow::DataChanged( _rDCEvt );
}

void SfxTemplatePanelControl::Resize()
{
    if(pImpl)
        pImpl->Resize();
    DockingWindow::Resize();
}

void SfxTemplatePanelControl::FreeResource()
{
    DockingWindow::FreeResource();
}

void SfxTemplatePanelControl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::INITSHOW )
    {
        SfxViewFrame *pFrame = mpBindings->GetDispatcher_Impl()->GetFrame();
        vcl::Window* pEditWin = pFrame->GetViewShell()->GetWindow();

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

void StyleTreeListBox_Impl::MakeExpanded_Impl(ExpandedEntries_t& rEntries) const
{
    SvTreeListEntry* pEntry;
    for (pEntry = FirstVisible(); pEntry; pEntry = NextVisible(pEntry))
    {
        if (IsExpanded(pEntry))
        {
            rEntries.push_back(GetEntryText(pEntry));
        }
    }
}

PopupMenu* StyleTreeListBox_Impl::CreateContextMenu()
{
    return pDialog->CreateContextMenu();
}

/** DoubleClick-Handler; calls the link.
    SV virtual method.
*/
bool StyleTreeListBox_Impl::DoubleClickHdl()
{
    aDoubleClickLink.Call(this);
    return false;
}

/** Command Handler; this executes a PopupMenu (designer-specific)
    SV virtual method.
*/
void StyleTreeListBox_Impl::Command( const CommandEvent& rCEvt )
{
    SvTreeListBox::Command(rCEvt);
}

bool StyleTreeListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    // handle <RETURN> as double click

    bool nRet = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if ( !rKeyCode.GetModifier() && KEY_RETURN == rKeyCode.GetCode() )
        {
            aDoubleClickLink.Call( this );
            nRet = true;
        }
    }

    if ( !nRet )
        nRet = DropListBox_Impl::Notify( rNEvt );

    return nRet;
}

/** NotifyMoving Handler; This leads via a link on the event to the dialog.
    SV virtual method.
*/
TriState StyleTreeListBox_Impl::NotifyMoving(SvTreeListEntry*  pTarget,
                                         SvTreeListEntry*  pEntry,
                                         SvTreeListEntry*& rpNewParent,
                                         sal_uIntPtr& lPos)
{
    if(!pTarget || !pEntry)
        return TRISTATE_FALSE;
    aParent = GetEntryText(pTarget);
    aStyle  = GetEntryText(pEntry);
    const bool bRet = (bool)aDropLink.Call(this);
    rpNewParent = pTarget;
    lPos=0;
    IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();
    for(SvTreeListEntry *pTmpEntry=FirstChild(pTarget);
        pTmpEntry && pCollator->compareString(
            GetEntryText(pTmpEntry),GetEntryText(pEntry)) < 0;
        pTmpEntry=NextSibling(pTmpEntry),lPos++) ;

    return bRet ? TRISTATE_INDET : TRISTATE_FALSE;
}

/** ExpandingHdl Handler; the current entry is noticed.
    SV virtual method.

    [Cross-reference]
    <StyleTreeListBox_Impl::ExpandedHdl()>
*/
bool  StyleTreeListBox_Impl::ExpandingHdl()
{
    pCurEntry = GetCurEntry();
    return true;
}

/**  ExpandedHdl Handler;
    SV virtual method.

    [Cross-reference]
    <StyleTreeListBox_Impl::ExpandingHdl()>
*/
void  StyleTreeListBox_Impl::ExpandedHdl()
{
    SvTreeListEntry *pEntry = GetHdlEntry();
    if(!IsExpanded(pEntry) && pCurEntry != GetCurEntry())
        SelectAll( false );
    pCurEntry = 0;
}

/** Constructor StyleTreeListBox_Impl */
StyleTreeListBox_Impl::StyleTreeListBox_Impl(SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle)
    : DropListBox_Impl(pParent->GetWindow(), nWinStyle, pParent)
    , pCurEntry(0)
{
    EnableContextMenuHandling();
}

/** Internal structure for the establishment of the hierarchical view */
class StyleTree_Impl;
typedef std::vector<StyleTree_Impl*> StyleTreeArr_Impl;

class StyleTree_Impl
{
private:
    OUString aName;
    OUString aParent;
    StyleTreeArr_Impl pChildren;

public:

    bool HasParent() const { return !aParent.isEmpty(); }

    StyleTree_Impl(const OUString &rName, const OUString &rParent):
        aName(rName), aParent(rParent), pChildren(0) {}
    ~StyleTree_Impl();
    void Put(StyleTree_Impl* pIns, sal_uIntPtr lPos=ULONG_MAX);
    sal_uIntPtr Count();

    OUString getName() { return aName; }
    OUString getParent() { return aParent; }
    StyleTree_Impl *operator[](size_t idx) const { return pChildren[idx]; }
};

sal_uIntPtr StyleTree_Impl::Count()
{
    return pChildren.size();
}

StyleTree_Impl::~StyleTree_Impl()
{
    for(StyleTreeArr_Impl::const_iterator it = pChildren.begin(); it != pChildren.end(); ++it)
        delete *it;
}

void StyleTree_Impl::Put(StyleTree_Impl* pIns, sal_uIntPtr lPos)
{
    if ( ULONG_MAX == lPos )
        pChildren.push_back( pIns );
    else
        pChildren.insert( pChildren.begin() + (sal_uInt16)lPos, pIns );
}



StyleTreeArr_Impl& MakeTree_Impl(StyleTreeArr_Impl& rArr)
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
                if(pCmp->getName() == pEntry->getParent())
                {
                    // Paste initial filter
                    sal_uInt16 nPos;
                    for( nPos = 0 ; nPos < pCmp->Count() &&
                             aSorter.compare((*pCmp)[nPos]->getName(), pEntry->getName()) < 0 ; nPos++)
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

inline bool IsExpanded_Impl( const ExpandedEntries_t& rEntries,
                             const OUString &rStr)
{
    for (size_t n = 0; n < rEntries.size(); ++n)
    {
        if (rEntries[n] == rStr)
            return true;
    }
    return false;
}

SvTreeListEntry* FillBox_Impl(SvTreeListBox *pBox,
                                 StyleTree_Impl* pEntry,
                                 const ExpandedEntries_t& rEntries,
                                 SvTreeListEntry* pParent = 0)
{
    SvTreeListEntry* pNewEntry = pBox->InsertEntry(pEntry->getName(), pParent);
    for(sal_uInt16 i = 0; i < pEntry->Count(); ++i)
        FillBox_Impl(pBox, (*pEntry)[i], rEntries, pNewEntry);
    return pNewEntry;
}

// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, vcl::Window* pW, bool )
    : mbIgnoreSelect(false)
    , pBindings(pB)
    , pWindow(pW)
    , pModule(NULL)
    , pIdle(NULL)
    , m_pStyleFamiliesId(NULL)
    , pStyleFamilies(NULL)
    , pStyleSheetPool(NULL)
    , pTreeBox(NULL)
    , pCurObjShell(NULL)
    , xModuleManager(frame::ModuleManager::create(::comphelper::getProcessComponentContext()))
    , m_pDeletionWatcher(NULL)

    , aFmtLb(this, WB_BORDER | WB_TABSTOP | WB_SORT | WB_QUICK_SEARCH)
    , aFilterLb(pW, WB_BORDER | WB_DROPDOWN | WB_TABSTOP)

    , nActFamily(0xffff)
    , nActFilter(0)
    , nAppFilter(0)

    , bDontUpdate(false)
    , bIsWater(false)
    , bEnabled(true)
    , bUpdate(false)
    , bUpdateFamily(false)
    , bCanEdit(false)
    , bCanDel(false)
    , bCanNew(true)
    , bCanHide(true)
    , bCanShow(false)
    , bWaterDisabled(false)
    , bNewByExampleDisabled(false)
    , bUpdateByExampleDisabled(false)
    , bTreeDrag(true)
    , bHierarchical(false)
    , m_bWantHierarchical(false)
    , bBindingUpdate(true)
{
    aFmtLb.SetAccessibleName(SfxResId(STR_STYLE_ELEMTLIST).toString());
    aFmtLb.SetHelpId( HID_TEMPLATE_FMT );
    aFilterLb.SetHelpId( HID_TEMPLATE_FILTER );
    aFmtLb.SetStyle( aFmtLb.GetStyle() | WB_SORT | WB_HIDESELECTION );
    vcl::Font aFont = aFmtLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFmtLb.SetFont( aFont );

    memset(pBoundItems, 0, sizeof(pBoundItems));
    memset(pFamilyState, 0, sizeof(pFamilyState));
}

sal_uInt16 SfxCommonTemplateDialog_Impl::StyleNrToInfoOffset(sal_uInt16 nId)
{
    const SfxStyleFamilyItem *pItem = pStyleFamilies->at( nId );
    return SfxTemplate::SfxFamilyIdToNId(pItem->GetFamily())-1;
}

void SfxTemplateDialog_Impl::EnableEdit(bool bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem( SID_STYLE_UPDATE_BY_EXAMPLE, bEnable);
}

void SfxCommonTemplateDialog_Impl::ReadResource()
{
    // Read global user resource
    for(sal_uInt16 i = 0; i < MAX_FAMILIES; ++i)
        pFamilyState[i] = 0;

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    pModule = pCurObjShell ? pCurObjShell->GetModule() : NULL;
    ResMgr* pMgr = pModule ? pModule->GetResMgr() : NULL;
    if (pMgr)
    {
        ResId aFamId( DLG_STYLE_DESIGNER, *pMgr );
        aFamId.SetRT(RSC_SFX_STYLE_FAMILIES);
        m_pStyleFamiliesId = new ResId( aFamId.GetId(), *pMgr );
        m_pStyleFamiliesId->SetRT(RSC_SFX_STYLE_FAMILIES);
        if (pMgr->IsAvailable(aFamId))
            pStyleFamilies = new SfxStyleFamilies( aFamId );
    }
    if (!pStyleFamilies)
        pStyleFamilies = new SfxStyleFamilies;

    nActFilter = pCurObjShell ? static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pCurObjShell ) ) : SFXSTYLEBIT_ALL;
    if ( pCurObjShell && SFXSTYLEBIT_ALL == nActFilter )
        nActFilter = pCurObjShell->GetAutoStyleFilterIndex();

    // Paste in the toolbox
    // reverse order, since always inserted at the head
    size_t nCount = pStyleFamilies->size();

    pBindings->ENTERREGISTRATIONS();

    size_t i;
    for (i = 0; i < nCount; ++i)
    {
        sal_uInt16 nSlot = 0;
        switch( (sal_uInt16)pStyleFamilies->at( i )->GetFamily() )
        {
            case SFX_STYLE_FAMILY_CHAR:
                nSlot = SID_STYLE_FAMILY1; break;
            case SFX_STYLE_FAMILY_PARA:
                nSlot = SID_STYLE_FAMILY2; break;
            case SFX_STYLE_FAMILY_FRAME:
                nSlot = SID_STYLE_FAMILY3; break;
            case SFX_STYLE_FAMILY_PAGE:
                nSlot = SID_STYLE_FAMILY4; break;
            case SFX_STYLE_FAMILY_PSEUDO:
                nSlot = SID_STYLE_FAMILY5; break;
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
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId( pItem->GetFamily() );
        InsertFamilyItem( nId, pItem );
    }

    LoadedFamilies();

    sal_uInt16 nStart = SID_STYLE_FAMILY1;
    sal_uInt16 nEnd = SID_STYLE_FAMILY4;

    for ( i = nStart; i <= nEnd; i++ )
        pBindings->Update(i);
}

void SfxCommonTemplateDialog_Impl::ClearResource()
{
    ClearFamilyList();
    impl_clear();
}

void SfxCommonTemplateDialog_Impl::impl_clear()
{
    delete pStyleFamilies;
    pStyleFamilies = NULL;
    sal_uInt16 i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(SfxTemplateItem, pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    pCurObjShell = NULL;

    DELETEZ( m_pStyleFamiliesId );
}

SfxCommonTemplateDialog_Impl::DeletionWatcher *
SfxCommonTemplateDialog_Impl::impl_setDeletionWatcher(
        DeletionWatcher *const pNewWatcher)
{
    DeletionWatcher *const pRet(m_pDeletionWatcher);
    m_pDeletionWatcher = pNewWatcher;
    return pRet;
}

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

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    OUString aEmpty;
#if defined STYLESPREVIEW
    Execute_Impl(SID_STYLE_END_PREVIEW,
        OUString(), OUString(),
        0, 0, 0, 0 );
#endif
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
    GetWindow()->Hide();
    impl_clear();
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = NULL;
    delete pTreeBox;
    delete pIdle;
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
}

namespace SfxTemplate
{
    sal_uInt16 SfxFamilyIdToNId(SfxStyleFamily nFamily)
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

    SfxStyleFamily NIdToSfxFamilyId(sal_uInt16 nId)
    {
        switch (nId)
        {
            case 1: return SFX_STYLE_FAMILY_CHAR;
            case 2: return SFX_STYLE_FAMILY_PARA;
            case 3: return SFX_STYLE_FAMILY_FRAME;
            case 4: return SFX_STYLE_FAMILY_PAGE;
            case 5: return SFX_STYLE_FAMILY_PSEUDO;
            default: return SFX_STYLE_FAMILY_ALL;
        }
    }
}

// Helper function: Access to the current family item
const SfxStyleFamilyItem *SfxCommonTemplateDialog_Impl::GetFamilyItem_Impl() const
{
    const size_t nCount = pStyleFamilies->size();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem *pItem = pStyleFamilies->at( i );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId(pItem->GetFamily());
        if(nId == nActFamily)
            return pItem;
    }
    return 0;
}

SfxStyleSheetBase *SfxCommonTemplateDialog_Impl::GetSelectedStyle() const
{
    if (!IsInitialized() || !pStyleSheetPool || !HasSelectedStyle())
        return NULL;
    const OUString aTemplName( GetSelectedEntry() );
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    return pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );
}

void SfxCommonTemplateDialog_Impl::SelectStyle(const OUString &rStr)
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
        EnableEdit( false );
        EnableHide( false );
        EnableShow( false );
    }

    if ( pTreeBox )
    {
        if ( !rStr.isEmpty() )
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
            pTreeBox->SelectAll( false );
    }
    else
    {
        bool bSelect = ! rStr.isEmpty();
        if ( bSelect )
        {
            SvTreeListEntry* pEntry = (SvTreeListEntry*)aFmtLb.FirstVisible();
            while ( pEntry && aFmtLb.GetEntryText( pEntry ) != rStr )
                pEntry = (SvTreeListEntry*)aFmtLb.NextVisible( pEntry );
            if ( !pEntry )
                bSelect = false;
            else
            {
                if (!aFmtLb.IsSelected(pEntry))
                {
                    aFmtLb.MakeVisible( pEntry );
                    aFmtLb.SelectAll(false);
                    aFmtLb.Select( pEntry );
                    bWaterDisabled = (pTreeBox || aFmtLb.GetSelectionCount() <= 1) ? sal_False : sal_True;
                    FmtSelectHdl( NULL );
                }
            }
        }

        if ( !bSelect )
        {
            aFmtLb.SelectAll( false );
            EnableEdit(false);
            EnableHide( false );
            EnableShow( false );
        }
    }
}

OUString SfxCommonTemplateDialog_Impl::GetSelectedEntry() const
{
    OUString aRet;
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

void SfxCommonTemplateDialog_Impl::EnableTreeDrag( bool bEnable )
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

void SfxCommonTemplateDialog_Impl::FillTreeBox()
{
    OSL_ENSURE( pTreeBox, "FillTreeBox() without treebox");
    if(pStyleSheetPool && nActFamily != 0xffff)
    {
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        if(!pItem)
            return;
        pStyleSheetPool->SetSearchMask(pItem->GetFamily(), SFXSTYLEBIT_ALL_VISIBLE);
        StyleTreeArr_Impl aArr;
        SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
        if(pStyle && pStyle->HasParentSupport() && bTreeDrag )
            pTreeBox->SetDragDropMode(SV_DRAGDROP_CTRL_MOVE);
        else
            pTreeBox->SetDragDropMode(SV_DRAGDROP_NONE);
        while(pStyle)
        {
            StyleTree_Impl* pNew = new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
            aArr.push_back(pNew);
            pStyle = pStyleSheetPool->Next();
        }
        MakeTree_Impl(aArr);
        ExpandedEntries_t aEntries;
        pTreeBox->MakeExpanded_Impl(aEntries);
        pTreeBox->SetUpdateMode( false );
        pTreeBox->Clear();
        const sal_uInt16 nCount = aArr.size();
        for(sal_uInt16 i = 0; i < nCount; ++i)
            FillBox_Impl(pTreeBox, aArr[i], aEntries);

        EnableItem(SID_STYLE_WATERCAN,false);

        SfxTemplateItem* pState = pFamilyState[nActFamily-1];

        if ( nCount )
            pTreeBox->Expand( pTreeBox->First() );

        for ( SvTreeListEntry* pEntry = pTreeBox->First(); pEntry; pEntry = pTreeBox->Next( pEntry ) )
        {
            if ( IsExpanded_Impl( aEntries, pTreeBox->GetEntryText( pEntry ) ) )
                pTreeBox->Expand( pEntry );
        }

        pTreeBox->SetUpdateMode( true );

        OUString aStyle;
        if(pState)  // Select current entry
            aStyle = pState->GetStyleName();
        SelectStyle(aStyle);
        EnableDelete();
    }
}

bool SfxCommonTemplateDialog_Impl::HasSelectedStyle() const
{
    return pTreeBox? pTreeBox->FirstSelected() != 0:
            aFmtLb.GetSelectionCount() != 0;
}

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
            CheckItem(nActFamily, true);    // check Button in Toolbox
            aFilterLb.SetUpdateMode(false);
            aFilterLb.Clear();
            //insert hierarchical at the beginning
            sal_uInt16 nPos = aFilterLb.InsertEntry(SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString(), 0);
            aFilterLb.SetEntryData( nPos, reinterpret_cast<void*>(SFXSTYLEBIT_ALL) );
            const SfxStyleFilter& rFilter = pItem->GetFilterList();
            for( size_t i = 0; i < rFilter.size(); ++i)
            {
                sal_uIntPtr nFilterFlags = rFilter[ i ]->nFlags;
                nPos = aFilterLb.InsertEntry( rFilter[ i ]->aName );
                aFilterLb.SetEntryData( nPos, reinterpret_cast<void*>(nFilterFlags) );
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
            aFilterLb.SetUpdateMode(true);
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
            EnableItem(SID_STYLE_WATERCAN,false);

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
                  aStrings[nPos] == aFmtLb.GetEntryText(pEntry))
            {
                ++nPos;
                pEntry = aFmtLb.Next( pEntry );
            }

            if( nPos < nCount || pEntry )
            {
                // Fills the display box
                aFmtLb.SetUpdateMode(false);
                aFmtLb.Clear();

                for(nPos = 0; nPos < nCount; ++nPos)
                    aFmtLb.InsertEntry(aStrings[nPos], 0, false, nPos);

                aFmtLb.SetUpdateMode(true);
            }
            // Selects the current style if any
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            OUString aStyle;
            if(pState)
                aStyle = pState->GetStyleName();
#if defined STYLESPREVIEW
            mbIgnoreSelect = true; // in case we get a selection change
            // in anycase we should stop any preview
            Execute_Impl(SID_STYLE_END_PREVIEW,
            String(), String(),
            0, 0, 0, 0 );
#endif
            SelectStyle(aStyle);
            EnableDelete();
        }
    }
}

// Updated display: Watering the house
void SfxCommonTemplateDialog_Impl::SetWaterCanState(const SfxBoolItem *pItem)
{
    bWaterDisabled =  pItem == 0;

    if(!bWaterDisabled)
        //make sure the watercan is only activated when there is (only) one selection
        bWaterDisabled = (pTreeBox || aFmtLb.GetSelectionCount() <= 1) ? sal_False : sal_True;

    if(pItem && !bWaterDisabled)
    {
        CheckItem(SID_STYLE_WATERCAN, pItem->GetValue());
        EnableItem( SID_STYLE_WATERCAN, true );
    }
    else
        if(!bWaterDisabled)
            EnableItem(SID_STYLE_WATERCAN, true);
        else
            EnableItem(SID_STYLE_WATERCAN, false);

// Ignore while in watercan mode statusupdates

    size_t nCount = pStyleFamilies->size();
    pBindings->EnterRegistrations();
    for(size_t n = 0; n < nCount; n++)
    {
        SfxControllerItem *pCItem=pBoundItems[n];
        bool bChecked = pItem && pItem->GetValue();
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

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>
void SfxCommonTemplateDialog_Impl::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    DELETEZ(pFamilyState[nIdx]);
    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem(*pItem);
    bUpdate = true;

    // If used templates (how the hell you find this out??)
    bUpdateFamily = true;
}

// Notice from SfxBindings that the update is completed. Pushes out the update
// of the display.
void SfxCommonTemplateDialog_Impl::Update_Impl()
{
    bool bDocChanged=false;
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
            bDocChanged=true;
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
         CheckItem(nActFamily, false);
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
         CheckItem( nActFamily, true );
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
         CheckItem( nActFamily, true );
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
         if ( pStyleItem && 0 == pStyleItem->GetFilterList()[ nActFilter ]->nFlags
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
     const OUString aStyle(pItem->GetStyleName());
     SelectStyle(aStyle);
     EnableDelete();
     EnableNew( bCanNew );
}

IMPL_LINK( SfxCommonTemplateDialog_Impl, TimeOut, Timer *, pTim )
{
    (void)pTim; // unused
    if(!bDontUpdate)
    {
        bDontUpdate=true;
        if(!pTreeBox)
            UpdateStyles_Impl(UPDATE_FAMILY_LIST);
        else
        {
            FillTreeBox();
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            if(pState)
            {
                const OUString aStyle(pState->GetStyleName());
                SelectStyle(aStyle);
                EnableDelete();
            }
        }
        bDontUpdate=false;
        DELETEZ(pIdle);
    }
    else
        pIdle->Start();
    return 0;
}

void SfxCommonTemplateDialog_Impl::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    // tap update
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if(pSfxSimpleHint)
    {
        switch(pSfxSimpleHint->GetId())
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
                    bUpdate = false;
                    Update_Impl();
                }
                else if ( bUpdateFamily )
                {
                    UpdateFamily_Impl();
                }

                if( pStyleSheetPool )
                {
                    OUString aStr = GetSelectedEntry();
                    if( !aStr.isEmpty() && pStyleSheetPool )
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
                            EnableEdit(false);
                            EnableHide(false);
                            EnableShow(false);
                        }
                    }
                }
                break;
            }

        // Necessary if switching between documents and in both documents
        // the same template is used. Do not immediately call Update_Impl,
        // for the case that one of the documents is an internal InPlaceObjekt!
          case SFX_HINT_DOCCHANGED:
            bUpdate = true;
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

    sal_uIntPtr nId = pSfxSimpleHint ? pSfxSimpleHint->GetId() : 0;

    if(!bDontUpdate && nId != SFX_HINT_DYING &&
       (dynamic_cast<const SfxStyleSheetPoolHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetHintExtended*>(&rHint)))
    {
        if(!pIdle)
        {
            pIdle=new Idle;
            pIdle->SetPriority(SchedulerPriority::LOWEST);
            pIdle->SetIdleHdl(LINK(this,SfxCommonTemplateDialog_Impl,TimeOut));
        }
        pIdle->Start();

    }
}

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void SfxCommonTemplateDialog_Impl::FilterSelect(
                sal_uInt16 nEntry,  // Idx of the new Filters
                bool bForce )   // Force update, even if the new filter is
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

// Internal: Perform functions through the Dispatcher
bool SfxCommonTemplateDialog_Impl::Execute_Impl(
    sal_uInt16 nId, const OUString &rStr, const OUString& rRefStr, sal_uInt16 nFamily,
    sal_uInt16 nMask, sal_uInt16 *pIdx, const sal_uInt16* pModifier)
{
    SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, nMask );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask )
        pItems[ nCount++ ] = &aMask;
    if(SID_STYLE_UPDATE_BY_EXAMPLE == nId)
    {
        // Special solution for Numbering update in Writer
        const OUString aTemplName(GetSelectedEntry());
        aUpdName.SetValue(aTemplName);
        pItems[ nCount++ ] = &aUpdName;
    }

    if ( !rRefStr.isEmpty() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = 0;

    DeletionWatcher aDeleted(*this);
    sal_uInt16 nModi = pModifier ? *pModifier : 0;
    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD | SfxCallMode::MODAL,
        pItems, nModi );

    // Dialog can be destroyed while in Execute() because started
    // subdialogs are not modal to it (#i97888#).
    if ( !pItem || aDeleted )
        return false;

    if ( (nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId) && (pTreeBox || aFmtLb.GetSelectionCount() <= 1) )
    {
        const SfxUInt16Item *pFilterItem = PTR_CAST(SfxUInt16Item, pItem);
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

    return true;
}

// Handler der Listbox der Filter
void SfxCommonTemplateDialog_Impl::EnableHierarchical(bool const bEnable)
{
    if (bEnable)
    {
        if (!bHierarchical)
        {
            // Turn on treeView
            bHierarchical=true;
            m_bWantHierarchical = true;
            SaveSelection(); // fdo#61429 store "hierarchical"
            const OUString aSelectEntry( GetSelectedEntry());
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
            pTreeBox->SetDoubleClickHdl(
                    LINK(this, SfxCommonTemplateDialog_Impl,  ApplyHdl));
            pTreeBox->SetDropHdl(LINK(this, SfxCommonTemplateDialog_Impl,  DropHdl));
            pTreeBox->SetOptimalImageIndent();
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
        m_bWantHierarchical = false; // before FilterSelect
        FilterSelect(aFilterLb.GetSelectEntryPos() - 1, bHierarchical );
        bHierarchical=false;
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

// Select-Handler for the Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(sal_uInt16 nEntry)
{
    if( nEntry != nActFamily )
    {
        CheckItem( nActFamily, false );
        nActFamily = nEntry;
        SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
        SfxUInt16Item aItem( SID_STYLE_FAMILY, nEntry );
        pDispat->Execute( SID_STYLE_FAMILY, SfxCallMode::SYNCHRON, &aItem, 0L );
        pBindings->Invalidate( SID_STYLE_FAMILY );
        pBindings->Update( SID_STYLE_FAMILY );
        UpdateFamily_Impl();
    }
}

void SfxCommonTemplateDialog_Impl::ActionSelect(sal_uInt16 nEntry)
{
    OUString aEmpty;
    switch(nEntry)
    {
        case SID_STYLE_WATERCAN:
        {
            const bool bState = IsCheckedItem(nEntry);
            bool bCheck;
            SfxBoolItem aBool;
            // when a template is chosen.
            if (!bState && HasSelectedStyle())
            {
                const OUString aTemplName(
                    GetSelectedEntry());
                Execute_Impl(
                    SID_STYLE_WATERCAN, aTemplName, aEmpty,
                    (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
                bCheck = true;
            }
            else
            {
                Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
                bCheck = false;
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

                boost::scoped_ptr<SfxNewStyleDlg> pDlg(new SfxNewStyleDlg(pWindow, *pStyleSheetPool));
                    // why? : FloatingWindow must not be parent of a modal dialog
                if(RET_OK == pDlg->Execute())
                {
                    pStyleSheetPool->SetSearchMask(eFam, nFilter);
                    const OUString aTemplName(pDlg->GetName());
                    Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE,
                                 aTemplName, aEmpty,
                                 (sal_uInt16)GetFamilyItem_Impl()->GetFamily(),
                                 nFilter);
                }
                pStyleSheetPool->SetSearchMask( eFam, nFilter );
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
            SfxGetpApp()->GetDispatcher_Impl()->Execute(nEntry);
        break;
        default: OSL_FAIL("not implemented"); break;
    }
}

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

sal_Int32 SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh )
{
    OSL_ENSURE( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    sal_Int32 nFilter = -1;

    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
    sal_Int32 nDefault = -1;
    nFilter = aFactoryProps.getUnpackedValueOrDefault( "ooSetupFactoryStyleFilter", nDefault );

    m_bWantHierarchical =
        (nFilter & SFXSTYLEBIT_HIERARCHY) ? sal_True : sal_False;
    nFilter &= ~SFXSTYLEBIT_HIERARCHY; // clear it

    return nFilter;
}

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

IMPL_LINK( SfxCommonTemplateDialog_Impl, DropHdl, StyleTreeListBox_Impl *, pBox )
{
    bDontUpdate=true;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    long ret= pStyleSheetPool->SetParent(eFam,pBox->GetStyle(), pBox->GetParent())? 1L: 0L;
    bDontUpdate=false;
    return ret;
}

// Handler for the New-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl(void *)
{
    OUString aEmpty;
    if ( nActFamily != 0xffff && (pTreeBox || aFmtLb.GetSelectionCount() <= 1))
    {
        vcl::Window* pTmp;
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );

        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam=pItem->GetFamily();
        sal_uInt16 nMask;
        if( nActFilter != SFXSTYLEBIT_ALL )
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

// Handler for the edit-Buttons
void SfxCommonTemplateDialog_Impl::EditHdl(void *)
{
    if(IsInitialized() && HasSelectedStyle())
    {
        sal_uInt16 nFilter = nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        vcl::Window* pTmp;
        //DefModalDialogParent set for modality of the following dialogs
        pTmp = Application::GetDefDialogParent();
        if ( ISA(SfxTemplateDialog_Impl) )
            Application::SetDefDialogParent( pWindow->GetParent() );
        else
            Application::SetDefDialogParent( pWindow );
        if ( Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),
                          (sal_uInt16)GetFamilyItem_Impl()->GetFamily(), 0, &nFilter ) )
        {
        }
        Application::SetDefDialogParent( pTmp );
    }
}

// Handler for the Delete-Buttons
void SfxCommonTemplateDialog_Impl::DeleteHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        bool bUsedStyle = false;     // one of the selected styles are used in the document?

        std::vector<SvTreeListEntry*> aList;
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb.FirstSelected();
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

        OUString aMsg = SfxResId(STR_DELETE_STYLE_USED).toString();
        aMsg += SfxResId(STR_DELETE_STYLE).toString();

        while (pEntry)
        {
            aList.push_back( pEntry );
            // check the style is used or not
            const OUString aTemplName(pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb.GetEntryText(pEntry));

            SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( aTemplName, pItem->GetFamily(), SFXSTYLEBIT_ALL );

            if ( pStyle->IsUsed() )  // pStyle is in use in the document?
            {
                if (bUsedStyle) // add a separator for the second and later styles
                    aMsg += ", ";
                aMsg += aTemplName;
                bUsedStyle = true;
            }

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb.NextSelected(pEntry);
        }

        bool aApproved = false;

        // we only want to show the dialog once and if we want to delete a style in use (UX-advice)
        if ( bUsedStyle )
        {
        #if defined UNX
            MessageDialog aBox(SfxGetpApp()->GetTopWindow(), aMsg,
                               VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
        #else
            MessageDialog aBox(GetWindow(), aMsg,
                               VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
        #endif
            aApproved = aBox.Execute() == RET_YES;
        }

        // if there are no used styles selected or the user approved the changes
        if ( !bUsedStyle || aApproved )
        {
            std::vector<SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();

            for (; it != itEnd; ++it)
            {
                const OUString aTemplName(pTreeBox ? pTreeBox->GetEntryText(*it) : aFmtLb.GetEntryText(*it));
                bDontUpdate = true; // To prevent the Treelistbox to shut down while deleting
                Execute_Impl( SID_STYLE_DELETE, aTemplName,
                              OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

                if ( pTreeBox )
                {
                    pTreeBox->RemoveParentKeepChildren( *it );
                    bDontUpdate = false;
                }
            }
            bDontUpdate = false; //if everything is deleted set bDontUpdate back to false
            UpdateStyles_Impl(UPDATE_FAMILY_LIST); //and force-update the list
        }
    }
}

void SfxCommonTemplateDialog_Impl::HideHdl(void *)
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb.FirstSelected();

        while (pEntry)
        {
            OUString aTemplName = pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb.GetEntryText(pEntry);

            Execute_Impl( SID_STYLE_HIDE, aTemplName,
                          OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb.NextSelected(pEntry);
        }
    }
}

void SfxCommonTemplateDialog_Impl::ShowHdl(void *)
{

    if ( IsInitialized() && HasSelectedStyle() )
    {
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb.FirstSelected();

        while (pEntry)
        {
            OUString aTemplName = pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb.GetEntryText(pEntry);

            Execute_Impl( SID_STYLE_SHOW, aTemplName,
                          OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb.NextSelected(pEntry);
        }
    }
}

void SfxCommonTemplateDialog_Impl::EnableDelete()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        OSL_ENSURE(pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
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
            EnableDel(true);
        }
        else
        {
            EnableDel(false);
        }
    }
    else
    {
        EnableDel(false);
    }
}

// After selecting a focused item if possible again on the app window
void SfxCommonTemplateDialog_Impl::ResetFocus()
{
    if(ISA(SfxTemplateDialog_Impl))
    {
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell *pVu = pViewFrame->GetViewShell();
        vcl::Window *pAppWin = pVu ? pVu->GetWindow(): 0;
        if(pAppWin)
            pAppWin->GrabFocus();
    }
}

// Double-click on a style sheet in the ListBox is applied.
IMPL_LINK( SfxCommonTemplateDialog_Impl, ApplyHdl, Control *, pControl )
{
    (void)pControl; //unused
    // only if that region is allowed
    if ( IsInitialized() && 0 != pFamilyState[nActFamily-1] &&
         !GetSelectedEntry().isEmpty() )
    {
        sal_uInt16 nModifier = aFmtLb.GetModifier();
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), OUString(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
    }
    ResetFocus();
    return 0;
}

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
             0 != pFamilyState[nActFamily-1] && (pTreeBox || aFmtLb.GetSelectionCount() <= 1) )
        {
            OUString aEmpty;
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
    {
        SelectStyle( pListBox->GetEntryText( pListBox->GetHdlEntry() ));
#if defined STYLESPREVIEW
        sal_uInt16 nModifier = aFmtLb.GetModifier();
        if ( mbIgnoreSelect )
        {
            Execute_Impl(SID_STYLE_END_PREVIEW,
            String(), String(),
            0, 0, 0, 0 );
            mbIgnoreSelect = false;
        }
        else
        {
            Execute_Impl(SID_STYLE_PREVIEW,
                     GetSelectedEntry(), String(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
        }
#endif
    }

    return 0;
}

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

SfxStyleFamily SfxCommonTemplateDialog_Impl::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SFX_STYLE_FAMILY_PARA;
    else
        return pFamilyItem->GetFamily();
}

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    if( nId == SID_STYLE_NEW_BY_EXAMPLE )
    {
        bNewByExampleDisabled = (pTreeBox || aFmtLb.GetSelectionCount() <= 1) ? !bEnable : sal_True;
    }
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
        bUpdateByExampleDisabled = (pTreeBox || aFmtLb.GetSelectionCount() <= 1) ? !bEnable : sal_True;

    EnableItem(nId, bEnable);
}

PopupMenu* SfxCommonTemplateDialog_Impl::CreateContextMenu()
{
    if ( bBindingUpdate )
    {
        pBindings->Invalidate( SID_STYLE_NEW, true, false );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = false;
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

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(
    SfxBindings* pB, SfxTemplatePanelControl* pDlgWindow )
    : SfxCommonTemplateDialog_Impl( pB, pDlgWindow, true ),
      m_pFloat          ( pDlgWindow ),
      m_bZoomIn         ( false ),
      m_aActionTbL        ( pDlgWindow, this ),
      m_aActionTbR      ( pDlgWindow, SfxResId( TB_ACTION ) )
{
    pDlgWindow->FreeResource();
    Initialize();
}

void SfxTemplateDialog_Impl::Initialize()
{
    SfxCommonTemplateDialog_Impl::Initialize();

    m_aActionTbL.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    m_aActionTbR.SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_aActionTbR.SetDropdownClickHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRClick));
    m_aActionTbL.Show();
    m_aActionTbR.Show();
    vcl::Font aFont = aFilterLb.GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFilterLb.SetFont( aFont );
    m_aActionTbL.SetHelpId( HID_TEMPLDLG_TOOLBOX_LEFT );
}

void SfxTemplateDialog_Impl::EnableFamilyItem( sal_uInt16 nId, bool bEnable )
{
    m_aActionTbL.EnableItem( nId, bEnable );
}

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
    m_aActionTbL.InsertItem( nId, pItem->GetImage(), pItem->GetText(), ToolBoxItemBits::NONE, 0);
    m_aActionTbL.SetHelpId( nId, sHelpId );
}

void SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    m_aActionTbR.HideItem(SID_STYLE_UPDATE_BY_EXAMPLE);
    m_aActionTbR.SetItemBits( SID_STYLE_NEW_BY_EXAMPLE,
            ToolBoxItemBits::DROPDOWNONLY|m_aActionTbR.GetItemBits( SID_STYLE_NEW_BY_EXAMPLE ));
}

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
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId( pItem->GetFamily() );
        m_aActionTbL.SetItemImage( nId, pItem->GetImage() );
    }
}

void SfxTemplateDialog_Impl::updateNonFamilyImages()
{
    m_aActionTbR.SetImageList( ImageList( SfxResId( DLG_STYLE_DESIGNER ) ) );
}

void SfxTemplateDialog_Impl::ClearFamilyList()
{
    m_aActionTbL.Clear();
}

void SfxCommonTemplateDialog_Impl::InvalidateBindings()
{
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, true, false);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, true, false);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, true, false);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, true, false );
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, true, false );
    pBindings->Update( SID_STYLE_DRAGHIERARCHIE );
}

SfxTemplateDialog_Impl::~SfxTemplateDialog_Impl()
{
}

void SfxTemplateDialog_Impl::LoadedFamilies()
{
    updateFamilyImages();
    Resize();
}

// Override Resize-Handler ( StarView )
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

void SfxTemplateDialog_Impl::EnableItem(sal_uInt16 nMesId, bool bCheck)
{
    OUString aEmpty;
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            if(!bCheck && IsCheckedItem(SID_STYLE_WATERCAN))
                Execute_Impl(SID_STYLE_WATERCAN, aEmpty, aEmpty, 0);
            //fall-through
        case SID_STYLE_NEW_BY_EXAMPLE:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
            m_aActionTbR.EnableItem(nMesId,bCheck);
            break;
    }
}

void SfxTemplateDialog_Impl::CheckItem(sal_uInt16 nMesId, bool bCheck)
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

bool SfxTemplateDialog_Impl::IsCheckedItem(sal_uInt16 nMesId)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            return m_aActionTbR.GetItemState(SID_STYLE_WATERCAN)==TRISTATE_TRUE;
        default:
            return m_aActionTbL.GetItemState(nMesId)==TRISTATE_TRUE;
    }
}

IMPL_LINK_INLINE_START( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    FamilySelect(nEntry);
    return 0;
}
IMPL_LINK_INLINE_END( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox )

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

IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxRSelect, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry != SID_STYLE_NEW_BY_EXAMPLE ||
            ToolBoxItemBits::DROPDOWN != (pBox->GetItemBits(nEntry)&ToolBoxItemBits::DROPDOWN))
        ActionSelect(nEntry);
    return 0;
}

IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxRClick, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry == SID_STYLE_NEW_BY_EXAMPLE &&
            ToolBoxItemBits::DROPDOWN == (pBox->GetItemBits(nEntry)&ToolBoxItemBits::DROPDOWN))
    {
        //create a popup menu in Writer
        boost::scoped_ptr<PopupMenu> pMenu(new PopupMenu);
        uno::Reference< container::XNameAccess > xNameAccess(
                frame::theUICommandDescription::get(
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

IMPL_LINK( SfxTemplateDialog_Impl, MenuSelectHdl, Menu*, pMenu)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    ActionSelect(nMenuId);
    return 0;
}

void SfxCommonTemplateDialog_Impl::SetFamily( sal_uInt16 nId )
{
    if ( nId != nActFamily )
    {
        if ( nActFamily != 0xFFFF )
            CheckItem( nActFamily, false );
        nActFamily = nId;
        if ( nId != 0xFFFF )
            bUpdateFamily = true;
    }
}

void SfxCommonTemplateDialog_Impl::UpdateFamily_Impl()
{
    bUpdateFamily = false;

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

    bWaterDisabled = false;
    bCanNew = (pTreeBox || aFmtLb.GetSelectionCount() <= 1) ? sal_True : sal_False;
    bTreeDrag = true;
    bUpdateByExampleDisabled = false;

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
                      OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
}

void SfxCommonTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    //does nothing
}

DropToolBox_Impl::DropToolBox_Impl(vcl::Window* pParent, SfxTemplateDialog_Impl* pTemplateDialog) :
    ToolBox(pParent),
    DropTargetHelper(this),
    rParent(*pTemplateDialog)
{
}

DropToolBox_Impl::~DropToolBox_Impl()
{
}

sal_Int8 DropToolBox_Impl::AcceptDrop( const AcceptDropEvent& rEvt )
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
    if ( nItemId != SfxTemplate::SfxFamilyIdToNId( SFX_STYLE_FAMILY_PAGE )&&
        IsDropFormatSupported( SotClipboardFormatId::OBJECTDESCRIPTOR ) &&
        !rParent.bNewByExampleDisabled )
    {
        nReturn = DND_ACTION_COPY;
    }
    return nReturn;
}

sal_Int8 DropToolBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
     return rParent.aFmtLb.ExecuteDrop(rEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
