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


#include <sal/macros.h>
#include "svx/ParseContext.hxx"
#include "stringlistresource.hxx"
#include "svx/fmresids.hrc"

#include <svx/dialmgr.hxx>

#include <unotools/syslocale.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>

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
    SolarMutexGuard aGuard;

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
    return SvtSysLocale().GetLanguageTag().getLocale();
}

//-----------------------------------------------------------------------------
OUString OSystemParseContext::getErrorMessage(ErrorCode _eCode) const
{
    OUString aMsg;
    SolarMutexGuard aGuard;
    switch (_eCode)
    {
        case ERROR_GENERAL:                 aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_ERROR); break;
        case ERROR_VALUE_NO_LIKE:           aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_VALUE_NO_LIKE); break;
        case ERROR_FIELD_NO_LIKE:           aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_FIELD_NO_LIKE); break;
        case ERROR_INVALID_COMPARE:         aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_CRIT_NO_COMPARE); break;
        case ERROR_INVALID_INT_COMPARE:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_INT_NO_VALID); break;
        case ERROR_INVALID_DATE_COMPARE:    aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_ACCESS_DAT_NO_VALID); break;
        case ERROR_INVALID_REAL_COMPARE:    aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_REAL_NO_VALID); break;
        case ERROR_INVALID_TABLE:           aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE); break;
        case ERROR_INVALID_TABLE_OR_QUERY:  aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE_OR_QUERY); break;
        case ERROR_INVALID_COLUMN:          aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_COLUMN); break;
        case ERROR_INVALID_TABLE_EXIST:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE_EXISTS); break;
        case ERROR_INVALID_QUERY_EXIST:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_QUERY_EXISTS); break;
        case ERROR_NONE: break;
    }
    return aMsg;
}

//-----------------------------------------------------------------------------
OString OSystemParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
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
            OSL_FAIL( "OSystemParseContext::getIntlKeywordAscii: illegal argument!" );
            break;
    }

    OSL_ENSURE( nIndex < m_aLocalizedKeywords.size(), "OSystemParseContext::getIntlKeywordAscii: invalid index!" );

    OString sKeyword;
    if ( nIndex < m_aLocalizedKeywords.size() )
        sKeyword = OUStringToOString(m_aLocalizedKeywords[nIndex], RTL_TEXTENCODING_UTF8);
    return sKeyword;
}

// -----------------------------------------------------------------------------
IParseContext::InternationalKeyCode OSystemParseContext::getIntlKeyCode(const OString& rToken) const
{
    static const IParseContext::InternationalKeyCode Intl_TokenID[] =
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
        OString aKey = getIntlKeywordAscii(Intl_TokenID[i]);
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
    if ( 1 == osl_atomic_increment( &getCounter() ) )
    {   // first instance
        getSharedContext( new OSystemParseContext );
    }
}

// -----------------------------------------------------------------------------
OParseContextClient::~OParseContextClient()
{
    {
        ::osl::MutexGuard aGuard( getSafteyMutex() );
        if ( 0 == osl_atomic_decrement( &getCounter() ) )
            delete getSharedContext(NULL,sal_True);
    }
}
// -----------------------------------------------------------------------------
const OSystemParseContext* OParseContextClient::getParseContext() const
{
    return getSharedContext();
}
// -----------------------------------------------------------------------------





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
