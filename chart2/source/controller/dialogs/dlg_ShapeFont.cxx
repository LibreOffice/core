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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_ShapeFont.hxx"
#include "ViewElementListProvider.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"

#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <editeng/flstitem.hxx>

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

ShapeFontDialog::ShapeFontDialog( Window* pParent, const SfxItemSet* pAttr,
    const ViewElementListProvider* pViewElementListProvider )
    :SfxTabDialog( pParent, SchResId( DLG_SHAPE_FONT ), pAttr )
    ,m_pViewElementListProvider( pViewElementListProvider )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION );
}

ShapeFontDialog::~ShapeFontDialog()
{
}

void ShapeFontDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    SfxAllItemSet aSet( *( GetInputSetImpl()->GetPool() ) );
    switch ( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
            {
                aSet.Put( SvxFontListItem( m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST ) );
                rPage.PageCreated( aSet );
            }
            break;
        case RID_SVXPAGE_CHAR_EFFECTS:
            {
                aSet.Put( SfxUInt16Item( SID_DISABLE_CTL, DISABLE_CASEMAP ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
