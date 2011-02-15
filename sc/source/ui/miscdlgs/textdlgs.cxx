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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

//      ohne precompiled Headers uebersetzen !!!



// INCLUDE ---------------------------------------------------------------

#include <svx/svxids.hrc>

//CHINA001 #include <svx/chardlg.hxx>
#include <editeng/flstitem.hxx>
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #include <svx/tabstpge.hxx>
#include <sfx2/objsh.hxx>
#include <svl/cjkoptions.hxx>

#include "textdlgs.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include <svx/svxids.hrc> //add CHINA001
#include <svl/intitem.hxx> //add CHINA001
#include <svx/flagsdef.hxx> //CHINA001
// -----------------------------------------------------------------------

ScCharDlg::ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent, ScResId( RID_SCDLG_CHAR ), pAttr ),
        rOutAttrs           ( *pAttr ),
        rDocShell           ( *pDocShell )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, 0);
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, 0);
    AddTabPage( RID_SVXPAGE_CHAR_POSITION ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_POSITION, SvxCharPositionPage::Create, 0);
}

// -----------------------------------------------------------------------

void __EXPORT ScCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001
    switch( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            //CHINA001 ( (SvxCharNamePage&) rPage ).SetFontList( aItem );
            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            //CHINA001 ( (SvxCharEffectsPage&) rPage ).DisableControls(
                            //CHINA001 DISABLE_CASEMAP);
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP)); //CHINA001
            rPage.PageCreated(aSet);
            break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------

ScParagraphDlg::ScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ) :
        SfxTabDialog        ( pParent, ScResId( RID_SCDLG_PARAGRAPH ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    FreeResource();

    SvtCJKOptions aCJKOptions;

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );//CHINA001 AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );//CHINA001 AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create, 0);
    //AddTabPage( RID_SVXPAGE_EXT_PARAGRAPH, SvxExtParagraphTabPage::Create, 0);
    if ( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);//CHINA001 AddTabPage( RID_SVXPAGE_PARA_ASIAN, SvxAsianTabPage::Create,0);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
    AddTabPage( RID_SVXPAGE_TABULATOR );//CHINA001 AddTabPage( RID_SVXPAGE_TABULATOR, SvxTabulatorTabPage::Create, 0);
}

// -----------------------------------------------------------------------

void __EXPORT ScParagraphDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_TABULATOR:
            {
            //CHINA001 ( (SvxTabulatorTabPage&) rPage ).
            //CHINA001      DisableControls( TABTYPE_ALL &~TABTYPE_LEFT |
            //CHINA001                       TABFILL_ALL &~TABFILL_NONE );
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));//add CHINA001
            aSet.Put(SfxUInt16Item(SID_SVXTABULATORTABPAGE_CONTROLFLAGS,(TABTYPE_ALL &~TABTYPE_LEFT) |
                                (TABFILL_ALL &~TABFILL_NONE) ));
            rPage.PageCreated(aSet);//add CHINA001
            }
        break;
    }
}



