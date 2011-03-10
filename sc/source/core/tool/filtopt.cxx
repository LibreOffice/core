/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "filtopt.hxx"
#include "miscuno.hxx"

using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

//------------------------------------------------------------------

#define CFGPATH_FILTER          "Office.Calc/Filter/Import"

#define SCFILTOPT_COLSCALE      0
#define SCFILTOPT_ROWSCALE      1
#define SCFILTOPT_WK3           2
#define SCFILTOPT_COUNT         3

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
    ConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM( CFGPATH_FILTER )) ),
    bWK3Flag( false ),
    fExcelColScale( 0 ),
    fExcelRowScale( 0 )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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

    OSL_FAIL("trying to commit changed ScFilterOptions?");
}

void ScFilterOptions::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    OSL_FAIL("properties have been changed");
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
