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

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <tools/resmgr.hxx>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include "svids.hrc"
#include "svdata.hxx"
#include "gtk/fpicker/SalGtkPicker.hxx"

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

//---------------------------------------
// translate control ids to resource ids
//---------------------------------------
static const struct
{
    sal_Int32 ctrlId;
    sal_Int16 resId;
} CtrlIdToResIdTable[] = {
    { CHECKBOX_AUTOEXTENSION,                   STR_FPICKER_AUTO_EXTENSION },
    { CHECKBOX_PASSWORD,                        STR_FPICKER_PASSWORD },
    { CHECKBOX_FILTEROPTIONS,                   STR_FPICKER_FILTER_OPTIONS },
    { CHECKBOX_READONLY,                        STR_FPICKER_READONLY },
    { CHECKBOX_LINK,                            STR_FPICKER_INSERT_AS_LINK },
    { CHECKBOX_PREVIEW,                         STR_FPICKER_SHOW_PREVIEW },
    { PUSHBUTTON_PLAY,                          STR_FPICKER_PLAY },
    { LISTBOX_VERSION_LABEL,                    STR_FPICKER_VERSION },
    { LISTBOX_TEMPLATE_LABEL,                   STR_FPICKER_TEMPLATES },
    { LISTBOX_IMAGE_TEMPLATE_LABEL,             STR_FPICKER_IMAGE_TEMPLATE },
    { CHECKBOX_SELECTION,                       STR_FPICKER_SELECTION },
    { FOLDERPICKER_TITLE,                       STR_FPICKER_FOLDER_DEFAULT_TITLE },
    { FOLDER_PICKER_DEF_DESCRIPTION,            STR_FPICKER_FOLDER_DEFAULT_DESCRIPTION },
    { FILE_PICKER_OVERWRITE_PRIMARY,            STR_FPICKER_ALREADYEXISTOVERWRITE_PRIMARY },
    { FILE_PICKER_OVERWRITE_SECONDARY,          STR_FPICKER_ALREADYEXISTOVERWRITE_SECONDARY },
    { FILE_PICKER_ALLFORMATS,                   STR_FPICKER_ALLFORMATS },
    { FILE_PICKER_TITLE_OPEN,                   STR_FPICKER_OPEN },
    { FILE_PICKER_TITLE_SAVE,                   STR_FPICKER_SAVE },
    { FILE_PICKER_FILE_TYPE,                    STR_FPICKER_TYPE }
};

static sal_Int16 CtrlIdToResId( sal_Int32 aControlId )
{
    for ( size_t i = 0; i < SAL_N_ELEMENTS( CtrlIdToResIdTable ); i++ )
    {
        if ( CtrlIdToResIdTable[i].ctrlId == aControlId )
            return CtrlIdToResIdTable[i].resId;
    }
    return -1;
}

OUString SalGtkPicker::getResString( sal_Int32 aId )
{
    OUString aResString;
    try
    {
        // translate the control id to a resource id
        sal_Int16 aResId = CtrlIdToResId( aId );
        if ( aResId > -1 )
            aResString = ResId(aResId, *ImplGetResMgr()).toString();
    }
    catch(...)
    {
    }

    return aResString.replace('~', '_');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
