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
#include "precompiled_testshl2.hxx"

#include <vector>
#include <string>
#include <iostream>
#include <rtl/string.hxx>

typedef std::vector<std::string>  StringList;
typedef std::vector<rtl::OString> OStringList;

void split( const rtl::OString& opt,
            const rtl::OString& _sSeparator,
            OStringList& optLine )
{
    optLine.clear();
    // const sal_Int32 cSetLen = cSet.getLength();
    sal_Int32 index = 0;
    sal_Int32 oldIndex = 0;

    // sal_Int32 i;
    // sal_Int32 j = 0;
    while ( opt.getLength() > 0)
    {
        // for ( i = 0; i < cSetLen; i++ )
        // {
        index = opt.indexOf( _sSeparator, oldIndex);
        if( index != -1 )
        {
            optLine.push_back( opt.copy( oldIndex, index - oldIndex ) );
            oldIndex = index + _sSeparator.getLength();
        }
        // }
        else // if (index == -1)
        {
            optLine.push_back( opt.copy( oldIndex ) );
            break;
        }
    }
} ///< split

// -----------------------------------------------------------------------------

StringList splitNameAtDot(rtl::OString const& _sName)
{
    StringList aList;
    int nIndex = 0;
    // int nLastIndex = 0;
    while ((nIndex = _sName.indexOf(".")) != -1)
    {
        rtl::OString nValue;
    }

    return aList;
}

// -----------------------------------------------------------------------------
/*
bool checkFilter(JobList m_aJobFilter, std::string const& _sNodeName, std::string const& _sName)
{
    std::string sFilter = m_aJobFilter.m_aJobList.begin();

    StringList aFilter;
    aFilter.push_back("rtl_OUString");
    aFilter.push_back("ctors");
    aFilter.push_back("*");

    StringList aMyName;
    aMyName.push_back("rtl_OUString");
    aMyName.push_back("ctors");
    aMyName.push_back("ctor_001");

}
*/

void showList(OStringList const& _sList)
{
    for(OStringList::const_iterator it = _sList.begin();
        it != _sList.end();
        ++it)
    {
        rtl::OString aStr = *it;
        std::cout << aStr.getStr() << std::endl;
    }
}

bool match(OStringList const& _aFilter, OStringList const& _aName)
{
    OStringList::const_iterator aFilterIter = _aFilter.begin();
    OStringList::const_iterator aValueIter = _aName.begin();

    bool bMatch = false;

    while (aFilterIter != _aFilter.end() && aValueIter != _aName.end())
    {
        rtl::OString sFilter = *aFilterIter;
        rtl::OString sName   = *aValueIter;

        if (sFilter == sName)
        {
            bMatch = true;
            ++aFilterIter;
            ++aValueIter;
        }
        else if (sFilter == "*")
        {
            bMatch = true;
            break;
        }
        else
        {
            // Filter does not match
            bMatch = false;
            break;
        }
    }
    return bMatch;
}

// -----------------------------------------------------------------------------
void test_normal_filter()
{
    OStringList aFilter;
    split("rtl_OUString.ctors.*", ".", aFilter);
    showList(aFilter);

    OStringList aName;
    split("rtl_OUString.ctors.ctor_001", ".", aName);
    showList(aName);

    if (match(aFilter, aName))
    {
        std::cout << "Name matches filter." << std::endl;
    }
}

void test_normal_filter_other_sep()
{
    OStringList aFilter;
    split("rtl_OUString::ctors::*", "::", aFilter);
    showList(aFilter);

    OStringList aName;
    split("rtl_OUString::ctors::ctor_001", "::", aName);
    showList(aName);

    if (match(aFilter, aName))
    {
        std::cout << "Name matches filter." << std::endl;
    }
}

void test_no_name()
{
    OStringList aFilter;
    split("rtl_OUString.*", ".", aFilter);
    showList(aFilter);

    OStringList aName;
    split("", ".", aName);
    showList(aName);

    if (match(aFilter, aName))
    {
        std::cout << "Name matches filter." << std::endl;
    }
}

void test_name_longer_filter()
{
    OStringList aFilter;
    split("rtl_OUString.*", ".", aFilter);
    showList(aFilter);

    OStringList aName;
    split("rtl_OUString.ctor.ctor_001", ".", aName);
    showList(aName);

    if (match(aFilter, aName))
    {
        std::cout << "Name matches filter." << std::endl;
    }
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */ , char* /* argv */ [] )
#else
    int _cdecl main( int /* argc */, char* /* argv */ [] )
#endif
{
    test_normal_filter();
    test_normal_filter_other_sep();
    test_no_name();
    test_name_longer_filter();

    // split("rtl_OUString.*", ".", aFilter);
    // showList(aFilter);


    // test_autoptr();
    return 0;
}
