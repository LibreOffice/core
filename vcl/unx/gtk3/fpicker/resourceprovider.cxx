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

#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include <fpicker/strings.hrc>
#include <fpicker/fpsofficeResMgr.hxx>
#include "SalGtkPicker.hxx"

using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

// translate control ids to resource ids

const struct
{
    sal_Int32 ctrlId;
    TranslateId resId;
} CtrlIdToResIdTable[] = {
    { CHECKBOX_AUTOEXTENSION,                   STR_SVT_FILEPICKER_AUTO_EXTENSION },
    { CHECKBOX_PASSWORD,                        STR_SVT_FILEPICKER_PASSWORD },
    { CHECKBOX_GPGENCRYPTION,                   STR_SVT_FILEPICKER_GPGENCRYPT },
    { CHECKBOX_FILTEROPTIONS,                   STR_SVT_FILEPICKER_FILTER_OPTIONS },
    { CHECKBOX_READONLY,                        STR_SVT_FILEPICKER_READONLY },
    { CHECKBOX_LINK,                            STR_SVT_FILEPICKER_INSERT_AS_LINK },
    { CHECKBOX_PREVIEW,                         STR_SVT_FILEPICKER_SHOW_PREVIEW },
    { PUSHBUTTON_PLAY,                          STR_SVT_FILEPICKER_PLAY },
    { LISTBOX_VERSION_LABEL,                    STR_SVT_FILEPICKER_VERSION },
    { LISTBOX_TEMPLATE_LABEL,                   STR_SVT_FILEPICKER_TEMPLATES },
    { LISTBOX_IMAGE_TEMPLATE_LABEL,             STR_SVT_FILEPICKER_IMAGE_TEMPLATE },
    { LISTBOX_IMAGE_ANCHOR_LABEL,               STR_SVT_FILEPICKER_IMAGE_ANCHOR },
    { CHECKBOX_SELECTION,                       STR_SVT_FILEPICKER_SELECTION },
    { FOLDERPICKER_TITLE,                       STR_SVT_FOLDERPICKER_DEFAULT_TITLE },
    { FOLDER_PICKER_DEF_DESCRIPTION,            STR_SVT_FOLDERPICKER_DEFAULT_DESCRIPTION },
    { FILE_PICKER_OVERWRITE_PRIMARY,            STR_SVT_ALREADYEXISTOVERWRITE },
    { FILE_PICKER_OVERWRITE_SECONDARY,          STR_SVT_ALREADYEXISTOVERWRITE_SECONDARY },
    { FILE_PICKER_ALLFORMATS,                   STR_SVT_ALLFORMATS },
    { FILE_PICKER_TITLE_OPEN,                   STR_FILEDLG_OPEN },
    { FILE_PICKER_TITLE_SAVE,                   STR_FILEDLG_SAVE },
    { FILE_PICKER_FILE_TYPE,                    STR_FILEDLG_TYPE }
};

static TranslateId CtrlIdToResId( sal_Int32 aControlId )
{
    for (auto & i : CtrlIdToResIdTable)
    {
        if ( i.ctrlId == aControlId )
            return i.resId;
    }
    return {};
}

OUString SalGtkPicker::getResString( sal_Int32 aId )
{
    OUString aResString;
    // translate the control id to a resource id
    TranslateId pResId = CtrlIdToResId( aId );
    if (pResId)
        aResString = FpsResId(pResId);
    return aResString.replace('~', '_');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
