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
