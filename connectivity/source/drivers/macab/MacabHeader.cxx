/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "MacabHeader.hxx"
#include "MacabRecord.hxx"
#include "macabutilities.hxx"

#include <math.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <connectivity/dbconversion.hxx>
#include <o3tl/make_unique.hxx>

using namespace connectivity::macab;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;
using namespace ::dbtools;


MacabHeader::MacabHeader(const sal_Int32 _size, std::vector<std::unique_ptr<macabfield>> const & _fields)
{
    size = _size;
    for (auto const & i : _fields)
        fields.emplace_back(new macabfield(*i));
}


MacabHeader::MacabHeader()
{
    size = 0;
}


MacabHeader::~MacabHeader()
{
}


void MacabHeader::operator+= (const MacabHeader *r)
{
    /* Add one MacabHeader to another. Anything not already in the header is
     * added to the end of it.
     */
    sal_Int32 rSize = r->getSize();
    for(sal_Int32 i = 0; i < rSize; i++)
    {
        if(!contains(r->get(i)))
            fields.emplace_back(new macabfield(*r->get(i)));
    }
}


OUString MacabHeader::getString(const sal_Int32 i) const
{
    OUString nRet;

    if(i < size)
    {
        if(fields[i] == nullptr || fields[i]->getValue() == nullptr || CFGetTypeID(fields[i]->getValue()) != CFStringGetTypeID())
            return OUString();
        try
        {
            nRet = CFStringToOUString(static_cast<CFStringRef>(fields[i]->getValue()));
        }
        catch(...){ }
    }

    return nRet;
}


void MacabHeader::sortRecord()
{
    std::sort(fields.begin(), fields.end(),
        [](std::unique_ptr<macabfield> const & a, std::unique_ptr<macabfield> const & b) -> bool
        {
            return compareFields(a.get(), b.get()) > 0;
        });
}

sal_Int32 MacabHeader::compareFields(const macabfield *_field1, const macabfield *_field2)
{
    /* Comparing two fields in a MacabHeader is different than comparing two
     * fields in a MacabRecord. It starts in the same way (if one of the two
     * fields is NULL, it belongs after the other, so it is considered
     * "greater"). But, then, all headers are CFStrings, no matter what
     * type they claim to be (since they actually hold the expected type for
     * the records with that header). That being said, all we have to do is
     * the built-in CFStringCompare.
     */
    if(_field1 == _field2)
        return 0;
    if(_field1 == nullptr)
        return 1;
    if(_field2 == nullptr)
        return -1;

    CFComparisonResult result = CFStringCompare(
        static_cast<CFStringRef>(_field1->getValue()),
        static_cast<CFStringRef>(_field2->getValue()),
        0); // 0 = no options (like ignore case)

    return (sal_Int32) result;
}


sal_Int32 MacabHeader::getColumnNumber(const OUString& s) const
{
    sal_Int32 i;
    for(i = 0; i < size; i++)
    {
        if(getString(i) == s)
            break;
    }

    if(i == size)
        i = -1;

    return i;
}


MacabHeader *MacabHeader::begin()
{
    return this;
}


MacabHeader::iterator::iterator ()
{
}


MacabHeader::iterator::~iterator ()
{
}

MacabHeader::iterator& MacabHeader::iterator::operator= (MacabHeader *_record)
{
    id = 0;
    record = _record;
    return *this;
}


void MacabHeader::iterator::operator++ ()
{
    id++;
}


bool MacabHeader::iterator::operator!= (const sal_Int32 i) const
{
    return(id != i);
}


bool MacabHeader::iterator::operator== (const sal_Int32 i) const
{
    return(id == i);
}


macabfield *MacabHeader::iterator::operator* () const
{
    return record->get(id);
}


sal_Int32 MacabHeader::end() const
{
    return size;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
