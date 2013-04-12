/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "slidehack.hxx"
#include "GroupSlidesDialog.hxx"

#include <vcl/msgbox.hxx>

using namespace ::SlideHack;

namespace sd
{

namespace {
    OriginDetailsPtr findLeadOrigin( const std::vector< SdPage * > &rPages )
    {
        for( size_t i = 0; i < rPages.size(); i++)
        {
            if ( rPages[i]->getOrigin().get() )
                return rPages[i]->getOrigin();
        }
        return OriginDetailsPtr();
    }
}

void SdGroupSlidesDialog::addGroupsToCombo( ComboBox /* *pBox, */ SdDrawDocument *pDoc )
{
#if 0
    mpGroupCombo->Clear();
#endif

    sal_uInt32 nPages = pDoc->GetSdPageCount( PK_STANDARD );
    StorePtr pStore = SlideHack::Store::getStore();
    OSL_ASSERT( pStore.get() );
    for ( sal_uInt32 nPage = 0; nPage < nPages; ++nPage )
    {
        SdPage *pPage = pDoc->GetSdPage( nPage, PK_STANDARD );
        OriginDetailsPtr pOrigin = pPage->getOrigin();
        if ( !pOrigin.get() )
            continue;
        GroupPtr pGroup = pStore->lookupGroup( pOrigin );
        if ( !pGroup.get() )
            continue;
        if ( std::find( maGroups.begin(), maGroups.end(), pGroup ) != maGroups.end() )
            continue;
        maGroups.push_back( pGroup );
#if 0
        // FIXME: we want a widget that does something like this -
        // an Autocomplete hdl on an Edit ? or ...
        mpGroupCombo->InsertEntry( pGroup->getMetaData()->getName() );
#endif
    }
}

SdGroupSlidesDialog::SdGroupSlidesDialog(Window* pWindow, SdDrawDocument* pActDoc,
                                         const std::vector< SdPage * > &rPages )
  : ModalDialog( pWindow, "GroupSlidesDialog", "modules/simpress/ui/groupslides.ui" ),
    mpDoc( pActDoc ),
    maPages( rPages )
{
    get( mpAddBtn, "add_btn" );
    get( mpCancelBtn, "cancel_btn" );
//    get( mpGroupCombo, "cb_group" );
    get( mpGroupEdit, "ed_group" );
    get( mpTitle, "ed_title" );
    get( mpKeywords, "ed_keywords" );

    mpAddBtn->SetClickHdl( LINK( this, SdGroupSlidesDialog, AddHdl ) );
    mpCancelBtn->SetClickHdl( LINK( this, SdGroupSlidesDialog, CancelHdl ) );

//    addGroupsToCombo( mpGroupCombo, mpDoc );
//    mpGroupCombo->SetSelectHdl( LINK( this, SdGroupSlidesDialog, GroupSelectHdl ) );
//    mpGroupCombo->SetDoubleClickHdl( LINK( this, SdGroupSlidesDialog, GroupDoubleClickHdl ) );
//    mpGroupCombo->EnableAutocomplete( true );
}

SdGroupSlidesDialog::~SdGroupSlidesDialog()
{
}

IMPL_LINK_NOARG(SdGroupSlidesDialog, AddHdl)
{
    SAL_DEBUG("Add to group");
    EndDialog(0);
    return 0;
}

int SdGroupSlidesDialog::endDialog( bool bSuccessSoSave )
{
    if ( bSuccessSoSave )
    {
        // FIXME: input validation
        SlideHack::Store::getStore()->createGroup( mpGroupEdit->GetText(),
                                                   mpTitle->GetText(),
                                                   mpKeywords->GetText(),
                                                   maPages );
#if 0
        sal_uInt16 nSelected = mpGroupCombo->GetSelectEntryPos();
        SAL_DEBUG("complete: " << (int) nSelected );
        if ( nSelected < maGroups.size() )
        {
            SAL_DEBUG("one selected");
        }
        else
        {
            SAL_DEBUG("new group");
        }
#endif
    }

    EndDialog(0);

    return bSuccessSoSave ? 0 : 1;
}

void SdGroupSlidesDialog::populateEdits( GroupPtr pGroup )
{
    mpTitle->SetText( pGroup->getMetaData()->getTopic() );
    mpKeywords->SetText( "fixme herrings pengins flippers" );
}

IMPL_LINK_NOARG( SdGroupSlidesDialog, CancelHdl )
{
    return endDialog( false ) ;
}

#if 0
IMPL_LINK_NOARG( SdGroupSlidesDialog, GroupSelectHdl )
{
    sal_uInt16 nSelected = mpGroupCombo->GetSelectEntryPos();
    OSL_ASSERT( nSelected < maGroups.size() );
    SAL_DEBUG("select hdl " << (int) nSelected );
    populateEdits( maGroups[ nSelected ] );
    return 0;
}

IMPL_LINK_NOARG( SdGroupSlidesDialog, GroupDoubleClickHdl )
{
    GroupSelectHdl( 0 );
    return endDialog( true ) ;
}
#endif

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
