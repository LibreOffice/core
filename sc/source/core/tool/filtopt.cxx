/*************************************************************************
 *
 *  $RCSfile: filtopt.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 14:56:08 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "filtopt.hxx"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------

#define CFGPATH_FILTER          "Office.Calc/Filter/Import"

#define SCFILTOPT_COLSCALE      0
#define SCFILTOPT_ROWSCALE      1
#define SCFILTOPT_WK3           2
#define SCFILTOPT_COUNT         3

//! expand options for bBreakShared
//! (former "EXCELBREAKSHAREDFORMULA" in ini file)

Sequence<OUString> ScFilterOptions::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "MS_Excel/ColScale",            // SCFILTOPT_COLSCALE
        "MS_Excel/RowScale",            // SCFILTOPT_ROWSCALE
        "Lotus123/WK3"                  // SCFILTOPT_WK3
    };
    Sequence<OUString> aNames(SCFILTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCFILTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScFilterOptions::ScFilterOptions() :
    ConfigItem( OUString::createFromAscii( CFGPATH_FILTER ) ),
    bWK3Flag( FALSE ),
    bBreakShared( TRUE ),
    fExcelColScale( 0 ),
    fExcelRowScale( 0 )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCFILTOPT_COLSCALE:
                        pValues[nProp] >>= fExcelColScale;
                        break;
                    case SCFILTOPT_ROWSCALE:
                        pValues[nProp] >>= fExcelRowScale;
                        break;
                    case SCFILTOPT_WK3:
                        bWK3Flag = ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] );
                        break;
                }
            }
        }
    }
}


void ScFilterOptions::Commit()
{
    // options are never modified from office

    DBG_ERROR("trying to commit changed ScFilterOptions?");
}

void ScFilterOptions::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERROR("properties have been changed")
}


