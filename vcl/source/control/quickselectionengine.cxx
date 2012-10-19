/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/


#include "vcl/quickselectionengine.hxx"
#include "vcl/event.hxx"
#include "vcl/timer.hxx"
#include "vcl/i18nhelp.hxx"
#include "vcl/svapp.hxx"

#include <boost/optional.hpp>

//........................................................................
namespace vcl
{
//........................................................................

    //====================================================================
    //= QuickSelectionEngine_Data
    //====================================================================
    struct QuickSelectionEngine_Data
    {
        ISearchableStringList&              rEntryList;
        String                              sCurrentSearchString;
        ::boost::optional< sal_Unicode >    aSingleSearchChar;
        Timer                               aSearchTimeout;

        QuickSelectionEngine_Data( ISearchableStringList& _entryList )
            :rEntryList( _entryList )
            ,sCurrentSearchString()
            ,aSingleSearchChar()
            ,aSearchTimeout()
        {
            aSearchTimeout.SetTimeout( 2500 );
            aSearchTimeout.SetTimeoutHdl( LINK( this, QuickSelectionEngine_Data, SearchStringTimeout ) );
        }

        ~QuickSelectionEngine_Data()
        {
            aSearchTimeout.Stop();
        }

        DECL_LINK( SearchStringTimeout, Timer* );
    };

    //--------------------------------------------------------------------
    namespace
    {
        static void lcl_reset( QuickSelectionEngine_Data& _data )
        {
            _data.sCurrentSearchString.Erase();
            _data.aSingleSearchChar.reset();
            _data.aSearchTimeout.Stop();
        }
    }

    //--------------------------------------------------------------------
    IMPL_LINK( QuickSelectionEngine_Data, SearchStringTimeout, Timer*, /*EMPTYARG*/ )
    {
        lcl_reset( *this );
        return 1;
    }

    //--------------------------------------------------------------------
    static StringEntryIdentifier findMatchingEntry( const String& _searchString, QuickSelectionEngine_Data& _engineData )
    {
        const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetLocaleI18nHelper();
            // TODO: do we really need the Window's settings here? The original code used it ...

        String sEntryText;
        // get the "current + 1" entry
        StringEntryIdentifier pSearchEntry = _engineData.rEntryList.CurrentEntry( sEntryText );
        if ( pSearchEntry )
            pSearchEntry = _engineData.rEntryList.NextEntry( pSearchEntry, sEntryText );
        // loop 'til we find another matching entry
        StringEntryIdentifier pStartedWith = pSearchEntry;
        while ( pSearchEntry )
        {
            if ( rI18nHelper.MatchString( _searchString, sEntryText ) != 0 )
                break;

            pSearchEntry = _engineData.rEntryList.NextEntry( pSearchEntry, sEntryText );
            if ( pSearchEntry == pStartedWith )
                pSearchEntry = NULL;
        }

        return pSearchEntry;
    }

    //====================================================================
    //= QuickSelectionEngine
    //====================================================================
    //--------------------------------------------------------------------
    QuickSelectionEngine::QuickSelectionEngine( ISearchableStringList& _entryList )
        :m_pData( new QuickSelectionEngine_Data( _entryList ) )
    {
    }

    //--------------------------------------------------------------------
    QuickSelectionEngine::~QuickSelectionEngine()
    {
    }

    //--------------------------------------------------------------------
    bool QuickSelectionEngine::HandleKeyEvent( const KeyEvent& _keyEvent )
    {
        sal_Unicode c = _keyEvent.GetCharCode();

        if ( ( c >= 32 ) && ( c != 127 ) && !_keyEvent.GetKeyCode().IsMod2() )
        {
            m_pData->sCurrentSearchString += c;
            OSL_TRACE( "QuickSelectionEngine::HandleKeyEvent: searching for %s", rtl::OUStringToOString(m_pData->sCurrentSearchString, RTL_TEXTENCODING_UTF8).getStr() );

            if ( m_pData->sCurrentSearchString.Len() == 1 )
            {   // first character in the search -> remmeber
                m_pData->aSingleSearchChar.reset( c );
            }
            else if ( m_pData->sCurrentSearchString.Len() > 1 )
            {
                if ( !!m_pData->aSingleSearchChar && ( *m_pData->aSingleSearchChar != c ) )
                    // we already have a "single char", but the current one is different -> reset
                    m_pData->aSingleSearchChar.reset();
            }

            XubString aSearchTemp( m_pData->sCurrentSearchString );

            StringEntryIdentifier pMatchingEntry = findMatchingEntry( aSearchTemp, *m_pData );
            OSL_TRACE( "QuickSelectionEngine::HandleKeyEvent: found %p", pMatchingEntry );
            if ( !pMatchingEntry && ( aSearchTemp.Len() > 1 ) && !!m_pData->aSingleSearchChar )
            {
                // if there's only one letter in the search string, use a different search mode
                aSearchTemp = *m_pData->aSingleSearchChar;
                pMatchingEntry = findMatchingEntry( aSearchTemp, *m_pData );
            }

            if ( pMatchingEntry )
            {
                m_pData->rEntryList.SelectEntry( pMatchingEntry );
                m_pData->aSearchTimeout.Start();
            }
            else
            {
                lcl_reset( *m_pData );
            }

            return true;
        }
        return false;
    }

    //--------------------------------------------------------------------
    void QuickSelectionEngine::Reset()
    {
        lcl_reset( *m_pData );
    }

//........................................................................
} // namespace vcl
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
