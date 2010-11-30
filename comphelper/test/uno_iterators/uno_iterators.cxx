/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include <algorithm>
#include <comphelper/stlunosequence.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <iostream>
#include <list>
#include <rtl/textenc.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <vector>

using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::std;

// some helpers for testing (imperative)
void fill_testdata(Sequence<OUString>& seq)
{
    OUStringBuffer buf;
    for(sal_Int32 i=0; i<seq.getLength(); i++)
    {
        buf.appendAscii("Sampletext ");
        buf.append(i);
        seq[i] = buf.makeStringAndClear();
    }
}

void print_oustring(const OUString& ustr)
{ cout << OUStringToOString(ustr, RTL_TEXTENCODING_ASCII_US).getStr() << endl; }

void print_sequence(const Sequence<OUString>& seq)
{
    cout << "Sequence of " << seq.getLength() << " OUStrings: " << endl;
    for(sal_Int32 i=0; i<seq.getLength(); i++)
        print_oustring(seq[i]);
    cout << endl;
}

// some helpers for testing (functional)
class TestdataGenerator
{
    public:
        TestdataGenerator() : m_Index(0) {};
        OUString operator()() { return m_Buf.appendAscii("Sampletext ").append(m_Index++).makeStringAndClear(); };
        OUStringBuffer m_Buf;
        sal_Int32 m_Index;
};

void fill_testdata_stl(StlUnoSequence<OUString>& stl_seq)
{
    generate(stl_seq.begin(), stl_seq.end(), TestdataGenerator());
}

void print_sequence_stl(const StlUnoSequence<OUString>& stl_seq)
{
    cout << "Sequence of " << stl_seq.size() << " OUStrings: " << endl;
    for_each(stl_seq.begin(), stl_seq.end(), &print_oustring);
    cout << endl;
}


// code samples

// imperative loops (just to show they work, using for_each would be better most of the time
void classic_loops()
{
    Sequence<OUString> s(10);
    fill_testdata(s);
    StlUnoSequence<OUString>::iterator stl_s_it;

    cout << "for iteration" << endl;
    for(stl_s_it = stl_begin(s); stl_s_it != stl_end(s); stl_s_it++)
        cout << OUStringToOString(*stl_s_it, RTL_TEXTENCODING_ASCII_US).getStr() << endl;

    cout << "reverse for iteration" << endl;
    for(stl_s_it = stl_end(s); stl_s_it != stl_begin(s); stl_s_it--)
        cout << OUStringToOString(*(stl_s_it-1), RTL_TEXTENCODING_ASCII_US).getStr() << endl;

    cout << "skipping for iteration" << endl;
    for(stl_s_it = stl_begin(s); stl_s_it != stl_end(s); stl_s_it+=2)
        cout << OUStringToOString(*stl_s_it, RTL_TEXTENCODING_ASCII_US).getStr() << endl;

    cout << "skipping reverse for iteration" << endl;
    for(stl_s_it = stl_end(s); stl_s_it != stl_begin(s); stl_s_it-=2)
        std::cout << OUStringToOString(*(stl_s_it-1), RTL_TEXTENCODING_ASCII_US).getStr() << endl;
}

void stl_algos()
{
    Sequence<OUString> s(10);
    fill_testdata(s);

    random_shuffle(stl_begin(s), stl_end(s));
    cout << "shuffed" << std::endl;
    print_sequence(s);

    sort(stl_begin(s), stl_end(s));
    cout << "sorted" << std::endl;
    print_sequence(s);
}

void stl_conversions()
{
    Sequence<OUString> s(10);
    fill_testdata(s);
    StlUnoSequence<OUString> stl_s = StlUnoSequence<OUString>::createInstance(s);

    // convert to stl::vector, modify in vector, copy back, print
    cout << "vector conversion" << endl;
    vector<OUString> vec(stl_s.begin(), stl_s.end());
    vec[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( "changed in vector" ));
    copy(vec.begin(), vec.end(), stl_s.begin());
    print_sequence(s);

    // convert to stl::list, modify in list, copy back, print
    cout << "list conversion" << endl;
    list<OUString> l(stl_s.begin(), stl_s.end());
    l.pop_back();
    l.push_back(OUString( RTL_CONSTASCII_USTRINGPARAM( "changed in list" )));
    copy(l.begin(), l.end(), stl_s.begin());
    print_sequence(s);
}

// inserts the second half of the second sequence after the first element of the first sequence
void stl_inserting()
{
    Sequence<OUString> s1(10);
    Sequence<OUString> s2(10);
    Sequence<OUString> result(15);
    StlUnoSequence<OUString> stl_s1 = StlUnoSequence<OUString>::createInstance(s1);
    StlUnoSequence<OUString> stl_s2 = StlUnoSequence<OUString>::createInstance(s2);
    StlUnoSequence<OUString> stl_result = StlUnoSequence<OUString>::createInstance(result);
    fill_testdata(s1);
    fill_testdata(s2);

    list<OUString> temp(stl_s1.begin(), stl_s1.end());
    copy(stl_s2.begin()+5, stl_s2.end(), insert_iterator<list<OUString> >(temp, ++temp.begin()));
    copy(temp.begin(), temp.end(), stl_result.begin());
    print_sequence(result);
}

void stl_compare()
{
    Sequence<OUString> s1(10);
    Sequence<OUString> s2(10);
    StlUnoSequence<OUString> stl_s1 = StlUnoSequence<OUString>::createInstance(s1);
    StlUnoSequence<OUString> stl_s2 = StlUnoSequence<OUString>::createInstance(s2);
    if (stl_s1 == stl_s2)
        cout << "sequences are equal." << endl;
    s2[9] = OUString( RTL_CONSTASCII_USTRINGPARAM( "ZZZZZ" ));
    if(stl_s1 < stl_s2)
        cout << "first sequence is smaller." << endl;
}

void stl_const_sequence()
{
    const Sequence<OUString> s(10);
    for(StlUnoSequence<OUString>::const_iterator stl_s_it = stl_begin(s); stl_s_it != stl_end(s); stl_s_it++)
        cout << OUStringToOString(*stl_s_it, RTL_TEXTENCODING_ASCII_US).getStr() << endl;
}

void stl_helpers()
{
    Sequence<OUString> s(10);
    StlUnoSequence<OUString> stl_s = StlUnoSequence<OUString>::createInstance(s);
    fill_testdata_stl(stl_s);
    print_sequence_stl(stl_s);
}

int main()
{
    cout << "--- CLASSIC LOOPS" << endl;
    classic_loops();

    cout << "--- SOME STL ALGORITHMS" << endl;
    stl_algos();

    cout << "--- SOME STL CONVERSIONS" << endl;
    stl_conversions();

    cout << "--- INSERTING IN SEQUENCE" << endl;
    stl_inserting();

    cout << "--- COMPARING" << endl;
    stl_compare();

    cout << "--- CONST SEQUENCE" << endl;
    stl_const_sequence();

    cout << "--- HELPERS IN STL-STYLE" << endl;
    stl_helpers();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
