/*************************************************************************
 *
 *  $RCSfile: ParseContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:10:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "ParseContext.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#include "dialmgr.hxx"
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif


using namespace svxform;
using namespace ::connectivity;
//==========================================================================
//= OSystemParseContext
//==========================================================================
DBG_NAME(OSystemParseContext);
//-----------------------------------------------------------------------------
OSystemParseContext::OSystemParseContext() : IParseContext()
{
    DBG_CTOR(OSystemParseContext,NULL);
    vos::OGuard aGuard( Application::GetSolarMutex() );
    m_aSQLInternationals = ByteString(SVX_RES(RID_STR_SVT_SQL_INTERNATIONAL),RTL_TEXTENCODING_UTF8);
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
        case ERROR_GENERAL_HINT:            aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_ERROR_BEFORE); break;
        case ERROR_VALUE_NO_LIKE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_VALUE_NO_LIKE); break;
        case ERROR_FIELD_NO_LIKE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_FIELD_NO_LIKE); break;
        case ERROR_INVALID_COMPARE:         aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_CRIT_NO_COMPARE); break;
        case ERROR_INVALID_INT_COMPARE:     aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_INT_NO_VALID); break;
        case ERROR_INVALID_STRING_COMPARE:  aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_NO_NUM); break;
        case ERROR_INVALID_DATE_COMPARE:    aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_ACCESS_DAT_NO_VALID); break;
        case ERROR_INVALID_REAL_COMPARE:    aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_REAL_NO_VALID); break;
        case ERROR_INVALID_TABLE:           aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_TABLE); break;
        case ERROR_INVALID_COLUMN:          aMsg = SVX_RES(RID_STR_SVT_SQL_SYNTAX_COLUMN); break;
    }
    return aMsg;
}

//-----------------------------------------------------------------------------
::rtl::OString OSystemParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
{
    ByteString aKeyword;
    switch (_eKey)
    {
        case KEY_LIKE:      aKeyword = m_aSQLInternationals.GetToken(0); break;
        case KEY_NOT:       aKeyword = m_aSQLInternationals.GetToken(1); break;
        case KEY_NULL:      aKeyword = m_aSQLInternationals.GetToken(2); break;
        case KEY_TRUE:      aKeyword = m_aSQLInternationals.GetToken(3); break;
        case KEY_FALSE:     aKeyword = m_aSQLInternationals.GetToken(4); break;
        case KEY_IS:        aKeyword = m_aSQLInternationals.GetToken(5); break;
        case KEY_BETWEEN:   aKeyword = m_aSQLInternationals.GetToken(6); break;
        case KEY_OR:        aKeyword = m_aSQLInternationals.GetToken(7); break;
        case KEY_AND:       aKeyword = m_aSQLInternationals.GetToken(8); break;
        case KEY_AVG:       aKeyword = m_aSQLInternationals.GetToken(9); break;
        case KEY_COUNT:     aKeyword = m_aSQLInternationals.GetToken(10); break;
        case KEY_MAX:       aKeyword = m_aSQLInternationals.GetToken(11); break;
        case KEY_MIN:       aKeyword = m_aSQLInternationals.GetToken(12); break;
        case KEY_SUM:       aKeyword = m_aSQLInternationals.GetToken(13); break;
    }
    return aKeyword;
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
        KEY_MIN, KEY_SUM
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
        if ( _pContext && !s_pSharedContext || _bSet )
            s_pSharedContext = _pContext;
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





