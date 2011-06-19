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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "fltdlg.hxx"

#include "ids.hrc"

#include "fltdlg.hrc"

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XStringWidth.hpp>
#include <cppuhelper/implbase1.hxx>
#include <unotools/localfilehelper.hxx>
#include <tools/urlobj.hxx>

#include <vcl/button.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

namespace uui
{

/*-************************************************************************************************************//**
    @short      initialize filter dialog with start values
    @descr      We set some neccessary informations on these instance for later working and create internal structures.
                After construction user should call "SetFilters()" and "SetURL()" to fill listbox with selectable filter
                names and set file name of file, which should be used for selected filter.

    @seealso    method SetFilters()
    @seealso    method SetURL()

    @param      "pParentWindow"  , parent window for dialog
    @param      "pResMgr"        , ressource manager
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
FilterDialog::FilterDialog( Window* pParentWindow ,
                            ResMgr* pResMgr       )
    :   ModalDialog  ( pParentWindow, ResId( DLG_FILTER_SELECT, *pResMgr ) )
    ,   m_ftURL      ( this, ResId( FT_URL, *pResMgr            )                   )
    ,   m_lbFilters  ( this, ResId( LB_FILTERS, *pResMgr        )                   )
    ,   m_btnOK      ( this, ResId( BTN_OK, *pResMgr            )                   )
    ,   m_btnCancel  ( this, ResId( BTN_CANCEL, *pResMgr        )                   )
    ,   m_btnHelp    ( this, ResId( BTN_HELP, *pResMgr          )                   )
{
    FreeResource();
}

/*-************************************************************************************************************//**
    @short      set file name on dialog control
    @descr      We convert given URL (it must be an URL!) into valid file name and show it on our dialog.

    @seealso    -

    @param      "sURL", URL for showing
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
void FilterDialog::SetURL( const String& sURL )
{
    // convert it and use given pure string as fallback if conversion failed
    m_ftURL.SetText( impl_buildUIFileName(sURL) );
}

/*-************************************************************************************************************//**
    @short      change list of filter names
    @descr      We save given pointer internal and use it to fill our listbox with given names.
                Saved list pointer is used on method "AskForFilter()" too, to find user selected item
                and return pointer into these list as result of operation.
                So it's possible to call dialog again and again for different or same filter list
                and ask user for his decision by best performance!

    @attention  Don't free memory of given list after this call till object will die ... or
                you call "ChangeFilters( NULL )"! Then we forget it too.

    @seealso    method AskForFilter()

    @param      "pFilterNames", pointer to list of filter names, which should be used for later operations.
    @return     -

    @onerror    We clear list box and forget our currently set filter informations completly!
    @threadsafe no
*//*-*************************************************************************************************************/
void FilterDialog::ChangeFilters( const FilterNameList* pFilterNames )
{
    m_pFilterNames = pFilterNames;
    m_lbFilters.Clear();
    if( m_pFilterNames != NULL )
    {
        for( FilterNameListPtr pItem  = m_pFilterNames->begin();
                               pItem != m_pFilterNames->end()  ;
                               ++pItem                         )
        {
            m_lbFilters.InsertEntry( pItem->sUI );
        }
    }
}

/*-************************************************************************************************************//**
    @short      ask user for his decision
    @descr      We show the dialog and if user finish it with "OK" - we try to find selected item in internal saved
                name list (which you must set in "ChangeFilters()"!). If we return sal_True as result, you can use out
                parameter "pSelectedItem" as pointer into your FilterNameList to get selected item realy ...
                but if we return sal_False ... user hsa cancel the dialog ... you shouldnt do that. pSelectedItem isnt
                set to any valid value then. We don't change them ...

    @seealso    method ChangeFilters()

    @param      "pSelectedItem", returns result of selection as pointer into set list of filter names
                                 (valid for function return sal_True only!)
    @return     true  => pSelectedItem parameter points into name list and represent use decision
                false => use has cancelled dialog (pSelectedItem isnt valid then!)

    @onerror    We return false ... but don't change pSelectedItem!
    @threadsafe no
*//*-*************************************************************************************************************/
bool FilterDialog::AskForFilter( FilterNameListPtr& pSelectedItem )
{
    bool bSelected = sal_False;

    if( m_pFilterNames != NULL )
    {
        if( ModalDialog::Execute() == RET_OK )
        {
            String sEntry = m_lbFilters.GetSelectEntry();
            if( sEntry.Len() > 0 )
            {
                int nPos = m_lbFilters.GetSelectEntryPos();
                if( nPos < (int)(m_pFilterNames->size()) )
                {
                    pSelectedItem  = m_pFilterNames->begin();
                    pSelectedItem += nPos;
                    bSelected      = ( pSelectedItem != m_pFilterNames->end() );
                }
            }
        }
    }

    return bSelected;
}

/*-************************************************************************************************************//**
    @short      helper class to calculate length of given string
    @descr      Instances of it can be used as callback for INetURLObject::getAbbreviated() method to build
                short URLs to show it on GUI. We use in ctor set OutputDevice to call special VCL method ...

    @seealso    method OutputDevice::GetTextWidth()
    @seealso    method InetURLObject::getAbbreviated()

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
class StringCalculator : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        StringCalculator( const OutputDevice* pDevice )
            : m_pDevice( pDevice )
        {
        }

        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& sString ) throw( ::com::sun::star::uno::RuntimeException )
        {
            return (sal_Int32)(m_pDevice->GetTextWidth(String(sString)));
        }

    private:
        const OutputDevice* m_pDevice;
};

/*-************************************************************************************************************//**
    @short      try to build short name of given URL to show it n GUI
    @descr      We detect type of given URL automaticly and build this short name depend on this type ...
                If we couldnt make it right we return full given string without any changes ...

    @seealso    class LocalFileHelper
    @seealso    method InetURLObject::getAbbreviated()

    @param      "sName", file name
    @return     A short file name ...

    @onerror    We return given name without any changes.
    @threadsafe no
*//*-*************************************************************************************************************/
String FilterDialog::impl_buildUIFileName( const String& sName )
{
    String sShortName( sName );

    if( ::utl::LocalFileHelper::ConvertURLToSystemPath( sName, sShortName ) == sal_True )
    {
        // its a system file ... build short name by using osl functionality
    }
    else
    {
        // otherwise its really a url ... build short name by using INetURLObject
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > xStringCalculator( new StringCalculator(&m_ftURL) );
        if( xStringCalculator.is() == sal_True )
        {
            INetURLObject aBuilder   ( sName );
            Size          aSize      = m_ftURL.GetOutputSize();
                          sShortName = aBuilder.getAbbreviated( xStringCalculator, aSize.Width(), INetURLObject::DECODE_UNAMBIGUOUS );
        }
    }

    return sShortName;
}

}   // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
