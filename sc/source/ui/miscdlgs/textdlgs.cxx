/*************************************************************************
 *
 *  $RCSfile: textdlgs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-27 08:49:11 $
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

//      ohne precompiled Headers uebersetzen !!!

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/svxids.hrc>
#define ITEMID_FONTLIST SID_ATTR_CHAR_FONTLIST
#define ITEMID_TABSTOP  0

#include <svx/chardlg.hxx>
#include <svx/flstitem.hxx>
#include <svx/paragrph.hxx>
#include <svx/tabstpge.hxx>
#include <sfx2/objsh.hxx>

#include "textdlgs.hxx"
#include "scresid.hxx"
#include "sc.hrc"



// -----------------------------------------------------------------------

ScCharDlg::ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent, ScResId( RID_SCDLG_CHAR ), pAttr ),
        rOutAttrs           ( *pAttr ),
        rDocShell           ( *pDocShell )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, 0);
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, 0);
    AddTabPage( RID_SVXPAGE_CHAR_POSITION, SvxCharPositionPage::Create, 0);
}

// -----------------------------------------------------------------------

void __EXPORT ScCharDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            ( (SvxCharNamePage&) rPage ).SetFontList( aItem );
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            ( (SvxCharEffectsPage&) rPage ).DisableControls(
                            DISABLE_CASEMAP);
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

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create, 0);
    //AddTabPage( RID_SVXPAGE_EXT_PARAGRAPH, SvxExtParagraphTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_TABULATOR, SvxTabulatorTabPage::Create, 0);
}

// -----------------------------------------------------------------------

void __EXPORT ScParagraphDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_TABULATOR:
            ( (SvxTabulatorTabPage&) rPage ).
                    DisableControls( TABTYPE_ALL &~TABTYPE_LEFT |
                                     TABFILL_ALL &~TABFILL_NONE );
        break;
    }
}



