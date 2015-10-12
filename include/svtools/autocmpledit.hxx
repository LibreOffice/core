/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_AUTOCMPLEDIT_HXX
#define INCLUDED_SVTOOLS_AUTOCMPLEDIT_HXX

#include <svtools/svtdllapi.h>

#include <vcl/edit.hxx>

#include <vector>

class SVT_DLLPUBLIC AutocompleteEdit : public Edit
{
private:
    std::vector< OUString > m_aEntries;
    std::vector< OUString > m_aMatching;
    std::vector< OUString >::size_type m_nCurrent;

    void AutoCompleteHandler( Edit* );
    bool Match( const OUString& rText );
    bool PreNotify( NotifyEvent& rNEvt ) override;

public:
    AutocompleteEdit( vcl::Window* pParent );

    void AddEntry( const OUString& rEntry );
    void ClearEntries();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
