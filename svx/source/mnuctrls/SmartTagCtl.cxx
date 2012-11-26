/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/SmartTagCtl.hxx>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#ifndef _COM_SUN_STAR_SMARTTAGS_XSTRINGKEYMAP_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif
#include <svtools/stdmenu.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#include <svx/SmartTagItem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SvxSmartTagsControl, SvxSmartTagItem);

//--------------------------------------------------------------------

SvxSmartTagsControl::SvxSmartTagsControl
(
    sal_uInt16          _nId,
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

const sal_uInt16 MN_ST_INSERT_START = 500;

void SvxSmartTagsControl::FillMenu()
{
    if ( !mpSmartTagItem )
        return;

    sal_uInt16 nMenuPos = 0;
    sal_uInt16 nSubMenuPos = 0;
    sal_uInt16 nMenuId = 1;
    sal_uInt16 nSubMenuId = MN_ST_INSERT_START;

    const Sequence < Sequence< Reference< smarttags::XSmartTagAction > > >& rActionComponentsSequence = mpSmartTagItem->GetActionComponentsSequence();
    const Sequence < Sequence< sal_Int32 > >& rActionIndicesSequence = mpSmartTagItem->GetActionIndicesSequence();
    const Sequence< Reference< container::XStringKeyMap > >& rStringKeyMaps = mpSmartTagItem->GetStringKeyMaps();
    const lang::Locale& rLocale = mpSmartTagItem->GetLocale();
    const rtl::OUString aApplicationName = mpSmartTagItem->GetApplicationName();
    const rtl::OUString aRangeText = mpSmartTagItem->GetRangeText();
    const Reference<text::XTextRange>& xTextRange = mpSmartTagItem->GetTextRange();
    const Reference<frame::XController>& xController = mpSmartTagItem->GetController();

    for ( sal_uInt16 j = 0; j < rActionComponentsSequence.getLength(); ++j )
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
        for ( sal_uInt16 i = 0; i < rActionComponents.getLength(); ++i )
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

void SvxSmartTagsControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    mrParent.EnableItem( GetId(), SFX_ITEM_DISABLED != eState );

    if ( SFX_ITEM_AVAILABLE == eState )
    {
        const SvxSmartTagItem* pSmartTagItem = dynamic_cast< const SvxSmartTagItem* >( pState );
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
    //SfxBoolItem aBool(SID_OPEN_SMARTTAGOPTIONS, sal_True);
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


