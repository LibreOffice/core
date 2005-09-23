/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppIconControl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:16:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
DBG_NAME(OApplicationIconControl)
//==================================================================
OApplicationIconControl::OApplicationIconControl(Window* _pParent)
    : SvtIconChoiceCtrl(_pParent,WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                                WB_TABSTOP | WB_CLIPCHILDREN | WB_NOVSCROLL | WB_SMART_ARRANGE | WB_NOHSCROLL | WB_CENTER)
    ,DropTargetHelper(this)
    ,m_pActionListener(NULL)
{
    DBG_CTOR(OApplicationIconControl,NULL);

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

    DBG_DTOR(OApplicationIconControl,NULL);
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
            nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
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
