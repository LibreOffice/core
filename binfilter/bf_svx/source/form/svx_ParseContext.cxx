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
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "ParseContext.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

namespace binfilter {


using namespace svxform;
using namespace ::connectivity;
//==========================================================================
//= OSystemParseContext
//==========================================================================
/*N*/ DBG_NAME(OSystemParseContext)
//-----------------------------------------------------------------------------
/*N*/ OSystemParseContext::OSystemParseContext() : IParseContext()
/*N*/ {
/*N*/ 	DBG_CTOR(OSystemParseContext,NULL);
/*N*/ 	vos::OGuard aGuard( Application::GetSolarMutex() );
/*N*/ 	m_aSQLInternationals = ByteString(SVX_RES(RID_STR_SVT_SQL_INTERNATIONAL),RTL_TEXTENCODING_ASCII_US);
/*N*/ }

//-----------------------------------------------------------------------------
/*?*/ OSystemParseContext::~OSystemParseContext()
/*?*/ {
/*?*/ 	DBG_DTOR(OSystemParseContext,NULL);
/*?*/ }

//-----------------------------------------------------------------------------
/*?*/ ::com::sun::star::lang::Locale OSystemParseContext::getPreferredLocale( ) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); ::com::sun::star::lang::Locale alocal; return alocal;//STRIP001 
/*?*/ //STRIP001 	return SvtSysLocale().GetLocaleData().getLocale();
/*?*/ }

//-----------------------------------------------------------------------------
/*?*/ ::rtl::OUString OSystemParseContext::getErrorMessage(ErrorCode _eCode) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); String astring; return astring;//STRIP001 
/*?*/ }

//-----------------------------------------------------------------------------
/*?*/ ::rtl::OString OSystemParseContext::getIntlKeywordAscii(InternationalKeyCode _eKey) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); ByteString aKeyword; return aKeyword;//STRIP001 
/*?*/ }

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
/*?*/ IParseContext::InternationalKeyCode OSystemParseContext::getIntlKeyCode(const ::rtl::OString& rToken) const
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return KEY_NONE;//STRIP001 
/*?*/ }


// =============================================================================
// =============================================================================
namespace
{
    // -----------------------------------------------------------------------------
/*N*/ 	::osl::Mutex& getSafteyMutex()
/*N*/ 	{
/*N*/ 		static ::osl::Mutex s_aSafety;
/*N*/ 		return s_aSafety;
/*N*/ 	}
    // -----------------------------------------------------------------------------
/*N*/ 	oslInterlockedCount& getCounter()
/*N*/ 	{
/*N*/ 		static oslInterlockedCount s_nCounter;
/*N*/ 		return s_nCounter;
/*N*/ 	}
    // -----------------------------------------------------------------------------
/*N*/ 	OSystemParseContext* getSharedContext(OSystemParseContext* _pContext = NULL,sal_Bool _bSet = sal_False)
/*N*/ 	{
/*N*/ 		static OSystemParseContext* s_pSharedContext = NULL;
/*N*/ 		if ( _pContext && !s_pSharedContext || _bSet )
/*N*/ 			s_pSharedContext = _pContext;
/*N*/ 		return s_pSharedContext;
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
/*N*/ OParseContextClient::OParseContextClient()
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( getSafteyMutex() );
/*N*/ 	if ( 1 == osl_incrementInterlockedCount( &getCounter() ) )
/*N*/ 	{	// first instance
/*N*/ 		getSharedContext( new OSystemParseContext );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------
/*N*/ OParseContextClient::~OParseContextClient()
/*N*/ {
/*N*/ 	{
/*N*/ 		::osl::MutexGuard aGuard( getSafteyMutex() );
/*N*/ 		if ( 0 == osl_decrementInterlockedCount( &getCounter() ) )
/*N*/ 			delete getSharedContext(NULL,sal_True);
/*N*/ 	}
/*N*/ }
}
