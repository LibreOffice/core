/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mnemonicengine.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-06-12 05:36:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef VCL_MNEMONICENGINE_HXX
#include "mnemonicengine.hxx"
#endif

#include "i18nhelp.hxx"
#include "svapp.hxx"
#include "event.hxx"

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
        BOOL bAccelKey = _rKEvt.GetKeyCode().IsMod2();
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
