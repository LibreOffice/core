/*************************************************************************
 *
 *  $RCSfile: convuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <tools/isolang.hxx>

#include "convuno.hxx"
#include "global.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  everything is static...

void ScUnoConversion::FillRange( ScRange& rDest, const table::CellRangeAddress& rSource )
{
    rDest.aStart.Set( rSource.StartColumn, rSource.StartRow, rSource.Sheet );
    rDest.aEnd.Set( rSource.EndColumn, rSource.EndRow, rSource.Sheet );
}

void ScUnoConversion::FillAddress( table::CellRangeAddress& rDest, const ScRange& rSource )
{
    DBG_ASSERT( rSource.aStart.Tab() == rSource.aEnd.Tab(),
                "FillAddress: unterschiedliche Tabellen" );

    rDest.Sheet       = rSource.aStart.Tab();
    rDest.StartColumn = rSource.aStart.Col();
    rDest.StartRow    = rSource.aStart.Row();
    rDest.EndColumn   = rSource.aEnd.Col();
    rDest.EndRow      = rSource.aEnd.Row();
}

LanguageType ScUnoConversion::GetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    String aLangStr = rLocale.Language;
    String aCtryStr = rLocale.Country;
    //  Variant is ignored

    LanguageType eRet = ConvertIsoNamesToLanguage( aLangStr, aCtryStr );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;         //! or throw an exception?

    return eRet;
}

void ScUnoConversion::FillLocale( lang::Locale& rLocale, LanguageType eLang )
{
    String aLangStr, aCtryStr;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );
    rLocale.Language = aLangStr;
    rLocale.Country  = aCtryStr;
}



