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

#include <masterlayoutdlg.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

using namespace ::sd;

MasterLayoutDialog::MasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage)
    : GenericDialogController(pParent, "modules/simpress/ui/masterlayoutdlg.ui", "MasterLayoutDialog")
    , mpDoc(pDoc)
    , mpCurrentPage(pCurrentPage)
    , mxCBDate(m_xBuilder->weld_check_button("datetime"))
    , mxCBPageNumber(m_xBuilder->weld_check_button("pagenumber"))
    , mxCBSlideNumber(m_xBuilder->weld_check_button("slidenumber"))
    , mxCBHeader(m_xBuilder->weld_check_button("header"))
    , mxCBFooter(m_xBuilder->weld_check_button("footer"))
{
    if( mpCurrentPage && !mpCurrentPage->IsMasterPage() )
    {
        mpCurrentPage = static_cast<SdPage*>(&(mpCurrentPage->TRG_GetMasterPage()));
    }

    if( mpCurrentPage == nullptr )
    {
        mpCurrentPage = pDoc->GetMasterSdPage( 0, PageKind::Standard );
        OSL_FAIL( "MasterLayoutDialog::MasterLayoutDialog() - no current page?" );
    }

    switch( mpCurrentPage->GetPageKind() )
    {
        case PageKind::Standard:
        {
            mxCBHeader->set_sensitive(false);
            mxCBPageNumber->set_label(mxCBSlideNumber->get_label());
            break;
        }
        case PageKind::Notes:
            break;
        case PageKind::Handout:
            break;
    }

    mbOldHeader = mpCurrentPage->GetPresObj( PRESOBJ_HEADER ) != nullptr;
    mbOldDate = mpCurrentPage->GetPresObj( PRESOBJ_DATETIME ) != nullptr;
    mbOldFooter = mpCurrentPage->GetPresObj( PRESOBJ_FOOTER ) != nullptr;
    mbOldPageNumber = mpCurrentPage->GetPresObj( PRESOBJ_SLIDENUMBER ) != nullptr;

    mxCBHeader->set_active( mbOldHeader );
    mxCBDate->set_active( mbOldDate );
    mxCBFooter->set_active( mbOldFooter );
    mxCBPageNumber->set_active( mbOldPageNumber );
}

MasterLayoutDialog::~MasterLayoutDialog()
{
}

short MasterLayoutDialog::run()
{
    if (GenericDialogController::run() == RET_OK)
        applyChanges();
    return RET_OK;
}

void MasterLayoutDialog::applyChanges()
{
    mpDoc->BegUndo(m_xDialog->get_title());

    if( (mpCurrentPage->GetPageKind() != PageKind::Standard) && (mbOldHeader != mxCBHeader->get_active() ) )
    {
        if( mbOldHeader )
            remove( PRESOBJ_HEADER );
        else
            create( PRESOBJ_HEADER );
    }

    if( mbOldFooter != mxCBFooter->get_active() )
    {
        if( mbOldFooter )
            remove( PRESOBJ_FOOTER );
        else
            create( PRESOBJ_FOOTER );
    }

    if( mbOldDate != mxCBDate->get_active() )
    {
        if( mbOldDate )
            remove( PRESOBJ_DATETIME );
        else
            create( PRESOBJ_DATETIME );
    }

    if( mbOldPageNumber != mxCBPageNumber->get_active() )
    {
        if( mbOldPageNumber )
            remove( PRESOBJ_SLIDENUMBER );
        else
            create( PRESOBJ_SLIDENUMBER );
    }

    mpDoc->EndUndo();
}

void MasterLayoutDialog::create(PresObjKind eKind)
{
    mpCurrentPage->CreateDefaultPresObj(eKind);
}

void MasterLayoutDialog::remove( PresObjKind eKind )
{
    mpCurrentPage->DestroyDefaultPresObj(eKind);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
