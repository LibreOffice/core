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
#include "precompiled_svx.hxx"

#include "svx/ParseContext.hxx"
#include "stringlistresource.hxx"
#include "svx/fmresids.hrc"

#include <svx/dialmgr.hxx>

#include <unotools/syslocale.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <vos/mutex.hxx>

using namespace svxform;
using namespace ::connectivity;
//==========================================================================
//= OSystemParseContext
//==========================================================================
DBG_NAME(OSystemParseContext)
//-----------------------------------------------------------------------------
OSystemParseContext::OSystemParseContext() : IParseContext()
{
    DBG_CTOR(OSystemParseContext,NULL);
    vos::OGuard aGuard( Application::GetSolarMutex() );

    ::svx::StringListResource aKeywords( SVX_RES( RID_RSC_SQL_INTERNATIONAL ) );
    aKeywords.get( m_aLocalizedKeywords );
}

//-----------------------------------------------------------------------------
OSystemParseContext::~OSystemParseContext()
{
    DBG_DTOR(OSystemParseContext,NULL);
}

//-----------------------------------------------------------------------------
::com::sun::star::lang::Locale OSystemParseContext::getPreferredLocale( ) const
{
    return SvtSysLocale().GetLocaleData().getLocale();
}

//-----------------------------------------------------------------------------
::rtl::OUString OSystemParseContext::getErrorMessage(ErrorCode _eCode) const
{
    String aMsg;
    vos::OGuard aGuard( Application::GetSolarMutex() );
    switch (_eCode)
    {
        case ERROR_GENERAL:                 aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_ERROR); break;
        case ERROR_VALUE_NO_LIKE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_VALUE_NO_LIKE); break;
        case ERROR_FIELD_NO_LIKE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_FIELD_NO_LIKE); break;
        case ERROR_INVALID_COMPARE:         aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_CRIT_NO_COMPARE); break;
        case ERROR_INVALID_INT_COMPARE:     aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_INT_NO_VALID); break;
        case ERROR_INVALID_DATE_COMPARE:    aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_ACCESS_DAT_NO_VALID); break;
        case ERROR_INVALID_REAL_COMPARE:    aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_REAL_NO_VALID); break;
        case ERROR_INVALID_TABLE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_TABLE); break;
        case ERROR_INVALID_TABLE_OR_QUERY:  aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_TABLE_OR_QUERY); break;
        case ERROR_INVALID_COLUMN:          aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_COLUMN); break;
        case ERROR_INVALID_TABLE_EXIST:     aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_TABLE_EXISTS); break;
        case ERROR_INVALID_QUERY_EXIST:     aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_QUERY_EXISTS); break;
        case ERROR_NONE: break;
    }
    return aMsg;
}

//-----------------------------------------------------------------------------
::rtl::OString OSystemParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
    size_t nIndex = 0;
    switch ( _eKey )
    {
        case KEY_LIKE:      nIndex = 0; break;
        case KEY_NOT:       nIndex = 1; break;
        case KEY_NULL:      nIndex = 2; break;
        case KEY_TRUE:      nIndex = 3; break;
        case KEY_FALSE:     nIndex = 4; break;
        case KEY_IS:        nIndex = 5; break;
        case KEY_BETWEEN:   nIndex = 6; break;
        case KEY_OR:        nIndex = 7; break;
        case KEY_AND:       nIndex = 8; break;
        case KEY_AVG:       nIndex = 9; break;
        case KEY_COUNT:     nIndex = 10; break;
        case KEY_MAX:       nIndex = 11; break;
        case KEY_MIN:       nIndex = 12; break;
        case KEY_SUM:       nIndex = 13; break;
        case KEY_EVERY:     nIndex = 14; break;
        case KEY_ANY:       nIndex = 15; break;
        case KEY_SOME:      nIndex = 16; break;
        case KEY_STDDEV_POP: nIndex = 17; break;
        case KEY_STDDEV_SAMP: nIndex = 18; break;
        case KEY_VAR_SAMP:  nIndex = 19; break;
        case KEY_VAR_POP:   nIndex = 20; break;
        case KEY_COLLECT:   nIndex = 21; break;
        case KEY_FUSION:    nIndex = 22; break;
        case KEY_INTERSECTION: nIndex = 23; break;
        case KEY_NONE:
            DBG_ERROR( "OSystemParseContext::getIntlKeywordAscii: illegal argument!" );
            break;
    }

    OSL_ENSURE( nIndex < m_aLocalizedKeywords.size(), "OSystemParseContext::getIntlKeywordAscii: invalid index!" );

    ByteString sKeyword;
    if ( nIndex < m_aLocalizedKeywords.size() )
        sKeyword = ByteString( m_aLocalizedKeywords[nIndex], RTL_TEXTENCODING_UTF8 );
    return sKeyword;
}

//-----------------------------------------------------------------------------
static sal_Unicode lcl_getSeparatorChar( const String& _rSeparator, sal_Unicode _nFallback )
{
    DBG_ASSERT( 0 < _rSeparator.Len(), "::lcl_getSeparatorChar: invalid decimal separator!" );

    sal_Unicode nReturn( _nFallback );
    if ( _rSeparator.Len() )
        nReturn = static_cast< sal_Char >( _rSeparator.GetBuffer( )[0] );
    return nReturn;
}

//-----------------------------------------------------------------------------
sal_Unicode OSystemParseContext::getNumDecimalSep( ) const
{
    return lcl_getSeparatorChar( SvtSysLocale().GetLocaleData().getNumDecimalSep(), '.' );
}

//-----------------------------------------------------------------------------
sal_Unicode OSystemParseContext::getNumThousandSep( ) const
{
    return lcl_getSeparatorChar( SvtSysLocale().GetLocaleData().getNumThousandSep(), ',' );
}
// -----------------------------------------------------------------------------
IParseContext::InternationalKeyCode OSystemParseContext::getIntlKeyCode(const ::rtl::OString& rToken) const
{
    static IParseContext::InternationalKeyCode Intl_TokenID[] =
    {
        KEY_LIKE, KEY_NOT, KEY_NULL, KEY_TRUE,
        KEY_FALSE, KEY_IS, KEY_BETWEEN, KEY_OR,
        KEY_AND, KEY_AVG, KEY_COUNT, KEY_MAX,
        KEY_MIN, KEY_SUM, KEY_EVERY,
        KEY_ANY, KEY_SOME, KEY_STDDEV_POP,
        KEY_STDDEV_SAMP, KEY_VAR_SAMP, KEY_VAR_POP,
        KEY_COLLECT, KEY_FUSION, KEY_INTERSECTION
    };

    sal_uInt32 nCount = sizeof Intl_TokenID / sizeof Intl_TokenID[0];
    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        ::rtl::OString aKey = getIntlKeywordAscii(Intl_TokenID[i]);
        if (rToken.equalsIgnoreAsciiCase(aKey))
            return Intl_TokenID[i];
    }

    return KEY_NONE;
}


// =============================================================================
// =============================================================================
namespace
{
    // -----------------------------------------------------------------------------
    ::osl::Mutex& getSafteyMutex()
    {
        static ::osl::Mutex s_aSafety;
        return s_aSafety;
    }
    // -----------------------------------------------------------------------------
    oslInterlockedCount& getCounter()
    {
        static oslInterlockedCount s_nCounter;
        return s_nCounter;
    }
    // -----------------------------------------------------------------------------
    OSystemParseContext* getSharedContext(OSystemParseContext* _pContext = NULL,sal_Bool _bSet = sal_False)
    {
        static OSystemParseContext* s_pSharedContext = NULL;
        if ( _pContext && !s_pSharedContext )
        {
            s_pSharedContext = _pContext;
            return s_pSharedContext;
        }
        if ( _bSet )
        {
            OSystemParseContext* pReturn = _pContext ? _pContext : s_pSharedContext;
            s_pSharedContext = _pContext;
            return pReturn;
        }
        return s_pSharedContext;
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
OParseContextClient::OParseContextClient()
{
    ::osl::MutexGuard aGuard( getSafteyMutex() );
    if ( 1 == osl_incrementInterlockedCount( &getCounter() ) )
    {   // first instance
        getSharedContext( new OSystemParseContext );
    }
}

// -----------------------------------------------------------------------------
OParseContextClient::~OParseContextClient()
{
    {
        ::osl::MutexGuard aGuard( getSafteyMutex() );
        if ( 0 == osl_decrementInterlockedCount( &getCounter() ) )
            delete getSharedContext(NULL,sal_True);
    }
}
// -----------------------------------------------------------------------------
const OSystemParseContext* OParseContextClient::getParseContext() const
{
    return getSharedContext();
}
// -----------------------------------------------------------------------------





