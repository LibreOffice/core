/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imoptdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:24:43 $
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
#include "precompiled_sc.hxx"



#include "imoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

static const sal_Char pStrFix[] = "FIX";

//------------------------------------------------------------------------
//  Der Options-String darf kein Semikolon mehr enthalten (wegen Pickliste)
//  darum ab Version 336 Komma stattdessen


ScImportOptions::ScImportOptions( const String& rStr )
{
    bFixedWidth = FALSE;
    nFieldSepCode = 0;
    if ( rStr.GetTokenCount(',') >= 3 )
    {
        String aToken( rStr.GetToken( 0, ',' ) );
        if( aToken.EqualsIgnoreCaseAscii( pStrFix ) )
            bFixedWidth = TRUE;
        else
            nFieldSepCode = (sal_Unicode) aToken.ToInt32();
        nTextSepCode  = (sal_Unicode) rStr.GetToken(1,',').ToInt32();
        aStrFont      = rStr.GetToken(2,',');
        eCharSet      = ScGlobal::GetCharsetValue(aStrFont);
        bSaveAsShown  = (rStr.GetToken( 3, ',' ).ToInt32() ? TRUE : FALSE);
    }
}

//------------------------------------------------------------------------

String ScImportOptions::BuildString() const
{
    String  aResult;

    if( bFixedWidth )
        aResult.AppendAscii( pStrFix );
    else
        aResult += String::CreateFromInt32(nFieldSepCode);
    aResult += ',';
    aResult += String::CreateFromInt32(nTextSepCode);
    aResult += ',';
    aResult += aStrFont;
    aResult += ',';
    aResult += String::CreateFromInt32( bSaveAsShown ? 1 : 0 );

    return aResult;
}

//------------------------------------------------------------------------

void ScImportOptions::SetTextEncoding( rtl_TextEncoding nEnc )
{
    eCharSet = (nEnc == RTL_TEXTENCODING_DONTKNOW ?
        gsl_getSystemTextEncoding() : nEnc);
    aStrFont = ScGlobal::GetCharsetString( nEnc );
}
