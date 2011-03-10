/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>

#include <sfx2/doctdlg.hxx>
#include "docvor.hxx"
#include "sfxresid.hxx"
#include "sfxtypes.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>

#include <sfx2/sfx.hrc>
#include "doc.hrc"
#include "doctdlg.hrc"
#include <sfx2/basedlgs.hxx>

//=========================================================================

SfxDocumentTemplateDlg::SfxDocumentTemplateDlg( Window * pParent, SfxDocumentTemplates* pTempl ) :

    ModalDialog( pParent, SfxResId( DLG_DOC_TEMPLATE ) ),

    aEditFL     ( this, SfxResId( FL_EDIT ) ),
    aNameEd     ( this, SfxResId( ED_NAME ) ),
    aTemplateFL ( this, SfxResId( FL_STYLESHEETS ) ),
    aRegionFt   ( this, SfxResId( FT_SECTION ) ),
    aRegionLb   ( this, SfxResId( LB_SECTION ) ),
    aTemplateFt ( this, SfxResId( FT_STYLESHEETS ) ),
    aTemplateLb ( this, SfxResId( LB_STYLESHEETS ) ),

    aOkBt       ( this, SfxResId( BT_OK ) ),
    aCancelBt   ( this, SfxResId( BT_CANCEL ) ),
    aHelpBt     ( this, SfxResId( BT_HELP ) ),
    aEditBt     ( this, SfxResId( BT_EDIT ) ),
    aOrganizeBt ( this, SfxResId( BT_ORGANIZE ) ),

    pTemplates  ( pTempl ),
    pHelper     ( NULL )

{
    FreeResource();

    pHelper = new SfxModalDefParentHelper( this );
    aOrganizeBt.SetClickHdl(LINK(this, SfxDocumentTemplateDlg, OrganizeHdl));
    aNameEd.SetModifyHdl(LINK(this, SfxDocumentTemplateDlg, NameModify));
    aOkBt.SetClickHdl(LINK(this, SfxDocumentTemplateDlg, OkHdl));
    aEditBt.SetClickHdl(LINK(this, SfxDocumentTemplateDlg, EditHdl));
    Init();
}

//-------------------------------------------------------------------------

SfxDocumentTemplateDlg::~SfxDocumentTemplateDlg()
{
    delete pHelper;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxDocumentTemplateDlg, EditHdl, Button *, pBut )
{
    (void)pBut; //unused
    if ( !aRegionLb.GetSelectEntryCount() ||
         !aTemplateLb.GetSelectEntryCount())
        return 0;

    const SfxStringItem aRegion( SID_TEMPLATE_REGIONNAME, aRegionLb.GetSelectEntry() );
    const SfxStringItem aName( SID_TEMPLATE_NAME, aTemplateLb.GetSelectEntry() );
    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENTEMPLATE, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, &aRegion, &aName, 0L );
    EndDialog(RET_EDIT_STYLE);

    return 0;
}

//-------------------------------------------------------------------------

void SfxDocumentTemplateDlg::Init()
{
    if(!pTemplates->IsConstructed())
        pTemplates->Construct();

    const USHORT nCount = pTemplates->GetRegionCount();
    for(USHORT i = 0; i < nCount; ++i)
        aRegionLb.InsertEntry(pTemplates->GetFullRegionName(i));
    if(!nCount)
        aRegionLb.InsertEntry(String(SfxResId(STR_STANDARD)));
    aRegionLb.SelectEntryPos(0);
    if(nCount)
    {
        aRegionLb.SetSelectHdl(LINK(this, SfxDocumentTemplateDlg, RegionSelect));
        RegionSelect(&aRegionLb);
        aTemplateLb.SetSelectHdl(LINK(this, SfxDocumentTemplateDlg, TemplateSelect));
        aTemplateLb.SetDoubleClickHdl(LINK(this, SfxDocumentTemplateDlg, EditHdl));
    }
    else {
        Link aLink;
        aTemplateLb.SetSelectHdl(aLink);
        aTemplateLb.SetDoubleClickHdl(aLink);
    }
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxDocumentTemplateDlg, OrganizeHdl, Button *, pButton )
{
    (void)pButton; //unused
    SfxTemplateOrganizeDlg *pDlg =
        new SfxTemplateOrganizeDlg(this, pTemplates);
    const short nRet = pDlg->Execute();
    delete pDlg;
    if(RET_OK == nRet)
    {
        // View aktualisieren
        aRegionLb.SetUpdateMode( FALSE );
        aRegionLb.Clear();
        Init();
        aRegionLb.SetUpdateMode( TRUE );
        aRegionLb.Invalidate();
        aRegionLb.Update();
        aCancelBt.SetText(String(SfxResId(STR_CLOSE)));
    }
    else if(RET_EDIT_STYLE == nRet)
        EndDialog(RET_CANCEL);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxDocumentTemplateDlg, OkHdl, Control *, pControl )
{
    (void)pControl; //unused
    // Test whether a template with this name exists
    if(LISTBOX_ENTRY_NOTFOUND != aTemplateLb.GetEntryPos(
        GetTemplateName())) {
        QueryBox aQuery(this, SfxResId(MSG_CONFIRM_OVERWRITE_TEMPLATE));
        if(RET_NO == aQuery.Execute())
            return 0;
    }
    EndDialog(RET_OK);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxDocumentTemplateDlg, RegionSelect, ListBox *, pBox )
{
    const USHORT nRegion = pBox->GetSelectEntryPos();
    const USHORT nCount = pTemplates->GetCount(nRegion);
    aTemplateLb.SetUpdateMode(FALSE);
    aTemplateLb.Clear();
    for(USHORT i = 0; i < nCount; ++i)
        aTemplateLb.InsertEntry(pTemplates->GetName(nRegion, i));
    aTemplateLb.SelectEntryPos(0);
    aTemplateLb.SetUpdateMode(TRUE);
    aTemplateLb.Invalidate();
    aTemplateLb.Update();
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxDocumentTemplateDlg, TemplateSelect, ListBox *, pBox )
{
    aNameEd.SetText(pBox->GetSelectEntry());
    NameModify(&aNameEd);
    return 0;
}
IMPL_LINK_INLINE_END( SfxDocumentTemplateDlg, TemplateSelect, ListBox *, pBox )

//-------------------------------------------------------------------------

IMPL_LINK( SfxDocumentTemplateDlg, NameModify, Edit *, pBox )
{
    const String &rText=pBox->GetText();
    if(!rText.Len())
        aEditBt.Enable();
    else
    {
        aTemplateLb.SelectEntry(rText);
        aEditBt.Enable( aTemplateLb.GetSelectEntry() == rText );
    }

    aOkBt.Enable( rText.Len() > 0 );
    return 0;
}

//-------------------------------------------------------------------------

String SfxDocumentTemplateDlg::GetTemplatePath()
{
    const String& rPath=GetTemplateName();
    if(pTemplates->GetRegionCount())
        return pTemplates->GetTemplatePath(
            aRegionLb.GetSelectEntryPos(), rPath);
    return pTemplates->GetDefaultTemplatePath(rPath);
}

//-------------------------------------------------------------------------

void SfxDocumentTemplateDlg::NewTemplate(const String &rPath)
{
    pTemplates->NewTemplate(
        aRegionLb.GetSelectEntryPos(), GetTemplateName(), rPath);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
