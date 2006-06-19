/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macropg.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 15:18:24 $
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

#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif

#define ITEMID_MACRO 0
#include "macropg.hxx"
#undef ITEMID_MACRO

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/macrconf.hxx>
#include <sfx2/sfxdefs.hxx>

#ifndef _HEADERTABLISTBOX_HXX
#include "headertablistbox.hxx"
#endif
#ifndef _MACROPG_IMPL_HXX
#include "macropg_impl.hxx"
#endif

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "dialmgr.hxx"
#include "selector.hxx"
#include "cfg.hxx"
#include "macropg.hrc"
#include "helpid.hrc"
#include "dialogs.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

_SvxMacroTabPage_Impl::_SvxMacroTabPage_Impl( void ) :
    pAssignPB( NULL ),
    pDeletePB( NULL ),
    pStrEvent( NULL ),
    pAssignedMacro( NULL ),
    pEventLB( NULL ),
    bReadOnly( FALSE )
{
}

_SvxMacroTabPage_Impl::~_SvxMacroTabPage_Impl()
{
    delete pAssignPB;
    delete pDeletePB;
    delete pStrEvent;
    delete pAssignedMacro;
    delete pEventLB;
}

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs[] =
    {
        2, // Number of Tabs
        0, 90
    };

#define TAB_WIDTH_MIN        10

// IDs for items in HeaderBar of EventLB
#define    ITEMID_EVENT        1
#define    ITMEID_ASSMACRO        2


#define LB_EVENTS_ITEMPOS    1
#define LB_MACROS_ITEMPOS    2


IMPL_LINK( _HeaderTabListBox, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    DBG_ASSERT( pBar == &maHeaderBar, "*_HeaderTabListBox::HeaderEndDrag_Impl: something is wrong here..." );

    if( !maHeaderBar.GetCurItemId() )
        return 0;

    if( !maHeaderBar.IsItemMode() )
    {
        Size    aSz;
        USHORT    _nTabs = maHeaderBar.GetItemCount();
        long    nTmpSz = 0;
        long    nWidth = maHeaderBar.GetItemSize( ITEMID_EVENT );
        long    nBarWidth = maHeaderBar.GetSizePixel().Width();

        if( nWidth < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, TAB_WIDTH_MIN );
        else if( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, nBarWidth - TAB_WIDTH_MIN );

        {
            long    _nWidth;
            for( USHORT i = 1 ; i < _nTabs ; ++i )
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
    maHeaderBar( this, WB_BOTTOMBORDER )
{
//  maListBox.SetWindowBits( WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
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

void _HeaderTabListBox::Show( BOOL bVisible, USHORT nFlags )
{
    maListBox.Show( bVisible, nFlags );
    maHeaderBar.Show( bVisible, nFlags );
}

void _HeaderTabListBox::Enable( BOOL bEnable, BOOL bChild )
{
    maListBox.Enable( bEnable, bChild );
    maHeaderBar.Enable( bEnable, bChild );
}

// assign button ("Add Command") is enabled only if it is not read only
// delete button ("Remove Command") is enabled if a current binding exists
//     and it is not read only
void _SvxMacroTabPage::EnableButtons( const String& /*rLangName*/ )
{
    const SvLBoxEntry* pE = mpImpl->pEventLB->GetListBox().FirstSelected();
    if ( pE )
    {
        SvLBoxString* pEventMacro = (SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS );
        mpImpl->pDeletePB->Enable( 0 != pEventMacro && !mpImpl->bReadOnly );

        mpImpl->pAssignPB->Enable( !mpImpl->bReadOnly );
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
    mpImpl = new _SvxMacroTabPage_Impl;
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
    aUIStrings.clear();
    DELETEZ( mpImpl );
}
// -----------------------------------------------------------------------------
void _SvxMacroTabPage::InitResources()
{
    // the event name to UI string mappings for App Events
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnStartApp")),String( SVX_RES( RID_SVXSTR_EVENT_STARTAPP ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnCloseApp")),String( SVX_RES( RID_SVXSTR_EVENT_CLOSEAPP ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnNew")),String( SVX_RES( RID_SVXSTR_EVENT_CREATEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnUnload")),String( SVX_RES( RID_SVXSTR_EVENT_CLOSEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnPrepareUnload")),String( SVX_RES( RID_SVXSTR_EVENT_PREPARECLOSEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnLoad")),String( SVX_RES( RID_SVXSTR_EVENT_OPENDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSave")),String( SVX_RES( RID_SVXSTR_EVENT_SAVEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSaveAs")),String( SVX_RES( RID_SVXSTR_EVENT_SAVEASDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSaveDone")),String( SVX_RES( RID_SVXSTR_EVENT_SAVEDOCDONE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnSaveAsDone")),String( SVX_RES( RID_SVXSTR_EVENT_SAVEASDOCDONE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnFocus")),String( SVX_RES( RID_SVXSTR_EVENT_ACTIVATEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnUnfocus")),String( SVX_RES( RID_SVXSTR_EVENT_DEACTIVATEDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnPrint")),String( SVX_RES( RID_SVXSTR_EVENT_PRINTDOC ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnModifyChanged")),String( SVX_RES( RID_SVXSTR_EVENT_MODIFYCHANGED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnMailMerge")),String( SVX_RES( RID_SVXSTR_EVENT_MAILMERGE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnPageCountChange")),String( SVX_RES( RID_SVXSTR_EVENT_PAGECOUNTCHANGE ))));

    // the event name to UI string mappings for forms & dialogs
    //
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveAction")),String( SVX_RES( RID_SVXSTR_EVENT_APPROVEACTIONPERFORMED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("actionPerformed")),String( SVX_RES( RID_SVXSTR_EVENT_ACTIONPERFORMED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("changed")),String( SVX_RES( RID_SVXSTR_EVENT_CHANGED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("textChanged")),String( SVX_RES( RID_SVXSTR_EVENT_TEXTCHANGED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("itemStateChanged")),String( SVX_RES( RID_SVXSTR_EVENT_ITEMSTATECHANGED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("focusGained")),String( SVX_RES( RID_SVXSTR_EVENT_FOCUSGAINED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("focusLost")),String( SVX_RES( RID_SVXSTR_EVENT_FOCUSLOST ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("keyPressed")),String( SVX_RES( RID_SVXSTR_EVENT_KEYTYPED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("keyReleased")),String( SVX_RES( RID_SVXSTR_EVENT_KEYUP ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mouseEntered")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSEENTERED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mouseDragged")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSEDRAGGED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mouseMoved")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSEMOVED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mousePressed")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSEPRESSED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mouseReleased")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSERELEASED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mouseExited")),String( SVX_RES( RID_SVXSTR_EVENT_MOUSEEXITED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveReset")),String( SVX_RES( RID_SVXSTR_EVENT_APPROVERESETTED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("resetted")),String( SVX_RES( RID_SVXSTR_EVENT_RESETTED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveSubmit")),String( SVX_RES( RID_SVXSTR_EVENT_SUBMITTED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveUpdate")),String( SVX_RES( RID_SVXSTR_EVENT_BEFOREUPDATE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("updated")),String( SVX_RES( RID_SVXSTR_EVENT_AFTERUPDATE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("loaded")),String( SVX_RES( RID_SVXSTR_EVENT_LOADED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("reloading")),String( SVX_RES( RID_SVXSTR_EVENT_RELOADING ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("reloaded")),String( SVX_RES( RID_SVXSTR_EVENT_RELOADED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unloading")),String( SVX_RES( RID_SVXSTR_EVENT_UNLOADING ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unloaded")),String( SVX_RES( RID_SVXSTR_EVENT_UNLOADED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("confirmDelete")),String( SVX_RES( RID_SVXSTR_EVENT_CONFIRMDELETE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveRowChange")),String( SVX_RES( RID_SVXSTR_EVENT_APPROVEROWCHANGE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rowChanged")),String( SVX_RES( RID_SVXSTR_EVENT_ROWCHANGE ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveCursorMove")),String( SVX_RES( RID_SVXSTR_EVENT_POSITIONING ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cursorMoved")),String( SVX_RES( RID_SVXSTR_EVENT_POSITIONED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("approveParameter")),String( SVX_RES( RID_SVXSTR_EVENT_APPROVEPARAMETER ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("errorOccured")),String( SVX_RES( RID_SVXSTR_EVENT_ERROROCCURED ))));
    aUIStrings.insert(UIEventsStringHash::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("adjustmentValueChanged")),String( SVX_RES( RID_SVXSTR_EVENT_ADJUSTMENTVALUECHANGED ))));
}

// the following method is called when the user clicks OK
// We use the contents of the hashes to replace the settings
BOOL _SvxMacroTabPage::FillItemSet( SfxItemSet& /*rSet*/ )
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
                catch(Exception e)
                {}
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
                catch(Exception e)
                {}
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
    catch(Exception&)
    {
    }
    // what is the return value about??
    return FALSE;
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
            Sequence< beans::PropertyValue > emptyProps(2);
            ::rtl::OUString sEmpty;
            emptyProps[0].Name = ::rtl::OUString::createFromAscii("EventType");
            emptyProps[0].Value <<= ::rtl::OUString::createFromAscii("Script");
            emptyProps[1].Name = ::rtl::OUString::createFromAscii("Script");
            emptyProps[1].Value <<= sEmpty;
            Any aEmptyProps;
            aEmptyProps <<= emptyProps;
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
    catch(Exception&)
    {
    }
    DisplayAppEvents(bAppEvents);
}

void _SvxMacroTabPage::SetReadOnly( BOOL bSet )
{
    mpImpl->bReadOnly = bSet;
}

BOOL _SvxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

// displays the app events if appEvents=true, otherwise displays the doc events
void _SvxMacroTabPage::DisplayAppEvents( bool appEvents)
{
    bAppEvents = appEvents;

    SvHeaderTabListBox&        rListBox = mpImpl->pEventLB->GetListBox();
    mpImpl->pEventLB->SetUpdateMode( FALSE );
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
    sal_Int32 nEventCount = eventNames.getLength();
    for(sal_Int32 nEvent = 0; nEvent < nEventCount; ++nEvent )
    {
        EventsHash::iterator h_it = eventsHash->find( eventNames[nEvent] );
        if( h_it == eventsHash->end() )
            continue;
        ::rtl::OUString eventName = h_it->first;
        ::rtl::OUString eventURL = h_it->second.second;
        //DF better to add the L10N version
        // might also have a hash between event name & L10N display name
        //for some reason the hash seems now to be empty?? Why??
        UIEventsStringHash::iterator ui_it = aUIStrings.find(eventName);
        String sTmp;
        if( ui_it != aUIStrings.end() )
        {
            // we have the L10N string
            sTmp = ui_it->second;
        }
        else
        {
            //no UI name => do not add
            continue;
        }
        sTmp += '\t';
        SvLBoxEntry*    _pE = rListBox.InsertEntry( sTmp );
        ::rtl::OUString* pEventName = new ::rtl::OUString( eventName );
        _pE->SetUserData( (void*)pEventName );
        String sNew( eventURL );
        _pE->ReplaceItem( new SvLBoxString( _pE, 0, sNew ), LB_MACROS_ITEMPOS );
        rListBox.GetModel()->InvalidateEntry( _pE );
        rListBox.Select( _pE );
        rListBox.MakeVisible( _pE );
    }
    pE = rListBox.GetEntry(0);
    if( pE )
        rListBox.Select( pE );
    rListBox.SetUpdateMode( TRUE );
    EnableButtons( String() );
}

// select event handler on the listbox
IMPL_STATIC_LINK( _SvxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    _SvxMacroTabPage_Impl*    pImpl = pThis->mpImpl;
    SvHeaderTabListBox&        rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry*            pE = rListBox.FirstSelected();
    ULONG                    nPos;

    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    pThis->EnableButtons( String() );
    return 0;
}

// handler for double click on the listbox, and for the assign/delete buttons
IMPL_STATIC_LINK( _SvxMacroTabPage, AssignDeleteHdl_Impl, PushButton*, pBtn )
{
    _SvxMacroTabPage_Impl*    pImpl = pThis->mpImpl;
    SvHeaderTabListBox& rListBox = pImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.FirstSelected();
    ULONG nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = rListBox.GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const BOOL bAssEnabled = pBtn != pImpl->pDeletePB && pImpl->pAssignPB->IsEnabled();

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
    if( pBtn == pImpl->pDeletePB )
    {
        // delete pressed
        sEventType = ::rtl::OUString::createFromAscii("Script");
        sEventURL = ::rtl::OUString();
        if(!pThis->bAppEvents)
                pThis->bDocModified = true;
    }
    else if( bAssEnabled )
    {
        // assign pressed
        SvxScriptSelectorDialog* pDlg = new SvxScriptSelectorDialog( pThis, FALSE );
        if( pDlg )
        {
            short ret = pDlg->Execute();
            if ( ret )
            {
                sEventType = ::rtl::OUString::createFromAscii("Script");
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
    pImpl->pEventLB->SetUpdateMode( FALSE );
    pE->ReplaceItem( new SvLBoxString( pE, 0, sEventURL ), LB_MACROS_ITEMPOS );
    rListBox.GetModel()->InvalidateEntry( pE );
    rListBox.Select( pE );
    rListBox.MakeVisible( pE );
    rListBox.SetUpdateMode( TRUE );

    pThis->EnableButtons( String() );
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
    rListBox.SetDoubleClickHdl( aLnk );

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

    mpImpl->pEventLB->Enable( TRUE );

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
        catch (Exception e)
        {}
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
            catch (Exception e)
            {}
        }
    }
}

// returns the two props EventType & Script for a given event name
Any _SvxMacroTabPage::GetPropsByName( const ::rtl::OUString& eventName, const EventsHash& eventsHash )
{
    Any ret;

    Sequence< beans::PropertyValue > props(2);
    props[0].Name = ::rtl::OUString::createFromAscii("EventType");
    props[0].Value <<= ::rtl::OUString::createFromAscii("Script");
    props[1].Name = ::rtl::OUString::createFromAscii("Script");
    ::rtl::OUString sTmp;
    const EventsHash::const_iterator h_it = eventsHash.find( eventName );
    sTmp = h_it->second.second;
    props[1].Value <<= sTmp;

    ret <<= props;
    return ret;
}

// converts the Any returned by GetByName into a pair which can be stored in
// the EventHash
::std::pair< ::rtl::OUString, ::rtl::OUString  > _SvxMacroTabPage::GetPairFromAny( Any aAny )
{
    Sequence< beans::PropertyValue > props;
    ::rtl::OUString type, url;
    if( sal_True == ( aAny >>= props ) )
    {
        sal_Int32 nCount = props.getLength();
        for( sal_Int32 index = 0; index < nCount ; ++index )
        {
            if ( props[ index ].Name.compareToAscii( "EventType" ) == 0 )
                props[ index ].Value >>= type;
            else if ( props[ index ].Name.compareToAscii( "Script" ) == 0 )
                props[ index ].Value >>= url;
        }
    }
    return ::std::make_pair( type, url );
}

SvxMacroTabPage::SvxMacroTabPage( Window* pParent, const ResId& rResId, const SfxItemSet& rSet, Reference< container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex )
    : _SvxMacroTabPage( pParent, rResId, rSet )
{
    mpImpl->pStrEvent            = new String(                    SVX_RES( STR_EVENT ) );
    mpImpl->pAssignedMacro        = new String(                    SVX_RES( STR_ASSMACRO ) );
    mpImpl->pEventLB            = new _HeaderTabListBox( this,    SVX_RES( LB_EVENT ) );
    mpImpl->pAssignPB            = new PushButton( this,            SVX_RES( PB_ASSIGN ) );
    mpImpl->pDeletePB            = new PushButton( this,            SVX_RES( PB_DELETE ) );

    FreeResource();

    // must be done after FreeResource is called
    InitResources();

    mpImpl->pEventLB->GetListBox().SetHelpId( HID_SVX_MACRO_LB_EVENT );

    InitAndSetHandler( xNameReplace, Reference< container::XNameReplace>(0), Reference< util::XModifiable >(0));
    DisplayAppEvents(true);
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.GetEntry( (ULONG)nSelectedIndex );
    if( pE )
        rListBox.Select(pE);
}

SvxMacroTabPage::~SvxMacroTabPage()
{
}

SfxTabPage* SvxMacroTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet, Reference< container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex )
{
    return new SvxMacroTabPage( pParent, SVX_RES( RID_SVXPAGE_MACROASSIGN ), rAttrSet, xNameReplace, nSelectedIndex );
}

SvxMacroAssignDlg::SvxMacroAssignDlg( Window* pParent, SfxItemSet& rSet, Reference< container::XNameReplace > xNameReplace, sal_uInt16 nSelectedIndex  )
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    SetTabPage( SvxMacroTabPage::Create( this, rSet, xNameReplace, nSelectedIndex ) );
}

SvxMacroAssignDlg::~SvxMacroAssignDlg()
{
}

