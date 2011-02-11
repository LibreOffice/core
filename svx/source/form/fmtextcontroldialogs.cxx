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
#include "precompiled_svx.hxx"
#include "fmtextcontroldialogs.hxx"
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _SVX_CHARDLG_HXX
//#include "chardlg.hxx"
#endif
#ifndef _SVX_PARAGRPH_HXX
//#include "paragrph.hxx"
#endif
#include <editeng/eeitem.hxx>
#ifndef _SVX_TABSTPGE_HXX
//#include "tabstpge.hxx"
#endif

#include "svx/flagsdef.hxx"
#include <svl/intitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <svl/cjkoptions.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= TextControlCharAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlCharAttribDialog::TextControlCharAttribDialog( Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList )
        :SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_CHARATTR ), &_rCoreSet )
        ,m_aFontList( _rFontList )
    {
        FreeResource();

        AddTabPage( RID_SVXPAGE_CHAR_NAME);
        AddTabPage( RID_SVXPAGE_CHAR_EFFECTS);
        AddTabPage( RID_SVXPAGE_CHAR_POSITION);
    }

    //--------------------------------------------------------------------
    TextControlCharAttribDialog::~TextControlCharAttribDialog()
    {
    }

    //--------------------------------------------------------------------
    void TextControlCharAttribDialog::PageCreated( sal_uInt16 _nId, SfxTabPage& _rPage )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

        switch( _nId )
        {
            case RID_SVXPAGE_CHAR_NAME:
                aSet.Put (m_aFontList);
                _rPage.PageCreated(aSet);
                break;

            case RID_SVXPAGE_CHAR_EFFECTS:
                aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
                _rPage.PageCreated(aSet);
                break;

            case RID_SVXPAGE_CHAR_POSITION:
                aSet.Put( SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER) );
                _rPage.PageCreated(aSet);
                break;
        }
    }

    //====================================================================
    //= TextControlParaAttribDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextControlParaAttribDialog::TextControlParaAttribDialog( Window* _pParent, const SfxItemSet& _rCoreSet )
        :SfxTabDialog( _pParent, SVX_RES( RID_SVXDLG_TEXTCONTROL_PARAATTR ), &_rCoreSet )
    {
        FreeResource();

        AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
        AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );

        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage( RID_SVXPAGE_PARA_ASIAN );
        else
            RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

        AddTabPage( RID_SVXPAGE_TABULATOR );
    }

    //--------------------------------------------------------------------
    TextControlParaAttribDialog::~TextControlParaAttribDialog()
    {
    }

//........................................................................
}   // namespace svx
//........................................................................

