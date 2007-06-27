/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmartTagCtl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:41:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "SmartTagCtl.hxx"

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGACTION_HPP_
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#endif
#ifndef _COM_SUN_STAR_SMARTTAGS_XSTRINGKEYMAP_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif
#ifndef _STDMENU_HXX //autogen
#include <svtools/stdmenu.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#include "SmartTagItem.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SvxSmartTagsControl, SvxSmartTagItem);

//--------------------------------------------------------------------

SvxSmartTagsControl::SvxSmartTagsControl
(
    USHORT          _nId,
    Menu&           rMenu,
    SfxBindings&    /*rBindings*/
) :
    mpMenu  ( new PopupMenu ),
    mrParent    ( rMenu ),
    mpSmartTagItem( 0 )
{
    rMenu.SetPopupMenu( _nId, mpMenu );
}

//--------------------------------------------------------------------

const USHORT MN_ST_INSERT_START = 500;

void SvxSmartTagsControl::FillMenu()
{
    if ( !mpSmartTagItem )
        return;

    USHORT nMenuPos = 0;
    USHORT nSubMenuPos = 0;
    USHORT nMenuId = 1;
    USHORT nSubMenuId = MN_ST_INSERT_START;

    const Sequence < Sequence< Reference< smarttags::XSmartTagAction > > >& rActionComponentsSequence = mpSmartTagItem->GetActionComponentsSequence();
    const Sequence < Sequence< sal_Int32 > >& rActionIndicesSequence = mpSmartTagItem->GetActionIndicesSequence();
    const Sequence< Reference< container::XStringKeyMap > >& rStringKeyMaps = mpSmartTagItem->GetStringKeyMaps();
    const lang::Locale& rLocale = mpSmartTagItem->GetLocale();
    const rtl::OUString aApplicationName = mpSmartTagItem->GetApplicationName();
    const rtl::OUString aRangeText = mpSmartTagItem->GetRangeText();
    const Reference<text::XTextRange>& xTextRange = mpSmartTagItem->GetTextRange();
    const Reference<frame::XController>& xController = mpSmartTagItem->GetController();

    for ( USHORT j = 0; j < rActionComponentsSequence.getLength(); ++j )
    {
        Reference< container::XStringKeyMap > xSmartTagProperties = rStringKeyMaps[j];

        // Get all actions references associated with the current smart tag type:
        const Sequence< Reference< smarttags::XSmartTagAction > >& rActionComponents = rActionComponentsSequence[j];
        const Sequence< sal_Int32 >& rActionIndices = rActionIndicesSequence[j];

        if ( 0 == rActionComponents.getLength() || 0 == rActionIndices.getLength() )
            continue;

        // Ask first entry for the smart tag type caption:
        Reference< smarttags::XSmartTagAction > xAction = rActionComponents[0];

        if ( !xAction.is() )
            continue;

        const sal_Int32 nSmartTagIndex = rActionIndices[0];
        const rtl::OUString aSmartTagType = xAction->getSmartTagName( nSmartTagIndex );
        const rtl::OUString aSmartTagCaption = xAction->getSmartTagCaption( nSmartTagIndex, rLocale );

        // no sub-menues if there's only one smart tag type listed:
        PopupMenu* pSbMenu = mpMenu;
        if ( 1 < rActionComponentsSequence.getLength() )
        {
            mpMenu->InsertItem( nMenuId, aSmartTagCaption, 0, nMenuPos++);
            pSbMenu = new PopupMenu;
            mpMenu->SetPopupMenu( nMenuId++, pSbMenu );
        }
        pSbMenu->SetSelectHdl( LINK( this, SvxSmartTagsControl, MenuSelect ) );

        // sub-menu starts with smart tag caption and separator
        const rtl::OUString aSmartTagCaption2 = aSmartTagCaption + C2U(": ") + aRangeText;
        nSubMenuPos = 0;
        pSbMenu->InsertItem( nMenuId++, aSmartTagCaption2, MIB_NOSELECT, nSubMenuPos++ );
        pSbMenu->InsertSeparator( nSubMenuPos++ );

        // Add subitem for every action reference for the current smart tag type:
        for ( USHORT i = 0; i < rActionComponents.getLength(); ++i )
        {
            xAction = rActionComponents[i];

            for ( sal_Int32 k = 0; k < xAction->getActionCount( aSmartTagType, xController ); ++k )
            {
                const sal_uInt32 nActionID = xAction->getActionID( aSmartTagType, k, xController );
                rtl::OUString aActionCaption = xAction->getActionCaptionFromID( nActionID,
                                                                                aApplicationName,
                                                                                rLocale,
                                                                                xSmartTagProperties,
                                                                                aRangeText,
                                                                                rtl::OUString(),
                                                                                xController,
                                                                                xTextRange );

                pSbMenu->InsertItem( nSubMenuId++, aActionCaption, 0, nSubMenuPos++ );
                InvokeAction aEntry( xAction, xSmartTagProperties, nActionID );
                maInvokeActions.push_back( aEntry );
            }
        }
    }
}

//--------------------------------------------------------------------

void SvxSmartTagsControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* pState )

{
    mrParent.EnableItem( GetId(), SFX_ITEM_DISABLED != eState );

    if ( SFX_ITEM_AVAILABLE == eState )
    {
        const SvxSmartTagItem* pSmartTagItem = PTR_CAST( SvxSmartTagItem, pState );
        if ( 0 != pSmartTagItem )
        {
            delete mpSmartTagItem;
            mpSmartTagItem = new SvxSmartTagItem( *pSmartTagItem );
            FillMenu();
        }
    }
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxSmartTagsControl, MenuSelect, PopupMenu *, pMen )
{
    if ( !mpSmartTagItem )
        return 0;

    sal_uInt16 nMyId = pMen->GetCurItemId();

    if ( nMyId < MN_ST_INSERT_START) return 0;
    nMyId -= MN_ST_INSERT_START;

    // compute smarttag lib index and action index
    Reference< smarttags::XSmartTagAction > xSmartTagAction = maInvokeActions[ nMyId ].mxAction;

    // execute action
    if ( xSmartTagAction.is() )
    {
        xSmartTagAction->invokeAction( maInvokeActions[ nMyId ].mnActionID,
                                       mpSmartTagItem->GetApplicationName(),
                                       mpSmartTagItem->GetController(),
                                       mpSmartTagItem->GetTextRange(),
                                       maInvokeActions[ nMyId ].mxSmartTagProperties,
                                       mpSmartTagItem->GetRangeText(),
                                       rtl::OUString(),
                                       mpSmartTagItem->GetLocale() );
    }

    // ohne dispatcher!!!
    // GetBindings().Execute( GetId(), SFX_CALLMODE_RECORD,meine beiden items, 0L );*/
    //SfxBoolItem aBool(SID_OPEN_SMARTTAGOPTIONS, TRUE);
    //GetBindings().GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SFX_CALLMODE_ASYNCHRON, &aBool, 0L );

    return 0;
}
IMPL_LINK_INLINE_END( SvxSmartTagsControl, MenuSelect, PopupMenu *, pMen )

//--------------------------------------------------------------------

SvxSmartTagsControl::~SvxSmartTagsControl()
{
    delete mpSmartTagItem;
    delete mpMenu;
}

//--------------------------------------------------------------------

PopupMenu* SvxSmartTagsControl::GetPopup() const
{
    return mpMenu;
}


