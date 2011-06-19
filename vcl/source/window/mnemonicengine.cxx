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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/mnemonicengine.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

//........................................................................
namespace vcl
{
//........................................................................

    //====================================================================
    //= MnemonicEngine_Data
    //====================================================================
    struct MnemonicEngine_Data
    {
        IMnemonicEntryList& rEntryList;

        MnemonicEngine_Data( IMnemonicEntryList& _rEntryList )
            :rEntryList( _rEntryList )
        {
        }
    };

    //--------------------------------------------------------------------
    namespace
    {
        const void* lcl_getEntryForMnemonic( IMnemonicEntryList& _rEntryList, sal_Unicode _cMnemonic, bool& _rbAmbiguous )
        {
            _rbAmbiguous = false;

            const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

            String sEntryText;
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

    //====================================================================
    //= MnemonicEngine
    //====================================================================
    //--------------------------------------------------------------------
    MnemonicEngine::MnemonicEngine( IMnemonicEntryList& _rEntryList )
        :m_pData( new MnemonicEngine_Data( _rEntryList ) )
    {
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    MnemonicEngine::~MnemonicEngine()
    {
    }

//........................................................................
} // namespace vcl
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
