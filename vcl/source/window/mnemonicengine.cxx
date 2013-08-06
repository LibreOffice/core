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

#include <vcl/mnemonicengine.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

namespace vcl
{
    struct MnemonicEngine_Data
    {
        IMnemonicEntryList& rEntryList;

        MnemonicEngine_Data( IMnemonicEntryList& _rEntryList )
            :rEntryList( _rEntryList )
        {
        }
    };

    namespace
    {
        const void* lcl_getEntryForMnemonic( IMnemonicEntryList& _rEntryList, sal_Unicode _cMnemonic, bool& _rbAmbiguous )
        {
            _rbAmbiguous = false;

            const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

            OUString sEntryText;
            const void* pSearchEntry = _rEntryList.FirstSearchEntry( sEntryText );

            const void* pFirstFoundEntry = NULL;
            bool bCheckingAmbiguity = false;
            const void* pStartedWith = pSearchEntry;
            while ( pSearchEntry )
            {
                if ( rI18nHelper.MatchMnemonic( sEntryText, _cMnemonic ) )
                {
                    if ( bCheckingAmbiguity )
                    {
                        // that's the second (at least) entry with this mnemonic
                        _rbAmbiguous = true;
                        return pFirstFoundEntry;
                    }

                    pFirstFoundEntry = pSearchEntry;
                    bCheckingAmbiguity = true;
                }

                pSearchEntry = _rEntryList.NextSearchEntry( pSearchEntry, sEntryText );
                if ( pSearchEntry == pStartedWith )
                    break;
            }

            return pFirstFoundEntry;
        }
    }

    MnemonicEngine::MnemonicEngine( IMnemonicEntryList& _rEntryList )
        :m_pData( new MnemonicEngine_Data( _rEntryList ) )
    {
    }

    bool MnemonicEngine::HandleKeyEvent( const KeyEvent& _rKEvt )
    {
        sal_Bool bAccelKey = _rKEvt.GetKeyCode().IsMod2();
        if ( !bAccelKey )
            return false;

        sal_Unicode cChar = _rKEvt.GetCharCode();
        bool bAmbiguous = false;
        const void* pEntry = lcl_getEntryForMnemonic( m_pData->rEntryList, cChar, bAmbiguous );
        if ( !pEntry )
            return false;

        m_pData->rEntryList.SelectSearchEntry( pEntry );
        if ( !bAmbiguous )
            m_pData->rEntryList.ExecuteSearchEntry( pEntry );

        // handled
        return true;
    }

    MnemonicEngine::~MnemonicEngine()
    {
    }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
