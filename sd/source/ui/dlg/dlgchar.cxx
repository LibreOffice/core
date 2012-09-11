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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <svx/flagsdef.hxx>
#include <sfx2/objsh.hxx>

#include "sdresid.hxx"
#include "dlg_char.hxx"
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdCharDlg::SdCharDlg( Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent, SdResId( TAB_CHAR ), pAttr ),
        rDocShell           ( *pDocShell )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION );
}

// -----------------------------------------------------------------------

void SdCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
