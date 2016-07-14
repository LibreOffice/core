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

#include <memory>

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
#include <officecfg/Office/Common.hxx>

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

#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <comphelper/string.hxx>

#include <sfx2/StyleManager.hxx>
#include <sfx2/StylePreviewRenderer.hxx>
#include <o3tl/make_unique.hxx>
using namespace css;
using namespace css::beans;
using namespace css::frame;
using namespace css::uno;

namespace
{

class StyleLBoxString : public SvLBoxString
{
    SfxStyleFamily meStyleFamily;
    SvViewDataItem* mpViewData;

public:
    StyleLBoxString(const OUString& sText,
                    const SfxStyleFamily& eStyleFamily);

    virtual ~StyleLBoxString();

    virtual void Paint(const Point& aPos,
                       SvTreeListBox& rDevice,
                       vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual void InitViewData(SvTreeListBox* pView,
                              SvTreeListEntry* pEntry,
                              SvViewDataItem* pViewData) override;
};


StyleLBoxString::StyleLBoxString(const OUString& sText, const SfxStyleFamily& eStyleFamily)
    : SvLBoxString(sText)
    , meStyleFamily(eStyleFamily)
    , mpViewData(nullptr)
{}

StyleLBoxString::~StyleLBoxString()
{}

void StyleLBoxString::InitViewData(SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if (!pViewData)
    {
        pViewData = pView->GetViewDataItem(pEntry, this);
    }
    mpViewData = pViewData;
}

void StyleLBoxString::Paint(
    const Point& aPos, SvTreeListBox& rDevice, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    bool bPainted = false;

    SfxObjectShell* pShell = SfxObjectShell::Current();
    sfx2::StyleManager* pStyleManager = pShell? pShell->GetStyleManager(): nullptr;

    if (pStyleManager)
    {
        SfxStyleSheetBase* pStyleSheet = pStyleManager->Search(GetText(), meStyleFamily);

        if (pStyleSheet)
        {
            sal_Int32 nSize = 32 * rRenderContext.GetDPIScaleFactor();
            std::unique_ptr<sfx2::StylePreviewRenderer> pStylePreviewRenderer(
                pStyleManager->CreateStylePreviewRenderer(rRenderContext, pStyleSheet, nSize));

            if (pStylePreviewRenderer)
            {
                if (pStylePreviewRenderer->recalculate())
                {
                    mpViewData->maSize = pStylePreviewRenderer->getRenderSize();
                }
                else
                {
                    SvLBoxString::InitViewData( &rDevice, const_cast<SvTreeListEntry*>(&rEntry), mpViewData);
                }

                Rectangle aPaintRectangle = pView->GetPaintRectangle();
                bPainted = pStylePreviewRenderer->render(aPaintRectangle);
            }
        }
    }

    if (!bPainted)
    {
        rRenderContext.DrawText(aPos, GetText());
    }
}

} // end anonymous namespace

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


class SfxCommonTemplateDialog_Impl::DeletionWatcher
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

    DeletionWatcher(const DeletionWatcher&) = delete;
    DeletionWatcher& operator=(const DeletionWatcher&) = delete;

    // Signal that the dialog was deleted
    void signal()
    {
        m_pDialog = nullptr;
        if (m_pPrevious)
            m_pPrevious->signal();
    }

    // Return true if the dialog was deleted
    operator bool_type() const
    {
        return m_pDialog ? nullptr : &DeletionWatcher::signal;
    }

private:
    SfxCommonTemplateDialog_Impl* m_pDialog;
    DeletionWatcher *const m_pPrevious; /// let's add more epicycles!
};

void DropListBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();

    bool bHitEmptySpace = ( nullptr == GetEntry( rMEvt.GetPosPixel(), true ) );
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
        if (pDialog->GetActualFamily() == SfxStyleFamily::Page ||
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
                    PostUserEvent( LINK( this, DropListBox_Impl, OnAsyncExecuteDrop ), nullptr, true );

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

IMPL_LINK_NOARG_TYPED(DropListBox_Impl, OnAsyncExecuteDrop, void*, void)
{
    pDialog->ActionSelect( SID_STYLE_NEW_BY_EXAMPLE );
}

bool DropListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if(!rKeyCode.GetModifier())
        {
            if( pDialog->bCanDel && KEY_DELETE == rKeyCode.GetCode())
            {
                pDialog->DeleteHdl();
                bRet =  true;
            }
            else if( KEY_RETURN == rKeyCode.GetCode())
            {
                GetDoubleClickHdl().Call(this);
                bRet = true;
            }
        }
    }
    if(!bRet)
        bRet = SvTreeListBox::Notify( rNEvt );
    return bRet;
}

/** ListBox class that starts a PopupMenu (designer specific) in the
    command handler.
*/
SfxActionListBox::SfxActionListBox(SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits)
    : DropListBox_Impl(pParent->GetWindow(), nWinBits, pParent)
{
    EnableContextMenuHandling();
}

void SfxActionListBox::Recalc()
{
    if (officecfg::Office::Common::StylesAndFormatting::Preview::get())
    {
        SetEntryHeight(32 * GetDPIScaleFactor());
        RecalcViewData();
    }
}

VclPtr<PopupMenu> SfxActionListBox::CreateContextMenu()
{

    if( !( GetSelectionCount() > 0 ) )
    {
        pDialog->EnableEdit( false );
        pDialog->EnableDel( false );
    }
    return pDialog->CreateContextMenu();
}

SfxTemplatePanelControl::SfxTemplatePanelControl(SfxBindings* pBindings, vcl::Window* pParentWindow)
    : Window(pParentWindow)
    , pImpl(new SfxTemplateDialog_Impl(pBindings, this))
    , mpBindings(pBindings)
{
    OSL_ASSERT(mpBindings!=nullptr);

    pImpl->updateNonFamilyImages();

    SetStyle(GetStyle() & ~WB_DOCKABLE);
}

SfxTemplatePanelControl::~SfxTemplatePanelControl()
{
    disposeOnce();
}

void SfxTemplatePanelControl::dispose()
{
    pImpl.reset();
    Window::dispose();
}

void SfxTemplatePanelControl::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( ( DataChangedEventType::SETTINGS == _rDCEvt.GetType() ) &&
         ( AllSettingsFlags::STYLE & _rDCEvt.GetFlags() ) )
    {
        pImpl->updateFamilyImages();
        pImpl->updateNonFamilyImages();
    }

    Window::DataChanged( _rDCEvt );
}

void SfxTemplatePanelControl::Resize()
{
    if(pImpl)
        pImpl->Resize();
    Window::Resize();
}

void SfxTemplatePanelControl::StateChanged( StateChangedType nStateChange )
{
    if (nStateChange == StateChangedType::InitShow)
    {
        SfxViewFrame* pFrame = mpBindings->GetDispatcher_Impl()->GetFrame();
        vcl::Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Size aSize = pEditWin->GetSizePixel();
        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        Size aWinSize = GetSizePixel();
        aPoint.X() += aSize.Width() - aWinSize.Width() - 20;
        aPoint.Y() += aSize.Height() / 2 - aWinSize.Height() / 2;
        // SetFloatingPos( aPoint );
    }

    Window::StateChanged( nStateChange );
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

VclPtr<PopupMenu> StyleTreeListBox_Impl::CreateContextMenu()
{
    return pDialog->CreateContextMenu();
}

/** DoubleClick-Handler; calls the link.
    SV virtual method.
*/
bool StyleTreeListBox_Impl::DoubleClickHdl()
{
    aDoubleClickLink.Call(nullptr);
    return false;
}

bool StyleTreeListBox_Impl::Notify( NotifyEvent& rNEvt )
{
    // handle <RETURN> as double click

    bool bRet = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        if ( !rKeyCode.GetModifier() && KEY_RETURN == rKeyCode.GetCode() )
        {
            aDoubleClickLink.Call( nullptr );
            bRet = true;
        }
    }

    if ( !bRet )
        bRet = DropListBox_Impl::Notify( rNEvt );

    return bRet;
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
    const bool bRet = aDropLink.Call(*this);
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
    pCurEntry = nullptr;
}

/** Constructor StyleTreeListBox_Impl */
StyleTreeListBox_Impl::StyleTreeListBox_Impl(SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle)
    : DropListBox_Impl(pParent->GetWindow(), nWinStyle, pParent)
    , pCurEntry(nullptr)
{
    EnableContextMenuHandling();
}

void StyleTreeListBox_Impl::Recalc()
{
    if (officecfg::Office::Common::StylesAndFormatting::Preview::get())
    {
        SetEntryHeight(32 * GetDPIScaleFactor());
        RecalcViewData();
    }
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
    size_t Count();

    const OUString& getName() { return aName; }
    const OUString& getParent() { return aParent; }
    StyleTree_Impl *operator[](size_t idx) const { return pChildren[idx]; }
};

size_t StyleTree_Impl::Count()
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
    for (const auto & rEntry : rEntries)
    {
        if (rEntry == rStr)
            return true;
    }
    return false;
}

SvTreeListEntry* FillBox_Impl(SvTreeListBox* pBox,
                              StyleTree_Impl* pEntry,
                              const ExpandedEntries_t& rEntries,
                              SfxStyleFamily eStyleFamily,
                              SvTreeListEntry* pParent = nullptr)
{
    SvTreeListEntry* pTreeListEntry = pBox->InsertEntry(pEntry->getName(), pParent);

    if (officecfg::Office::Common::StylesAndFormatting::Preview::get())
    {
        pTreeListEntry->ReplaceItem(o3tl::make_unique<StyleLBoxString>(pEntry->getName(), eStyleFamily), 1);
    }

    pBox->GetModel()->InvalidateEntry(pTreeListEntry);

    for(size_t i = 0; i < pEntry->Count(); ++i)
    {
        FillBox_Impl(pBox, (*pEntry)[i], rEntries, eStyleFamily, pTreeListEntry);
    }
    return pTreeListEntry;
}

// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl( SfxBindings* pB, vcl::Window* pW, bool )
    : pBindings(pB)
    , pWindow(pW)
    , pModule(nullptr)
    , pIdle(nullptr)
    , m_pStyleFamiliesId(nullptr)
    , pStyleFamilies(nullptr)
    , pStyleSheetPool(nullptr)
    , pTreeBox(nullptr)
    , pCurObjShell(nullptr)
    , xModuleManager(frame::ModuleManager::create(::comphelper::getProcessComponentContext()))
    , m_pDeletionWatcher(nullptr)

    , aFmtLb( VclPtr<SfxActionListBox>::Create(this, WB_BORDER | WB_TABSTOP | WB_SORT | WB_QUICK_SEARCH) )
    , aFilterLb( VclPtr<ListBox>::Create(pW, WB_BORDER | WB_DROPDOWN | WB_TABSTOP) )

    , nActFamily(0xffff)
    , nActFilter(0)
    , nAppFilter(0)

    , bDontUpdate(false)
    , bIsWater(false)
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
    aFmtLb->SetAccessibleName(SfxResId(STR_STYLE_ELEMTLIST).toString());
    aFmtLb->SetHelpId( HID_TEMPLATE_FMT );
    aFilterLb->SetHelpId( HID_TEMPLATE_FILTER );
    aFmtLb->SetStyle( aFmtLb->GetStyle() | WB_SORT | WB_HIDESELECTION );
    vcl::Font aFont = aFmtLb->GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFmtLb->SetFont( aFont );

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
    for(SfxTemplateItem* & rp : pFamilyState)
        rp = nullptr;

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    pModule = pCurObjShell ? pCurObjShell->GetModule() : nullptr;
    ResMgr* pMgr = pModule ? pModule->GetResMgr() : nullptr;
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
        switch( pStyleFamilies->at( i )->GetFamily() )
        {
            case SfxStyleFamily::Char:
                nSlot = SID_STYLE_FAMILY1; break;
            case SfxStyleFamily::Para:
                nSlot = SID_STYLE_FAMILY2; break;
            case SfxStyleFamily::Frame:
                nSlot = SID_STYLE_FAMILY3; break;
            case SfxStyleFamily::Page:
                nSlot = SID_STYLE_FAMILY4; break;
            case SfxStyleFamily::Pseudo:
                nSlot = SID_STYLE_FAMILY5; break;
            case SfxStyleFamily::Table:
                nSlot = SID_STYLE_FAMILY6; break;
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
        pBoundItems[i] = nullptr;

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
    pStyleFamilies = nullptr;
    sal_uInt16 i;
    for ( i = 0; i < MAX_FAMILIES; ++i )
        DELETEX(SfxTemplateItem, pFamilyState[i]);
    for ( i = 0; i < COUNT_BOUND_FUNC; ++i )
        delete pBoundItems[i];
    pCurObjShell = nullptr;

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

    aFilterLb->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FilterSelectHdl ) );
    aFmtLb->SetDoubleClickHdl( LINK( this, SfxCommonTemplateDialog_Impl, TreeListApplyHdl ) );
    aFmtLb->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, FmtSelectHdl ) );
    aFmtLb->SetSelectionMode(SelectionMode::Multiple);


    aFilterLb->Show();
    if (!bHierarchical)
        aFmtLb->Show();
}

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
#if defined STYLESPREVIEW
    Execute_Impl(SID_STYLE_END_PREVIEW,
        OUString(), OUString(),
        0, 0, 0, 0 );
#endif
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
    GetWindow()->Hide();
    impl_clear();
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = nullptr;
    pTreeBox.disposeAndClear();
    delete pIdle;
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
    aFmtLb.disposeAndClear();
    aFilterLb.disposeAndClear();
}

namespace SfxTemplate
{
    sal_uInt16 SfxFamilyIdToNId(SfxStyleFamily nFamily)
    {
        switch ( nFamily )
        {
            case SfxStyleFamily::Char:   return 1;
            case SfxStyleFamily::Para:   return 2;
            case SfxStyleFamily::Frame:  return 3;
            case SfxStyleFamily::Page:   return 4;
            case SfxStyleFamily::Pseudo: return 5;
            case SfxStyleFamily::Table:  return 6;
            default:                      return 0;
        }
    }

    SfxStyleFamily NIdToSfxFamilyId(sal_uInt16 nId)
    {
        switch (nId)
        {
            case 1: return SfxStyleFamily::Char;
            case 2: return SfxStyleFamily::Para;
            case 3: return SfxStyleFamily::Frame;
            case 4: return SfxStyleFamily::Page;
            case 5: return SfxStyleFamily::Pseudo;
            case 6: return SfxStyleFamily::Table;
            default: return SfxStyleFamily::All;
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
    return nullptr;
}

void SfxCommonTemplateDialog_Impl::GetSelectedStyle() const
{
    if (!IsInitialized() || !pStyleSheetPool || !HasSelectedStyle())
        return;
    const OUString aTemplName( GetSelectedEntry() );
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );
}

/**
 * Is it safe to show the water-can / fill icon. If we've a
 * hierarchical widget - we have only single select, otherwise
 * we need to check if we have a multi-selection. We either have
 * a pTreeBox showing or an aFmtLb (which we hide when not shown)
 */
bool SfxCommonTemplateDialog_Impl::IsSafeForWaterCan() const
{
    if ( pTreeBox.get() != nullptr )
        return pTreeBox->FirstSelected() != nullptr;
    else
        return aFmtLb->GetSelectionCount() == 1;
}

void SfxCommonTemplateDialog_Impl::SelectStyle(const OUString &rStr)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if ( !pItem )
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( rStr, eFam );
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
            SvTreeListEntry* pEntry = aFmtLb->FirstVisible();
            while ( pEntry && aFmtLb->GetEntryText( pEntry ) != rStr )
                pEntry = aFmtLb->NextVisible( pEntry );
            if ( !pEntry )
                bSelect = false;
            else
            {
                if (!aFmtLb->IsSelected(pEntry))
                {
                    aFmtLb->MakeVisible( pEntry );
                    aFmtLb->SelectAll(false);
                    aFmtLb->Select( pEntry );
                    bWaterDisabled = !IsSafeForWaterCan();
                    FmtSelectHdl( nullptr );
                }
            }
        }

        if ( !bSelect )
        {
            aFmtLb->SelectAll( false );
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
        SvTreeListEntry* pEntry = aFmtLb->FirstSelected();
        if ( pEntry )
            aRet = aFmtLb->GetEntryText( pEntry );
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
                pTreeBox->SetDragDropMode(DragDropMode::CTRL_MOVE);
            else
                pTreeBox->SetDragDropMode(DragDropMode::NONE);
        }
    }
    bTreeDrag = bEnable;
}

void SfxCommonTemplateDialog_Impl::FillTreeBox()
{
    OSL_ENSURE( pTreeBox, "FillTreeBox() without treebox");
    if (pStyleSheetPool && nActFamily != 0xffff)
    {
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        if (!pItem)
            return;
        pStyleSheetPool->SetSearchMask(pItem->GetFamily(), SFXSTYLEBIT_ALL_VISIBLE);
        StyleTreeArr_Impl aArr;
        SfxStyleSheetBase* pStyle = pStyleSheetPool->First();

        if(pStyle && pStyle->HasParentSupport() && bTreeDrag )
            pTreeBox->SetDragDropMode(DragDropMode::CTRL_MOVE);
        else
            pTreeBox->SetDragDropMode(DragDropMode::NONE);

        while (pStyle)
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

        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            FillBox_Impl(pTreeBox, aArr[i], aEntries, pItem->GetFamily());
        }
        pTreeBox->Recalc();

        EnableItem(SID_STYLE_WATERCAN, false);

        SfxTemplateItem* pState = pFamilyState[nActFamily - 1];

        if (nCount)
            pTreeBox->Expand(pTreeBox->First());

        for (SvTreeListEntry* pEntry = pTreeBox->First(); pEntry; pEntry = pTreeBox->Next(pEntry))
        {
            if (IsExpanded_Impl(aEntries, pTreeBox->GetEntryText(pEntry)))
                pTreeBox->Expand(pEntry);
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
    return pTreeBox? pTreeBox->FirstSelected() != nullptr:
            aFmtLb->GetSelectionCount() != 0;
}

// internal: Refresh the display
// nFlags: what we should update.
void SfxCommonTemplateDialog_Impl::UpdateStyles_Impl(StyleFlags nFlags)
{
    OSL_ENSURE(nFlags != StyleFlags::NONE, "nothing to do");
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

    SfxFilterTupel* pT = ( nActFilter < pItem->GetFilterList().size() ? pItem->GetFilterList()[nActFilter] : nullptr );
    sal_uInt16 nFilter     = pT ? pT->nFlags : 0;
    if(!nFilter)   // automatic
        nFilter = nAppFilter;

    OSL_ENSURE(pStyleSheetPool, "no StyleSheetPool");
    if(pStyleSheetPool)
    {
        pStyleSheetPool->SetSearchMask(eFam, nFilter);
        pItem = GetFamilyItem_Impl();
        if(nFlags & StyleFlags::UpdateFamily)   // Update view type list (Hierarchical, All, etc.
        {
            CheckItem(nActFamily);    // check Button in Toolbox
            aFilterLb->SetUpdateMode(false);
            aFilterLb->Clear();
            //insert hierarchical at the beginning
            sal_Int32 nPos = aFilterLb->InsertEntry(SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString(), 0);
            aFilterLb->SetEntryData( nPos, reinterpret_cast<void*>(SFXSTYLEBIT_ALL) );
            const SfxStyleFilter& rFilter = pItem->GetFilterList();
            for(const SfxFilterTupel* i : rFilter)
            {
                sal_uIntPtr nFilterFlags = i->nFlags;
                nPos = aFilterLb->InsertEntry( i->aName );
                aFilterLb->SetEntryData( nPos, reinterpret_cast<void*>(nFilterFlags) );
            }
            if(nActFilter < aFilterLb->GetEntryCount() - 1)
                aFilterLb->SelectEntryPos(nActFilter + 1);
            else
            {
                nActFilter = 0;
                aFilterLb->SelectEntryPos(1);
                SfxFilterTupel* pActT = ( nActFilter < rFilter.size() ) ? rFilter[ nActFilter ] : nullptr;
                sal_uInt16 nFilterFlags = pActT ? pActT->nFlags : 0;
                pStyleSheetPool->SetSearchMask(eFam, nFilterFlags);
            }

            // if the tree view again, select family hierarchy
            if (pTreeBox || m_bWantHierarchical)
            {
                aFilterLb->SelectEntry(SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString());
                EnableHierarchical(true);
            }

            // show maximum 14 entries
            aFilterLb->SetDropDownLineCount( MAX_FILTER_ENTRIES );
            aFilterLb->SetUpdateMode(true);
        }
        else
        {
            if (nActFilter < aFilterLb->GetEntryCount() - 1)
                aFilterLb->SelectEntryPos(nActFilter + 1);
            else
            {
                nActFilter = 0;
                aFilterLb->SelectEntryPos(1);
            }
        }

        if(nFlags & StyleFlags::UpdateFamilyList)
        {
            EnableItem(SID_STYLE_WATERCAN,false);

            SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
            SvTreeListEntry* pEntry = aFmtLb->First();
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
                  aStrings[nPos] == aFmtLb->GetEntryText(pEntry))
            {
                ++nPos;
                pEntry = aFmtLb->Next( pEntry );
            }

            if( nPos < nCount || pEntry )
            {
                // Fills the display box
                aFmtLb->SetUpdateMode(false);
                aFmtLb->Clear();

                for(nPos = 0; nPos < nCount; ++nPos)
                {
                    SvTreeListEntry* pTreeListEntry = aFmtLb->InsertEntry(aStrings[nPos], nullptr, false, nPos);
                    if (officecfg::Office::Common::StylesAndFormatting::Preview::get())
                    {
                        pTreeListEntry->ReplaceItem(o3tl::make_unique<StyleLBoxString>(aStrings[nPos], eFam), 1);
                    }
                    aFmtLb->GetModel()->InvalidateEntry(pTreeListEntry);
                }
                aFmtLb->Recalc();
                aFmtLb->SetUpdateMode(true);
            }
            // Selects the current style if any
            SfxTemplateItem *pState = pFamilyState[nActFamily-1];
            OUString aStyle;
            if(pState)
                aStyle = pState->GetStyleName();
#if defined STYLESPREVIEW
            mbIgnoreSelect = true; // in case we get a selection change
            // in any case we should stop any preview
            Execute_Impl(SID_STYLE_END_PREVIEW,
            OUString(), OUString(),
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
    bWaterDisabled = (pItem == nullptr);

    if(!bWaterDisabled)
        //make sure the watercan is only activated when there is (only) one selection
        bWaterDisabled = !IsSafeForWaterCan();

    if(pItem && !bWaterDisabled)
    {
        CheckItem(SID_STYLE_WATERCAN, pItem->GetValue());
        EnableItem( SID_STYLE_WATERCAN );
    }
    else
    {
        if(!bWaterDisabled)
            EnableItem(SID_STYLE_WATERCAN);
        else
            EnableItem(SID_STYLE_WATERCAN, false);
    }

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
    SfxStyleSheetBasePool* pNewPool = nullptr;
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
            pStyleSheetPool = nullptr;
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

     SfxTemplateItem *pItem = nullptr;
     // current region not within the allowed region or default
     if(nActFamily == 0xffff || nullptr == (pItem = pFamilyState[nActFamily-1] ) )
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
         CheckItem( nActFamily );
         nActFilter = static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pDocShell ) );
         if ( SFXSTYLEBIT_ALL == nActFilter )
            nActFilter = pDocShell->GetAutoStyleFilterIndex();

         nAppFilter = pItem->GetValue();
         if(!pTreeBox)
         {
             UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
         }
         else
             FillTreeBox();
     }
     else
     {
         // other filters for automatic
         CheckItem( nActFamily );
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
         if ( pStyleItem && 0 == pStyleItem->GetFilterList()[ nActFilter ]->nFlags
            && nAppFilter != pItem->GetValue())
         {
             nAppFilter = pItem->GetValue();
             if(!pTreeBox)
                 UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
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

IMPL_LINK_NOARG_TYPED( SfxCommonTemplateDialog_Impl, TimeOut, Idle *, void )
{
    if(!bDontUpdate)
    {
        bDontUpdate=true;
        if(!pTreeBox)
            UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
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
                        SfxStyleSheetBase *pStyle = pStyleSheetPool->Find( aStr, eFam );
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
            pStyleSheetPool=nullptr;
            break;
          }
        }
    }

    // Do not set timer when the stylesheet pool is in the box, because it is
    // possible that a new one is registered after the timer is up -
    // works bad in UpdateStyles_Impl ()!

    const sal_uInt32 nId = pSfxSimpleHint ? pSfxSimpleHint->GetId() : 0;

    if(!bDontUpdate && nId != SFX_HINT_DYING &&
       (dynamic_cast<const SfxStyleSheetPoolHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetHintExtended*>(&rHint)))
    {
        if(!pIdle)
        {
            pIdle=new Idle("SfxCommonTemplate");
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
        pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
        if ( pOldStyleSheetPool != pStyleSheetPool )
        {
            if ( pOldStyleSheetPool )
                EndListening(*pOldStyleSheetPool);
            if ( pStyleSheetPool )
                StartListening(*pStyleSheetPool);
        }

        UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
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

    pItems[ nCount++ ] = nullptr;

    DeletionWatcher aDeleted(*this);
    sal_uInt16 nModi = pModifier ? *pModifier : 0;
    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD | SfxCallMode::MODAL,
        pItems, nModi );

    // Dialog can be destroyed while in Execute() because started
    // subdialogs are not modal to it (#i97888#).
    if ( !pItem || aDeleted )
        return false;

    if ( (nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId) && (pTreeBox || aFmtLb->GetSelectionCount() <= 1) )
    {
        const SfxUInt16Item *pFilterItem = dynamic_cast< const SfxUInt16Item* >(pItem);
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
            aFmtLb->Hide();

            pTreeBox = VclPtr<StyleTreeListBox_Impl>::Create(

                    this, WB_HASBUTTONS | WB_HASLINES |
                    WB_BORDER | WB_TABSTOP | WB_HASLINESATROOT |
                    WB_HASBUTTONSATROOT | WB_HIDESELECTION | WB_QUICK_SEARCH );
            pTreeBox->SetFont( aFmtLb->GetFont() );

            pTreeBox->SetPosSizePixel(aFmtLb->GetPosPixel(), aFmtLb->GetSizePixel());
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
        pTreeBox.disposeAndClear();
        aFmtLb->Show();
        // If bHierarchical, then the family can have changed
        // minus one since hierarchical is inserted at the start
        m_bWantHierarchical = false; // before FilterSelect
        FilterSelect(aFilterLb->GetSelectEntryPos() - 1, bHierarchical );
        bHierarchical=false;
    }
}

IMPL_LINK_TYPED( SfxCommonTemplateDialog_Impl, FilterSelectHdl, ListBox&, rBox, void )
{
    if (SfxResId(STR_STYLE_FILTER_HIERARCHICAL).toString() == rBox.GetSelectEntry())
    {
        EnableHierarchical(true);
    }
    else
    {
        EnableHierarchical(false);
    }
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
        pDispat->ExecuteList(SID_STYLE_FAMILY, SfxCallMode::SYNCHRON, { &aItem });
        pBindings->Invalidate( SID_STYLE_FAMILY );
        pBindings->Update( SID_STYLE_FAMILY );
        UpdateFamily_Impl();
    }
}

void SfxCommonTemplateDialog_Impl::ActionSelect(sal_uInt16 nEntry)
{
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
                    SID_STYLE_WATERCAN, aTemplName, "",
                    (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );
                bCheck = true;
            }
            else
            {
                Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
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

                ScopedVclPtrInstance< SfxNewStyleDlg > pDlg(pWindow, *pStyleSheetPool);
                // why? : FloatingWindow must not be parent of a modal dialog
                if(RET_OK == pDlg->Execute())
                {
                    pStyleSheetPool->SetSearchMask(eFam, nFilter);
                    const OUString aTemplName(pDlg->GetName());
                    Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE,
                                 aTemplName, "",
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
                    "", "",
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
    catch ( css::frame::UnknownModuleException& )
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

    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
    sal_Int32 nDefault = -1;
    sal_Int32 nFilter = aFactoryProps.getUnpackedValueOrDefault( "ooSetupFactoryStyleFilter", nDefault );

    m_bWantHierarchical =
        (nFilter & SFXSTYLEBIT_HIERARCHY) != 0;
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

IMPL_LINK_TYPED( SfxCommonTemplateDialog_Impl, DropHdl, StyleTreeListBox_Impl&, rBox, bool )
{
    bDontUpdate = true;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    bool ret = pStyleSheetPool->SetParent(eFam, rBox.GetStyle(), rBox.GetParent());
    bDontUpdate = false;
    return ret;
}

// Handler for the New-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl()
{
    if ( nActFamily != 0xffff && (pTreeBox || aFmtLb->GetSelectionCount() <= 1))
    {
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
                     "", GetSelectedEntry(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     nMask);
    }
}

// Handler for the edit-Buttons
void SfxCommonTemplateDialog_Impl::EditHdl()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        sal_uInt16 nFilter = nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        if ( Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),
                          (sal_uInt16)GetFamilyItem_Impl()->GetFamily(), 0, &nFilter ) )
        {
        }
    }
}

// Handler for the Delete-Buttons
void SfxCommonTemplateDialog_Impl::DeleteHdl()
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        bool bUsedStyle = false;     // one of the selected styles are used in the document?

        std::vector<SvTreeListEntry*> aList;
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb->FirstSelected();
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

        OUString aMsg = SfxResId(STR_DELETE_STYLE_USED).toString();
        aMsg += SfxResId(STR_DELETE_STYLE).toString();

        while (pEntry)
        {
            aList.push_back( pEntry );
            // check the style is used or not
            const OUString aTemplName(pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb->GetEntryText(pEntry));

            SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );

            if ( pStyle->IsUsed() )  // pStyle is in use in the document?
            {
                if (bUsedStyle) // add a separator for the second and later styles
                    aMsg += ", ";
                aMsg += aTemplName;
                bUsedStyle = true;
            }

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb->NextSelected(pEntry);
        }

        bool aApproved = false;

        // we only want to show the dialog once and if we want to delete a style in use (UX-advice)
        if ( bUsedStyle )
        {
        #if defined UNX
            ScopedVclPtrInstance<MessageDialog> aBox(SfxGetpApp()->GetTopWindow(), aMsg,
                               VclMessageType::Question, VCL_BUTTONS_YES_NO);
        #else
            ScopedVclPtrInstance<MessageDialog> aBox(GetWindow(), aMsg,
                               VclMessageType::Question, VCL_BUTTONS_YES_NO);
        #endif
            aApproved = aBox->Execute() == RET_YES;
        }

        // if there are no used styles selected or the user approved the changes
        if ( !bUsedStyle || aApproved )
        {
            std::vector<SvTreeListEntry*>::const_iterator it = aList.begin(), itEnd = aList.end();

            for (; it != itEnd; ++it)
            {
                const OUString aTemplName(pTreeBox ? pTreeBox->GetEntryText(*it) : aFmtLb->GetEntryText(*it));
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
            UpdateStyles_Impl(StyleFlags::UpdateFamilyList);; //and force-update the list
        }
    }
}

void SfxCommonTemplateDialog_Impl::HideHdl()
{
    if ( IsInitialized() && HasSelectedStyle() )
    {
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb->FirstSelected();

        while (pEntry)
        {
            OUString aTemplName = pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb->GetEntryText(pEntry);

            Execute_Impl( SID_STYLE_HIDE, aTemplName,
                          OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb->NextSelected(pEntry);
        }
    }
}

void SfxCommonTemplateDialog_Impl::ShowHdl()
{

    if ( IsInitialized() && HasSelectedStyle() )
    {
        SvTreeListEntry* pEntry = pTreeBox ? pTreeBox->FirstSelected() : aFmtLb->FirstSelected();

        while (pEntry)
        {
            OUString aTemplName = pTreeBox ? pTreeBox->GetEntryText(pEntry) : aFmtLb->GetEntryText(pEntry);

            Execute_Impl( SID_STYLE_SHOW, aTemplName,
                          OUString(), (sal_uInt16)GetFamilyItem_Impl()->GetFamily() );

            pEntry = pTreeBox ? pTreeBox->NextSelected(pEntry) : aFmtLb->NextSelected(pEntry);
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
            EnableDel();
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
    if ( dynamic_cast< const SfxTemplateDialog_Impl* >(this) !=  nullptr )
    {
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell *pVu = pViewFrame->GetViewShell();
        vcl::Window *pAppWin = pVu ? pVu->GetWindow(): nullptr;
        if(pAppWin)
            pAppWin->GrabFocus();
    }
}

IMPL_LINK_NOARG_TYPED( SfxCommonTemplateDialog_Impl, TreeListApplyHdl, SvTreeListBox *, bool )
{
    ApplyHdl(nullptr);
    return false;
}

// Double-click on a style sheet in the ListBox is applied.
IMPL_LINK_NOARG_TYPED( SfxCommonTemplateDialog_Impl, ApplyHdl, LinkParamNone*, void )
{
    // only if that region is allowed
    if ( IsInitialized() && nullptr != pFamilyState[nActFamily-1] &&
         !GetSelectedEntry().isEmpty() )
    {
        sal_uInt16 nModifier = aFmtLb->GetModifier();
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), OUString(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     0, nullptr, &nModifier );
    }
    ResetFocus();
}

// Selection of a template during the Watercan-Status
IMPL_LINK_TYPED( SfxCommonTemplateDialog_Impl, FmtSelectHdl, SvTreeListBox *, pListBox, void )
{
    // Trigger Help PI, if this is permitted of call handlers and field
    if( !pListBox || pListBox->IsSelected( pListBox->GetHdlEntry() ) )
    {
        // Only when the watercan is on
        if ( IsInitialized() &&
             IsCheckedItem(SID_STYLE_WATERCAN) &&
             // only if that region is allowed
             nullptr != pFamilyState[nActFamily-1] && (pTreeBox || aFmtLb->GetSelectionCount() <= 1) )
        {
            Execute_Impl(SID_STYLE_WATERCAN,
                         "", "", 0);
            Execute_Impl(SID_STYLE_WATERCAN,
                         GetSelectedEntry(), "",
                         ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily());
        }
        EnableItem(SID_STYLE_WATERCAN, !bWaterDisabled);
        EnableDelete();
    }
    if( pListBox )
    {
        SelectStyle( pListBox->GetEntryText( pListBox->GetHdlEntry() ));
#if defined STYLESPREVIEW
        sal_uInt16 nModifier = aFmtLb->GetModifier();
        if ( mbIgnoreSelect )
        {
            Execute_Impl(SID_STYLE_END_PREVIEW,
            OUString(), OUString(),
            0, 0, 0, 0 );
            mbIgnoreSelect = false;
        }
        else
        {
            Execute_Impl(SID_STYLE_PREVIEW,
                     GetSelectedEntry(), OUString(),
                     ( sal_uInt16 )GetFamilyItem_Impl()->GetFamily(),
                     0, 0, &nModifier );
        }
#endif
    }
}

IMPL_LINK_TYPED( SfxCommonTemplateDialog_Impl, MenuSelectHdl, Menu*, pMenu, bool )
{
    nLastItemId = pMenu->GetCurItemId();
    Application::PostUserEvent(
        LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectAsyncHdl ) );
    return true;
}

IMPL_LINK_NOARG_TYPED( SfxCommonTemplateDialog_Impl, MenuSelectAsyncHdl, void*, void )
{
    switch(nLastItemId) {
    case ID_NEW: NewHdl(); break;
    case ID_EDIT: EditHdl(); break;
    case ID_DELETE: DeleteHdl(); break;
    case ID_HIDE: HideHdl(); break;
    case ID_SHOW: ShowHdl(); break;
    }
}

SfxStyleFamily SfxCommonTemplateDialog_Impl::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    bool bDisable = !bEnable || !IsSafeForWaterCan();
    if( nId == SID_STYLE_NEW_BY_EXAMPLE )
        bNewByExampleDisabled = bDisable;
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
        bUpdateByExampleDisabled = bDisable;

    EnableItem(nId, bEnable);
}

VclPtr<PopupMenu> SfxCommonTemplateDialog_Impl::CreateContextMenu()
{
    if ( bBindingUpdate )
    {
        pBindings->Invalidate( SID_STYLE_NEW, true );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = false;
    }
    VclPtr<PopupMenu> pMenu = VclPtr<PopupMenu>::Create( SfxResId( MN_CONTEXT_TEMPLDLG ) );
    pMenu->SetSelectHdl( LINK( this, SfxCommonTemplateDialog_Impl, MenuSelectHdl ) );
    pMenu->EnableItem( ID_EDIT, bCanEdit );
    pMenu->EnableItem( ID_DELETE, bCanDel );
    pMenu->EnableItem( ID_NEW, bCanNew );
    pMenu->EnableItem( ID_HIDE, bCanHide );
    pMenu->EnableItem( ID_SHOW, bCanShow );

    return pMenu;
}

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

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(SfxBindings* pB, SfxTemplatePanelControl* pDlgWindow)
    : SfxCommonTemplateDialog_Impl(pB, pDlgWindow, true)
    , m_pFloat(pDlgWindow)
    , m_aActionTbL(VclPtrInstance<DropToolBox_Impl>(pDlgWindow, this))
    , m_aActionTbR(VclPtrInstance<ToolBox>(pDlgWindow))
{
    try
    {
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
        if (xUICommands.is())
        {
            uno::Any aCommand = xUICommands->getByName(".uno:StyleApply");
            OUString sLabel = lcl_GetLabel( aCommand );
            m_aActionTbR->InsertItem( SID_STYLE_WATERCAN, sLabel );
            m_aActionTbR->SetHelpId(SID_STYLE_WATERCAN, HID_TEMPLDLG_WATERCAN);

            aCommand = xUICommands->getByName(".uno:StyleNewByExample");
            sLabel = lcl_GetLabel( aCommand );
            m_aActionTbR->InsertItem( SID_STYLE_NEW_BY_EXAMPLE, sLabel );
            m_aActionTbR->SetHelpId(SID_STYLE_NEW_BY_EXAMPLE, HID_TEMPLDLG_NEWBYEXAMPLE);

            aCommand = xUICommands->getByName(".uno:StyleUpdateByExample");
            sLabel = lcl_GetLabel( aCommand );
            m_aActionTbR->InsertItem( SID_STYLE_UPDATE_BY_EXAMPLE, sLabel );
            m_aActionTbR->SetHelpId(SID_STYLE_UPDATE_BY_EXAMPLE, HID_TEMPLDLG_UPDATEBYEXAMPLE);
        }
    }
    catch (const uno::Exception&)
    {
    }

    Initialize();
}

void SfxTemplateDialog_Impl::Initialize()
{
    SfxCommonTemplateDialog_Impl::Initialize();

    m_aActionTbL->SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    m_aActionTbR->SetSelectHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_aActionTbR->SetDropdownClickHdl(LINK(this, SfxTemplateDialog_Impl, ToolBoxRClick));
    m_aActionTbL->Show();
    m_aActionTbR->Show();
    vcl::Font aFont = aFilterLb->GetFont();
    aFont.SetWeight( WEIGHT_NORMAL );
    aFilterLb->SetFont( aFont );
    m_aActionTbL->SetHelpId( HID_TEMPLDLG_TOOLBOX_LEFT );
}

void SfxTemplateDialog_Impl::EnableFamilyItem( sal_uInt16 nId, bool bEnable )
{
    m_aActionTbL->EnableItem( nId, bEnable );
}

// Insert element into dropdown filter "Frame Styles", "List Styles", etc.
void SfxTemplateDialog_Impl::InsertFamilyItem(sal_uInt16 nId,const SfxStyleFamilyItem *pItem)
{
    OString sHelpId;
    switch( pItem->GetFamily() )
    {
        case SfxStyleFamily::Char:     sHelpId = ".uno:CharStyle"; break;
        case SfxStyleFamily::Para:     sHelpId = ".uno:ParaStyle"; break;
        case SfxStyleFamily::Frame:    sHelpId = ".uno:FrameStyle"; break;
        case SfxStyleFamily::Page:     sHelpId = ".uno:PageStyle"; break;
        case SfxStyleFamily::Pseudo:   sHelpId = ".uno:ListStyle"; break;
        case SfxStyleFamily::Table:    sHelpId = ".uno:TableStyle"; break;
        default: OSL_FAIL("unknown StyleFamily"); break;
    }
    m_aActionTbL->InsertItem( nId, pItem->GetImage(), pItem->GetText(), ToolBoxItemBits::NONE, 0);
    m_aActionTbL->SetHelpId( nId, sHelpId );
}

void SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    m_aActionTbR->HideItem(SID_STYLE_UPDATE_BY_EXAMPLE);
    m_aActionTbR->SetItemBits( SID_STYLE_NEW_BY_EXAMPLE,
            ToolBoxItemBits::DROPDOWNONLY|m_aActionTbR->GetItemBits( SID_STYLE_NEW_BY_EXAMPLE ));
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
        m_aActionTbL->SetItemImage( nId, pItem->GetImage() );
    }
}

void SfxTemplateDialog_Impl::updateNonFamilyImages()
{
    m_aActionTbR->SetImageList( ImageList( SfxResId( DLG_STYLE_DESIGNER ) ) );
}

void SfxTemplateDialog_Impl::ClearFamilyList()
{
    m_aActionTbL->Clear();
}

void SfxCommonTemplateDialog_Impl::InvalidateBindings()
{
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, true);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, true);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, true);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, true);
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, true);
    pBindings->Update( SID_STYLE_DRAGHIERARCHIE );
}

SfxTemplateDialog_Impl::~SfxTemplateDialog_Impl()
{
    m_pFloat.clear();
    m_aActionTbL.disposeAndClear();
    m_aActionTbR.disposeAndClear();
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
    if (m_pFloat == nullptr)
        return;
    Size aDlgSize=m_pFloat->PixelToLogic(m_pFloat->GetOutputSizePixel());
    Size aSizeATL=m_pFloat->PixelToLogic(m_aActionTbL->CalcWindowSizePixel());
    Size aSizeATR=m_pFloat->PixelToLogic(m_aActionTbR->CalcWindowSizePixel());
    Size aMinSize = GetMinOutputSizePixel();

    long nListHeight = m_pFloat->PixelToLogic( aFilterLb->GetSizePixel() ).Height();
    long nWidth = aDlgSize.Width()- 2 * SFX_TEMPLDLG_HFRAME;

    m_aActionTbL->SetPosSizePixel(m_pFloat->LogicToPixel(Point(SFX_TEMPLDLG_HFRAME,SFX_TEMPLDLG_VTOPFRAME)),
                                 m_pFloat->LogicToPixel(aSizeATL));

    // only change the position of the right toolbox, when the window is wide
    // enough
    Point aPosATR(aDlgSize.Width()-SFX_TEMPLDLG_HFRAME-aSizeATR.Width(),SFX_TEMPLDLG_VTOPFRAME);
    if(aDlgSize.Width() >= aMinSize.Width())
        m_aActionTbR->SetPosPixel(m_pFloat->LogicToPixel(aPosATR));
    else
        m_aActionTbR->SetPosPixel( m_pFloat->LogicToPixel(
            Point( SFX_TEMPLDLG_HFRAME + aSizeATL.Width() + SFX_TEMPLDLG_MIDHSPACE,
                   SFX_TEMPLDLG_VTOPFRAME ) ) );

    m_aActionTbR->SetSizePixel(m_pFloat->LogicToPixel(aSizeATR));

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
        aFilterLb->SetPosPixel(aFilterPos);
        aFmtLb->SetPosPixel( aFmtPos );
        if(pTreeBox)
            pTreeBox->SetPosPixel(aFmtPos);
    }
    else
        aFmtSize.Height() += aFilterSize.Height();

    aFilterLb->SetSizePixel(aFilterSize);
    aFmtLb->SetSizePixel( aFmtSize );
    if(pTreeBox)
        pTreeBox->SetSizePixel(aFmtSize);
}

Size SfxTemplateDialog_Impl::GetMinOutputSizePixel()
{
    if (m_pFloat != nullptr)
    {
        Size aSizeATL=m_pFloat->PixelToLogic(m_aActionTbL->CalcWindowSizePixel());
        Size aSizeATR=m_pFloat->PixelToLogic(m_aActionTbR->CalcWindowSizePixel());
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
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            if(!bCheck && IsCheckedItem(SID_STYLE_WATERCAN))
                Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
            SAL_FALLTHROUGH;
        case SID_STYLE_NEW_BY_EXAMPLE:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
            m_aActionTbR->EnableItem(nMesId,bCheck);
            break;
    }
}

void SfxTemplateDialog_Impl::CheckItem(sal_uInt16 nMesId, bool bCheck)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            bIsWater=bCheck;
            m_aActionTbR->CheckItem(SID_STYLE_WATERCAN,bCheck);
            break;
        default:
            m_aActionTbL->CheckItem(nMesId,bCheck); break;
    }
}

bool SfxTemplateDialog_Impl::IsCheckedItem(sal_uInt16 nMesId)
{
    switch(nMesId)
    {
        case SID_STYLE_WATERCAN :
            return m_aActionTbR->GetItemState(SID_STYLE_WATERCAN)==TRISTATE_TRUE;
        default:
            return m_aActionTbL->GetItemState(nMesId)==TRISTATE_TRUE;
    }
}

IMPL_LINK_TYPED( SfxTemplateDialog_Impl, ToolBoxLSelect, ToolBox *, pBox, void )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    FamilySelect(nEntry);
}

IMPL_LINK_TYPED( SfxTemplateDialog_Impl, ToolBoxRSelect, ToolBox *, pBox, void )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry != SID_STYLE_NEW_BY_EXAMPLE ||
            ToolBoxItemBits::DROPDOWN != (pBox->GetItemBits(nEntry)&ToolBoxItemBits::DROPDOWN))
        ActionSelect(nEntry);
}

IMPL_LINK_TYPED( SfxTemplateDialog_Impl, ToolBoxRClick, ToolBox *, pBox, void )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();
    if(nEntry == SID_STYLE_NEW_BY_EXAMPLE &&
            ToolBoxItemBits::DROPDOWN == (pBox->GetItemBits(nEntry)&ToolBoxItemBits::DROPDOWN))
    {
        //create a popup menu in Writer
        ScopedVclPtrInstance<PopupMenu> pMenu;
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
            return;
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
                            PopupMenuFlags::ExecuteDown );
            pBox->EndSelection();
        }
        catch (const uno::Exception&)
        {
        }
        pBox->Invalidate();
    }
}

IMPL_LINK_TYPED( SfxTemplateDialog_Impl, MenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    ActionSelect(nMenuId);
    return false;
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
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            StartListening(*pStyleSheetPool);
    }

    bWaterDisabled = false;
    bCanNew = pTreeBox || aFmtLb->GetSelectionCount() <= 1;
    bTreeDrag = true;
    bUpdateByExampleDisabled = false;

    if (pStyleSheetPool)
    {
        if (!pTreeBox)
            UpdateStyles_Impl(StyleFlags::UpdateFamily | StyleFlags::UpdateFamilyList);
        else
        {
            UpdateStyles_Impl(StyleFlags::UpdateFamily);
            FillTreeBox();
        }
    }

    InvalidateBindings();

    if (IsCheckedItem(SID_STYLE_WATERCAN) &&
         // only if that area is allowed
         nullptr != pFamilyState[nActFamily - 1])
    {
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(),
                     OUString(),
                     static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));
    }
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
    if ( nItemId != SfxTemplate::SfxFamilyIdToNId( SfxStyleFamily::Page )&&
        IsDropFormatSupported( SotClipboardFormatId::OBJECTDESCRIPTOR ) &&
        !rParent.bNewByExampleDisabled )
    {
        nReturn = DND_ACTION_COPY;
    }
    return nReturn;
}

sal_Int8 DropToolBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
     return rParent.aFmtLb->ExecuteDrop(rEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
