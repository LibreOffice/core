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

#include <macropg.hxx>
#include <svl/eitem.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <bitmaps.hlst>
#include <cfgutil.hxx>
#include <dialmgr.hxx>
#include <helpids.h>
#include <headertablistbox.hxx>
#include "macropg_impl.hxx"
#include <o3tl/safeint.hxx>
#include <svl/macitem.hxx>
#include <svx/svxids.hrc>
#include <strings.hrc>
#include <comphelper/namedvaluecollection.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

constexpr OUString aVndSunStarUNO = u"vnd.sun.star.UNO:"_ustr;

SvxMacroTabPage_Impl::SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet )
    : bReadOnly(false)
    , bIDEDialogMode(false)
{
    const SfxPoolItem* pItem;
    if ( SfxItemState::SET == rAttrSet.GetItemState( SID_ATTR_MACROITEM, false, &pItem ) )
        bIDEDialogMode = static_cast<const SfxBoolItem*>(pItem)->GetValue();
}

MacroEventListBox::MacroEventListBox(std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTreeView(std::move(xTreeView))
{
    m_xTreeView->set_help_id(HID_MACRO_HEADERTABLISTBOX);
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 70, m_xTreeView->get_height_rows(9));
}

// assign button ("Add Command") is enabled only if it is not read only
// delete button ("Remove Command") is enabled if a current binding exists
//     and it is not read only
void SvxMacroTabPage_::EnableButtons()
{
    mpImpl->xDeleteAllPB->set_sensitive(m_nAssignedEvents != 0);
    int nEvent = mpImpl->xEventLB->get_selected_index();
    const EventPair* pEventPair = nEvent == -1 ? nullptr : LookupEvent(mpImpl->xEventLB->get_id(nEvent));
    const bool bAssigned = pEventPair && !pEventPair->second.isEmpty();
    mpImpl->xDeletePB->set_sensitive(!mpImpl->bReadOnly && bAssigned);
    mpImpl->xAssignPB->set_sensitive(!mpImpl->bReadOnly);
    if (mpImpl->xAssignComponentPB)
        mpImpl->xAssignComponentPB->set_sensitive( !mpImpl->bReadOnly );
}

SvxMacroTabPage_::SvxMacroTabPage_(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription,
    const OUString& rID, const SfxItemSet& rAttrSet)
    : SfxTabPage(pPage, pController, rUIXMLDescription, rID, &rAttrSet)
    , m_nAssignedEvents(0)
    , bDocModified(false)
    , bAppEvents(false)
    , bInitialized(false)
{
    mpImpl.reset( new SvxMacroTabPage_Impl( rAttrSet ) );
}

SvxMacroTabPage_::~SvxMacroTabPage_()
{
    mpImpl.reset();
}

void SvxMacroTabPage_::InitResources()
{
    // Note: the order here controls the order in which the events are displayed in the UI!

    // the event name to UI string mappings for App Events
    aDisplayNames.emplace_back( "OnStartApp",            RID_CUISTR_EVENT_STARTAPP );
    aDisplayNames.emplace_back( "OnCloseApp",            RID_CUISTR_EVENT_CLOSEAPP );
    aDisplayNames.emplace_back( "OnCreate",              RID_CUISTR_EVENT_CREATEDOC );
    aDisplayNames.emplace_back( "OnNew",                 RID_CUISTR_EVENT_NEWDOC );
    aDisplayNames.emplace_back( "OnLoadFinished",        RID_CUISTR_EVENT_LOADDOCFINISHED );
    aDisplayNames.emplace_back( "OnLoad",                RID_CUISTR_EVENT_OPENDOC );
    aDisplayNames.emplace_back( "OnPrepareUnload",       RID_CUISTR_EVENT_PREPARECLOSEDOC );
    aDisplayNames.emplace_back( "OnUnload",              RID_CUISTR_EVENT_CLOSEDOC ) ;
    aDisplayNames.emplace_back( "OnViewCreated",         RID_CUISTR_EVENT_VIEWCREATED );
    aDisplayNames.emplace_back( "OnPrepareViewClosing",  RID_CUISTR_EVENT_PREPARECLOSEVIEW );
    aDisplayNames.emplace_back( "OnViewClosed",          RID_CUISTR_EVENT_CLOSEVIEW ) ;
    aDisplayNames.emplace_back( "OnFocus",               RID_CUISTR_EVENT_ACTIVATEDOC );
    aDisplayNames.emplace_back( "OnUnfocus",             RID_CUISTR_EVENT_DEACTIVATEDOC );
    aDisplayNames.emplace_back( "OnSave",                RID_CUISTR_EVENT_SAVEDOC );
    aDisplayNames.emplace_back( "OnSaveDone",            RID_CUISTR_EVENT_SAVEDOCDONE );
    aDisplayNames.emplace_back( "OnSaveFailed",          RID_CUISTR_EVENT_SAVEDOCFAILED );
    aDisplayNames.emplace_back( "OnSaveAs",              RID_CUISTR_EVENT_SAVEASDOC );
    aDisplayNames.emplace_back( "OnSaveAsDone",          RID_CUISTR_EVENT_SAVEASDOCDONE );
    aDisplayNames.emplace_back( "OnSaveAsFailed",        RID_CUISTR_EVENT_SAVEASDOCFAILED );
    aDisplayNames.emplace_back( "OnCopyTo",              RID_CUISTR_EVENT_COPYTODOC );
    aDisplayNames.emplace_back( "OnCopyToDone",          RID_CUISTR_EVENT_COPYTODOCDONE );
    aDisplayNames.emplace_back( "OnCopyToFailed",        RID_CUISTR_EVENT_COPYTODOCFAILED );
    aDisplayNames.emplace_back( "OnPrint",               RID_CUISTR_EVENT_PRINTDOC );
    aDisplayNames.emplace_back( "OnModifyChanged",       RID_CUISTR_EVENT_MODIFYCHANGED );
    aDisplayNames.emplace_back( "OnTitleChanged",        RID_CUISTR_EVENT_TITLECHANGED );

    // application specific events
    aDisplayNames.emplace_back( "OnMailMerge",           RID_CUISTR_EVENT_MAILMERGE );
    aDisplayNames.emplace_back( "OnMailMergeFinished",           RID_CUISTR_EVENT_MAILMERGE_END );
    aDisplayNames.emplace_back( "OnFieldMerge",           RID_CUISTR_EVENT_FIELDMERGE );
    aDisplayNames.emplace_back( "OnFieldMergeFinished",           RID_CUISTR_EVENT_FIELDMERGE_FINISHED );
    aDisplayNames.emplace_back( "OnPageCountChange",     RID_CUISTR_EVENT_PAGECOUNTCHANGE );
    aDisplayNames.emplace_back( "OnSubComponentOpened",  RID_CUISTR_EVENT_SUBCOMPONENT_OPENED );
    aDisplayNames.emplace_back( "OnSubComponentClosed",  RID_CUISTR_EVENT_SUBCOMPONENT_CLOSED );
    aDisplayNames.emplace_back( "OnSelect",              RID_CUISTR_EVENT_SELECTIONCHANGED );
    aDisplayNames.emplace_back( "OnDoubleClick",         RID_CUISTR_EVENT_DOUBLECLICK );
    aDisplayNames.emplace_back( "OnRightClick",          RID_CUISTR_EVENT_RIGHTCLICK );
    aDisplayNames.emplace_back( "OnCalculate",           RID_CUISTR_EVENT_CALCULATE );
    aDisplayNames.emplace_back( "OnChange",              RID_CUISTR_EVENT_CONTENTCHANGED );

    // the event name to UI string mappings for forms & dialogs

    aDisplayNames.emplace_back( "approveAction",         RID_CUISTR_EVENT_APPROVEACTIONPERFORMED );
    aDisplayNames.emplace_back( "actionPerformed",       RID_CUISTR_EVENT_ACTIONPERFORMED );
    aDisplayNames.emplace_back( "changed",               RID_CUISTR_EVENT_CHANGED );
    aDisplayNames.emplace_back( "textChanged",           RID_CUISTR_EVENT_TEXTCHANGED );
    aDisplayNames.emplace_back( "itemStateChanged",      RID_CUISTR_EVENT_ITEMSTATECHANGED );
    aDisplayNames.emplace_back( "focusGained",           RID_CUISTR_EVENT_FOCUSGAINED );
    aDisplayNames.emplace_back( "focusLost",             RID_CUISTR_EVENT_FOCUSLOST );
    aDisplayNames.emplace_back( "keyPressed",            RID_CUISTR_EVENT_KEYTYPED );
    aDisplayNames.emplace_back( "keyReleased",           RID_CUISTR_EVENT_KEYUP );
    aDisplayNames.emplace_back( "mouseEntered",          RID_CUISTR_EVENT_MOUSEENTERED );
    aDisplayNames.emplace_back( "mouseDragged",          RID_CUISTR_EVENT_MOUSEDRAGGED );
    aDisplayNames.emplace_back( "mouseMoved",            RID_CUISTR_EVENT_MOUSEMOVED );
    aDisplayNames.emplace_back( "mousePressed",          RID_CUISTR_EVENT_MOUSEPRESSED );
    aDisplayNames.emplace_back( "mouseReleased",         RID_CUISTR_EVENT_MOUSERELEASED );
    aDisplayNames.emplace_back( "mouseExited",           RID_CUISTR_EVENT_MOUSEEXITED );
    aDisplayNames.emplace_back( "approveReset",          RID_CUISTR_EVENT_APPROVERESETTED );
    aDisplayNames.emplace_back( "resetted",              RID_CUISTR_EVENT_RESETTED );
    aDisplayNames.emplace_back( "approveSubmit",         RID_CUISTR_EVENT_SUBMITTED );
    aDisplayNames.emplace_back( "approveUpdate",         RID_CUISTR_EVENT_BEFOREUPDATE );
    aDisplayNames.emplace_back( "updated",               RID_CUISTR_EVENT_AFTERUPDATE );
    aDisplayNames.emplace_back( "loaded",                RID_CUISTR_EVENT_LOADED );
    aDisplayNames.emplace_back( "reloading",             RID_CUISTR_EVENT_RELOADING );
    aDisplayNames.emplace_back( "reloaded",              RID_CUISTR_EVENT_RELOADED );
    aDisplayNames.emplace_back( "unloading",             RID_CUISTR_EVENT_UNLOADING );
    aDisplayNames.emplace_back( "unloaded",              RID_CUISTR_EVENT_UNLOADED );
    aDisplayNames.emplace_back( "confirmDelete",         RID_CUISTR_EVENT_CONFIRMDELETE );
    aDisplayNames.emplace_back( "approveRowChange",      RID_CUISTR_EVENT_APPROVEROWCHANGE );
    aDisplayNames.emplace_back( "rowChanged",            RID_CUISTR_EVENT_ROWCHANGE );
    aDisplayNames.emplace_back( "approveCursorMove",     RID_CUISTR_EVENT_POSITIONING );
    aDisplayNames.emplace_back( "cursorMoved",           RID_CUISTR_EVENT_POSITIONED );
    aDisplayNames.emplace_back( "approveParameter",      RID_CUISTR_EVENT_APPROVEPARAMETER );
    aDisplayNames.emplace_back( "errorOccured",          RID_CUISTR_EVENT_ERROROCCURRED );
    aDisplayNames.emplace_back( "adjustmentValueChanged",   RID_CUISTR_EVENT_ADJUSTMENTVALUECHANGED );
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
            for (auto const& appEvent : m_appEventsHash)
            {
                eventName = appEvent.first;
                try
                {
                    m_xAppEvents->replaceByName( eventName, GetPropsByName( eventName, m_appEventsHash ) );
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("cui.customize");
                }
            }
        }
        if( m_xDocEvents.is() && bDocModified )
        {
            for (auto const& docEvent : m_docEventsHash)
            {
                eventName = docEvent.first;
                try
                {
                    m_xDocEvents->replaceByName( eventName, GetPropsByName( eventName, m_docEventsHash ) );
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("cui.customize");
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
                for (auto & appEvent : m_appEventsHash)
                {
                    appEvent.second.second.clear();
                }
            }
            if( m_xDocEvents.is() && bDocModified )
            {
                for (auto & docEvent : m_docEventsHash)
                {
                    docEvent.second.second.clear();
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

namespace
{
    std::u16string_view GetEventDisplayText(std::u16string_view rURL)
    {
        if (rURL.empty())
            return std::u16string_view();
        bool bUNO = o3tl::starts_with(rURL, aVndSunStarUNO);
        std::u16string_view aPureMethod;
        if (bUNO)
        {
            aPureMethod = rURL.substr(aVndSunStarUNO.getLength());
        }
        else
        {
            aPureMethod = rURL.substr(strlen("vnd.sun.star.script:"));
            aPureMethod = aPureMethod.substr( 0, aPureMethod.find( '?' ) );
        }
        return aPureMethod;
    }

    OUString GetEventDisplayImage(std::u16string_view rURL)
    {
        if (rURL.empty())
            return OUString();
        size_t nIndex = rURL.find(aVndSunStarUNO);
        bool bUNO = nIndex == 0;
        return bUNO ? RID_SVXBMP_COMPONENT : RID_SVXBMP_MACRO;
    }
}

// displays the app events if appEvents=true, otherwise displays the doc events
void SvxMacroTabPage_::DisplayAppEvents( bool appEvents)
{
    bAppEvents = appEvents;

    mpImpl->xEventLB->freeze();
    mpImpl->xEventLB->clear();
    m_nAssignedEvents = 0;
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
    {
        mpImpl->xEventLB->thaw();
        return;
    }

    for (auto const& displayableEvent : aDisplayNames)
    {
        OUString sEventName( OUString::createFromAscii( displayableEvent.pAsciiEventName ) );
        if ( !nameReplace->hasByName( sEventName ) )
            continue;

        EventsHash::iterator h_it = eventsHash->find( sEventName );
        if( h_it == eventsHash->end() )
        {
            OSL_FAIL( "SvxMacroTabPage_::DisplayAppEvents: something's suspicious here!" );
            continue;
        }

        OUString eventURL = h_it->second.second;
        OUString displayName(CuiResId(displayableEvent.pEventResourceID));

        int nRow = mpImpl->xEventLB->n_children();
        mpImpl->xEventLB->append(sEventName, displayName);
        mpImpl->xEventLB->set_image(nRow, GetEventDisplayImage(eventURL), 1);
        mpImpl->xEventLB->set_text(nRow, OUString(GetEventDisplayText(eventURL)), 2);

        if (!eventURL.isEmpty())
            ++m_nAssignedEvents;
    }

    mpImpl->xEventLB->thaw();

    if (mpImpl->xEventLB->n_children())
    {
        mpImpl->xEventLB->select(0);
        mpImpl->xEventLB->scroll_to_row(0);
    }

    EnableButtons();
}

// select event handler on the listbox
IMPL_LINK_NOARG( SvxMacroTabPage_, SelectEvent_Impl, weld::TreeView&, void)
{
    int nEntry = mpImpl->xEventLB->get_selected_index();

    if (nEntry == -1)
    {
        DBG_ASSERT(false, "Where does the empty entry come from?" );
        return;
    }

    EnableButtons();
}

IMPL_LINK( SvxMacroTabPage_, AssignDeleteHdl_Impl, weld::Button&, rBtn, void )
{
    GenericHandler_Impl(&rBtn);
}

IMPL_LINK_NOARG( SvxMacroTabPage_, DoubleClickHdl_Impl, weld::TreeView&, bool)
{
    GenericHandler_Impl(nullptr);
    return true;
}

const EventPair* SvxMacroTabPage_::LookupEvent(const OUString& rEventName)
{
    const EventPair* pRet = nullptr;
    if (bAppEvents)
    {
        EventsHash::iterator h_it = m_appEventsHash.find(rEventName);
        if (h_it != m_appEventsHash.end() )
            pRet = &h_it->second;
    }
    else
    {
        EventsHash::iterator h_it = m_docEventsHash.find(rEventName);
        if (h_it != m_docEventsHash.end() )
            pRet = &h_it->second;
    }
    return pRet;
}

// handler for double click on the listbox, and for the assign/delete buttons
void SvxMacroTabPage_::GenericHandler_Impl(const weld::Button* pBtn)
{
    weld::TreeView& rListBox = *mpImpl->xEventLB;
    int nEntry = rListBox.get_selected_index();
    if (nEntry == -1)
    {
        DBG_ASSERT(false, "Where does the empty entry come from?");
        return;
    }

    const bool bAssEnabled = pBtn != mpImpl->xDeletePB.get() && mpImpl->xAssignPB->get_sensitive();

    OUString sEventName = rListBox.get_id(nEntry);

    OUString sEventURL;
    OUString sEventType;
    if (const EventPair* pEventPair = LookupEvent(sEventName))
    {
        sEventType = pEventPair->first;
        sEventURL = pEventPair->second;
    }

    if (!sEventURL.isEmpty())
        --m_nAssignedEvents;

    bool bDoubleClick = (pBtn == nullptr);
    bool bUNOAssigned = sEventURL.startsWith( aVndSunStarUNO );
    if (pBtn == mpImpl->xDeletePB.get())
    {
        // delete pressed
        sEventType =  "Script" ;
        sEventURL.clear();
        if (!bAppEvents)
            bDocModified = true;
    }
    else if (   (   ( pBtn != nullptr )
                &&  ( pBtn == mpImpl->xAssignComponentPB.get() )
                )
            ||  (   bDoubleClick
                &&  bUNOAssigned
                )
            )
    {
        AssignComponentDialog aAssignDlg(GetFrameWeld(), sEventURL);

        short ret = aAssignDlg.run();
        if( ret )
        {
            sEventType = "UNO";
            sEventURL = aAssignDlg.getURL();
            if (!bAppEvents)
                bDocModified = true;
        }
    }
    else if( bAssEnabled )
    {
        // assign pressed
        SvxScriptSelectorDialog aDlg(GetFrameWeld(), GetFrame());
        short ret = aDlg.run();
        if ( ret )
        {
            sEventType = "Script";
            sEventURL = aDlg.GetScriptURL();
            if (!bAppEvents)
                bDocModified = true;
        }
    }

    // update the hashes
    if (bAppEvents)
    {
        EventsHash::iterator h_it = m_appEventsHash.find(sEventName);
        h_it->second.first = sEventType;
        h_it->second.second = sEventURL;
    }
    else
    {
        EventsHash::iterator h_it = m_docEventsHash.find(sEventName);
        h_it->second.first = sEventType;
        h_it->second.second = sEventURL;
    }

    if (!sEventURL.isEmpty())
        ++m_nAssignedEvents;

    rListBox.set_image(nEntry, GetEventDisplayImage(sEventURL), 1);
    rListBox.set_text(nEntry, OUString(GetEventDisplayText(sEventURL)), 2);

    rListBox.select(nEntry );
    rListBox.scroll_to_row(nEntry);

    EnableButtons();
}

IMPL_LINK_NOARG(SvxMacroTabPage_, DeleteAllHdl_Impl, weld::Button&, void)
{
    OUString sEventType =  u"Script"_ustr ;
    OUString sEmptyString;

    mpImpl->xEventLB->all_foreach([this, &sEventType, &sEmptyString](weld::TreeIter& rEntry) {
        weld::TreeView& rListBox = *mpImpl->xEventLB;
        OUString sEventName = rListBox.get_id(rEntry);
        // update the hashes
        if (bAppEvents)
        {
            EventsHash::iterator h_it = m_appEventsHash.find(sEventName);
            assert(h_it != m_appEventsHash.end());
            h_it->second.first = sEventType;
            h_it->second.second = sEmptyString;
        }
        else
        {
            EventsHash::iterator h_it = m_docEventsHash.find(sEventName);
            assert(h_it != m_docEventsHash.end());
            h_it->second.first = sEventType;
            h_it->second.second = sEmptyString;
        }

        rListBox.set_image(rEntry, sEmptyString, 1);
        rListBox.set_text(rEntry, sEmptyString, 2);
        return false;
    });

    if (!bAppEvents)
        bDocModified = true;

    m_nAssignedEvents = 0;

    EnableButtons();
}

// pass in the XNameReplace.
// can remove the 3rd arg once issue ?? is fixed
void SvxMacroTabPage_::InitAndSetHandler( const Reference< container::XNameReplace>& xAppEvents, const Reference< container::XNameReplace>& xDocEvents, const Reference< util::XModifiable >& xModifiable )
{
    m_xAppEvents = xAppEvents;
    m_xDocEvents = xDocEvents;
    m_xModifiable = xModifiable;
    Link<weld::Button&,void>     aLnk(LINK(this, SvxMacroTabPage_, AssignDeleteHdl_Impl ));
    mpImpl->xDeletePB->connect_clicked(aLnk);
    mpImpl->xAssignPB->connect_clicked(aLnk);
    mpImpl->xDeleteAllPB->connect_clicked(LINK(this, SvxMacroTabPage_, DeleteAllHdl_Impl));
    if( mpImpl->xAssignComponentPB )
        mpImpl->xAssignComponentPB->connect_clicked( aLnk );
    mpImpl->xEventLB->connect_row_activated( LINK(this, SvxMacroTabPage_, DoubleClickHdl_Impl ) );
    mpImpl->xEventLB->connect_changed( LINK( this, SvxMacroTabPage_, SelectEvent_Impl ));

    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(mpImpl->xEventLB->get_approximate_digit_width() * 32),
        mpImpl->xEventLB->get_checkbox_column_width()
    };
    mpImpl->xEventLB->set_column_fixed_widths(aWidths);

    mpImpl->xEventLB->show();
    mpImpl->xEventLB->set_sensitive(true);

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
    if(!m_xDocEvents.is())
        return;

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

// returns the two props EventType & Script for a given event name
Any SvxMacroTabPage_::GetPropsByName( const OUString& eventName, EventsHash& eventsHash )
{
    const EventPair& rAssignedEvent(eventsHash[eventName]);

    Any aReturn;
    ::comphelper::NamedValueCollection aProps;
    if ( !(rAssignedEvent.first.isEmpty() || rAssignedEvent.second.isEmpty()) )
    {
        aProps.put( u"EventType"_ustr, rAssignedEvent.first );
        aProps.put( u"Script"_ustr, rAssignedEvent.second );
    }
    aReturn <<= aProps.getPropertyValues();

    return aReturn;
}

// converts the Any returned by GetByName into a pair which can be stored in
// the EventHash
EventPair SvxMacroTabPage_::GetPairFromAny( const Any& aAny )
{
    Sequence< beans::PropertyValue > props;
    OUString type, url;
    if( aAny >>= props )
    {
        ::comphelper::NamedValueCollection aProps( props );
        type = aProps.getOrDefault( u"EventType"_ustr, type );
        url = aProps.getOrDefault( u"Script"_ustr, url );
    }
    return std::make_pair( type, url );
}

SvxMacroTabPage::SvxMacroTabPage(weld::Container* pPage, weld::DialogController* pController,
    const Reference< frame::XFrame >& _rxDocumentFrame,
    const SfxItemSet& rSet,
    Reference< container::XNameReplace > const & xNameReplace,
    sal_uInt16 nSelectedIndex)
    : SvxMacroTabPage_(pPage, pController, u"cui/ui/macroassignpage.ui"_ustr, u"MacroAssignPage"_ustr, rSet)
{
    mpImpl->xEventLB = m_xBuilder->weld_tree_view(u"assignments"_ustr);
    mpImpl->xEventLB->set_size_request(mpImpl->xEventLB->get_approximate_digit_width() * 70,
                                       mpImpl->xEventLB->get_height_rows(9));
    mpImpl->xAssignPB = m_xBuilder->weld_button(u"assign"_ustr);
    mpImpl->xDeletePB = m_xBuilder->weld_button(u"delete"_ustr);
    mpImpl->xDeleteAllPB = m_xBuilder->weld_button(u"deleteall"_ustr);
    mpImpl->xAssignComponentPB = m_xBuilder->weld_button(u"component"_ustr);

    SetFrame( _rxDocumentFrame );

    if( !mpImpl->bIDEDialogMode )
    {
        mpImpl->xAssignComponentPB->hide();
        mpImpl->xAssignComponentPB->set_sensitive(false);
    }

    InitResources();

    InitAndSetHandler( xNameReplace, Reference< container::XNameReplace>(nullptr), Reference< util::XModifiable >(nullptr));
    DisplayAppEvents(true);
    mpImpl->xEventLB->select(nSelectedIndex);
}

SvxMacroAssignDlg::SvxMacroAssignDlg(weld::Window* pParent, const Reference< frame::XFrame >& _rxDocumentFrame, const SfxItemSet& rSet,
    const Reference< container::XNameReplace >& xNameReplace, sal_uInt16 nSelectedIndex)
        : SvxMacroAssignSingleTabDialog(pParent, rSet)
{
    SetTabPage(std::make_unique<SvxMacroTabPage>(get_content_area(), this, _rxDocumentFrame, rSet, xNameReplace, nSelectedIndex));
}

IMPL_LINK_NOARG(AssignComponentDialog, ButtonHandler, weld::Button&, void)
{
    OUString aMethodName = mxMethodEdit->get_text();
    maURL.clear();
    if( !aMethodName.isEmpty() )
    {
        maURL = aVndSunStarUNO;
        maURL += aMethodName;
    }
    m_xDialog->response(RET_OK);
}

AssignComponentDialog::AssignComponentDialog(weld::Window* pParent, OUString aURL)
    : GenericDialogController(pParent, u"cui/ui/assigncomponentdialog.ui"_ustr, u"AssignComponent"_ustr)
    , maURL(std::move( aURL ))
    , mxMethodEdit(m_xBuilder->weld_entry(u"methodEntry"_ustr))
    , mxOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    mxOKButton->connect_clicked(LINK(this, AssignComponentDialog, ButtonHandler));

    OUString aMethodName;
    if( maURL.startsWith( aVndSunStarUNO ) )
    {
        aMethodName = maURL.copy( aVndSunStarUNO.getLength() );
    }
    mxMethodEdit->set_text(aMethodName);
    mxMethodEdit->select_region(0, -1);
}

AssignComponentDialog::~AssignComponentDialog()
{
}

IMPL_LINK_NOARG(SvxMacroAssignSingleTabDialog, OKHdl_Impl, weld::Button&, void)
{
    m_xSfxPage->FillItemSet(nullptr);
    m_xDialog->response(RET_OK);
}

SvxMacroAssignSingleTabDialog::SvxMacroAssignSingleTabDialog(weld::Window *pParent,
    const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet, u"cui/ui/macroassigndialog.ui"_ustr, u"MacroAssignDialog"_ustr)
{
    GetOKButton().connect_clicked(LINK(this, SvxMacroAssignSingleTabDialog, OKHdl_Impl));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
