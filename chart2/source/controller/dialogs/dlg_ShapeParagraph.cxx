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

#include "dlg_ShapeParagraph.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"

#include <svl/cjkoptions.hxx>
#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

namespace chart
{

ShapeParagraphDialog::ShapeParagraphDialog( Window* pParent, const SfxItemSet* pAttr )
    :SfxTabDialog( pParent, SchResId( DLG_SHAPE_PARAGRAPH ), pAttr )
{
    FreeResource();

    SvtCJKOptions aCJKOptions;

    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
    if ( aCJKOptions.IsAsianTypographyEnabled() )
    {
        AddTabPage( RID_SVXPAGE_PARA_ASIAN );
    }
    else
    {
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
    }
    AddTabPage( RID_SVXPAGE_TABULATOR );
}

ShapeParagraphDialog::~ShapeParagraphDialog()
{
}

void ShapeParagraphDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    SfxAllItemSet aSet( *( GetInputSetImpl()->GetPool() ) );
    switch ( nId )
    {
        case RID_SVXPAGE_TABULATOR:
            {
                aSet.Put( SfxUInt16Item( SID_SVXTABULATORTABPAGE_CONTROLFLAGS,
                    ( TABTYPE_ALL &~TABTYPE_LEFT ) | ( TABFILL_ALL &~TABFILL_NONE ) ) );
                rPage.PageCreated( aSet );
            }
            break;
        default:
            {
            }
            break;
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
