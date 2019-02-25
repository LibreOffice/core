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
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <avmedia/mediaitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <vcl/EnumContext.hxx>

#include <mediash.hxx>
#include <strings.hrc>
#include <viewdata.hxx>
#include <drawview.hxx>
#include <scresid.hxx>

#define ShellClass_ScMediaShell
#include <scslots.hxx>

SFX_IMPL_INTERFACE(ScMediaShell, ScDrawShell)

void ScMediaShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Media_Objectbar);

    GetStaticInterface()->RegisterPopupMenu("media");
}


ScMediaShell::ScMediaShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetName( ScResId( SCSTR_MEDIASHELL ) );
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Media));
}

ScMediaShell::~ScMediaShell()
{
}

void ScMediaShell::GetMediaState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
    {
        SfxWhichIter    aIter( rSet );
        sal_uInt16          nWhich = aIter.FirstWhich();

        while( nWhich )
        {
            if( SID_AVMEDIA_TOOLBOX == nWhich )
            {
                std::unique_ptr<SdrMarkList> pMarkList(new SdrMarkList( pView->GetMarkedObjectList() ));
                bool            bDisable = true;

                if( 1 == pMarkList->GetMarkCount() )
                {
                    SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                    if( dynamic_cast<const SdrMediaObj*>( pObj) )
                    {
                        ::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );

                        static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
                        rSet.Put( aItem );
                        bDisable = false;
                    }
                }

                if( bDisable )
                    rSet.DisableItem( SID_AVMEDIA_TOOLBOX );
            }

            nWhich = aIter.NextWhich();
        }
    }
}

void ScMediaShell::ExecuteMedia( const SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView && SID_AVMEDIA_TOOLBOX == rReq.GetSlot() )
    {
        const SfxItemSet*   pArgs = rReq.GetArgs();
        const SfxPoolItem*  pItem;

        if( !pArgs || ( SfxItemState::SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, false, &pItem ) ) )
            pItem = nullptr;

        if( pItem )
        {
            std::unique_ptr<SdrMarkList> pMarkList(new SdrMarkList( pView->GetMarkedObjectList() ));

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( dynamic_cast<const SdrMediaObj*>( pObj) )
                {
                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem(
                        static_cast< const ::avmedia::MediaItem& >( *pItem ) );
                }
            }
        }
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
