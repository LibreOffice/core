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

using namespace connectivity::macab;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;
using namespace ::dbtools;


MacabHeader::MacabHeader(const sal_Int32 _size, macabfield **_fields)
{
    sal_Int32 i;
    size = _size;
    fields = std::make_unique<macabfield *[]>(size);
    for(i = 0; i < size; i++)
    {
        if(_fields[i] == nullptr)
        {
            fields[i] = nullptr;
        }
        else
        {
            /* The constructor duplicates the macabfields it gets because they
             * are either deleted later or used for other purposes.
             */
            fields[i] = new macabfield;
            fields[i]->type = _fields[i]->type;
            fields[i]->value = _fields[i]->value;
            if (fields[i]->value)
                CFRetain(fields[i]->value);
        }
    }

}


MacabHeader::MacabHeader()
{
    size = 0;
    fields = nullptr;
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
    if(rSize != 0) // If the new header does actually have fields
    {
        /* If our header is currently empty, just copy all of the fields from
         * the new header to this one.
         */
        if(size == 0)
        {
            sal_Int32 i;
            size = rSize;
            fields = std::make_unique<macabfield *[]>(size);
            for(i = 0; i < size; i++)
            {
                fields[i] = r->copy(i);
            }
        }

        /* Otherwise, only add the duplicates. We do this with a two-pass
         * approach. First, find out how many fields to add, then reallocate
         * the size of the fields array and add the old ones at the end.
         * (More precisely, we create a _new_ fields array with the new length
         * allocated to it, then get all of the fields from the current
         * fields array to it, then copy the non-duplicates from the new
         * header to the end.)
         */
        else
        {
            sal_Int32 i;
            sal_Int32 numToAdd = 0, numAdded = 0;
            macabfield **newFields;
            for( i = 0; i < rSize; i++)
            {
                if(!contains(r->get(i)))
                {
                    numToAdd++;
                }
            }

            newFields = new macabfield *[size+numToAdd];
            for(i = 0; i < size; i++)
            {
                newFields[i] = copy(i);
            }

            for( i = 0; i < rSize; i++)
            {
                if(!contains(r->get(i)))
                {
                    newFields[size+numAdded] = r->copy(i);
                    numAdded++;
                    if(numAdded == numToAdd)
                        break;
                }
            }

            releaseFields();
            size += numAdded;
            fields.reset(newFields);
        }
    }
}


OUString MacabHeader::getString(const sal_Int32 i) const
{
    OUString nRet;

    if(i < size)
    {
        if(fields[i] == nullptr || fields[i]->value == nullptr || CFGetTypeID(fields[i]->value) != CFStringGetTypeID())
            return OUString();
        try
        {
            nRet = CFStringToOUString(static_cast<CFStringRef>(fields[i]->value));
        }
        catch(...){ }
    }

    return nRet;
}


void MacabHeader::sortRecord()
{
    sortRecord(0,size);
}


macabfield **MacabHeader::sortRecord(const sal_Int32 _start, const sal_Int32 _length)
{
    /* Sort using mergesort. Because it uses mergesort, it is recursive and
     * not in place (so it creates a new array at every step of the
     * recursion), so if you prefer to use a different sort, please feel
     * free to implement it.
     */
    macabfield** sorted = new macabfield *[_length];
    if(_length <= 2)
    {
        if(_length == 2)
        {
            if(compareFields(fields[_start], fields[_start+1]) > 0)
            {
                sorted[0] = get(_start+1);
                sorted[1] = get(_start);
            }
            else
            {
                sorted[0] = get(_start);
                sorted[1] = get(_start+1);
            }
        }
        else if(_length == 1)
        {
            sorted[0] = get(_start);
        }
    }
    else
    {
        sal_Int32 halfLength = floor(_length/2);
        sal_Int32 fp = 0, lp = 0;
        sal_Int32 i;
        macabfield **firstHalf = sortRecord(_start, halfLength);
        macabfield **lastHalf = sortRecord(_start+halfLength, _length-halfLength);

        for(i = 0; i < _length; i++)
        {
            if(compareFields(firstHalf[fp],lastHalf[lp]) < 0)
            {
                sorted[i] = firstHalf[fp++];
                if(fp == halfLength)
                {
                    for( i++; i < _length; i++)
                    {
                        sorted[i] = lastHalf[lp++];
                    }
                    break;
                }
            }
            else
            {
                sorted[i] = lastHalf[lp++];
                if(lp == _length - halfLength)
                {
                    for( i++; i < _length; i++)
                    {
                        sorted[i] = firstHalf[fp++];
                    }
                    break;
                }
            }
        }
        if(_length == size)
        {
            fields.reset(sorted);
        }
        delete firstHalf;
        delete lastHalf;
    }
    return sorted;
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
        static_cast<CFStringRef>(_field1->value),
        static_cast<CFStringRef>(_field2->value),
        0); // 0 = no options (like ignore case)

    return static_cast<sal_Int32>(result);
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
