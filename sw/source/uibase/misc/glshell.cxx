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

#include <com/sun/star/frame/XTitle.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/srchitem.hxx>
#include <svl/macitem.hxx>
#include <gloshdl.hxx>

#include <editeng/acorrcfg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <uitool.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <glshell.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentState.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <cmdid.h>
#include <swerror.h>
#include <strings.hrc>

#define ShellClass_SwWebGlosDocShell
#define ShellClass_SwGlosDocShell

#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <memory>
#include <utility>

using namespace ::com::sun::star;

SFX_IMPL_SUPERCLASS_INTERFACE(SwGlosDocShell, SwDocShell)

void SwGlosDocShell::InitInterface_Impl()
{
}

SFX_IMPL_SUPERCLASS_INTERFACE(SwWebGlosDocShell, SwWebDocShell)

void SwWebGlosDocShell::InitInterface_Impl()
{
}


static void lcl_Execute( SwDocShell& rSh, SfxRequest& rReq )
{
    if ( rReq.GetSlot() == SID_SAVEDOC )
    {
        if( !rSh.HasName() )
        {
            rReq.SetReturnValue( SfxBoolItem( 0, rSh.Save() ) );
        }
        else
        {
            const SfxBoolItem* pRes = static_cast< const SfxBoolItem* >(
                                        rSh.ExecuteSlot( rReq,
                                        rSh.SfxObjectShell::GetInterface() ));
            if( pRes->GetValue() )
                rSh.GetDoc()->getIDocumentState().ResetModified();
        }
    }
}

static void lcl_GetState( SwDocShell& rSh, SfxItemSet& rSet )
{
    if( SfxItemState::DEFAULT >= rSet.GetItemState( SID_SAVEDOC, false ))
    {
        if( !rSh.GetDoc()->getIDocumentState().IsModified() )
            rSet.DisableItem( SID_SAVEDOC );
        else
            rSet.Put( SfxStringItem( SID_SAVEDOC, SwResId(STR_SAVE_GLOSSARY)));
    }
}

static bool lcl_Save( SwWrtShell& rSh, const OUString& rGroupName,
                const OUString& rShortNm, const OUString& rLongNm )
{
    const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
    std::unique_ptr<SwTextBlocks> pBlock(::GetGlossaries()->GetGroupDoc( rGroupName ));

    SvxMacro aStart = SvxMacro(OUString(), OUString());
    SvxMacro aEnd = SvxMacro(OUString(), OUString());
    SwGlossaryHdl* pGlosHdl;

    pGlosHdl = rSh.GetView().GetGlosHdl();
    pGlosHdl->GetMacros( rShortNm, aStart, aEnd, pBlock.get() );

    sal_uInt16 nRet = rSh.SaveGlossaryDoc( *pBlock, rLongNm, rShortNm,
                                rCfg.IsSaveRelFile(),
                                pBlock->IsOnlyTextBlock( rShortNm ) );

    if(aStart.HasMacro() || aEnd.HasMacro() )
    {
        SvxMacro* pStart = aStart.HasMacro() ? &aStart : nullptr;
        SvxMacro* pEnd = aEnd.HasMacro() ? &aEnd : nullptr;
        pGlosHdl->SetMacros( rShortNm, pStart, pEnd, pBlock.get() );
    }

    rSh.EnterStdMode();
    if( USHRT_MAX != nRet )
        rSh.ResetModified();
    return nRet != USHRT_MAX;
}

SwGlosDocShell::SwGlosDocShell(bool bNewShow)
    : SwDocShell( bNewShow
            ? SfxObjectCreateMode::STANDARD : SfxObjectCreateMode::INTERNAL )
{
}

SwGlosDocShell::~SwGlosDocShell(  )
{
}

void SwGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}

void SwGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

bool SwGlosDocShell::Save()
{
    // In case of an API object which holds this document, it is possible that the WrtShell is already
    // dead. For instance, if the doc is modified via this API object, and then, upon office shutdown,
    // the document's view is closed (by the SFX framework) _before_ the API object is release and
    // tries to save the doc, again.
    // 96380 - 2002-03-03 - fs@openoffice.org
    if ( GetWrtShell() )
        return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
    else
    {
        SetModified( false );
        return false;
    }
}

SwWebGlosDocShell::SwWebGlosDocShell()
    : SwWebDocShell()
{
}

SwWebGlosDocShell::~SwWebGlosDocShell(  )
{
}

void SwWebGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}

void SwWebGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

bool SwWebGlosDocShell::Save()
{
    // same comment as in SwGlosDocShell::Save - see there
    if ( GetWrtShell() )
        return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
    else
    {
        SetModified( false );
        return false;
    }
}

SwDocShellRef SwGlossaries::EditGroupDoc( const OUString& rGroup, const OUString& rShortName, bool bShow )
{
    SwDocShellRef xDocSh;

    std::unique_ptr<SwTextBlocks> pGroup = GetGroupDoc( rGroup );
    if (pGroup && pGroup->GetCount())
    {
        // query which view is registered. In WebWriter there is no normal view
        SfxInterfaceId nViewId = nullptr != SwView::Factory() ? SFX_INTERFACE_SFXDOCSH : SfxInterfaceId(6);
        const OUString sLongName = pGroup->GetLongName(pGroup->GetIndex( rShortName ));

        if( SfxInterfaceId(6) == nViewId )
        {
            SwWebGlosDocShell* pDocSh = new SwWebGlosDocShell();
            xDocSh = pDocSh;
            pDocSh->DoInitNew();
            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName);
            pDocSh->SetGroupName( rGroup );
        }
        else
        {
            SwGlosDocShell* pDocSh = new SwGlosDocShell(bShow);
            xDocSh = pDocSh;
            pDocSh->DoInitNew();
            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName );
            pDocSh->SetGroupName( rGroup );
        }

        // set document title
        SfxViewFrame* pFrame = bShow ? SfxViewFrame::LoadDocument( *xDocSh, nViewId ) : SfxViewFrame::LoadHiddenDocument( *xDocSh, nViewId );
        const OUString aDocTitle(SwResId( STR_GLOSSARY ) + " " + sLongName);

        bool const bDoesUndo =
            xDocSh->GetDoc()->GetIDocumentUndoRedo().DoesUndo();
        xDocSh->GetDoc()->GetIDocumentUndoRedo().DoUndo( false );

        xDocSh->GetWrtShell()->InsertGlossary( *pGroup, rShortName );
        if( !xDocSh->GetDoc()->getIDocumentDeviceAccess().getPrinter( false ) )
        {
            // we create a default SfxPrinter.
            // ItemSet is deleted by Sfx!
            auto pSet = std::make_unique<SfxItemSet>(
                xDocSh->GetDoc()->GetAttrPool(),
                svl::Items<
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER>{});
            VclPtr<SfxPrinter> pPrinter = VclPtr<SfxPrinter>::Create( std::move(pSet) );

            // and append it to the document.
            xDocSh->GetDoc()->getIDocumentDeviceAccess().setPrinter( pPrinter, true, true );
        }

        xDocSh->SetTitle( aDocTitle );
        try
        {
            // set the UI-title
            uno::Reference< frame::XTitle > xTitle( xDocSh->GetModel(), uno::UNO_QUERY_THROW );
            xTitle->setTitle( aDocTitle );
        }
        catch (const uno::Exception&)
        {
        }

        xDocSh->GetDoc()->GetIDocumentUndoRedo().DoUndo( bDoesUndo );
        xDocSh->GetDoc()->getIDocumentState().ResetModified();
        if ( bShow )
            pFrame->GetFrame().Appear();
    }
    return xDocSh;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
