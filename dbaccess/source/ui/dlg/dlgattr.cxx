/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"


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


