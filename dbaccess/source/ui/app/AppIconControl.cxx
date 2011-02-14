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
#ifndef DBAUI_APPICONCONTROL_HXX
#include "AppIconControl.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
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

    struct CategoryDescriptor
    {
        sal_uInt16      nLabelResId;
        ElementType eType;
        sal_uInt16      nImageResId;
        sal_uInt16      nImageResIdHC;
    }   aCategories[] = {
        { RID_STR_TABLES_CONTAINER,     E_TABLE,    IMG_TABLEFOLDER_TREE_L, IMG_TABLEFOLDER_TREE_LHC    },
        { RID_STR_QUERIES_CONTAINER,    E_QUERY,    IMG_QUERYFOLDER_TREE_L, IMG_QUERYFOLDER_TREE_LHC    },
        { RID_STR_FORMS_CONTAINER,      E_FORM,     IMG_FORMFOLDER_TREE_L,  IMG_FORMFOLDER_TREE_LHC     },
        { RID_STR_REPORTS_CONTAINER,    E_REPORT,   IMG_REPORTFOLDER_TREE_L,IMG_REPORTFOLDER_TREE_LHC   }
    };
    for ( size_t i=0; i < sizeof(aCategories)/sizeof(aCategories[0]); ++i)
    {
        SvxIconChoiceCtrlEntry* pEntry = InsertEntry(
            String( ModuleRes( aCategories[i].nLabelResId ) ),
            Image( ModuleRes( aCategories[i].nImageResId ) ),
            Image( ModuleRes( aCategories[i].nImageResIdHC ) ) );
        if ( pEntry )
            pEntry->SetUserData( new ElementType( aCategories[i].eType ) );
    }

    SetChoiceWithCursor( sal_True );
    SetSelectionMode(SINGLE_SELECTION);
}
// -----------------------------------------------------------------------------
OApplicationIconControl::~OApplicationIconControl()
{
    sal_uLong nCount = GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
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
