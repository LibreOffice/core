/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourceprovider.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:52:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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

#ifndef _SVTOOLS_SVTOOLS_HRC_
#include <svtools/svtools.hrc>
#endif

#ifndef _SVTOOLS_FILEDLG2_HRC_
#include <svtools/filedlg2.hrc>
#endif

#ifndef _NSSTRING_OOOADDITIONS_HXX_
#include "NSString_OOoAdditions.hxx"
#endif

#include "resourceprovider.hxx"

//------------------------------------------------------------
// namespace directives
//------------------------------------------------------------

using rtl::OUString;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

//------------------------------------------------------------
//
//------------------------------------------------------------

static const char* RES_NAME = "fps_office";
static const char* OTHER_RES_NAME = "svt";

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
    { LISTBOX_VERSION_LABEL,                    STR_SVT_FILEPICKER_VERSION },
    { LISTBOX_TEMPLATE_LABEL,                   STR_SVT_FILEPICKER_TEMPLATES },
    { LISTBOX_IMAGE_TEMPLATE_LABEL,             STR_SVT_FILEPICKER_IMAGE_TEMPLATE },
    { CHECKBOX_SELECTION,                       STR_SVT_FILEPICKER_SELECTION },
    { FOLDERPICKER_TITLE,                       STR_SVT_FOLDERPICKER_DEFAULT_TITLE },
    { FOLDER_PICKER_DEF_DESCRIPTION,            STR_SVT_FOLDERPICKER_DEFAULT_DESCRIPTION },
    { FILE_PICKER_OVERWRITE,                    STR_SVT_ALREADYEXISTOVERWRITE },
    { LISTBOX_FILTER_LABEL,                     STR_SVT_FILEPICKER_FILTER_TITLE}
};

_Entry OtherCtrlIdToResIdTable[] = {
    { FILE_PICKER_TITLE_OPEN,                   STR_FILEDLG_OPEN },
    { FILE_PICKER_TITLE_SAVE,                   STR_FILEDLG_SAVE },
    { FILE_PICKER_FILE_TYPE,                    STR_FILEDLG_TYPE }
};


const sal_Int32 SIZE_TABLE = sizeof( CtrlIdToResIdTable ) / sizeof( _Entry );
const sal_Int32 OTHER_SIZE_TABLE = sizeof( OtherCtrlIdToResIdTable ) / sizeof( _Entry );

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
        m_ResMgr = ResMgr::CreateResMgr( RES_NAME );
        m_OtherResMgr = ResMgr::CreateResMgr( OTHER_RES_NAME );
    }

    //-------------------------------------
    //
    //-------------------------------------

    ~CResourceProvider_Impl( )
    {
        delete m_ResMgr;
        delete m_OtherResMgr;
    }

    //-------------------------------------
    //
    //-------------------------------------

    NSString* getResString( sal_Int16 aId )
    {
        String   aResString;
        OUString aResOUString;

        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        try
        {
            OSL_ASSERT( m_ResMgr && m_OtherResMgr );

            // translate the control id to a resource id
            sal_Int16 aResId = CtrlIdToResId( aId );
            if ( aResId > -1 )
                aResString = String( ResId( aResId, *m_ResMgr ) );
            else
            {
                aResId = OtherCtrlIdToResId( aId );
                if ( aResId > -1 ) {
                    aResString = String( ResId( aResId, *m_OtherResMgr ) );
                }
            }
            if ( aResId > -1 )
                aResOUString = OUString( aResString );
        }
        catch(...)
        {
        }

        return [NSString stringWithOUString:aResOUString];
    }

public:
    ResMgr* m_ResMgr;
    ResMgr* m_OtherResMgr;
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

NSString* CResourceProvider::getResString( sal_Int32 aId )
{
    NSString* sImmutable = m_pImpl->getResString( aId );
    NSMutableString *sMutableString = [NSMutableString stringWithString:sImmutable];
    [sMutableString replaceOccurrencesOfString:@"~" withString:@"" options:0 range:NSMakeRange(0, [sMutableString length])];

    NSString *result = [NSString stringWithString:sMutableString];

    return result;
}
