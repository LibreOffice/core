/*************************************************************************
 *
 *  $RCSfile: doctdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#pragma hdrstop

#include "doctdlg.hxx"
#include "docvor.hxx"
#include "sfxresid.hxx"
#include "sfxtypes.hxx"
#include "dispatch.hxx"

#include "sfx.hrc"
#include "doc.hrc"
#include "doctdlg.hrc"
#include "basedlgs.hxx"

//=========================================================================

SfxDocumentTemplateDlg::SfxDocumentTemplateDlg(
    Window * pParent, SfxDocumentTemplates* pTempl )
:   ModalDialog( pParent, SfxResId(DLG_DOC_TEMPLATE) ),
    aRegionFt( this, ResId( FT_SECTION ) ),
    aRegionLb( this, ResId( LB_SECTION ) ),
    aTemplateFt( this, ResId( FT_STYLESHEETS ) ),
    aTemplateLb( this, ResId( LB_STYLESHEETS ) ),
    aNameEd( this, ResId( ED_NAME ) ),
    aTemplateGb( this, ResId( GB_STYLESHEETS ) ),
    aEditGb( this, ResId( GB_EDIT ) ),
    aOkBt( this, ResId( BT_OK ) ),
    aEditBt( this, ResId( BT_EDIT ) ),
    aCancelBt( this, ResId( BT_CANCEL ) ),
    aOrganizeBt( this, ResId( BT_ORGANIZE ) ),
    aHelpBt( this, ResId( BT_HELP ) ),
    pTemplates( pTempl )
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
        // pruefen, ob eine Vorlage diesen Namens existiert
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


