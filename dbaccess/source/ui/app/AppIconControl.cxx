/*************************************************************************
 *
 *  $RCSfile: AppIconControl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:29:36 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBAUI_APPICONCONTROL_HXX
#include "AppIconControl.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif
#include <memory>

using namespace ::dbaui;
//==================================================================
// class OApplicationIconControl
//==================================================================
OApplicationIconControl::OApplicationIconControl(Window* _pParent)
    : SvtIconChoiceCtrl(_pParent,WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                                WB_TABSTOP | WB_CLIPCHILDREN | WB_NOVSCROLL | WB_SMART_ARRANGE | WB_NOHSCROLL)
    ,DropTargetHelper(this)
    ,m_pActionListener(NULL)
{
    typedef ::std::pair< USHORT,USHORT> TUSHORTPair;
    typedef ::std::pair< ElementType,TUSHORTPair> TUSHORT2Pair;
    typedef ::std::pair< String,TUSHORT2Pair> TPair;
    static const TPair pTypes[] = {
            TPair(String(ModuleRes(RID_STR_TABLES_CONTAINER)),TUSHORT2Pair(E_TABLE,TUSHORTPair(IMG_TABLEFOLDER_TREE_L,IMG_TABLEFOLDER_TREE_LHC) ))
        ,   TPair(String(ModuleRes(RID_STR_QUERIES_CONTAINER)),TUSHORT2Pair(E_QUERY,TUSHORTPair(IMG_QUERYFOLDER_TREE_L,IMG_QUERYFOLDER_TREE_LHC)) )
        ,   TPair(String(ModuleRes(RID_STR_FORMS_CONTAINER)),TUSHORT2Pair(E_FORM,TUSHORTPair(IMG_FORMFOLDER_TREE_L,IMG_FORMFOLDER_TREE_LHC)) )
        ,   TPair(String(ModuleRes(RID_STR_REPORTS_CONTAINER)),TUSHORT2Pair(E_REPORT,TUSHORTPair(IMG_REPORTFOLDER_TREE_L,IMG_REPORTFOLDER_TREE_LHC)) )
        };

    for (size_t i=0; i < sizeof(pTypes)/sizeof(pTypes[0]); ++i)
    {
        SvxIconChoiceCtrlEntry* pEntry = InsertEntry(pTypes[i].first,Image(ModuleRes(pTypes[i].second.second.first)),Image(ModuleRes(pTypes[i].second.second.second)));
        if ( pEntry )
            pEntry->SetUserData(new ElementType(pTypes[i].second.first));
    }

    SetChoiceWithCursor( TRUE );
    SetSelectionMode(SINGLE_SELECTION);
    CreateAutoMnemonics();

}
// -----------------------------------------------------------------------------
OApplicationIconControl::~OApplicationIconControl()
{
    ULONG nCount = GetEntryCount();
    for ( ULONG i = 0; i < nCount; ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( i );
        if ( pEntry )
        {
            ::std::auto_ptr<ElementType> aType(static_cast<ElementType*>(pEntry->GetUserData()));
            pEntry->SetUserData(NULL);
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int8 OApplicationIconControl::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {

        SvxIconChoiceCtrlEntry* pEntry = GetEntry(_rEvt.maPosPixel);
        if ( pEntry )
        {
            SetCursor(pEntry);
            nDropOption = _rEvt.mnAction & m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
            m_aMousePos = _rEvt.maPosPixel;
        }
    }

    return nDropOption;
}
// -----------------------------------------------------------------------------
sal_Int8 OApplicationIconControl::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}
