/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filtopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:29:34 $
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
    bWK3Flag( FALSE ),
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


