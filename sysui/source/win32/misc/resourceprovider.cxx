/*************************************************************************
 *
 *  $RCSfile: resourceprovider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:34:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RESOURCEPROVIDER_HXX_
#include "resourceprovider.hxx"
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

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

#define RES_NAME svt

// because the label of a listbox is
// a control itself (static text) we
// have defined a control id for this
// label which is the listbox control
// id + 100
#define LB_LABEL_OFFSET 100

const rtl::OUString TILDE = OUString::createFromAscii( "~" );
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
        m_ResMgr = CREATEVERSIONRESMGR( RES_NAME );
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
