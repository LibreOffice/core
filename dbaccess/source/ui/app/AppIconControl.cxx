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

#include "AppIconControl.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "moduledbu.hxx"
#include "dbu_app.hrc"
#include <vcl/image.hxx>
#include "callbacks.hxx"
#include "AppElementType.hxx"
#include <memory>

using namespace ::dbaui;
// class OApplicationIconControl
OApplicationIconControl::OApplicationIconControl(vcl::Window* _pParent)
    : SvtIconChoiceCtrl(_pParent,WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                                WB_TABSTOP | WB_CLIPCHILDREN | WB_NOVSCROLL | WB_SMART_ARRANGE | WB_NOHSCROLL | WB_CENTER)
    ,DropTargetHelper(this)
    ,m_pActionListener(nullptr)
{

    const struct CategoryDescriptor
    {
        sal_uInt16      nLabelResId;
        ElementType eType;
        sal_uInt16      nImageResId;
    }   aCategories[] = {
        { RID_STR_TABLES_CONTAINER,     E_TABLE,    IMG_TABLEFOLDER_TREE_L  },
        { RID_STR_QUERIES_CONTAINER,    E_QUERY,    IMG_QUERYFOLDER_TREE_L  },
        { RID_STR_FORMS_CONTAINER,      E_FORM,     IMG_FORMFOLDER_TREE_L   },
        { RID_STR_REPORTS_CONTAINER,    E_REPORT,   IMG_REPORTFOLDER_TREE_L }
    };
    for (const CategoryDescriptor& aCategorie : aCategories)
    {
        SvxIconChoiceCtrlEntry* pEntry = InsertEntry(
            OUString( ModuleRes( aCategorie.nLabelResId ) ) ,
            Image(  ModuleRes( aCategorie.nImageResId ) ) );
        if ( pEntry )
            pEntry->SetUserData( new ElementType( aCategorie.eType ) );
    }

    SetChoiceWithCursor();
    SetSelectionMode(SINGLE_SELECTION);
}

OApplicationIconControl::~OApplicationIconControl()
{
    disposeOnce();
}

void OApplicationIconControl::dispose()
{
    sal_uLong nCount = GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( i );
        if ( pEntry )
        {
            std::unique_ptr<ElementType> aType(static_cast<ElementType*>(pEntry->GetUserData()));
            pEntry->SetUserData(nullptr);
        }
    }
    SvtIconChoiceCtrl::dispose();
}

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

sal_Int8 OApplicationIconControl::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
