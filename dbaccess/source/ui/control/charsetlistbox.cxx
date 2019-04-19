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

#include <charsetlistbox.hxx>

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <osl/diagnose.h>

namespace dbaui
{
    CharSetListBox::CharSetListBox(std::unique_ptr<weld::ComboBox> xControl)
        : m_xControl(std::move(xControl))
    {
        for (auto const& charset : m_aCharSets)
        {
            m_xControl->append_text(charset.getDisplayName());
        }
    }

    void CharSetListBox::SelectEntryByIanaName( const OUString& _rIanaName )
    {
        OCharsetDisplay::const_iterator aFind = m_aCharSets.findIanaName( _rIanaName );
        if (aFind == m_aCharSets.end())
        {
            OSL_FAIL( "CharSetListBox::SelectEntryByIanaName: unknown charset falling back to system language!" );
            aFind = m_aCharSets.findEncoding( RTL_TEXTENCODING_DONTKNOW );
        }

        if (aFind == m_aCharSets.end())
            m_xControl->set_active(-1);
        else
            m_xControl->set_active_text((*aFind).getDisplayName());
    }

    bool CharSetListBox::StoreSelectedCharSet( SfxItemSet& _rSet, const sal_uInt16 _nItemId )
    {
        bool bChangedSomething = false;
        if (m_xControl->get_value_changed_from_saved())
        {
            OCharsetDisplay::const_iterator aFind = m_aCharSets.findDisplayName(m_xControl->get_active_text());
            OSL_ENSURE( aFind != m_aCharSets.end(), "CharSetListBox::StoreSelectedCharSet: could not translate the selected character set!" );
            if ( aFind != m_aCharSets.end() )
            {
                _rSet.Put( SfxStringItem( _nItemId, (*aFind).getIanaName() ) );
                bChangedSomething = true;
            }
        }
        return bChangedSomething;
    }
} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
