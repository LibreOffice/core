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
#include "precompiled_dbaccess.hxx"


#ifndef DBAUI_SBATTRDLG_HXX
#include "dlgattr.hxx"
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SVX_NUMINF_HXX //autogen
#include <svx/numinf.hxx>
#endif

//CHINA001 #ifndef _SVX_CHARDLG_HXX //autogen
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #endif

//CHINA001 #ifndef _SVX_NUMFMT_HXX //autogen
//CHINA001 #include <svx/numfmt.hxx>
//CHINA001 #endif
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif

//CHINA001 #ifndef _SVX_ALIGN_HXX //autogen
//CHINA001 #include <svx/align.hxx>
//CHINA001 #endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svl/itemset.hxx>
#endif
#define _ZFORLIST_DECLARE_TABLE
#ifndef _ZFORLIST_HXX
#include <svl/zforlist.hxx>
#endif
#include <svx/svxids.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svl/intitem.hxx> //CHINA001
#endif //CHINA001
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
using namespace dbaui;

DBG_NAME(SbaSbAttrDlg)
//==================================================================
SbaSbAttrDlg::SbaSbAttrDlg(Window* pParent, const SfxItemSet* pCellAttrs, SvNumberFormatter* pFormatter, sal_uInt16 nFlags, sal_Bool bRow)
             : SfxTabDialog(pParent, ModuleRes( DLG_ATTR ), pCellAttrs )
             ,aTitle(ModuleRes(ST_ROW))
{
    DBG_CTOR(SbaSbAttrDlg,NULL);

    pNumberInfoItem = new SvxNumberInfoItem( pFormatter, 0 );

    if (bRow)
        SetText(aTitle);
    if( nFlags & TP_ATTR_CHAR )
    {
//        AddTabPage( RID_SVXPAGE_CHAR_STD,String(ModuleRes(TP_ATTR_CHAR)),SvxCharStdPage::Create,            0 );
        DBG_ERROR( "found flag TP_ATTR_CHAR" );
    }
    if( nFlags & TP_ATTR_NUMBER )
        AddTabPage( RID_SVXPAGE_NUMBERFORMAT,String(ModuleRes(TP_ATTR_NUMBER)) ); //CHINA001 AddTabPage( RID_SVXPAGE_NUMBERFORMAT,String(ModuleRes(TP_ATTR_NUMBER)),SvxNumberFormatTabPage::Create, 0 );
    if( nFlags & TP_ATTR_ALIGN )
        AddTabPage( RID_SVXPAGE_ALIGNMENT,String(ModuleRes(TP_ATTR_ALIGN)) );//CHINA001 AddTabPage( RID_SVXPAGE_ALIGNMENT,String(ModuleRes(TP_ATTR_ALIGN)),SvxAlignmentTabPage::Create, 0 );
    FreeResource();
}

// -----------------------------------------------------------------------
SbaSbAttrDlg::~SbaSbAttrDlg()
{
    delete pNumberInfoItem;

    DBG_DTOR(SbaSbAttrDlg,NULL);
}

// -----------------------------------------------------------------------
void SbaSbAttrDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch ( nPageId )
    {
        case RID_SVXPAGE_NUMBERFORMAT:
        {
            //CHINA001 ((SvxNumberFormatTabPage&)rTabPage).
                //CHINA001 SetNumberFormatList( *pNumberInfoItem );
            aSet.Put (SvxNumberInfoItem( pNumberInfoItem->GetNumberFormatter(), (const sal_uInt16)SID_ATTR_NUMBERFORMAT_INFO));
            rTabPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_STD:
        {
            //  ((SvxCharStdPage&)rTabPage).SetFontList(SBA_MOD_EXT()->FontListItem());
        }
        break;

        case RID_SVXPAGE_ALIGNMENT:
        {
            //CHINA001 ((SvxAlignmentTabPage&)rTabPage).SetFlags(WBA_NO_ORIENTATION|WBA_NO_LINEBREAK|WBA_NO_GRIDLINES|WBA_NO_VERTICAL|WBA_NO_LEFTINDENT);
//           aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, WBA_NO_ORIENTATION|WBA_NO_LINEBREAK|WBA_NO_GRIDLINES|WBA_NO_VERTICAL|WBA_NO_LEFTINDENT));
//           rTabPage.PageCreated(aSet);
        }
        break;

        default:
        break;
    }
}


