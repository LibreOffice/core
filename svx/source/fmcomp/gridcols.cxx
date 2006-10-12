/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridcols.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:43:30 $
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
#include "precompiled_svx.hxx"

#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

namespace svxform
{

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

}   // namespace svxform
using namespace ::svxform;

//------------------------------------------------------------------------------
const ::comphelper::StringSequence& getColumnTypes()
{
    static ::comphelper::StringSequence aColumnTypes(10);
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

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName)
{
    const ::rtl::OUString aModelPrefix = ::rtl::OUString::createFromAscii("com.sun.star.form.component.");
    const ::rtl::OUString aCompatibleModelPrefix = ::rtl::OUString::createFromAscii("stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == ::svxform::FM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.indexOf(aModelPrefix);
#ifdef DBG_UTIL
        sal_Int32 nCompatiblePrefixPos = aModelName.indexOf(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1), "::getColumnTypeByModelName() : wrong servivce !");
#endif

        ::rtl::OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.getLength())
            : aModelName.copy(aCompatibleModelPrefix.getLength());

        const ::comphelper::StringSequence& rColumnTypes = getColumnTypes();
        nTypeId = findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

