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

#include "fltdlg.hxx"

#include <ids.hxx>

#include <com/sun/star/util/XStringWidth.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/urlobj.hxx>

#include <vcl/button.hxx>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>

namespace uui
{

/*-************************************************************************************************************
    @short      initialize filter dialog with start values
    @descr      We set some necessary information on these instance for later working and create internal structures.
                After construction user should call "SetFilters()" and "SetURL()" to fill listbox with selectable filter
                names and set file name of file, which should be used for selected filter.

    @seealso    method SetFilters()
    @seealso    method SetURL()

    @param      "pParentWindow"  , parent window for dialog
    @threadsafe no
*//*-*************************************************************************************************************/
FilterDialog::FilterDialog(weld::Window* pParentWindow)
    : GenericDialogController(pParentWindow, "uui/ui/filterselect.ui", "FilterSelectDialog")
    , m_pFilterNames(nullptr)
    , m_xFtURL(m_xBuilder->weld_label("url"))
    , m_xLbFilters(m_xBuilder->weld_tree_view("filters"))
{
    m_xLbFilters->set_size_request(m_xLbFilters->get_approximate_digit_width() * 42,
                                   m_xLbFilters->get_height_rows(15));
}

FilterDialog::~FilterDialog()
{
}

/*-************************************************************************************************************
    @short      set file name on dialog control
    @descr      We convert given URL (it must be an URL!) into valid file name and show it on our dialog.
    @param      "sURL", URL for showing
    @threadsafe no
*//*-*************************************************************************************************************/
void FilterDialog::SetURL( const OUString& sURL )
{
    // convert it and use given pure string as fallback if conversion failed
    m_xFtURL->set_label(impl_buildUIFileName(sURL));
}

/*-************************************************************************************************************
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
    @onerror    We clear list box and forget our currently set filter information completely!
    @threadsafe no
*//*-*************************************************************************************************************/
void FilterDialog::ChangeFilters( const FilterNameList* pFilterNames )
{
    m_pFilterNames = pFilterNames;
    m_xLbFilters->clear();
    if( m_pFilterNames != nullptr )
    {
        for( const auto& rItem : *m_pFilterNames )
        {
            m_xLbFilters->append_text(rItem.sUI);
        }
    }
}

/*-************************************************************************************************************
    @short      ask user for his decision
    @descr      We show the dialog and if user finish it with "OK" - we try to find selected item in internal saved
                name list (which you must set in "ChangeFilters()"!). If we return sal_True as result, you can use out
                parameter "pSelectedItem" as pointer into your FilterNameList to get selected item really ...
                but if we return sal_False ... user has cancel the dialog ... you should not do that. pSelectedItem is not
                set to any valid value then. We don't change them ...

    @seealso    method ChangeFilters()

    @param      "pSelectedItem", returns result of selection as pointer into set list of filter names
                                 (valid for function return sal_True only!)
    @return     true  => pSelectedItem parameter points into name list and represent use decision
                false => use has cancelled dialog (pSelectedItem is not valid then!)

    @onerror    We return false ... but don't change pSelectedItem!
    @threadsafe no
*//*-*************************************************************************************************************/
bool FilterDialog::AskForFilter( FilterNameListPtr& pSelectedItem )
{
    bool bSelected = false;

    if( m_pFilterNames != nullptr )
    {
        if (m_xDialog->run() == RET_OK)
        {
            OUString sEntry = m_xLbFilters->get_selected_text();
            if( !sEntry.isEmpty() )
            {
                int nPos = m_xLbFilters->get_selected_index();
                if( nPos < static_cast<int>(m_pFilterNames->size()) )
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

/*-************************************************************************************************************
    @short      helper class to calculate length of given string
    @descr      Instances of it can be used as callback for INetURLObject::getAbbreviated() method to build
                short URLs to show it on GUI. We use in ctor set OutputDevice to call special VCL method ...

    @seealso    method OutputDevice::GetTextWidth()
    @seealso    method InetURLObject::getAbbreviated()
    @threadsafe no
*//*-*************************************************************************************************************/
class StringCalculator : public ::cppu::WeakImplHelper< css::util::XStringWidth >
{
    public:
        explicit StringCalculator(weld::Widget* pDevice)
            : m_pDevice(pDevice)
        {
        }

        sal_Int32 SAL_CALL queryStringWidth( const OUString& sString ) override
        {
            return static_cast<sal_Int32>(m_pDevice->get_pixel_size(sString).Width());
        }

    private:
        weld::Widget* const m_pDevice;
};

/*-************************************************************************************************************
    @short      try to build short name of given URL to show it n GUI
    @descr      We detect type of given URL automatically and build this short name depend on this type ...
                If we couldn't make it right we return full given string without any changes ...

    @seealso    method InetURLObject::getAbbreviated()

    @param      "sName", file name
    @return     A short file name ...

    @onerror    We return given name without any changes.
    @threadsafe no
*//*-*************************************************************************************************************/
OUString FilterDialog::impl_buildUIFileName( const OUString& sName )
{
    OUString sShortName( sName );

    if (osl::FileBase::getSystemPathFromFileURL(sName, sShortName) == osl::FileBase::E_None)

    {
        // it's a system file ... build short name by using osl functionality
    }
    else
    {
        // otherwise its really a url ... build short name by using INetURLObject
        css::uno::Reference< css::util::XStringWidth > xStringCalculator(new StringCalculator(m_xFtURL.get()));
        if( xStringCalculator.is() )
        {
            INetURLObject aBuilder   ( sName );
            Size          aSize      = m_xLbFilters->get_preferred_size();
            sShortName = aBuilder.getAbbreviated( xStringCalculator, aSize.Width(), INetURLObject::DecodeMechanism::Unambiguous );
        }
    }

    return sShortName;
}

}   // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
