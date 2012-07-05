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


#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>

#include <sfx2/doctdlg.hxx>
#include "docvor.hxx"
#include "sfx2/sfxresid.hxx"
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

    const sal_uInt16 nCount = pTemplates->GetRegionCount();
    for(sal_uInt16 i = 0; i < nCount; ++i)
        aRegionLb.InsertEntry(pTemplates->GetFullRegionName(i));
    if(!nCount)
        aRegionLb.InsertEntry(SfxResId(STR_STANDARD).toString());
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
        aRegionLb.SetUpdateMode( sal_False );
        aRegionLb.Clear();
        Init();
        aRegionLb.SetUpdateMode( sal_True );
        aRegionLb.Invalidate();
        aRegionLb.Update();
        aCancelBt.SetText(SfxResId(STR_CLOSE).toString());
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
    const sal_uInt16 nRegion = pBox->GetSelectEntryPos();
    const sal_uInt16 nCount = pTemplates->GetCount(nRegion);
    aTemplateLb.SetUpdateMode(sal_False);
    aTemplateLb.Clear();
    for(sal_uInt16 i = 0; i < nCount; ++i)
        aTemplateLb.InsertEntry(pTemplates->GetName(nRegion, i));
    aTemplateLb.SelectEntryPos(0);
    aTemplateLb.SetUpdateMode(sal_True);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
