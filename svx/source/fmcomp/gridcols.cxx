/*************************************************************************
 *
 *  $RCSfile: gridcols.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
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

#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

IMPLEMENT_CONSTASCII_USTRING(FM_COL_TEXTFIELD, "TextField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_COMBOBOX, "ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_CHECKBOX, "CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_TIMEFIELD, "TimeField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_DATEFIELD, "DateField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_NUMERICFIELD, "NumericField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_CURRENCYFIELD, "CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_PATTERNFIELD, "PatternField");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_LISTBOX, "ListBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COL_FORMATTEDFIELD, "FormattedField");

//------------------------------------------------------------------------------
const ::utl::StringSequence& getColumnTypes()
{
    static ::utl::StringSequence aColumnTypes(10);
    if (!aColumnTypes.getConstArray()[0].getLength())
    {
        ::rtl::OUString* pNames = aColumnTypes.getArray();
        pNames[TYPE_CHECKBOX] = FM_COL_CHECKBOX;
        pNames[TYPE_COMBOBOX] = FM_COL_COMBOBOX;
        pNames[TYPE_CURRENCYFIELD] = FM_COL_CURRENCYFIELD;
        pNames[TYPE_DATEFIELD] = FM_COL_DATEFIELD;
        pNames[TYPE_FORMATTEDFIELD] = FM_COL_FORMATTEDFIELD;
        pNames[TYPE_LISTBOX] = FM_COL_LISTBOX;
        pNames[TYPE_NUMERICFIELD] = FM_COL_NUMERICFIELD;
        pNames[TYPE_PATTERNFIELD] = FM_COL_PATTERNFIELD;
        pNames[TYPE_TEXTFIELD] = FM_COL_TEXTFIELD;
        pNames[TYPE_TIMEFIELD] = FM_COL_TIMEFIELD;
    }
    return aColumnTypes;
}

//------------------------------------------------------------------
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    NameCompare(const void* pFirst, const void* pSecond)
{
    return ((::rtl::OUString*)pFirst)->compareTo(*(::rtl::OUString*)pSecond);
}

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName)
{
    const ::rtl::OUString aModelPrefix = ::rtl::OUString::createFromAscii("com.sun.star.form.component.");
    const ::rtl::OUString aCompatibleModelPrefix = ::rtl::OUString::createFromAscii("stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == FM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.search(aModelPrefix);
        sal_Int32 nCompatiblePrefixPos = aModelName.search(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1),
                "::getColumnTypeByModelName() : wrong servivce !");

        ::rtl::OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.len())
            : aModelName.copy(aCompatibleModelPrefix.len());

        const ::utl::StringSequence& rColumnTypes = getColumnTypes();
#if SUPD>583
        nTypeId = findPos(aColumnType, rColumnTypes);
#else
        const ::rtl::OUString* pStrList = rColumnTypes.getConstArray();
        ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aColumnType, (void*)pStrList, rColumnTypes.getLength(), sizeof(::rtl::OUString),
            &NameCompare);

        nTypeId = pResult ? (pResult - pStrList) : -1;
#endif
    }
    return nTypeId;
}

