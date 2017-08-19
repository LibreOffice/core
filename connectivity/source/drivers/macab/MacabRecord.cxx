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


#include "MacabRecord.hxx"
#include "macabutilities.hxx"
#include <com/sun/star/util/DateTime.hpp>
#include <o3tl/make_unique.hxx>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>
#include <connectivity/dbconversion.hxx>

using namespace connectivity::macab;
using namespace com::sun::star::util;
using namespace ::dbtools;


macabfield::macabfield(macabfield const & other)
{
    value = other.value;
    type = other.type;
    if (value)
        CFRetain(value);
}

macabfield::macabfield(macabfield && other)
{
    value = other.value;
    type = other.type;
    other.value = nullptr;
}

void macabfield::setValue(CFTypeRef v)
{
    if (value)
        CFRelease(value);
    value = v;
}

macabfield::~macabfield()
{
    if (value)
        CFRelease(value);
}

MacabRecord::MacabRecord()
{
    size = 0;
}


MacabRecord::MacabRecord(const sal_Int32 _size)
{
    size = _size;
    fields.resize(size);
}


MacabRecord::~MacabRecord()
{
}


void MacabRecord::insertAtColumn (CFTypeRef _value, ABPropertyType _type, const sal_Int32 _column)
{
    if(_column < size)
    {
        if (_value)
            CFRetain(_value);
        fields[_column].reset( new macabfield(_value, _type) );
    }
}


bool MacabRecord::contains (const macabfield *_field) const
{
    if(_field == nullptr)
        return false;
    else
        return contains(_field->getValue());
}


bool MacabRecord::contains (const CFTypeRef _value) const
{
    sal_Int32 i;
    for(i = 0; i < size; i++)
    {
        if(fields[i] != nullptr)
        {
            if(CFEqual(fields[i]->getValue(), _value))
            {
                return true;
            }
        }
    }

    return false;
}


sal_Int32 MacabRecord::getSize() const
{
    return size;
}


std::unique_ptr<macabfield> MacabRecord::copy(const sal_Int32 i) const
{
    /* Note: copy(i) creates a new macabfield identical to that at
     * location i, whereas get(i) returns a pointer to the macabfield
     * at location i.
     */
    if(i < size)
    {
        return std::unique_ptr<macabfield>(new macabfield(*fields[i]));
    }

    return nullptr;
}


macabfield *MacabRecord::get(const sal_Int32 i) const
{
    /* Note: copy(i) creates a new macabfield identical to that at
     * location i, whereas get(i) returns a pointer to the macabfield
     * at location i.
     */
    if(i < size)
    {
        return fields[i].get();
    }

    return nullptr;
}

sal_Int32 MacabRecord::compareFields(const macabfield *_field1, const macabfield *_field2)
{

    /* When comparing records, if either field is NULL (and the other is
     * not), that field is considered "greater than" the other, so that it
     * shows up later in the list when fields are ordered.
     */
    if(_field1 == _field2)
        return 0;
    if(_field1 == nullptr)
        return 1;
    if(_field2 == nullptr)
        return -1;

    /* If they aren't the same type, for now, return the one with
     * the smaller type ID... I don't know of a better way to compare
     * two different data types.
     */
    if(_field1->type != _field2->type)
        return(_field1->type - _field2->type);

    CFComparisonResult result;

    /* Carbon has a unique compare function for each data type: */
    switch(_field1->type)
    {
        case kABStringProperty:
            result = CFStringCompare(
                static_cast<CFStringRef>(_field1->getValue()),
                static_cast<CFStringRef>(_field2->getValue()),
                kCFCompareLocalized); // Specifies that the comparison should take into account differences related to locale, such as the thousands separator character.
            break;

        case kABDateProperty:
            result = CFDateCompare(
                static_cast<CFDateRef>(_field1->getValue()),
                static_cast<CFDateRef>(_field2->getValue()),
                nullptr); // NULL = unused variable
            break;

        case kABIntegerProperty:
        case kABRealProperty:
            result = CFNumberCompare(
                static_cast<CFNumberRef>(_field1->getValue()),
                static_cast<CFNumberRef>(_field2->getValue()),
                nullptr); // NULL = unused variable
        break;

        default:
            result = kCFCompareEqualTo; // can't compare
    }

    return (sal_Int32) result;
}


/* Create a macabfield out of an OUString and type. Together with the
 * method fieldToString() (below), it is possible to switch conveniently
 * between an OUString and a macabfield (for use when creating and handling
 * SQL statement).
 */
std::unique_ptr<macabfield> MacabRecord::createMacabField(const OUString& _newFieldString, const ABPropertyType _abType)
{
    std::unique_ptr<macabfield> newField;
    switch(_abType)
    {
        case kABStringProperty:
            newField.reset( new macabfield( OUStringToCFString(_newFieldString), _abType) );
            break;
        case kABDateProperty:
            {
                DateTime aDateTime = DBTypeConversion::toDateTime(_newFieldString);

                // bad format...
                if(aDateTime.Year == 0 && aDateTime.Month == 0 && aDateTime.Day == 0)
                {
                }
                else
                {
                    double nTime = DBTypeConversion::toDouble(aDateTime, DBTypeConversion::getStandardDate());
                    nTime -= kCFAbsoluteTimeIntervalSince1970;
                    newField.reset( new macabfield( CFDateCreate(nullptr, (CFAbsoluteTime) nTime), _abType) );
                }
            }
            break;
        case kABIntegerProperty:
            try
            {
                sal_Int64 nVal = _newFieldString.toInt64();

                newField.reset( new macabfield( CFNumberCreate(nullptr,kCFNumberLongType, &nVal), _abType) );
            }
            // bad format...
            catch(...)
            {
            }
            break;
        case kABRealProperty:
            try
            {
                double nVal = _newFieldString.toDouble();

                newField.reset( new macabfield( CFNumberCreate(nullptr,kCFNumberDoubleType, &nVal), _abType) );
            }
            // bad format...
            catch(...)
            {
            }
            break;
        default:
            ;
    }
    return newField;
}


/* Create an OUString out of a macabfield. Together with the method
 * createMacabField() (above), it is possible to switch conveniently
 * between an OUString and a macabfield (for use when creating and handling
 * SQL statement).
 */
OUString MacabRecord::fieldToString(const macabfield *_aField)
{
    if(_aField == nullptr)
        return OUString();

    OUString fieldString;

    switch(_aField->type)
    {
        case kABStringProperty:
            fieldString = CFStringToOUString(static_cast<CFStringRef>(_aField->getValue()));
            break;
        case kABDateProperty:
            {
                DateTime aTime = CFDateToDateTime(static_cast<CFDateRef>(_aField->getValue()));
                fieldString = DBTypeConversion::toDateTimeString(aTime);
            }
            break;
        case kABIntegerProperty:
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(_aField->getValue()) );
                sal_Int64 nVal;
                // Should we check for the wrong type here, e.g., a float?
                bool m_bSuccess = !CFNumberGetValue(static_cast<CFNumberRef>(_aField->getValue()), numberType, &nVal);
                if(m_bSuccess)
                    fieldString = OUString::number(nVal);
            }
            break;
        case kABRealProperty:
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(_aField->getValue()) );
                double nVal;
                // Should we check for the wrong type here, e.g., an int?
                bool m_bSuccess = !CFNumberGetValue(static_cast<CFNumberRef>(_aField->getValue()), numberType, &nVal);
                if(m_bSuccess)
                    fieldString = OUString::number(nVal);
            }
            break;
        default:
            ;
    }
    return fieldString;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
