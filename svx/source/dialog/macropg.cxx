/*************************************************************************
 *
 *  $RCSfile: macropg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:12:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef GCC
#pragma hdrstop
#endif

#define ITEMID_MACRO 0
#include "macropg.hxx"
#undef ITEMID_MACRO

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#include "cfg.hxx"
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include "macropg.hrc"
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
#include "dialogs.hrc"
#include "selector.hxx"

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


SvStringsDtor* _ImpGetRangeHdl( _SvxMacroTabPage*, const String& rLanguage );
SvStringsDtor* _ImpGetMacrosOfRangeHdl( _SvxMacroTabPage*, const String& rLanguage, const String& rRange );

static USHORT __FAR_DATA aPageRg[] = {
    SID_ATTR_MACROITEM, SID_ATTR_MACROITEM,
    0
};

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
        USHORT    nTabs = maHeaderBar.GetItemCount();
        long    nTmpSz = 0;
        long    nWidth = maHeaderBar.GetItemSize( ITEMID_EVENT );
        long    nBarWidth = maHeaderBar.GetSizePixel().Width();

        if( nWidth < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, TAB_WIDTH_MIN );
        else if( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            maHeaderBar.SetItemSize( ITEMID_EVENT, nBarWidth - TAB_WIDTH_MIN );

        {
            long    nWidth;
            for( USHORT i = 1 ; i < nTabs ; ++i )
            {
                nWidth = maHeaderBar.GetItemSize( i );
                aSz.Width() =  nWidth + nTmpSz;
                nTmpSz += nWidth;
                maListBox.SetTab( i, PixelToLogic( aSz, MapMode( MAP_APPFONT ) ).Width(), MAP_APPFONT );
            }
        }
    }
    return 1;
}

long _HeaderTabListBox::Notify( NotifyEvent& rNEvt )
{
    long    nRet = Control::Notify( rNEvt );

    if( rNEvt.GetWindow() != &maListBox && rNEvt.GetType() == EVENT_GETFOCUS )
        maListBox.GrabFocus();

    return nRet;
}

_HeaderTabListBox::_HeaderTabListBox( Window* pParent, const ResId& rId ) :
    Control( pParent, rId ),

    maListBox( this, WB_HSCROLL | WB_CLIPCHILDREN ),
    maHeaderBar( this, WB_BOTTOMBORDER )
{
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
void _SvxMacroTabPage::EnableButtons( const String& rLangName )
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

    OSL_TRACE("test event string is %s",::rtl::OUStringToOString( ::rtl::OUString(SVX_RES(RID_SVXSTR_EVENT_STARTAPP)), RTL_TEXTENCODING_ASCII_US ).pData->buffer);
    // the event name to UI string mappings for App Events
    aUIStrings[ ::rtl::OUString::createFromAscii("OnStartApp") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_STARTAPP ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnCloseApp") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_CLOSEAPP ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnNew") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_CREATEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnUnload") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_CLOSEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnPrepareUnload") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_PREPARECLOSEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnLoad") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_OPENDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnSave") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_SAVEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnSaveAs") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_SAVEASDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnSaveDone") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_SAVEDOCDONE ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnSaveAsDone") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_SAVEASDOCDONE ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnFocus") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_ACTIVATEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnUnfocus") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_DEACTIVATEDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnPrint") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_PRINTDOC ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnModifyChanged") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_MODIFYCHANGED ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnMailMerge") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_MAILMERGE ));
    aUIStrings[ ::rtl::OUString::createFromAscii("OnPageCountChange") ] = new ::rtl::OUString( SVX_RES( RID_SVXSTR_EVENT_PAGECOUNTCHANGE ));

    // the event name to UI string mappings for forms & dialogs
    //
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveAction" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_APPROVEACTIONPERFORMED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "actionPerformed" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_ACTIONPERFORMED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "changed" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_CHANGED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "textChanged" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_TEXTCHANGED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "itemStateChanged" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_ITEMSTATECHANGED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "focusGained" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_FOCUSGAINED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "focusLost" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_FOCUSLOST ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "keyPressed" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_KEYTYPED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "keyReleased" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_KEYUP ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mouseEntered" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSEENTERED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mouseDragged" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSEDRAGGED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mouseMoved" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSEMOVED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mousePressed" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSEPRESSED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mouseReleased" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSERELEASED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "mouseExited" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_MOUSEEXITED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveReset" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_APPROVERESETTED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "resetted" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_RESETTED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveSubmit" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_SUBMITTED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveUpdate" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_BEFOREUPDATE ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "updated" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_AFTERUPDATE ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "loaded" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_LOADED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "reloading" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_RELOADING ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "reloaded" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_RELOADED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "unloading" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_UNLOADING ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "unloaded" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_UNLOADED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "confirmDelete" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_CONFIRMDELETE ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveRowChange" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_APPROVEROWCHANGE ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "rowChanged" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_ROWCHANGE ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveCursorMove" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_POSITIONING ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "cursorMoved" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_POSITIONED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "approveParameter" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_APPROVEPARAMETER ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "errorOccured" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_ERROROCCURED ));
    aUIStrings[ ::rtl::OUString::createFromAscii( "adjustmentValueChanged" ) ] = new ::rtl::OUString( ResId( RID_SVXSTR_EVENT_ADJUSTMENTVALUECHANGED ));

    UIEventsStringHash::iterator ui_it = aUIStrings.begin();
    UIEventsStringHash::iterator ui_it_end = aUIStrings.end();
    for(;ui_it!=ui_it_end;++ui_it)
    {
        OSL_TRACE("event string %s",::rtl::OUStringToOString( *(ui_it->second), RTL_TEXTENCODING_ASCII_US ).pData->buffer);
    }
    OSL_TRACE("hash size %d",aUIStrings.size());
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

// the following method is called when the user clicks OK
// We use the contents of the hashes to replace the settings
BOOL _SvxMacroTabPage::FillItemSet( SfxItemSet& rSet )
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
            emptyProps[0].Name = ::rtl::OUString::createFromAscii("EventType");
            emptyProps[0].Value <<= ::rtl::OUString::createFromAscii("Script");
            emptyProps[1].Name = ::rtl::OUString::createFromAscii("Script");
            emptyProps[1].Value <<= ::rtl::OUString::createFromAscii("");
            Any aEmptyProps;
            aEmptyProps <<= emptyProps;
            ::rtl::OUString eventName;
            if( m_xAppEvents.is() )
            {
                EventsHash::iterator h_itEnd =  m_appEventsHash.end();
                EventsHash::iterator h_it = m_appEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second = ::rtl::OUString::createFromAscii("");
                }
            }
            if( m_xDocEvents.is() && bDocModified )
            {
                EventsHash::iterator h_itEnd =  m_docEventsHash.end();
                EventsHash::iterator h_it = m_docEventsHash.begin();
                for ( ; h_it !=  h_itEnd; ++h_it )
                {
                    h_it->second.second = ::rtl::OUString::createFromAscii("");
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
    OSL_TRACE("in _SvxMacroTabPage::SetReadOnly");
    mpImpl->bReadOnly = bSet;
}

BOOL _SvxMacroTabPage::IsReadOnly() const
{
    return mpImpl->bReadOnly;
}

// displays the app events if appEvents=true, otherwise displays the doc events
void _SvxMacroTabPage::DisplayAppEvents( bool appEvents)
{
    OSL_TRACE("in _SvxMacroTabPage::DisplayAppEvents");
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
        OSL_TRACE("no XnameReplace");
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
        OSL_TRACE("getting ui event string for %s",::rtl::OUStringToOString( eventName, RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        //for some reason the hash seems now to be empty?? Why??
        UIEventsStringHash::iterator ui_it = aUIStrings.find(eventName);
        /*UIEventsStringHash::iterator ui_it = aUIStrings.begin();
        UIEventsStringHash::iterator ui_it_end = aUIStrings.end();
        if(ui_it==ui_it_end)
            OSL_TRACE("mappings gone!!!");
        OSL_TRACE("hash size %d",aUIStrings.size());

        for(;ui_it!=ui_it_end;++ui_it)
        {
            OSL_TRACE("event string %s",::rtl::OUStringToOString( *(ui_it->second), RTL_TEXTENCODING_ASCII_US ).pData->buffer);
            if(eventName.equals(*(ui_it->first)))
                break;
        }
        OSL_TRACE("never here?");*/
        String sTmp;
        if( ui_it != aUIStrings.end() )
        {
            OSL_TRACE("setting UI string");
            // we have the L10N string
            sTmp = String(*(ui_it->second));
        }
        else
        {
            //no UI name => do not add
            continue;
        }
        sTmp += '\t';
        SvLBoxEntry*    pE = rListBox.InsertEntry( sTmp );
        ::rtl::OUString* pEventName = new ::rtl::OUString( eventName );
        pE->SetUserData( (void*)pEventName );
        String sNew( eventURL );
        pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
        rListBox.GetModel()->InvalidateEntry( pE );
        rListBox.Select( pE );
        rListBox.MakeVisible( pE );
    }
    rListBox.Select( rListBox.GetEntry(0) );
    rListBox.SetUpdateMode( TRUE );
    EnableButtons( String() );
}

// select event handler on the listbox
IMPL_STATIC_LINK( _SvxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    OSL_TRACE("In _SvxMacroTabPage::SelectEvent_Impl");
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
    OSL_TRACE("In _SvxMacroTabPage::AssignDeleteHdl");
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
    OSL_TRACE("event name is %s",::rtl::OUStringToOString( *pEventName , RTL_TEXTENCODING_ASCII_US ).pData->buffer);

    ::rtl::OUString sEventURL;
    ::rtl::OUString sEventType;
    if(pThis->bAppEvents)
    {
        EventsHash::iterator h_it = pThis->m_appEventsHash.find( *pEventName );
        if(h_it != pThis->m_docEventsHash.end() )
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
        sEventURL = ::rtl::OUString::createFromAscii("");
        if(!pThis->bAppEvents)
                pThis->bDocModified = true;
    }
    else if( bAssEnabled )
    {
        // assign pressed
        OSL_TRACE("Launching Script Selector");
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
        OSL_TRACE("++++++++++ No app events!");
        return;
    }
    Sequence< ::rtl::OUString > eventNames = m_xAppEvents->getElementNames();
    sal_Int32 nEventCount = eventNames.getLength();
    OSL_TRACE("app event count is %d",nEventCount);
    for(sal_Int32 nEvent = 0; nEvent < nEventCount; ++nEvent )
    {
        OSL_TRACE("app event is %s",::rtl::OUStringToOString( eventNames[nEvent], RTL_TEXTENCODING_ASCII_US ).pData->buffer);
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

    InitAndSetHandler( xNameReplace, Reference< container::XNameReplace>(0), Reference< util::XModifiable >(0));
    DisplayAppEvents(true);
    SvHeaderTabListBox& rListBox = mpImpl->pEventLB->GetListBox();
    SvLBoxEntry* pE = rListBox.GetEntry( (ULONG)nSelectedIndex );
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
    OSL_TRACE("in SvxMacroAssignDlg::SvxMacroAssignDlg");
    SetTabPage( SvxMacroTabPage::Create( this, rSet, xNameReplace, nSelectedIndex ) );
}

SvxMacroAssignDlg::~SvxMacroAssignDlg()
{
}

