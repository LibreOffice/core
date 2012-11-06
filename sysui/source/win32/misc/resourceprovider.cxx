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
#include <rtl/ustrbuf.hxx>
#include "resourceprovider.hxx"
#include <tools/resmgr.hxx>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include <svtools/svtools.hrc>

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

//------------------------------------------------------------
//
//------------------------------------------------------------

// because the label of a listbox is
// a control itself (static text) we
// have defined a control id for this
// label which is the listbox control
// id + 100
#define LB_LABEL_OFFSET 100

const rtl::OUString TILDE( "~" );
const sal_Unicode TILDE_SIGN = L'~';

#define FOLDERPICKER_TITLE            500
#define FOLDER_PICKER_DEF_DESCRIPTION 501

//------------------------------------------------------------
// we have to translate control ids to resource ids
//------------------------------------------------------------

struct _Entry
{
    sal_Int32 ctrlId;
    sal_Int16 resId;
};

_Entry CtrlIdToResIdTable[] = {
    { CHECKBOX_AUTOEXTENSION,                   STR_SVT_FILEPICKER_AUTO_EXTENSION },
    { CHECKBOX_PASSWORD,                        STR_SVT_FILEPICKER_PASSWORD },
    { CHECKBOX_FILTEROPTIONS,                   STR_SVT_FILEPICKER_FILTER_OPTIONS },
    { CHECKBOX_READONLY,                        STR_SVT_FILEPICKER_READONLY },
    { CHECKBOX_LINK,                            STR_SVT_FILEPICKER_INSERT_AS_LINK },
    { CHECKBOX_PREVIEW,                         STR_SVT_FILEPICKER_SHOW_PREVIEW },
    { PUSHBUTTON_PLAY,                          STR_SVT_FILEPICKER_PLAY },
    { LISTBOX_VERSION + LB_LABEL_OFFSET,        STR_SVT_FILEPICKER_VERSION },
    { LISTBOX_TEMPLATE + LB_LABEL_OFFSET,       STR_SVT_FILEPICKER_TEMPLATES },
    { LISTBOX_IMAGE_TEMPLATE + LB_LABEL_OFFSET, STR_SVT_FILEPICKER_IMAGE_TEMPLATE },
    { CHECKBOX_SELECTION,                       STR_SVT_FILEPICKER_SELECTION },
    { FOLDERPICKER_TITLE,                       STR_SVT_FOLDERPICKER_DEFAULT_TITLE },
    { FOLDER_PICKER_DEF_DESCRIPTION,            STR_SVT_FOLDERPICKER_DEFAULT_DESCRIPTION }
};

const sal_Int32 SIZE_TABLE = sizeof( CtrlIdToResIdTable ) / sizeof( _Entry );

//------------------------------------------------------------
//
//------------------------------------------------------------

sal_Int16 CtrlIdToResId( sal_Int32 aControlId )
{
    sal_Int16 aResId = -1;

    for ( sal_Int32 i = 0; i < SIZE_TABLE; i++ )
    {
        if ( CtrlIdToResIdTable[i].ctrlId == aControlId )
        {
            aResId = CtrlIdToResIdTable[i].resId;
            break;
        }
    }

    return aResId;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

class CResourceProvider_Impl
{
public:

    //-------------------------------------
    //
    //-------------------------------------

    CResourceProvider_Impl( )
    {
        m_ResMgr = ResMgr::CreateResMgr("svt");
    }

    //-------------------------------------
    //
    //-------------------------------------

    ~CResourceProvider_Impl( )
    {
        delete m_ResMgr;
    }

    //-------------------------------------
    //
    //-------------------------------------

    OUString getResString( sal_Int16 aId )
    {
        String   aResString;
        OUString aResOUString;

        try
        {
            OSL_ASSERT( m_ResMgr );

            // translate the control id to a resource id
            sal_Int16 aResId = CtrlIdToResId( aId );

            if ( aResId > -1 )
            {
                aResString = String( ResId( aResId, m_ResMgr ) );
                aResOUString = OUString( aResString );

                // remove '~' signs, if there are two '~' signs
                // in a row we remove only one of them
                if ( aResOUString.indexOf( TILDE ) > -1 )
                {
                    sal_Int32 nStrLen = aResOUString.getLength( );
                    rtl::OUStringBuffer aBuffer( nStrLen );
                    sal_Int32 i = 0;
                    const sal_Unicode* pPos  = aResOUString.getStr( );
                    const sal_Unicode* pNext = aResOUString.getStr( ) + 1;
                    const sal_Unicode* pEnd  = aResOUString.getStr( ) + nStrLen;

                    while( pPos < pEnd )
                    {
                        // we insert the next character only if the current character
                        // in not a '~' or the following character is also a '~'
                        if ( (*pPos != TILDE_SIGN) ||
                             ((*pPos == TILDE_SIGN) && (pNext < pEnd) && (*pNext == TILDE_SIGN)) )
                        {
                            aBuffer.insert( i, *pPos );
                            i++;
                        }

                        pPos++;
                        pNext++;
                    }

                    aResOUString = aBuffer.makeStringAndClear( );
                }
            }
        }
        catch(...)
        {
        }

        return aResOUString;
    }

public:
    ResMgr* m_ResMgr;
};

//------------------------------------------------------------
//
//------------------------------------------------------------

CResourceProvider::CResourceProvider( ) :
    m_pImpl( new CResourceProvider_Impl() )
{
}

//------------------------------------------------------------
//
//------------------------------------------------------------

CResourceProvider::~CResourceProvider( )
{
    delete m_pImpl;
}

//------------------------------------------------------------
//
//------------------------------------------------------------

OUString CResourceProvider::getResString( sal_Int32 aId )
{
   return m_pImpl->getResString( aId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
