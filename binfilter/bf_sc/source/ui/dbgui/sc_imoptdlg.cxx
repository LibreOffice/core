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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "imoptdlg.hxx"
#include "imoptdlg.hrc"

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

static const sal_Char pStrFix[] = "FIX";

namespace binfilter {

class ScDelimiterTable
{
public:
        ScDelimiterTable( const String& rDelTab )
            :	theDelTab ( rDelTab ),
                cSep	  ( '\t' ),
                nCount	  ( rDelTab.GetTokenCount('\t') ),
                nIter	  ( 0 )
            {}

    String	FirstDel()	{ nIter = 0; return theDelTab.GetToken( nIter, cSep ); }
    String	NextDel()	{ nIter +=2; return theDelTab.GetToken( nIter, cSep ); }

private:
    const String		theDelTab;
    const sal_Unicode	cSep;
    const xub_StrLen	nCount;
    xub_StrLen			nIter;
};

//------------------------------------------------------------------------

/*N*/ void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
/*N*/ {
/*N*/ 	eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
/*N*/ 		gsl_getSystemTextEncoding() : nEnc);
/*N*/ 	aStrFont = ScGlobal::GetCharsetString( nEnc );
/*N*/ }
}
