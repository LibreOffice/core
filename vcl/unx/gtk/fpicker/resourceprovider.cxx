/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    { FILE_PICKER_OVERWRITE,                    STR_FPICKER_ALREADYEXISTOVERWRITE },
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

rtl::OUString SalGtkPicker::getResString( sal_Int32 aId )
{
    rtl::OUString aResString;
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
