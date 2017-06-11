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

#include <sal/config.h>

#include <o3tl/make_unique.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <fpicker/fpicker.hrc>
#include <vcl/svapp.hxx>
#include <tools/resmgr.hxx>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include <svtools/filedlg2.hrc>
#include "NSString_OOoAdditions.hxx"
#include "fpsofficeResMgr.hxx"
#include "resourceprovider.hxx"

using rtl::OUString;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

static const char* const OTHER_RES_NAME = "svt";

// we have to translate control ids to resource ids

struct Entry
{
    sal_Int32 ctrlId;
    const char* resId;
};

struct OldEntry
{
    sal_Int32 ctrlId;
    sal_Int16 resId;
};

Entry const CtrlIdToResIdTable[] = {
    { CHECKBOX_AUTOEXTENSION,                   STR_SVT_FILEPICKER_AUTO_EXTENSION },
    { CHECKBOX_PASSWORD,                        STR_SVT_FILEPICKER_PASSWORD },
    { CHECKBOX_FILTEROPTIONS,                   STR_SVT_FILEPICKER_FILTER_OPTIONS },
    { CHECKBOX_READONLY,                        STR_SVT_FILEPICKER_READONLY },
    { CHECKBOX_LINK,                            STR_SVT_FILEPICKER_INSERT_AS_LINK },
    { CHECKBOX_PREVIEW,                         STR_SVT_FILEPICKER_SHOW_PREVIEW },
    { PUSHBUTTON_PLAY,                          STR_SVT_FILEPICKER_PLAY },
    { LISTBOX_VERSION_LABEL,                    STR_SVT_FILEPICKER_VERSION },
    { LISTBOX_TEMPLATE_LABEL,                   STR_SVT_FILEPICKER_TEMPLATES },
    { LISTBOX_IMAGE_TEMPLATE_LABEL,             STR_SVT_FILEPICKER_IMAGE_TEMPLATE },
    { CHECKBOX_SELECTION,                       STR_SVT_FILEPICKER_SELECTION },
    { FOLDERPICKER_TITLE,                       STR_SVT_FOLDERPICKER_DEFAULT_TITLE },
    { FOLDER_PICKER_DEF_DESCRIPTION,            STR_SVT_FOLDERPICKER_DEFAULT_DESCRIPTION },
    { FILE_PICKER_OVERWRITE,                    STR_SVT_ALREADYEXISTOVERWRITE },
    { LISTBOX_FILTER_LABEL,                     STR_SVT_FILEPICKER_FILTER_TITLE}
};

OldEntry const OtherCtrlIdToResIdTable[] = {
    { FILE_PICKER_TITLE_OPEN,                   STR_FILEDLG_OPEN },
    { FILE_PICKER_TITLE_SAVE,                   STR_FILEDLG_SAVE },
    { FILE_PICKER_FILE_TYPE,                    STR_FILEDLG_TYPE }
};

const sal_Int32 SIZE_TABLE = SAL_N_ELEMENTS( CtrlIdToResIdTable );
const sal_Int32 OTHER_SIZE_TABLE = SAL_N_ELEMENTS( OtherCtrlIdToResIdTable );

const char* CtrlIdToResId(sal_Int32 aControlId)
{
    const char *pResId = nullptr;

    for ( sal_Int32 i = 0; i < SIZE_TABLE; i++ )
    {
        if ( CtrlIdToResIdTable[i].ctrlId == aControlId )
        {
            pResId = CtrlIdToResIdTable[i].resId;
            break;
        }
    }

    return pResId;
}

sal_Int16 OtherCtrlIdToResId( sal_Int32 aControlId )
{
    sal_Int16 aResId = -1;

    for ( sal_Int32 i = 0; i < OTHER_SIZE_TABLE; i++ )
    {
        if ( OtherCtrlIdToResIdTable[i].ctrlId == aControlId )
        {
            aResId = OtherCtrlIdToResIdTable[i].resId;
            break;
        }
    }

    return aResId;
}

class CResourceProvider_Impl
{
public:
    CResourceProvider_Impl( )
    {
        m_OtherResMgr = ResMgr::CreateResMgr( OTHER_RES_NAME );
    }

    ~CResourceProvider_Impl( )
    {
        delete m_OtherResMgr;
    }

    NSString* getResString( sal_Int16 aId )
    {
        OUString aResString;

        const SolarMutexGuard aGuard;

        try
        {
            OSL_ASSERT(m_OtherResMgr);

            // translate the control id to a resource id
            const char* pResId = CtrlIdToResId(aId);
            if (pResId)
                aResString = FpsResId(pResId);
            else
            {
                sal_Int16 aResId = OtherCtrlIdToResId( aId );
                if ( aResId > -1 ) {
                    aResString = ResId( aResId, *m_OtherResMgr );
                }
            }
        }
        catch(...)
        {
        }

        return [NSString stringWithOUString:aResString];
    }

public:
    ResMgr* m_OtherResMgr;
};

CResourceProvider::CResourceProvider( ) :
    m_pImpl( o3tl::make_unique<CResourceProvider_Impl>() )
{
}

CResourceProvider::~CResourceProvider( )
{}

NSString* CResourceProvider::getResString( sal_Int32 aId )
{
    NSString* sImmutable = m_pImpl->getResString( aId );
    NSMutableString *sMutableString = [NSMutableString stringWithString:sImmutable];
    [sMutableString replaceOccurrencesOfString:@"~" withString:@"" options:0 range:NSMakeRange(0, [sMutableString length])];

    NSString *result = [NSString stringWithString:sMutableString];

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
