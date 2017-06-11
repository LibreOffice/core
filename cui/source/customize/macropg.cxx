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

#include <basic/basmgr.hxx>

#include "macropg.hxx"
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
#include <svl/eitem.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <dialmgr.hxx>
#include "cfgutil.hxx"
#include "cfg.hxx"
#include "helpid.hrc"
#include <cuires.hrc>
#include "headertablistbox.hxx"
#include "macropg_impl.hxx"
#include <svx/dialogs.hrc>
#include "strings.hrc"
#include <vcl/builderfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <o3tl/make_unique.hxx>

#include <algorithm>
#include <iterator>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static const char aVndSunStarUNO[] = "vnd.sun.star.UNO:";

SvxMacroTabPage_Impl::SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet )
    : pAssignPB(nullptr)
    , pAssignComponentPB(nullptr)
    , pDeletePB(nullptr)
    , pEventLB(nullptr)
    , bReadOnly(false)
    , bIDEDialogMode(false)
{
    const SfxPoolItem* pItem;
    if ( SfxItemState::SET == rAttrSet.GetItemState( SID_ATTR_MACROITEM, false, &pItem ) )
        bIDEDialogMode = static_cast<const SfxBoolItem*>(pItem)->GetValue();
}

// attention, this array is indexed directly (0, 1, ...) in the code
static long nTabs[] =
{
    2, // Number of Tabs
    0, 90
};

#define TAB_WIDTH_MIN        10

// IDs for items in HeaderBar of EventLB
#define    ITEMID_EVENT        1
#define    ITMEID_ASSMACRO        2


#define LB_MACROS_ITEMPOS    2


IMPL_LINK( MacroEventListBox, HeaderEndDrag_Impl, HeaderBar*, pBar, void )
{
    DBG_ASSERT( pBar == maHeaderBar.get(), "*MacroEventListBox::HeaderEndDrag_Impl: something is wrong here..." );
    (void)pBar;

    if( !maHeaderBar->GetCurItemId() )
        return;

    if( !maHeaderBar->IsItemMode() )
    {
        Size    aSz;
        sal_uInt16    _nTabs = maHeaderBar->GetItemCount();
        long    nWidth = maHeaderBar->GetItemSize( ITEMID_EVENT );
        long    nBarWidth = maHeaderBar->GetSizePixel().Width();

        if( nWidth < TAB_WIDTH_MIN )
            maHeaderBar->SetItemSize( ITEMID_EVENT, TAB_WIDTH_MIN );
        else if( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            maHeaderBar->SetItemSize( ITEMID_EVENT, nBarWidth - TAB_WIDTH_MIN );

        {
            long nTmpSz = 0;
            for( sal_uInt16 i = 1 ; i < _nTabs ; ++i )
            {
                long _nWidth = maHeaderBar->GetItemSize( i );
                aSz.Width() =  _nWidth + nTmpSz;
                nTmpSz += _nWidth;
                maListBox->SetTab( i, PixelToLogic( aSz, MapMode( MapUnit::MapAppFont ) ).Width() );
            }
        }
    }
}

bool MacroEventListBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bRet = Control::EventNotify(rNEvt);

    if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
    {
        if ( rNEvt.GetWindow() != maListBox.get() )
            if (maListBox)
                maListBox->GrabFocus();
    }

    return bRet;
}

MacroEventListBox::MacroEventListBox( vcl::Window* pParent, WinBits nStyle )
    : Control( pParent, nStyle )
    , maHeaderBar( VclPtr<HeaderBar>::Create( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER ) )
    , maListBox( VclPtr<SvHeaderTabListBox>::Create( this, WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP ) )
{
    maListBox->SetHelpId( HID_MACRO_HEADERTABLISTBOX );

    // enable the cell focus to show visible focus
    maListBox->EnableCellFocus();
}

MacroEventListBox::~MacroEventListBox()
{
    disposeOnce();
}

void MacroEventListBox::dispose()
{
    maHeaderBar.disposeAndClear();
    maListBox.disposeAndClear();
    Control::dispose();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(MacroEventListBox, WB_TABSTOP)

Size MacroEventListBox::GetOptimalSize() const
{
    return LogicToPixel(Size(192, 72), MapMode(MapUnit::MapAppFont ));
}

void MacroEventListBox::Resize()
{
    Control::Resize();

    // calc pos and size of header bar
    Point    aPnt( 0, 0 );
    Size    aSize( maHeaderBar->CalcWindowSizePixel() );
    Size    aCtrlSize( GetOutputSizePixel() );
    aSize.Width() = aCtrlSize.Width();
    maHeaderBar->SetPosSizePixel( aPnt, aSize );

    // calc pos and size of ListBox
    aPnt.Y() += aSize.Height();
    aSize.Height() = aCtrlSize.Height() - aSize.Height();
    maListBox->SetPosSizePixel( aPnt, aSize );
}

void MacroEventListBox::ConnectElements()
{
    Resize();

    // set handler
    maHeaderBar->SetEndDragHdl( LINK( this, MacroEventListBox, HeaderEndDrag_Impl ) );

    maListBox->InitHeaderBar( maHeaderBar.get() );
}

void MacroEventListBox::Show()
{
    maListBox->Show();
    maHeaderBar->Show();
}

void MacroEventListBox::Enable()
{
    maListBox->Enable();
    maHeaderBar->Enable();
}

// assign button ("Add Command") is enabled only if it is not read only
// delete button ("Remove Command") is enabled if a current binding exists
//     and it is not read only
void SvxMacroTabPage_::EnableButtons()
{
    const SvTreeListEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        mpImpl->pDeletePB->Enable( !mpImpl->bReadOnly );

        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly );
        if( mpImpl->pAssignComponentPB )
            mpImpl->pAssignComponentPB->Enable( !mpImpl->bReadOnly );
    }
}

SvxMacroTabPage_::SvxMacroTabPage_(vcl::Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription, const SfxItemSet& rAttrSet)
    : SfxTabPage( pParent, rID, rUIXMLDescription, &rAttrSet ),
    m_xAppEvents(nullptr),
    m_xDocEvents(nullptr),
    bDocModified(false),
    bAppEvents(false),
    bInitialized(false)
{
    mpImpl = new SvxMacroTabPage_Impl( rAttrSet );
}

SvxMacroTabPage_::~SvxMacroTabPage_()
{
    disposeOnce();
}

void SvxMacroTabPage_::dispose()
{
    DELETEZ( mpImpl );
    SfxTabPage::dispose();
}

void SvxMacroTabPage_::InitResources()
{
    // Note: the order here controls the order in which the events are displayed in the UI!

    // the event name to UI string mappings for App Events
    aDisplayNames.push_back( EventDisplayName( "OnStartApp",            RID_SVXSTR_EVENT_STARTAPP ) );
    aDisplayNames.push_back( EventDisplayName( "OnCloseApp",            RID_SVXSTR_EVENT_CLOSEAPP ) );
    aDisplayNames.push_back( EventDisplayName( "OnCreate",              RID_SVXSTR_EVENT_CREATEDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnNew",                 RID_SVXSTR_EVENT_NEWDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnLoadFinished",        RID_SVXSTR_EVENT_LOADDOCFINISHED ) );
    aDisplayNames.push_back( EventDisplayName( "OnLoad",                RID_SVXSTR_EVENT_OPENDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnPrepareUnload",       RID_SVXSTR_EVENT_PREPARECLOSEDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnUnload",              RID_SVXSTR_EVENT_CLOSEDOC ) ) ;
    aDisplayNames.push_back( EventDisplayName( "OnViewCreated",         RID_SVXSTR_EVENT_VIEWCREATED ) );
    aDisplayNames.push_back( EventDisplayName( "OnPrepareViewClosing",  RID_SVXSTR_EVENT_PREPARECLOSEVIEW ) );
    aDisplayNames.push_back( EventDisplayName( "OnViewClosed",          RID_SVXSTR_EVENT_CLOSEVIEW ) ) ;
    aDisplayNames.push_back( EventDisplayName( "OnFocus",               RID_SVXSTR_EVENT_ACTIVATEDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnUnfocus",             RID_SVXSTR_EVENT_DEACTIVATEDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnSave",                RID_SVXSTR_EVENT_SAVEDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnSaveDone",            RID_SVXSTR_EVENT_SAVEDOCDONE ) );
    aDisplayNames.push_back( EventDisplayName( "OnSaveFailed",          RID_SVXSTR_EVENT_SAVEDOCFAILED ) );
    aDisplayNames.push_back( EventDisplayName( "OnSaveAs",              RID_SVXSTR_EVENT_SAVEASDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnSaveAsDone",          RID_SVXSTR_EVENT_SAVEASDOCDONE ) );
    aDisplayNames.push_back( EventDisplayName( "OnSaveAsFailed",        RID_SVXSTR_EVENT_SAVEASDOCFAILED ) );
    aDisplayNames.push_back( EventDisplayName( "OnCopyTo",              RID_SVXSTR_EVENT_COPYTODOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnCopyToDone",          RID_SVXSTR_EVENT_COPYTODOCDONE ) );
    aDisplayNames.push_back( EventDisplayName( "OnCopyToFailed",        RID_SVXSTR_EVENT_COPYTODOCFAILED ) );
    aDisplayNames.push_back( EventDisplayName( "OnPrint",               RID_SVXSTR_EVENT_PRINTDOC ) );
    aDisplayNames.push_back( EventDisplayName( "OnModifyChanged",       RID_SVXSTR_EVENT_MODIFYCHANGED ) );
    aDisplayNames.push_back( EventDisplayName( "OnTitleChanged",        RID_SVXSTR_EVENT_TITLECHANGED ) );

    // application specific events
    aDisplayNames.push_back( EventDisplayName( "OnMailMerge",           RID_SVXSTR_EVENT_MAILMERGE ) );
    aDisplayNames.push_back( EventDisplayName( "OnMailMergeFinished",           RID_SVXSTR_EVENT_MAILMERGE_END ) );
    aDisplayNames.push_back( EventDisplayName( "OnFieldMerge",           RID_SVXSTR_EVENT_FIELDMERGE ) );
    aDisplayNames.push_back( EventDisplayName( "OnFieldMergeFinished",           RID_SVXSTR_EVENT_FIELDMERGE_FINISHED ) );
    aDisplayNames.push_back( EventDisplayName( "OnPageCountChange",     RID_SVXSTR_EVENT_PAGECOUNTCHANGE ) );
    aDisplayNames.push_back( EventDisplayName( "OnSubComponentOpened",  RID_SVXSTR_EVENT_SUBCOMPONENT_OPENED ) );
    aDisplayNames.push_back( EventDisplayName( "OnSubComponentClosed",  RID_SVXSTR_EVENT_SUBCOMPONENT_CLOSED ) );
    aDisplayNames.push_back( EventDisplayName( "OnSelect",              RID_SVXSTR_EVENT_SELECTIONCHANGED ) );
    aDisplayNames.push_back( EventDisplayName( "OnDoubleClick",         RID_SVXSTR_EVENT_DOUBLECLICK ) );
    aDisplayNames.push_back( EventDisplayName( "OnRightClick",          RID_SVXSTR_EVENT_RIGHTCLICK ) );
    aDisplayNames.push_back( EventDisplayName( "OnCalculate",           RID_SVXSTR_EVENT_CALCULATE ) );
    aDisplayNames.push_back( EventDisplayName( "OnChange",              RID_SVXSTR_EVENT_CONTENTCHANGED ) );

    // the event name to UI string mappings for forms & dialogs

    aDisplayNames.push_back( EventDisplayName( "approveAction",         RID_SVXSTR_EVENT_APPROVEACTIONPERFORMED ) );
    aDisplayNames.push_back( EventDisplayName( "actionPerformed",       RID_SVXSTR_EVENT_ACTIONPERFORMED ) );
    aDisplayNames.push_back( EventDisplayName( "changed",               RID_SVXSTR_EVENT_CHANGED ) );
    aDisplayNames.push_back( EventDisplayName( "textChanged",           RID_SVXSTR_EVENT_TEXTCHANGED ) );
    aDisplayNames.push_back( EventDisplayName( "itemStateChanged",      RID_SVXSTR_EVENT_ITEMSTATECHANGED ) );
    aDisplayNames.push_back( EventDisplayName( "focusGained",           RID_SVXSTR_EVENT_FOCUSGAINED ) );
    aDisplayNames.push_back( EventDisplayName( "focusLost",             RID_SVXSTR_EVENT_FOCUSLOST ) );
    aDisplayNames.push_back( EventDisplayName( "keyPressed",            RID_SVXSTR_EVENT_KEYTYPED ) );
    aDisplayNames.push_back( EventDisplayName( "keyReleased",           RID_SVXSTR_EVENT_KEYUP ) );
    aDisplayNames.push_back( EventDisplayName( "mouseEntered",          RID_SVXSTR_EVENT_MOUSEENTERED ) );
    aDisplayNames.push_back( EventDisplayName( "mouseDragged",          RID_SVXSTR_EVENT_MOUSEDRAGGED ) );
    aDisplayNames.push_back( EventDisplayName( "mouseMoved",            RID_SVXSTR_EVENT_MOUSEMOVED ) );
    aDisplayNames.push_back( EventDisplayName( "mousePressed",          RID_SVXSTR_EVENT_MOUSEPRESSED ) );
    aDisplayNames.push_back( EventDisplayName( "mouseReleased",         RID_SVXSTR_EVENT_MOUSERELEASED ) );
    aDisplayNames.push_back( EventDisplayName( "mouseExited",           RID_SVXSTR_EVENT_MOUSEEXITED ) );
    aDisplayNames.push_back( EventDisplayName( "approveReset",          RID_SVXSTR_EVENT_APPROVERESETTED ) );
    aDisplayNames.push_back( EventDisplayName( "resetted",              RID_SVXSTR_EVENT_RESETTED ) );
    aDisplayNames.push_back( EventDisplayName( "approveSubmit",         RID_SVXSTR_EVENT_SUBMITTED ) );
    aDisplayNames.push_back( EventDisplayName( "approveUpdate",         RID_SVXSTR_EVENT_BEFOREUPDATE ) );
    aDisplayNames.push_back( EventDisplayName( "updated",               RID_SVXSTR_EVENT_AFTERUPDATE ) );
    aDisplayNames.push_back( EventDisplayName( "loaded",                RID_SVXSTR_EVENT_LOADED ) );
    aDisplayNames.push_back( EventDisplayName( "reloading",             RID_SVXSTR_EVENT_RELOADING ) );
    aDisplayNames.push_back( EventDisplayName( "reloaded",              RID_SVXSTR_EVENT_RELOADED ) );
    aDisplayNames.push_back( EventDisplayName( "unloading",             RID_SVXSTR_EVENT_UNLOADING ) );
    aDisplayNames.push_back( EventDisplayName( "unloaded",              RID_SVXSTR_EVENT_UNLOADED ) );
    aDisplayNames.push_back( EventDisplayName( "confirmDelete",         RID_SVXSTR_EVENT_CONFIRMDELETE ) );
    aDisplayNames.push_back( EventDisplayName( "approveRowChange",      RID_SVXSTR_EVENT_APPROVEROWCHANGE ) );
    aDisplayNames.push_back( EventDisplayName( "rowChanged",            RID_SVXSTR_EVENT_ROWCHANGE ) );
    aDisplayNames.push_back( EventDisplayName( "approveCursorMove",     RID_SVXSTR_EVENT_POSITIONING ) );
    aDisplayNames.push_back( EventDisplayName( "cursorMoved",           RID_SVXSTR_EVENT_POSITIONED ) );
    aDisplayNames.push_back( EventDisplayName( "approveParameter",      RID_SVXSTR_EVENT_APPROVEPARAMETER ) );
    aDisplayNames.push_back( EventDisplayName( "errorOccured",          RID_SVXSTR_EVENT_ERROROCCURRED ) );
    aDisplayNames.push_back( EventDisplayName( "adjustmentValueChanged",   RID_SVXSTR_EVENT_ADJUSTMENTVALUECHANGED ) );
}

// the following method is called when the user clicks OK
// We use the contents of the hashes to replace the settings
bool SvxMacroTabPage_::FillItemSet( SfxItemSet* /*rSet*/ )
{
    try
    {
        OUString eventName;
        if( m_xAppEvents.is() )
        {
            EventsHash::iterator h_itEnd =  m_appEventsHash.end();
            EventsHash::iterator h_it = m_appEventsHash.begin();
            for ( ; h_it !=  h_itEnd; ++h_it )
            {
                eventName = h_it->first;
                try
                {
                    m_xAppEvents->replaceByName( eventName, GetPropsByName( eventName, m_appEventsHash ) );
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
        if( m_xDocEvents.is() && bDocModified )
        {
            EventsHash::iterator h_itEnd =  m_docEventsHash.end();
            EventsHash::iterator h_it = m_docEventsHash.begin();
            for ( ; h_it !=  h_itEnd; ++h_it )
            {
                eventName = h_it->first;
                try
                {
                    m_xDocEvents->replaceByName( eventName, GetPropsByName( eventName, m_docEventsHash ) );
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            // if we have a valid XModifiable (in the case of doc events)
            // call setModified(true)
            // in principle this should not be necessary (see issue ??)
            if(m_xModifiable.is())
            {
                m_xModifiable->setModified( true );
            }
        }
    }
    catch (const Exception&)
    {
    }
    // what is the return value about??
    return false;
}

// the following method clears the bindings in the hashes for both doc & app
void SvxMacroTabPage_::Reset( const SfxItemSet* )
{
    // called once in creation - don't reset the data this time
    if(!bInitialized)
    {
        bInitialized = true;
        return;
    }

    try
    {
            if( m_xAppEvents.is() )
            {
                EventsHash::iterator h_itEnd =  m_appEventsHash.end();
                EventsHash::iterator h_it = m_appEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second.clear();
                }
            }
            if( m_xDocEvents.is() && bDocModified )
            {
                EventsHash::iterator h_itEnd =  m_docEventsHash.end();
                EventsHash::iterator h_it = m_docEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second.clear();
                }
                // if we have a valid XModifiable (in the case of doc events)
                // call setModified(true)
                if(m_xModifiable.is())
                {
                    m_xModifiable->setModified( true );
                }
            }
    }
    catch (const Exception&)
    {
    }
    DisplayAppEvents(bAppEvents);
}

void SvxMacroTabPage_::SetReadOnly( bool bSet )
{
    mpImpl->bReadOnly = bSet;
}

bool SvxMacroTabPage_::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}


class IconLBoxString : public SvLBoxString
{
    Image* m_pMacroImg;
    Image* m_pComponentImg;

public:
    IconLBoxString( const OUString& sText, Image* pMacroImg, Image* pComponentImg );
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};


IconLBoxString::IconLBoxString( const OUString& sText,
    Image* pMacroImg, Image* pComponentImg )
        : SvLBoxString( sText )
        , m_pMacroImg( pMacroImg )
        , m_pComponentImg( pComponentImg )
{
}


void IconLBoxString::Paint(const Point& aPos, SvTreeListBox& /*aDevice*/, vcl::RenderContext& rRenderContext,
                           const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    OUString aURL(GetText());
    if (!aURL.isEmpty())
    {
        sal_Int32 nIndex = aURL.indexOf(aVndSunStarUNO);
        bool bUNO = nIndex == 0;

        const Image* pImg = bUNO ? m_pComponentImg : m_pMacroImg;
        rRenderContext.DrawImage(aPos, *pImg);

        OUString aPureMethod;
        if (bUNO)
        {
            aPureMethod = aURL.copy(strlen(aVndSunStarUNO));
        }
        else
        {
            aPureMethod = aURL.copy(strlen("vnd.sun.star.script:"));
            aPureMethod = aPureMethod.copy( 0, aPureMethod.indexOf( '?' ) );
        }

        Point aPnt(aPos);
        aPnt.X() += 20;
        rRenderContext.DrawText(aPnt, aPureMethod);
    }
}


// displays the app events if appEvents=true, otherwise displays the doc events
void SvxMacroTabPage_::DisplayAppEvents( bool appEvents)
{
    bAppEvents = appEvents;

    SvHeaderTabListBox&        rListBox = mpImpl->pEventLB->GetListBox();
    mpImpl->pEventLB->SetUpdateMode( false );
    rListBox.Clear();
    EventsHash* eventsHash;
    Reference< container::XNameReplace> nameReplace;
    if(bAppEvents)
    {
        eventsHash = &m_appEventsHash;
        nameReplace = m_xAppEvents;
    }
    else
    {
        eventsHash = &m_docEventsHash;
        nameReplace = m_xDocEvents;
    }
    // have to use the original XNameReplace since the hash iterators do
    // not guarantee the order in which the elements are returned
    if(!nameReplace.is())
        return;

    Sequence< OUString > eventNames = nameReplace->getElementNames();
    std::set< OUString > aEventNamesCache;
    std::copy(
        eventNames.getConstArray(),
        eventNames.getConstArray() + eventNames.getLength(),
        std::insert_iterator< std::set< OUString > >( aEventNamesCache, aEventNamesCache.end() )
    );

    for (   EventDisplayNames::const_iterator displayableEvent = aDisplayNames.begin();
            displayableEvent != aDisplayNames.end();
            ++displayableEvent
        )
    {
        OUString sEventName( OUString::createFromAscii( displayableEvent->pAsciiEventName ) );
        if ( !nameReplace->hasByName( sEventName ) )
            continue;

        EventsHash::iterator h_it = eventsHash->find( sEventName );
        if( h_it == eventsHash->end() )
        {
            OSL_FAIL( "SvxMacroTabPage_::DisplayAppEvents: something's suspicious here!" );
            continue;
        }

        OUString eventURL = h_it->second.second;
        OUString displayName(CuiResId(displayableEvent->pEventResourceID));

        displayName += "\t";

        SvTreeListEntry*    _pE = rListBox.InsertEntry( displayName );
        OUString* pEventName = new OUString( sEventName );
        _pE->SetUserData( static_cast<void*>(pEventName) );
        OUString sNew( eventURL );
        _pE->ReplaceItem(o3tl::make_unique<IconLBoxString>(sNew, &mpImpl->aMacroImg, &mpImpl->aComponentImg),
            LB_MACROS_ITEMPOS );
        rListBox.GetModel()->InvalidateEntry( _pE );
        rListBox.Select( _pE );
        rListBox.MakeVisible( _pE );
    }

    SvTreeListEntry* pE = rListBox.GetEntry(0);
    if( pE )
    {
        rListBox.Select( pE );
        rListBox.MakeVisible( pE );
    }

    rListBox.SetUpdateMode( true );
    EnableButtons();
}

// select event handler on the listbox
IMPL_LINK_NOARG( SvxMacroTabPage_, SelectEvent_Impl, SvTreeListBox*, void)
{
    SvHeaderTabListBox&        rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry*           pE = rListBox.FirstSelected();

    if( !pE || LISTBOX_ENTRY_NOTFOUND == rListBox.GetModel()->GetAbsPos( pE ) )
    {
        DBG_ASSERT( pE, "Where does the empty entry come from?" );
        return;
    }

    EnableButtons();
}

IMPL_LINK( SvxMacroTabPage_, AssignDeleteHdl_Impl, Button*, pBtn, void )
{
    GenericHandler_Impl( this, static_cast<PushButton*>(pBtn) );
}

IMPL_LINK_NOARG( SvxMacroTabPage_, DoubleClickHdl_Impl, SvTreeListBox*, bool)
{
    return GenericHandler_Impl( this, nullptr );
}

// handler for double click on the listbox, and for the assign/delete buttons
long SvxMacroTabPage_::GenericHandler_Impl( SvxMacroTabPage_* pThis, PushButton* pBtn )
{
    SvxMacroTabPage_Impl*    pImpl = pThis->mpImpl;
    SvHeaderTabListBox& rListBox = pImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.FirstSelected();
    if( !pE || LISTBOX_ENTRY_NOTFOUND == rListBox.GetModel()->GetAbsPos( pE ) )
    {
        DBG_ASSERT( pE, "Where does the empty entry come from?" );
        return 0;
    }

    const bool bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

    OUString* pEventName = static_cast<OUString*>(pE->GetUserData());

    OUString sEventURL;
    OUString sEventType;
    if(pThis->bAppEvents)
    {
        EventsHash::iterator h_it = pThis->m_appEventsHash.find( *pEventName );
        if(h_it != pThis->m_appEventsHash.end() )
        {
            sEventType = h_it->second.first;
            sEventURL = h_it->second.second;
        }
    }
    else
    {
        EventsHash::iterator h_it = pThis->m_docEventsHash.find( *pEventName );
        if(h_it != pThis->m_docEventsHash.end() )
        {
            sEventType = h_it->second.first;
            sEventURL = h_it->second.second;
        }
    }

    bool bDoubleClick = (pBtn == nullptr);
    bool bUNOAssigned = sEventURL.startsWith( aVndSunStarUNO );
    if( pBtn == pImpl->pDeletePB )
    {
        // delete pressed
        sEventType =  "Script" ;
        sEventURL.clear();
        if(!pThis->bAppEvents)
            pThis->bDocModified = true;
    }
    else if (   (   ( pBtn != nullptr )
                &&  ( pBtn == pImpl->pAssignComponentPB )
                )
            ||  (   bDoubleClick
                &&  bUNOAssigned
                )
            )
    {
        ScopedVclPtrInstance< AssignComponentDialog > pAssignDlg( pThis, sEventURL );

        short ret = pAssignDlg->Execute();
        if( ret )
        {
            sEventType = "UNO";
            sEventURL = pAssignDlg->getURL();
            if(!pThis->bAppEvents)
                pThis->bDocModified = true;
        }
    }
    else if( bAssEnabled )
    {
        // assign pressed
        ScopedVclPtrInstance< SvxScriptSelectorDialog > pDlg( pThis, false, pThis->GetFrame() );
        if( pDlg )
        {
            short ret = pDlg->Execute();
            if ( ret )
            {
                sEventType = "Script";
                sEventURL = pDlg->GetScriptURL();
                if(!pThis->bAppEvents)
                    pThis->bDocModified = true;
            }
        }
    }

    // update the hashes
    if(pThis->bAppEvents)
    {
        EventsHash::iterator h_it = pThis->m_appEventsHash.find( *pEventName );
        h_it->second.first = sEventType;
        h_it->second.second = sEventURL;
    }
    else
    {
        EventsHash::iterator h_it = pThis->m_docEventsHash.find( *pEventName );
        h_it->second.first = sEventType;
        h_it->second.second = sEventURL;
    }

    // update the listbox entry
    pImpl->pEventLB->SetUpdateMode( false );
    pE->ReplaceItem(o3tl::make_unique<IconLBoxString>(sEventURL, &pImpl->aMacroImg, &pImpl->aComponentImg),
        LB_MACROS_ITEMPOS );

    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( true );

    pThis->EnableButtons();
    return 0;
}

// pass in the XNameReplace.
// can remove the 3rd arg once issue ?? is fixed
void SvxMacroTabPage_::InitAndSetHandler( const Reference< container::XNameReplace>& xAppEvents, const Reference< container::XNameReplace>& xDocEvents, const Reference< util::XModifiable >& xModifiable )
{
    m_xAppEvents = xAppEvents;
    m_xDocEvents = xDocEvents;
    m_xModifiable = xModifiable;
    SvHeaderTabListBox&    rListBox = mpImpl->pEventLB->GetListBox();
    HeaderBar&             rHeaderBar = mpImpl->pEventLB->GetHeaderBar();
    Link<Button*,void>     aLnk(LINK(this, SvxMacroTabPage_, AssignDeleteHdl_Impl ));
    mpImpl->pDeletePB->SetClickHdl(    aLnk );
    mpImpl->pAssignPB->SetClickHdl(    aLnk );
    if( mpImpl->pAssignComponentPB )
        mpImpl->pAssignComponentPB->SetClickHdl( aLnk );
    rListBox.SetDoubleClickHdl( LINK(this, SvxMacroTabPage_, DoubleClickHdl_Impl ) );

    rListBox.SetSelectHdl( LINK( this, SvxMacroTabPage_, SelectEvent_Impl ));

    rListBox.SetSelectionMode( SelectionMode::Single );
    rListBox.SetTabs( &nTabs[0] );
    Size aSize( nTabs[ 2 ], 0 );
    rHeaderBar.InsertItem( ITEMID_EVENT, mpImpl->sStrEvent, LogicToPixel( aSize, MapMode( MapUnit::MapAppFont ) ).Width() );
    aSize.Width() = 1764;        // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, mpImpl->sAssignedMacro, LogicToPixel( aSize, MapMode( MapUnit::MapAppFont ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    long nMinLineHeight = mpImpl->aMacroImg.GetSizePixel().Height() + 2;
    if( nMinLineHeight > mpImpl->pEventLB->GetListBox().GetEntryHeight() )
        mpImpl->pEventLB->GetListBox().SetEntryHeight(
            sal::static_int_cast< short >(nMinLineHeight) );

    mpImpl->pEventLB->Enable();

    if(!m_xAppEvents.is())
    {
        return;
    }
    Sequence< OUString > eventNames = m_xAppEvents->getElementNames();
    sal_Int32 nEventCount = eventNames.getLength();
    for(sal_Int32 nEvent = 0; nEvent < nEventCount; ++nEvent )
    {
        //need exception handling here
        try
        {
            m_appEventsHash[ eventNames[nEvent] ] = GetPairFromAny( m_xAppEvents->getByName( eventNames[nEvent] ) );
        }
        catch (const Exception&)
        {
        }
    }
    if(m_xDocEvents.is())
    {
        eventNames = m_xDocEvents->getElementNames();
        nEventCount = eventNames.getLength();
        for(sal_Int32 nEvent = 0; nEvent < nEventCount; ++nEvent )
        {
            try
            {
                m_docEventsHash[ eventNames[nEvent] ] = GetPairFromAny( m_xDocEvents->getByName( eventNames[nEvent] ) );
            }
            catch (const Exception&)
            {
            }
        }
    }
}

// returns the two props EventType & Script for a given event name
Any SvxMacroTabPage_::GetPropsByName( const OUString& eventName, EventsHash& eventsHash )
{
    const std::pair< OUString, OUString >& rAssignedEvent( eventsHash[ eventName ] );

    Any aReturn;
    ::comphelper::NamedValueCollection aProps;
    if ( !(rAssignedEvent.first.isEmpty() || rAssignedEvent.second.isEmpty()) )
    {
        aProps.put( "EventType", rAssignedEvent.first );
        aProps.put( "Script", rAssignedEvent.second );
    }
    aReturn <<= aProps.getPropertyValues();

    return aReturn;
}

// converts the Any returned by GetByName into a pair which can be stored in
// the EventHash
std::pair< OUString, OUString  > SvxMacroTabPage_::GetPairFromAny( const Any& aAny )
{
    Sequence< beans::PropertyValue > props;
    OUString type, url;
    if( aAny >>= props )
    {
        ::comphelper::NamedValueCollection aProps( props );
        type = aProps.getOrDefault( "EventType", type );
        url = aProps.getOrDefault( "Script", url );
    }
    return std::make_pair( type, url );
}

SvxMacroTabPage::SvxMacroTabPage(vcl::Window* pParent,
    const Reference< frame::XFrame >& _rxDocumentFrame,
    const SfxItemSet& rSet,
    Reference< container::XNameReplace > const & xNameReplace,
    sal_uInt16 nSelectedIndex)
    : SvxMacroTabPage_(pParent, "MacroAssignPage", "cui/ui/macroassignpage.ui", rSet)
{
    mpImpl->sStrEvent = get<FixedText>("eventft")->GetText();
    mpImpl->sAssignedMacro = get<FixedText>("assignft")->GetText();
    get(mpImpl->pEventLB, "assignments");
    get(mpImpl->pAssignPB, "assign");
    get(mpImpl->pDeletePB, "delete");
    get(mpImpl->pAssignComponentPB, "component");
    mpImpl->aMacroImg = get<FixedImage>("macroimg")->GetImage();
    mpImpl->aComponentImg = get<FixedImage>("componentimg")->GetImage();

    SetFrame( _rxDocumentFrame );

    if( !mpImpl->bIDEDialogMode )
    {
        mpImpl->pAssignComponentPB->Hide();
        mpImpl->pAssignComponentPB->Disable();
    }

    InitResources();

    InitAndSetHandler( xNameReplace, Reference< container::XNameReplace>(nullptr), Reference< util::XModifiable >(nullptr));
    DisplayAppEvents(true);
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvTreeListEntry* pE = rListBox.GetEntry( (sal_uLong)nSelectedIndex );
    if( pE )
        rListBox.Select(pE);
}

SvxMacroAssignDlg::SvxMacroAssignDlg( vcl::Window* pParent, const Reference< frame::XFrame >& _rxDocumentFrame, const SfxItemSet& rSet,
    const Reference< container::XNameReplace >& xNameReplace, sal_uInt16 nSelectedIndex )
        : SvxMacroAssignSingleTabDialog(pParent, rSet)
{
    SetTabPage(VclPtr<SvxMacroTabPage>::Create(get_content_area(), _rxDocumentFrame, rSet, xNameReplace, nSelectedIndex));
}


IMPL_LINK_NOARG(AssignComponentDialog, ButtonHandler, Button*, void)
{
    OUString aMethodName = mpMethodEdit->GetText();
    maURL.clear();
    if( !aMethodName.isEmpty() )
    {
        maURL = aVndSunStarUNO;
        maURL += aMethodName;
    }
    EndDialog(1);
}

AssignComponentDialog::AssignComponentDialog( vcl::Window * pParent, const OUString& rURL )
    : ModalDialog( pParent, "AssignComponent", "cui/ui/assigncomponentdialog.ui" )
    , maURL( rURL )
{
    get(mpMethodEdit, "methodEntry");
    get(mpOKButton, "ok");
    mpOKButton->SetClickHdl(LINK(this, AssignComponentDialog, ButtonHandler));

    OUString aMethodName;
    if( maURL.startsWith( aVndSunStarUNO ) )
    {
        aMethodName = maURL.copy( strlen(aVndSunStarUNO) );
    }
    mpMethodEdit->SetText( aMethodName, Selection( 0, SELECTION_MAX ) );
}

AssignComponentDialog::~AssignComponentDialog()
{
    disposeOnce();
}

void AssignComponentDialog::dispose()
{
    mpMethodEdit.clear();
    mpOKButton.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG( SvxMacroAssignSingleTabDialog, OKHdl_Impl, Button *, void )
{
    GetTabPage()->FillItemSet( nullptr );
    EndDialog( RET_OK );
}


SvxMacroAssignSingleTabDialog::SvxMacroAssignSingleTabDialog(vcl::Window *pParent,
    const SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet, "MacroAssignDialog", "cui/ui/macroassigndialog.ui")
{
    GetOKButton()->SetClickHdl( LINK( this, SvxMacroAssignSingleTabDialog, OKHdl_Impl ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
