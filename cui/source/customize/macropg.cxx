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
#include <vcl/msgbox.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/svlbitm.hxx>
#include <svl/eitem.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdefs.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <dialmgr.hxx>
#include "selector.hxx"
#include "cfg.hxx"
#include "macropg.hrc"
#include "helpid.hrc"
#include <cuires.hrc>
#include "headertablistbox.hxx"
#include "macropg_impl.hxx"
#include <svx/dialogs.hrc> // RID_SVXPAGE_MACROASSIGN
#include <comphelper/namedvaluecollection.hxx>

#include <algorithm>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static ::rtl::OUString aVndSunStarUNO( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.UNO:") );
static ::rtl::OUString aVndSunStarScript( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.script:") );

_SvxMacroTabPage_Impl::_SvxMacroTabPage_Impl( const SfxItemSet& rAttrSet ) :
    pAssignFT( NULL ),
    pAssignPB( NULL ),
    pAssignComponentPB( NULL ),
    pDeletePB( NULL ),
    pMacroImg( NULL ),
    pComponentImg( NULL ),
    pStrEvent( NULL ),
    pAssignedMacro( NULL ),
    pEventLB( NULL ),
    bReadOnly( sal_False ),
    bIDEDialogMode( sal_False )
{
    const SfxPoolItem* pItem;
    if ( SFX_ITEM_SET == rAttrSet.GetItemState( SID_ATTR_MACROITEM, sal_False, &pItem ) )
        bIDEDialogMode = ((const SfxBoolItem*)pItem)->GetValue();
}

_SvxMacroTabPage_Impl::~_SvxMacroTabPage_Impl()
{
    delete pAssignFT;
    delete pAssignPB;
    delete pAssignComponentPB;
    delete pDeletePB;
    delete pMacroImg;
    delete pComponentImg;
    delete pStrEvent;
    delete pAssignedMacro;
    delete pEventLB;
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


IMPL_LINK( _HeaderTabListBox, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    DBG_ASSERT( pBar == &maHeaderBar, "*_HeaderTabListBox::HeaderEndDrag_Impl: something is wrong here..." );
    (void)pBar;

    if( !maHeaderBar.GetCurItemId() )
        return 0;

    if( !maHeaderBar.IsItemMode() )
    {
        Size    aSz;
        sal_uInt16    _nTabs = maHeaderBar.GetItemCount();
        long    nWidth = maHeaderBar.GetItemSize( ITEMID_EVENT );
        long    nBarWidth = maHeaderBar.GetSizePixel().Width();

        if( nWidth < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, TAB_WIDTH_MIN );
        else if( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, nBarWidth - TAB_WIDTH_MIN );

        {
            long _nWidth, nTmpSz = 0;
            for( sal_uInt16 i = 1 ; i < _nTabs ; ++i )
            {
                _nWidth = maHeaderBar.GetItemSize( i );
                aSz.Width() =  _nWidth + nTmpSz;
                nTmpSz += _nWidth;
                maListBox.SetTab( i, PixelToLogic( aSz, MapMode( MAP_APPFONT ) ).Width(), MAP_APPFONT );
            }
        }
    }
    return 1;
}

long _HeaderTabListBox::Notify( NotifyEvent& rNEvt )
{
    long    nRet = Control::Notify( rNEvt );

    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( rNEvt.GetWindow() != &maListBox )
            maListBox.GrabFocus();
    }

    return nRet;
}

_HeaderTabListBox::_HeaderTabListBox( Window* pParent, const ResId& rId ) :
    Control( pParent, rId ),
    maListBox( this, WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP ),
    maHeaderBar( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER )
{
    maListBox.SetHelpId( HID_MACRO_HEADERTABLISTBOX );
}

_HeaderTabListBox::~_HeaderTabListBox()
{
}

void _HeaderTabListBox::ConnectElements( void )
{
    // calc pos and size of header bar
    Point    aPnt( 0, 0 );
    Size    aSize( maHeaderBar.CalcWindowSizePixel() );
    Size    aCtrlSize( GetOutputSizePixel() );
    aSize.Width() = aCtrlSize.Width();
    maHeaderBar.SetPosSizePixel( aPnt, aSize );

    // calc pos and size of ListBox
    aPnt.Y() += aSize.Height();
    aSize.Height() = aCtrlSize.Height() - aSize.Height();
    maListBox.SetPosSizePixel( aPnt, aSize );

    // set handler
    maHeaderBar.SetEndDragHdl( LINK( this, _HeaderTabListBox, HeaderEndDrag_Impl ) );

    maListBox.InitHeaderBar( &maHeaderBar );
}

void _HeaderTabListBox::Show( sal_Bool bVisible, sal_uInt16 nFlags )
{
    maListBox.Show( bVisible, nFlags );
    maHeaderBar.Show( bVisible, nFlags );
}

void _HeaderTabListBox::Enable( bool bEnable, bool bChild )
{
    maListBox.Enable( bEnable, bChild );
    maHeaderBar.Enable( bEnable, bChild );
}

// assign button ("Add Command") is enabled only if it is not read only
// delete button ("Remove Command") is enabled if a current binding exists
//     and it is not read only
void _SvxMacroTabPage::EnableButtons()
{
    const SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        SvLBoxString* pEventMacro = (SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS );
        mpImpl->pDeletePB->Enable( 0 != pEventMacro && !mpImpl->bReadOnly );

        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly );
        if( mpImpl->pAssignComponentPB )
            mpImpl->pAssignComponentPB->Enable( !mpImpl->bReadOnly );
    }
}

_SvxMacroTabPage::_SvxMacroTabPage( Window* pParent, const ResId& rResId, const SfxItemSet& rAttrSet )
    : SfxTabPage( pParent, rResId, rAttrSet ),
    m_xAppEvents(0),
    m_xDocEvents(0),
    bReadOnly(false),
    bDocModified(false),
    bAppEvents(false),
    bInitialized(false)
{
    mpImpl = new _SvxMacroTabPage_Impl( rAttrSet );
}

_SvxMacroTabPage::~_SvxMacroTabPage()
{
    // need to delete the user data
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.GetEntry( 0 );
    while( pE )
    {
        ::rtl::OUString* pEventName = (::rtl::OUString*)pE->GetUserData();
        delete pEventName;
        pE->SetUserData((void*)0);
        pE = rListBox.NextSibling( pE );
    }
    DELETEZ( mpImpl );
}
// -----------------------------------------------------------------------------
void _SvxMacroTabPage::InitResources()
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
    //
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
    aDisplayNames.push_back( EventDisplayName( "errorOccurred",          RID_SVXSTR_EVENT_ERROROCCURRED ) );
    aDisplayNames.push_back( EventDisplayName( "adjustmentValueChanged",   RID_SVXSTR_EVENT_ADJUSTMENTVALUECHANGED ) );
}

// the following method is called when the user clicks OK
// We use the contents of the hashes to replace the settings
sal_Bool _SvxMacroTabPage::FillItemSet( SfxItemSet& /*rSet*/ )
{
    try
    {
        ::rtl::OUString eventName;
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
                m_xModifiable->setModified( sal_True );
            }
        }
    }
    catch (const Exception&)
    {
    }
    // what is the return value about??
    return sal_False;
}

// the following method clears the bindings in the hashes for both doc & app
void _SvxMacroTabPage::Reset()
{
    // called once in creation - don't reset the data this time
    if(!bInitialized)
    {
        bInitialized = true;
        return;
    }

    try
    {
            ::rtl::OUString sEmpty;
            if( m_xAppEvents.is() )
            {
                EventsHash::iterator h_itEnd =  m_appEventsHash.end();
                EventsHash::iterator h_it = m_appEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second = sEmpty;
                }
            }
            if( m_xDocEvents.is() && bDocModified )
            {
                EventsHash::iterator h_itEnd =  m_docEventsHash.end();
                EventsHash::iterator h_it = m_docEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second = sEmpty;
                }
                // if we have a valid XModifiable (in the case of doc events)
                // call setModified(true)
                if(m_xModifiable.is())
                {
                    m_xModifiable->setModified( sal_True );
                }
            }
    }
    catch (const Exception&)
    {
    }
    DisplayAppEvents(bAppEvents);
}

void _SvxMacroTabPage::SetReadOnly( sal_Bool bSet )
{
    mpImpl->bReadOnly = bSet;
}

sal_Bool _SvxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}


class IconLBoxString : public SvLBoxString
{
    Image* m_pMacroImg;
    Image* m_pComponentImg;
    int m_nxImageOffset;

    public:
        IconLBoxString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& sText,
            Image* pMacroImg, Image* pComponentImg );
        virtual void Paint(const Point& aPos, SvTreeListBox& aDevice, sal_uInt16 nFlags, SvLBoxEntry* pEntry );
};


IconLBoxString::IconLBoxString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& sText,
    Image* pMacroImg, Image* pComponentImg )
        : SvLBoxString( pEntry, nFlags, sText )
        , m_pMacroImg( pMacroImg )
        , m_pComponentImg( pComponentImg )
{
    m_nxImageOffset = 20;
}

//===============================================
void IconLBoxString::Paint( const Point& aPos, SvTreeListBox& aDevice,
                               sal_uInt16 /*nFlags*/, SvLBoxEntry* /*pEntry*/ )
{
    String aTxt( GetText() );
    if( aTxt.Len() )
    {
        ::rtl::OUString aURL( aTxt );
        sal_Int32 nIndex = aURL.indexOf( aVndSunStarUNO );
        bool bUNO = nIndex == 0;

        const Image* pImg = bUNO ? m_pComponentImg : m_pMacroImg;
        aDevice.DrawImage( aPos, *pImg );

        ::rtl::OUString aPureMethod;
        if( bUNO )
        {
            sal_Int32 nBegin = aVndSunStarUNO.getLength();
            aPureMethod = aURL.copy( nBegin );
        }
        else
        {
            sal_Int32 nBegin = aVndSunStarScript.getLength();
            aPureMethod = aURL.copy( nBegin );
            aPureMethod = aPureMethod.copy( 0, aPureMethod.indexOf( '?' ) );
        }

        Point aPnt(aPos);
        aPnt.X() += m_nxImageOffset;
        aDevice.DrawText( aPnt, aPureMethod );
    }
}


// displays the app events if appEvents=true, otherwise displays the doc events
void _SvxMacroTabPage::DisplayAppEvents( bool appEvents)
{
    bAppEvents = appEvents;

    SvHeaderTabListBox&        rListBox = mpImpl->pEventLB->GetListBox();
    mpImpl->pEventLB->SetUpdateMode( sal_False );
    rListBox.Clear();
    SvLBoxEntry*    pE = rListBox.GetEntry( 0 );
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
        return;
    }

    Sequence< ::rtl::OUString > eventNames = nameReplace->getElementNames();
    ::std::set< ::rtl::OUString > aEventNamesCache;
    ::std::copy(
        eventNames.getConstArray(),
        eventNames.getConstArray() + eventNames.getLength(),
        ::std::insert_iterator< ::std::set< ::rtl::OUString > >( aEventNamesCache, aEventNamesCache.end() )
    );

    for (   EventDisplayNames::const_iterator displayableEvent = aDisplayNames.begin();
            displayableEvent != aDisplayNames.end();
            ++displayableEvent
        )
    {
        ::rtl::OUString sEventName( ::rtl::OUString::createFromAscii( displayableEvent->pAsciiEventName ) );
        if ( !nameReplace->hasByName( sEventName ) )
            continue;

        EventsHash::iterator h_it = eventsHash->find( sEventName );
        if( h_it == eventsHash->end() )
        {
            OSL_FAIL( "_SvxMacroTabPage::DisplayAppEvents: something's suspicious here!" );
            continue;
        }

        ::rtl::OUString eventURL = h_it->second.second;
        String displayName( CUI_RES( displayableEvent->nEventResourceID ) );

        displayName += '\t';
        SvLBoxEntry*    _pE = rListBox.InsertEntry( displayName );
        ::rtl::OUString* pEventName = new ::rtl::OUString( sEventName );
        _pE->SetUserData( (void*)pEventName );
        String sNew( eventURL );
        _pE->ReplaceItem( new IconLBoxString( _pE, 0, sNew,
            mpImpl->pMacroImg, mpImpl->pComponentImg ), LB_MACROS_ITEMPOS );
        rListBox.GetModel()->InvalidateEntry( _pE );
        rListBox.Select( _pE );
        rListBox.MakeVisible( _pE );
    }

    pE = rListBox.GetEntry(0);
    if( pE )
    {
        rListBox.Select( pE );
        rListBox.MakeVisible( pE );
    }

    rListBox.SetUpdateMode( sal_True );
    EnableButtons();
}

// select event handler on the listbox
IMPL_STATIC_LINK( _SvxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    _SvxMacroTabPage_Impl*    pImpl = pThis->mpImpl;
    SvHeaderTabListBox&        rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry*            pE = rListBox.FirstSelected();
    sal_uLong                    nPos;

    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    pThis->EnableButtons();
    return 0;
}

IMPL_STATIC_LINK( _SvxMacroTabPage, AssignDeleteHdl_Impl, PushButton*, pBtn )
{
    return GenericHandler_Impl( pThis, pBtn );
}

IMPL_STATIC_LINK( _SvxMacroTabPage, DoubleClickHdl_Impl, SvTabListBox *, EMPTYARG )
{
    return GenericHandler_Impl( pThis, NULL );
}

// handler for double click on the listbox, and for the assign/delete buttons
long _SvxMacroTabPage::GenericHandler_Impl( _SvxMacroTabPage* pThis, PushButton* pBtn )
{
    _SvxMacroTabPage_Impl*    pImpl = pThis->mpImpl;
    SvHeaderTabListBox& rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.FirstSelected();
    sal_uLong nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const sal_Bool bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

    ::rtl::OUString* pEventName = (::rtl::OUString*)pE->GetUserData();

    ::rtl::OUString sEventURL;
    ::rtl::OUString sEventType;
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

    bool bDoubleClick = (pBtn == NULL);
    bool bUNOAssigned = (sEventURL.indexOf( aVndSunStarUNO ) == 0);
    if( pBtn == pImpl->pDeletePB )
    {
        // delete pressed
        sEventType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Script") );
        sEventURL = ::rtl::OUString();
        if(!pThis->bAppEvents)
            pThis->bDocModified = true;
    }
    else if (   (   ( pBtn != NULL )
                &&  ( pBtn == pImpl->pAssignComponentPB )
                )
            ||  (   bDoubleClick
                &&  bUNOAssigned
                )
            )
    {
        AssignComponentDialog* pAssignDlg = new AssignComponentDialog( pThis, sEventURL );

        short ret = pAssignDlg->Execute();
        if( ret )
        {
            sEventType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UNO"));
            sEventURL = pAssignDlg->getURL();
            if(!pThis->bAppEvents)
                pThis->bDocModified = true;
        }
        delete pAssignDlg;
    }
    else if( bAssEnabled )
    {
        // assign pressed
        SvxScriptSelectorDialog* pDlg = new SvxScriptSelectorDialog( pThis, sal_False, pThis->GetFrame() );
        if( pDlg )
        {
            short ret = pDlg->Execute();
            if ( ret )
            {
                sEventType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Script"));
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
    pImpl->pEventLB->SetUpdateMode( sal_False );
    pE->ReplaceItem( new IconLBoxString( pE, 0, sEventURL,
            pImpl->pMacroImg, pImpl->pComponentImg ), LB_MACROS_ITEMPOS );

    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( sal_True );

    pThis->EnableButtons();
    return 0;
}

// pass in the XNameReplace.
// can remove the 3rd arg once issue ?? is fixed
void _SvxMacroTabPage::InitAndSetHandler( Reference< container::XNameReplace> xAppEvents, Reference< container::XNameReplace> xDocEvents, Reference< util::XModifiable > xModifiable )
{
    m_xAppEvents = xAppEvents;
    m_xDocEvents = xDocEvents;
    m_xModifiable = xModifiable;
    SvHeaderTabListBox&    rListBox = mpImpl->pEventLB->GetListBox();
    HeaderBar&            rHeaderBar = mpImpl->pEventLB->GetHeaderBar();
    Link                aLnk(STATIC_LINK(this, _SvxMacroTabPage, AssignDeleteHdl_Impl ));
    mpImpl->pDeletePB->SetClickHdl(    aLnk );
    mpImpl->pAssignPB->SetClickHdl(    aLnk );
    if( mpImpl->pAssignComponentPB )
        mpImpl->pAssignComponentPB->SetClickHdl( aLnk );
    rListBox.SetDoubleClickHdl( STATIC_LINK(this, _SvxMacroTabPage, DoubleClickHdl_Impl ) );

    rListBox.SetSelectHdl( STATIC_LINK( this, _SvxMacroTabPage, SelectEvent_Impl ));

    rListBox.SetSelectionMode( SINGLE_SELECTION );
    rListBox.SetTabs( &nTabs[0], MAP_APPFONT );
    Size aSize( nTabs[ 2 ], 0 );
    rHeaderBar.InsertItem( ITEMID_EVENT, *mpImpl->pStrEvent, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    aSize.Width() = 1764;        // don't know what, so 42^2 is best to use...
    rHeaderBar.InsertItem( ITMEID_ASSMACRO, *mpImpl->pAssignedMacro, LogicToPixel( aSize, MapMode( MAP_APPFONT ) ).Width() );
    rListBox.SetSpaceBetweenEntries( 0 );

    mpImpl->pEventLB->Show();
    mpImpl->pEventLB->ConnectElements();

    long nMinLineHeight = mpImpl->pMacroImg->GetSizePixel().Height() + 2;
    if( nMinLineHeight > mpImpl->pEventLB->GetListBox().GetEntryHeight() )
        mpImpl->pEventLB->GetListBox().SetEntryHeight(
            sal::static_int_cast< short >(nMinLineHeight) );

    mpImpl->pEventLB->Enable( sal_True );

    if(!m_xAppEvents.is())
    {
        return;
    }
    Sequence< ::rtl::OUString > eventNames = m_xAppEvents->getElementNames();
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
Any _SvxMacroTabPage::GetPropsByName( const ::rtl::OUString& eventName, EventsHash& eventsHash )
{
    const ::std::pair< ::rtl::OUString, ::rtl::OUString >& rAssignedEvent( eventsHash[ eventName ] );

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
::std::pair< ::rtl::OUString, ::rtl::OUString  > _SvxMacroTabPage::GetPairFromAny( Any aAny )
{
    Sequence< beans::PropertyValue > props;
    ::rtl::OUString type, url;
    if( sal_True == ( aAny >>= props ) )
    {
        ::comphelper::NamedValueCollection aProps( props );
        type = aProps.getOrDefault( "EventType", type );
        url = aProps.getOrDefault( "Script", url );
    }
    return ::std::make_pair( type, url );
}

SvxMacroTabPage::SvxMacroTabPage( Window* pParent, const Reference< frame::XFrame >& _rxDocumentFrame, const SfxItemSet& rSet, Reference< container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex )
    : _SvxMacroTabPage( pParent, CUI_RES( RID_SVXPAGE_MACROASSIGN ), rSet )
{
    mpImpl->pStrEvent           = new String(                       CUI_RES( STR_EVENT ) );
    mpImpl->pAssignedMacro      = new String(                       CUI_RES( STR_ASSMACRO ) );
    mpImpl->pEventLB            = new _HeaderTabListBox( this,      CUI_RES( LB_EVENT ) );
    mpImpl->pAssignFT           = new FixedText( this,              CUI_RES( FT_ASSIGN ) );
    mpImpl->pAssignPB           = new PushButton( this,             CUI_RES( PB_ASSIGN ) );
    mpImpl->pDeletePB           = new PushButton( this,             CUI_RES( PB_DELETE ) );
    mpImpl->pAssignComponentPB  = new PushButton( this,         CUI_RES( PB_ASSIGN_COMPONENT ) );
    mpImpl->pMacroImg           = new Image(                        CUI_RES(IMG_MACRO) );
    mpImpl->pComponentImg       = new Image(                        CUI_RES(IMG_COMPONENT) );

    FreeResource();

    SetFrame( _rxDocumentFrame );

    if( !mpImpl->bIDEDialogMode )
    {
        Point aPosAssign = mpImpl->pAssignPB->GetPosPixel();
        Point aPosComp = mpImpl->pAssignComponentPB->GetPosPixel();

        Point aPosDelete = mpImpl->pDeletePB->GetPosPixel();
        long nYDiff = aPosComp.Y() - aPosAssign.Y();
        aPosDelete.Y() -= nYDiff;
        mpImpl->pDeletePB->SetPosPixel( aPosDelete );

        mpImpl->pAssignComponentPB->Hide();
        mpImpl->pAssignComponentPB->Disable();
    }

    // must be done after FreeResource is called
    InitResources();

    mpImpl->pEventLB->GetListBox().SetHelpId( HID_SVX_MACRO_LB_EVENT );

    InitAndSetHandler( xNameReplace, Reference< container::XNameReplace>(0), Reference< util::XModifiable >(0));
    DisplayAppEvents(true);
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.GetEntry( (sal_uLong)nSelectedIndex );
    if( pE )
        rListBox.Select(pE);
}

SvxMacroTabPage::~SvxMacroTabPage()
{
}

SvxMacroAssignDlg::SvxMacroAssignDlg( Window* pParent, const Reference< frame::XFrame >& _rxDocumentFrame, const SfxItemSet& rSet,
    const Reference< container::XNameReplace >& xNameReplace, sal_uInt16 nSelectedIndex )
        : SvxMacroAssignSingleTabDialog( pParent, rSet, 0 )
{
    SetTabPage( new SvxMacroTabPage( this, _rxDocumentFrame, rSet, xNameReplace, nSelectedIndex ) );
}

SvxMacroAssignDlg::~SvxMacroAssignDlg()
{
}


//===============================================

IMPL_LINK_NOARG(AssignComponentDialog, ButtonHandler)
{
    ::rtl::OUString aMethodName = maMethodEdit.GetText();
    maURL = ::rtl::OUString();
    if( !aMethodName.isEmpty() )
    {
        maURL = aVndSunStarUNO;
        maURL += aMethodName;
    }
    EndDialog(1);
    return 0;
}

AssignComponentDialog::AssignComponentDialog( Window * pParent, const ::rtl::OUString& rURL )
    : ModalDialog( pParent, CUI_RES( RID_SVXDLG_ASSIGNCOMPONENT ) )
    , maMethodLabel( this, CUI_RES( FT_METHOD ) )
    , maMethodEdit( this, CUI_RES( EDIT_METHOD ) )
    , maOKButton( this, CUI_RES( RID_PB_OK ) )
    , maCancelButton( this, CUI_RES( RID_PB_CANCEL ) )
    , maHelpButton( this, CUI_RES( RID_PB_HELP ) )
    , maURL( rURL )
{
    FreeResource();
    maOKButton.SetClickHdl(LINK(this, AssignComponentDialog, ButtonHandler));

    ::rtl::OUString aMethodName;
    if( !maURL.isEmpty() )
    {
        sal_Int32 nIndex = maURL.indexOf( aVndSunStarUNO );
        if( nIndex == 0 )
        {
            sal_Int32 nBegin = aVndSunStarUNO.getLength();
            aMethodName = maURL.copy( nBegin );
        }
    }
    maMethodEdit.SetText( aMethodName, Selection( 0, SELECTION_MAX ) );
}

AssignComponentDialog::~AssignComponentDialog()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxMacroAssignSingleTabDialog, OKHdl_Impl, Button *, pButton )
{
    (void)pButton; //unused
    pPage->FillItemSet( *pOutSet );
    EndDialog( RET_OK );
    return 0;
}

// -----------------------------------------------------------------------

SvxMacroAssignSingleTabDialog::SvxMacroAssignSingleTabDialog
    ( Window *pParent, const SfxItemSet& rSet, sal_uInt16 nUniqueId ) :
        SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),
        pFixedLine      ( 0 ),
        pOKBtn          ( 0 ),
        pCancelBtn      ( 0 ),
        pHelpBtn        ( 0 ),
        pPage           ( 0 ),
        pOptions        ( &rSet ),
        pOutSet         ( 0 )
{}


// -----------------------------------------------------------------------

SvxMacroAssignSingleTabDialog::~SvxMacroAssignSingleTabDialog()
{
    delete pFixedLine;
    delete pOKBtn;
    delete pCancelBtn;
    delete pHelpBtn;
    delete pPage;
}

// -----------------------------------------------------------------------

// According to SfxSingleTabDialog
void SvxMacroAssignSingleTabDialog::SetTabPage( SfxTabPage* pTabPage )
{
    pFixedLine = new FixedLine( this );

    pOKBtn = new OKButton( this, WB_DEFBUTTON );
    pOKBtn->SetClickHdl( LINK( this, SvxMacroAssignSingleTabDialog, OKHdl_Impl ) );

    pCancelBtn = new CancelButton( this );
    pHelpBtn = new HelpButton( this );

    pPage = pTabPage;

    if ( pPage )
    {
        String sUserData;
        pPage->SetUserData( sUserData );
        pPage->Reset( *pOptions );
        pPage->Show();

        // Set dialog's and buttons' size and position according to tabpage size
        long nSpaceX = LogicToPixel( Size( 6, 0 ), MAP_APPFONT ).Width();
        long nSpaceY = LogicToPixel( Size( 0, 6 ), MAP_APPFONT ).Height();
        long nHalfSpaceX = LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();
        long nHalfSpaceY = LogicToPixel( Size( 0, 3 ), MAP_APPFONT ).Height();

        pPage->SetPosPixel( Point() );
        Size aTabpageSize( pPage->GetSizePixel() );
        Size aDialogSize( aTabpageSize );
        Size aButtonSize = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
        long nButtonWidth  = aButtonSize.Width();
        long nButtonHeight = aButtonSize.Height();

        Size aFixedLineSize( aTabpageSize );
        long nFixedLineHeight = LogicToPixel( Size( 0, 8 ), MAP_APPFONT ).Height();
        aFixedLineSize.Height() = nFixedLineHeight;

        aDialogSize.Height() += nFixedLineHeight + nButtonHeight + nSpaceY + nHalfSpaceY;
        SetOutputSizePixel( aDialogSize );

        long nButtonPosY = aTabpageSize.Height() + nFixedLineHeight + nHalfSpaceY;
        long nHelpButtonPosX = nSpaceX;
        pHelpBtn->SetPosSizePixel( Point( nHelpButtonPosX, nButtonPosY), aButtonSize );
        pHelpBtn->Show();

        long nCancelButtonPosX = aDialogSize.Width() - nButtonWidth - nSpaceX + 1;
        pCancelBtn->SetPosSizePixel( Point( nCancelButtonPosX, nButtonPosY), aButtonSize );
        pCancelBtn->Show();

        long nOkButtonPosX = nCancelButtonPosX - nButtonWidth - nHalfSpaceX;
        pOKBtn->SetPosSizePixel( Point( nOkButtonPosX, nButtonPosY), aButtonSize );
        pOKBtn->Show();

        long nFixedLinePosY = aTabpageSize.Height();
        pFixedLine->SetPosSizePixel( Point( 0, nFixedLinePosY), aFixedLineSize );
        pFixedLine->Show();

        // Get text from TabPage
        SetText( pPage->GetText() );

        // Get IDs from TabPage
        SetHelpId( pPage->GetHelpId() );
        SetUniqueId( pPage->GetUniqueId() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
