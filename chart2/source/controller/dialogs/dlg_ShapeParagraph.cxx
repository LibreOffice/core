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
#include "precompiled_chart2.hxx"

#include "dlg_ShapeParagraph.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"

#include <svl/cjkoptions.hxx>
#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

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

//.............................................................................
} //namespace chart
//.............................................................................
