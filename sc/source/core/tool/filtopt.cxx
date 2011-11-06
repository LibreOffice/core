/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



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
    bWK3Flag( sal_False ),
    fExcelColScale( 0 ),
    fExcelRowScale( 0 )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
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

    DBG_ERROR("trying to commit changed ScFilterOptions?");
}

void ScFilterOptions::Notify( const Sequence<rtl::OUString>& /* aPropertyNames */ )
{
    DBG_ERROR("properties have been changed");
}


