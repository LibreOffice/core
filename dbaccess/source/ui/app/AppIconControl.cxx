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
#include <core_resource.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <vcl/image.hxx>
#include <callbacks.hxx>
#include <AppElementType.hxx>
#include <memory>

using namespace ::dbaui;
// class OApplicationIconControl
OApplicationIconControl::OApplicationIconControl(vcl::Window* _pParent)
    : SvtIconChoiceCtrl(_pParent,WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                                WB_TABSTOP | WB_CLIPCHILDREN | WB_NOVSCROLL | WB_SMART_ARRANGE | WB_NOHSCROLL | WB_CENTER)
    ,DropTargetHelper(this)
    ,m_pActionListener(nullptr)
{

    static const struct CategoryDescriptor
    {
        const char* pLabelResId;
        ElementType eType;
        const char* aImageResId;
    }   aCategories[] = {
        { RID_STR_TABLES_CONTAINER,     E_TABLE,    BMP_TABLEFOLDER_TREE_L  },
        { RID_STR_QUERIES_CONTAINER,    E_QUERY,    BMP_QUERYFOLDER_TREE_L  },
        { RID_STR_FORMS_CONTAINER,      E_FORM,     BMP_FORMFOLDER_TREE_L   },
        { RID_STR_REPORTS_CONTAINER,    E_REPORT,   BMP_REPORTFOLDER_TREE_L }
    };
    for (const CategoryDescriptor& aCategorie : aCategories)
    {
        SvxIconChoiceCtrlEntry* pEntry = InsertEntry(
            DBA_RES(aCategorie.pLabelResId) ,
            Image(StockImage::Yes, OUString::createFromAscii(aCategorie.aImageResId)));
        if ( pEntry )
            pEntry->SetUserData( new ElementType( aCategorie.eType ) );
    }

    SetChoiceWithCursor();
    SetSelectionMode(SelectionMode::Single);
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
    DropTargetHelper::dispose();
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
