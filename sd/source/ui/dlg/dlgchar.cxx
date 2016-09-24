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

#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <svx/flagsdef.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svxdlg.hxx>

#include "sdresid.hxx"
#include "dlg_char.hxx"
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>

/**
 * Constructor of tab dialog: append pages to dialog
 */
SdCharDlg::SdCharDlg( vcl::Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent
                             ,"DrawCharDialog"
                             ,"modules/sdraw/ui/drawchardialog.ui"
                             , pAttr ),
        rDocShell           ( *pDocShell )
{

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");

    mnCharName = AddTabPage( "RID_SVXPAGE_CHAR_NAME", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
    mnCharEffects =  AddTabPage( "RID_SVXPAGE_CHAR_EFFECTS", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
    mnCharPosition =  AddTabPage( "RID_SVXPAGE_CHAR_POSITION", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), nullptr );
    mnCharBackground = AddTabPage( "RID_SVXPAGE_BACKGROUND", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), nullptr );
}

void SdCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == mnCharName)
    {
        SvxFontListItem aItem(*( static_cast<const SvxFontListItem*>( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (nId == mnCharEffects)
    {
        rPage.PageCreated(aSet);
    }
    else if (nId == mnCharBackground)
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING)));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
