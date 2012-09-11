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

#undef SC_DLLIMPLEMENTATION

#include <svx/svxids.hrc>

#include <editeng/flstitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/cjkoptions.hxx>

#include "textdlgs.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include <svl/intitem.hxx>
#include <svx/flagsdef.hxx>
// -----------------------------------------------------------------------

ScCharDlg::ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent, ScResId( RID_SCDLG_CHAR ), pAttr ),
        rDocShell           ( *pDocShell )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION );
}

// -----------------------------------------------------------------------

void ScCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
            rPage.PageCreated(aSet);
            break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------

ScParagraphDlg::ScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ) :
        SfxTabDialog        ( pParent, ScResId( RID_SCDLG_PARAGRAPH ), pAttr )
{
    FreeResource();

    SvtCJKOptions aCJKOptions;

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
    if ( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
    AddTabPage( RID_SVXPAGE_TABULATOR );
}

// -----------------------------------------------------------------------

void ScParagraphDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_TABULATOR:
            {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put(SfxUInt16Item(SID_SVXTABULATORTABPAGE_CONTROLFLAGS,(TABTYPE_ALL &~TABTYPE_LEFT) |
                                (TABFILL_ALL &~TABFILL_NONE) ));
            rPage.PageCreated(aSet);
            }
        break;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
