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
#include <osl/mutex.hxx>

using namespace svxform;
using namespace ::connectivity;

OSystemParseContext::OSystemParseContext() : IParseContext()
{
    SolarMutexGuard aGuard;

    svx::StringListResource aKeywords( SVX_RES( RID_RSC_SQL_INTERNATIONAL ) );
    aKeywords.get( m_aLocalizedKeywords );
}


OSystemParseContext::~OSystemParseContext()
{
}


css::lang::Locale OSystemParseContext::getPreferredLocale( ) const
{
    return SvtSysLocale().GetLanguageTag().getLocale();
}


OUString OSystemParseContext::getErrorMessage(ErrorCode _eCode) const
{
    OUString aMsg;
    SolarMutexGuard aGuard;
    switch (_eCode)
    {
        case ErrorCode::General:               aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_ERROR); break;
        case ErrorCode::ValueNoLike:           aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_VALUE_NO_LIKE); break;
        case ErrorCode::FieldNoLike:           aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_FIELD_NO_LIKE); break;
        case ErrorCode::InvalidCompare:        aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_CRIT_NO_COMPARE); break;
        case ErrorCode::InvalidIntCompare:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_INT_NO_VALID); break;
        case ErrorCode::InvalidDateCompare:    aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_ACCESS_DAT_NO_VALID); break;
        case ErrorCode::InvalidRealCompare:    aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_REAL_NO_VALID); break;
        case ErrorCode::InvalidTableNosuch:    aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE); break;
        case ErrorCode::InvalidTableOrQuery:   aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE_OR_QUERY); break;
        case ErrorCode::InvalidColumn:         aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_COLUMN); break;
        case ErrorCode::InvalidTableExist:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_TABLE_EXISTS); break;
        case ErrorCode::InvalidQueryExist:     aMsg = SVX_RESSTR(RID_STR_SVT_SQL_SYNTAX_QUERY_EXISTS); break;
        case ErrorCode::None: break;
    }
    return aMsg;
}


OString OSystemParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
    size_t nIndex = 0;
    switch ( _eKey )
    {
        case InternationalKeyCode::Like:      nIndex = 0; break;
        case InternationalKeyCode::Not:       nIndex = 1; break;
        case InternationalKeyCode::Null:      nIndex = 2; break;
        case InternationalKeyCode::True:      nIndex = 3; break;
        case InternationalKeyCode::False:     nIndex = 4; break;
        case InternationalKeyCode::Is:        nIndex = 5; break;
        case InternationalKeyCode::Between:   nIndex = 6; break;
        case InternationalKeyCode::Or:        nIndex = 7; break;
        case InternationalKeyCode::And:       nIndex = 8; break;
        case InternationalKeyCode::Avg:       nIndex = 9; break;
        case InternationalKeyCode::Count:     nIndex = 10; break;
        case InternationalKeyCode::Max:       nIndex = 11; break;
        case InternationalKeyCode::Min:       nIndex = 12; break;
        case InternationalKeyCode::Sum:       nIndex = 13; break;
        case InternationalKeyCode::Every:     nIndex = 14; break;
        case InternationalKeyCode::Any:       nIndex = 15; break;
        case InternationalKeyCode::Some:      nIndex = 16; break;
        case InternationalKeyCode::StdDevPop: nIndex = 17; break;
        case InternationalKeyCode::StdDevSamp: nIndex = 18; break;
        case InternationalKeyCode::VarSamp:  nIndex = 19; break;
        case InternationalKeyCode::VarPop:   nIndex = 20; break;
        case InternationalKeyCode::Collect:   nIndex = 21; break;
        case InternationalKeyCode::Fusion:    nIndex = 22; break;
        case InternationalKeyCode::Intersection: nIndex = 23; break;
        case InternationalKeyCode::None:
            OSL_FAIL( "OSystemParseContext::getIntlKeywordAscii: illegal argument!" );
            break;
    }

    OSL_ENSURE( nIndex < m_aLocalizedKeywords.size(), "OSystemParseContext::getIntlKeywordAscii: invalid index!" );

    OString sKeyword;
    if ( nIndex < m_aLocalizedKeywords.size() )
        sKeyword = OUStringToOString(m_aLocalizedKeywords[nIndex], RTL_TEXTENCODING_UTF8);
    return sKeyword;
}


IParseContext::InternationalKeyCode OSystemParseContext::getIntlKeyCode(const OString& rToken) const
{
    static const IParseContext::InternationalKeyCode Intl_TokenID[] =
    {
        InternationalKeyCode::Like, InternationalKeyCode::Not, InternationalKeyCode::Null, InternationalKeyCode::True,
        InternationalKeyCode::False, InternationalKeyCode::Is, InternationalKeyCode::Between, InternationalKeyCode::Or,
        InternationalKeyCode::And, InternationalKeyCode::Avg, InternationalKeyCode::Count, InternationalKeyCode::Max,
        InternationalKeyCode::Min, InternationalKeyCode::Sum, InternationalKeyCode::Every,
        InternationalKeyCode::Any, InternationalKeyCode::Some, InternationalKeyCode::StdDevPop,
        InternationalKeyCode::StdDevSamp, InternationalKeyCode::VarSamp, InternationalKeyCode::VarPop,
        InternationalKeyCode::Collect, InternationalKeyCode::Fusion, InternationalKeyCode::Intersection
    };

    sal_uInt32 nCount = SAL_N_ELEMENTS(Intl_TokenID);
    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        OString aKey = getIntlKeywordAscii(Intl_TokenID[i]);
        if (rToken.equalsIgnoreAsciiCase(aKey))
            return Intl_TokenID[i];
    }

    return InternationalKeyCode::None;
}


namespace
{

    ::osl::Mutex& getSafteyMutex()
    {
        static ::osl::Mutex s_aSafety;
        return s_aSafety;
    }

    oslInterlockedCount& getCounter()
    {
        static oslInterlockedCount s_nCounter;
        return s_nCounter;
    }

    OSystemParseContext* getSharedContext(OSystemParseContext* _pContext, bool _bSet)
    {
        static OSystemParseContext* s_pSharedContext = nullptr;
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

}

OParseContextClient::OParseContextClient()
{
    ::osl::MutexGuard aGuard( getSafteyMutex() );
    if ( 1 == osl_atomic_increment( &getCounter() ) )
    {   // first instance
        getSharedContext( new OSystemParseContext, false );
    }
}


OParseContextClient::~OParseContextClient()
{
    {
        ::osl::MutexGuard aGuard( getSafteyMutex() );
        if ( 0 == osl_atomic_decrement( &getCounter() ) )
            delete getSharedContext(nullptr,true);
    }
}

const OSystemParseContext* OParseContextClient::getParseContext() const
{
    return getSharedContext(nullptr, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
