/*************************************************************************
 *
 *  $RCSfile: dlgattr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:18:09 $
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


#ifdef MAC
#ifdef ITEMID_NUMBERINFO
#undef ITEMID_NUMBERINFO
#endif
#endif

#define ITEMID_NUMBERINFO 0

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

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#define _ZFORLIST_DECLARE_TABLE
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#include <svx/svxids.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001

using namespace dbaui;

//==================================================================
SbaSbAttrDlg::SbaSbAttrDlg(Window* pParent, const SfxItemSet* pCellAttrs, SvNumberFormatter* pFormatter, USHORT nFlags, BOOL bRow)
             : SfxTabDialog(pParent, ModuleRes( DLG_ATTR ), pCellAttrs )
             ,aTitle(ResId(ST_ROW))
{
    pNumberInfoItem = new SvxNumberInfoItem( pFormatter );

    if (bRow)
        SetText(aTitle);
    if( nFlags & TP_ATTR_CHAR )
//        AddTabPage( RID_SVXPAGE_CHAR_STD,String(ResId(TP_ATTR_CHAR)),SvxCharStdPage::Create,            0 );
        DBG_ERROR( "found flag TP_ATTR_CHAR" );
    if( nFlags & TP_ATTR_NUMBER )
        AddTabPage( RID_SVXPAGE_NUMBERFORMAT,String(ResId(TP_ATTR_NUMBER)) ); //CHINA001 AddTabPage( RID_SVXPAGE_NUMBERFORMAT,String(ResId(TP_ATTR_NUMBER)),SvxNumberFormatTabPage::Create, 0 );
    if( nFlags & TP_ATTR_ALIGN )
        AddTabPage( RID_SVXPAGE_ALIGNMENT,String(ResId(TP_ATTR_ALIGN)) );//CHINA001 AddTabPage( RID_SVXPAGE_ALIGNMENT,String(ResId(TP_ATTR_ALIGN)),SvxAlignmentTabPage::Create, 0 );
    FreeResource();
}

// -----------------------------------------------------------------------
SbaSbAttrDlg::~SbaSbAttrDlg()
{
    delete pNumberInfoItem;
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
            aSet.Put (SvxNumberInfoItem( pNumberInfoItem->GetNumberFormatter(), (const USHORT)SID_ATTR_NUMBERFORMAT_INFO));
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


