/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdlgs.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:11:42 $
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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION

//      ohne precompiled Headers uebersetzen !!!



// INCLUDE ---------------------------------------------------------------

#include <svx/svxids.hrc>
#define ITEMID_FONTLIST SID_ATTR_CHAR_FONTLIST
#define ITEMID_TABSTOP  0

//CHINA001 #include <svx/chardlg.hxx>
#include <svx/flstitem.hxx>
//CHINA001 #include <svx/paragrph.hxx>
//CHINA001 #include <svx/tabstpge.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/cjkoptions.hxx>

#include "textdlgs.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include <svx/svxids.hrc> //add CHINA001
#include <svtools/intitem.hxx> //add CHINA001
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

void __EXPORT ScCharDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
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

void __EXPORT ScParagraphDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_TABULATOR:
            {
            //CHINA001 ( (SvxTabulatorTabPage&) rPage ).
            //CHINA001      DisableControls( TABTYPE_ALL &~TABTYPE_LEFT |
            //CHINA001                       TABFILL_ALL &~TABFILL_NONE );
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));//add CHINA001
            aSet.Put(SfxUInt16Item(SID_SVXTABULATORTABPAGE_CONTROLFLAGS,TABTYPE_ALL &~TABTYPE_LEFT |
                                TABFILL_ALL &~TABFILL_NONE ));
            rPage.PageCreated(aSet);//add CHINA001
            }
        break;
    }
}



