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

#include <sal/config.h>

#include <memory>
#include <utility>
#include <vector>

#include "MacabRecords.hxx"
#include "MacabRecord.hxx"
#include "MacabHeader.hxx"
#include "macabutilities.hxx"

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>
#include <com/sun/star/util/DateTime.hpp>

using namespace connectivity::macab;
using namespace com::sun::star::util;

namespace {

void manageDuplicateHeaders(macabfield **_headerNames, const sal_Int32 _length)
{
    /* If we have two cases of, say, phone: home, this makes it:
     * phone: home (1)
     * phone: home (2)
     */
    sal_Int32 i, j;
    sal_Int32 count;
    for(i = _length-1; i >= 0; i--)
    {
        count = 1;
        for( j = i-1; j >= 0; j--)
        {
            if(CFEqual(_headerNames[i]->value, _headerNames[j]->value))
            {
                count++;
            }
        }

        // duplicate!
        if(count != 1)
        {
            // There is probably a better way to do this...
            OUString newName = CFStringToOUString(static_cast<CFStringRef>(_headerNames[i]->value));
            CFRelease(_headerNames[i]->value);
            newName += " (" + OUString::number(count) + ")";
            _headerNames[i]->value = OUStringToCFString(newName);
        }
    }
}

}

MacabRecords::MacabRecords(const ABAddressBookRef _addressBook, MacabHeader *_header, MacabRecord **_records, sal_Int32 _numRecords)
{
    /* Variables passed in... */
    header = _header;
    recordsSize = _numRecords;
    currentRecord = _numRecords;
    records = _records;
    addressBook = _addressBook;

    /* Default variables... */
    recordType = kABPersonRecordType;

    /* Variables constructed... */
    bootstrap_CF_types();
    bootstrap_requiredProperties();
}


/* Creates a MacabRecords from another: copies the length, name, and
 * address book of the original, but the header or the records themselves.
 * The idea is that the only reason to copy a MacabRecords is to create
 * a filtered version of it, which can have the same length (to avoid
 * resizing) and will work from the same base addressbook, but might have
 * entirely different values and even (possibly in the future) a different
 * header.
 */
MacabRecords::MacabRecords(const MacabRecords *_copy)
{
    /* Variables passed in... */
    recordsSize = _copy->recordsSize;
    addressBook = _copy->addressBook;
    m_sName = _copy->m_sName;

    /* Default variables... */
    currentRecord = 0;
    header = nullptr;
    records = new MacabRecord *[recordsSize];
    recordType = kABPersonRecordType;

    /* Variables constructed... */
    bootstrap_CF_types();
    bootstrap_requiredProperties();
}


MacabRecords::MacabRecords(const ABAddressBookRef _addressBook)
{
    /* Variables passed in... */
    addressBook = _addressBook;

    /* Default variables... */
    recordsSize = 0;
    currentRecord = 0;
    records = nullptr;
    header = nullptr;
    recordType = kABPersonRecordType;

    /* Variables constructed... */
    bootstrap_CF_types();
    bootstrap_requiredProperties();
}


void MacabRecords::initialize()
{

    /* Make sure everything is NULL before initializing. (We usually just
     * initialize after we use the constructor that takes only a
     * MacabAddressBook, so these variables will most likely already be
     * NULL.
     */
    if(records != nullptr)
    {
        sal_Int32 i;

        for(i = 0; i < recordsSize; i++)
            delete records[i];

        delete [] records;
    }

    if(header != nullptr)
        delete header;

    /* We can handle both default record Address Book record types in
     * this method, though only kABPersonRecordType is ever used.
     */
    CFArrayRef allRecords;
    if(CFStringCompare(recordType, kABPersonRecordType, 0) == kCFCompareEqualTo)
        allRecords = ABCopyArrayOfAllPeople(addressBook);
    else
        allRecords = ABCopyArrayOfAllGroups(addressBook);

    ABRecordRef record;
    sal_Int32 i;
    recordsSize = static_cast<sal_Int32>(CFArrayGetCount(allRecords));
    records = new MacabRecord *[recordsSize];

    /* First, we create the header... */
    header = createHeaderForRecordType(allRecords, recordType);

    /* Then, we create each of the records... */
    for(i = 0; i < recordsSize; i++)
    {
        record = const_cast<ABRecordRef>(CFArrayGetValueAtIndex(allRecords, i));
        records[i] = createMacabRecord(record, header, recordType);
    }
    currentRecord = recordsSize;

    CFRelease(allRecords);
}


MacabRecords::~MacabRecords()
{
}


void MacabRecords::setHeader(MacabHeader *_header)
{
    if(header != nullptr)
        delete header;
    header = _header;
}


MacabHeader *MacabRecords::getHeader() const
{
    return header;
}


/* Inserts a MacabRecord at a given location. If there is already a
 * MacabRecord at that location, return it.
 */
MacabRecord *MacabRecords::insertRecord(MacabRecord *_newRecord, const sal_Int32 _location)
{
    MacabRecord *oldRecord;

    /* If the location is greater than the current allocated size of this
     * MacabRecords, allocate more space.
     */
    if(_location >= recordsSize)
    {
        sal_Int32 i;
        MacabRecord **newRecordsArray = new MacabRecord *[_location+1];
        for(i = 0; i < recordsSize; i++)
        {
            newRecordsArray[i] = records[i];
        }
        delete [] records;
        records = newRecordsArray;
    }

    /* Remember: currentRecord refers to one above the highest existing
     * record (i.e., it refers to where to place the next record if a
     * location is not given).
     */
    if(_location >= currentRecord)
        currentRecord = _location+1;

    oldRecord = records[_location];
    records[_location] = _newRecord;
    return oldRecord;
}


/* Insert a record at the next available place. */
void MacabRecords::insertRecord(MacabRecord *_newRecord)
{
    insertRecord(_newRecord, currentRecord);
}


MacabRecord *MacabRecords::getRecord(const sal_Int32 _location) const
{
    if(_location >= recordsSize)
        return nullptr;
    return records[_location];
}


macabfield *MacabRecords::getField(const sal_Int32 _recordNumber, const sal_Int32 _columnNumber) const
{
    if(_recordNumber >= recordsSize)
        return nullptr;

    MacabRecord *record = records[_recordNumber];

    if(_columnNumber < 0 || _columnNumber >= record->getSize())
        return nullptr;

    return record->get(_columnNumber);
}


macabfield *MacabRecords::getField(const sal_Int32 _recordNumber, const OUString& _columnName) const
{
    if(header != nullptr)
    {
        sal_Int32 columnNumber = header->getColumnNumber(_columnName);
        if(columnNumber == -1)
            return nullptr;

        return getField(_recordNumber, columnNumber);
    }
    else
    {
        // error: shouldn't access field with null header!
        return nullptr;
    }
}


sal_Int32 MacabRecords::getFieldNumber(const OUString& _columnName) const
{
    if(header != nullptr)
        return header->getColumnNumber(_columnName);
    else
        // error: shouldn't access field with null header!
        return -1;
}


/* Create the lcl_CFTypes array -- we need this because there is no
 * way to get the ABType of an object from the object itself, and the
 * function ABTypeOfProperty can't handle multiple levels of data
 * (e.g., it can tell us that "address" is of type
 * kABDictionaryProperty, but it cannot tell us that all of the keys
 * and values in the dictionary have type kABStringProperty. On the
 * other hand, we _can_ get the CFType out of any object.
 * Unfortunately, all information about CFTypeIDs comes with the
 * warning that they change between releases, so we build them
 * ourselves here. (The one that we can't build is for multivalues,
 * e.g., kABMultiStringProperty. All of these appear to have the
 * same type: 1, but there is no function that I've found to give
 * us that dynamically in case that number ever changes.
 */
void MacabRecords::bootstrap_CF_types()
{
    lcl_CFTypes = {
        {CFNumberGetTypeID(), kABIntegerProperty},
        {CFStringGetTypeID(), kABStringProperty},
        {CFDateGetTypeID(), kABDateProperty},
        {CFArrayGetTypeID(), kABArrayProperty},
        {CFDictionaryGetTypeID(), kABDictionaryProperty},
        {CFDataGetTypeID(), kABDataProperty}};
}


/* This is based on the possible fields required in the mail merge template
 * in sw. If the fields possible there change, it would be optimal to
 * change these fields as well.
 */
void MacabRecords::bootstrap_requiredProperties()
{
    requiredProperties = {
        kABTitleProperty, kABFirstNameProperty, kABLastNameProperty, kABOrganizationProperty,
        kABAddressProperty, kABPhoneProperty, kABEmailProperty};
}


/* Create the header for a given record type and a given array of records.
 * Because the array of records and the record type are given, if you want
 * to, you can run this method on the members of a group, or on any other
 * filtered list of people and get a header relevant to them (e.g., if
 * they only have home addresses, the work address fields won't show up).
 */
MacabHeader *MacabRecords::createHeaderForRecordType(const CFArrayRef _records, const CFStringRef _recordType) const
{
    /* We have two types of properties for a given record type, nonrequired
     * and required. Required properties are ones that will show up whether
     * or not they are empty. Nonrequired properties will only show up if
     * at least one record in the set has that property filled. The reason
     * is that some properties, like the kABTitleProperty are required by
     * the mail merge wizard (in module sw) but are by default not shown in
     * the macOS address book, so they would be weeded out at this stage
     * and not shown if they were not required.
     *
     * Note: with the addition of required properties, I am not sure that
     * this method still works for kABGroupRecordType (since the required
     * properites are all for kABPersonRecordType).
     *
     * Note: required properties are constructed in the method
     * bootstrap_requiredProperties() (above).
     */
    CFArrayRef allProperties = ABCopyArrayOfPropertiesForRecordType(addressBook, _recordType);
    CFStringRef *nonRequiredProperties;
    sal_Int32 numRecords = static_cast<sal_Int32>(CFArrayGetCount(_records));
    sal_Int32 numProperties = static_cast<sal_Int32>(CFArrayGetCount(allProperties));
    sal_Int32 numNonRequiredProperties = numProperties - requiredProperties.size();

    /* While searching through the properties for required properties, these
     * sal_Bools will keep track of what we have found.
     */
    bool bFoundRequiredProperties[requiredProperties.size()];


    /* We have three MacabHeaders: headerDataForProperty is where we
     * store the result of createHeaderForProperty(), which return a
     * MacabHeader for a single property. lcl_header is where we store
     * the MacabHeader that we are constructing. And, nonRequiredHeader
     * is where we construct the MacabHeader for non-required properties,
     * so that we can sort them before adding them to lcl_header.
     */
    MacabHeader *headerDataForProperty;
    MacabHeader *lcl_header = new MacabHeader();
    MacabHeader *nonRequiredHeader = new MacabHeader();

    /* Other variables... */
    sal_Int32 k;
    ABRecordRef record;
    CFStringRef property;


    /* Allocate and initialize... */
    nonRequiredProperties = new CFStringRef[numNonRequiredProperties];
    k = 0;
    for(std::vector<CFStringRef>::size_type i = 0; i < requiredProperties.size(); i++)
        bFoundRequiredProperties[i] = false;

    /* Determine the non-required properties... */
    for(sal_Int32 i = 0; i < numProperties; i++)
    {
        bool bFoundProperty = false;
        property = static_cast<CFStringRef>(CFArrayGetValueAtIndex(allProperties, i));
        for(std::vector<CFStringRef>::size_type j = 0; j < requiredProperties.size(); j++)
        {
            if(CFEqual(property, requiredProperties[j]))
            {
                bFoundProperty = true;
                bFoundRequiredProperties[j] = true;
                break;
            }
        }

        if(!bFoundProperty)
        {
            /* If we have found too many non-required properties */
            if(k == numNonRequiredProperties)
            {
                k++; // so that the OSL_ENSURE below fails
                break;
            }
            nonRequiredProperties[k] = property;
            k++;
        }
    }

    // Somehow, we got too many or too few non-required properties...
    // Most likely, one of the required properties no longer exists, which
    // we also test later.
    OSL_ENSURE(k == numNonRequiredProperties, "MacabRecords::createHeaderForRecordType: Found an unexpected number of non-required properties");

    /* Fill the header with required properties first... */
    for(std::vector<CFStringRef>::size_type i = 0; i < requiredProperties.size(); i++)
    {
        if(bFoundRequiredProperties[i])
        {
            /* The order of these matters (we want all address properties
             * before any phone properties, or else things will look weird),
             * so we get all possibilities for each property, going through
             * each record, and then go onto the next property.
             * (Note: the reason that we have to go through all records
             * in the first place is that properties like address, phone, and
             * e-mail are multi-value properties with an unknown number of
             * values. A user could specify thirteen different kinds of
             * e-mail addresses for one of her or his contacts, and we need to
             * get all of them.
             */
            for(sal_Int32 j = 0; j < numRecords; j++)
            {
                record = const_cast<ABRecordRef>(CFArrayGetValueAtIndex(_records, j));
                headerDataForProperty = createHeaderForProperty(record,requiredProperties[i],_recordType,true);
                if(headerDataForProperty != nullptr)
                {
                    (*lcl_header) += headerDataForProperty;
                    delete headerDataForProperty;
                }
            }
        }
        else
        {
            // Couldn't find a required property...
            OSL_FAIL(OString(OString("MacabRecords::createHeaderForRecordType: could not find required property: ") +
                                    OUStringToOString(CFStringToOUString(requiredProperties[i]), RTL_TEXTENCODING_ASCII_US)).getStr());
        }
    }

    /* And now, non-required properties... */
    for(sal_Int32 i = 0; i < numRecords; i++)
    {
        record = const_cast<ABRecordRef>(CFArrayGetValueAtIndex(_records, i));

        for(sal_Int32 j = 0; j < numNonRequiredProperties; j++)
        {
            property = nonRequiredProperties[j];
            headerDataForProperty = createHeaderForProperty(record,property,_recordType,false);
            if(headerDataForProperty != nullptr)
            {
                (*nonRequiredHeader) += headerDataForProperty;
                delete headerDataForProperty;
            }
        }

    }
    nonRequiredHeader->sortRecord();

    (*lcl_header) += nonRequiredHeader;
    delete nonRequiredHeader;

    CFRelease(allProperties);
    delete [] nonRequiredProperties;

    return lcl_header;
}


/* Create a header for a single property. Basically, this method gets
 * the property's value and type and then calls another method of
 * the same name to do the dirty work.
 */
MacabHeader *MacabRecords::createHeaderForProperty(const ABRecordRef _record, const CFStringRef _propertyName, const CFStringRef _recordType, const bool _isPropertyRequired) const
{
    // local variables
    CFStringRef localizedPropertyName;
    CFTypeRef propertyValue;
    ABPropertyType propertyType;
    MacabHeader *result;

    /* Get the property's value */
    propertyValue = ABRecordCopyValue(_record,_propertyName);
    if(propertyValue == nullptr && !_isPropertyRequired)
        return nullptr;

    propertyType = ABTypeOfProperty(addressBook, _recordType, _propertyName);
    localizedPropertyName = ABCopyLocalizedPropertyOrLabel(_propertyName);

    result = createHeaderForProperty(propertyType, propertyValue, localizedPropertyName);

    if(propertyValue != nullptr)
        CFRelease(propertyValue);

    return result;
}


/* Create a header for a single property. This method is recursive
 * because a single property might contain several sub-properties that
 * we also want to treat singly.
 */
MacabHeader *MacabRecords::createHeaderForProperty(const ABPropertyType _propertyType, const CFTypeRef _propertyValue, const CFStringRef _propertyName) const
{
    macabfield **headerNames = nullptr;
    sal_Int32 length = 0;

    switch(_propertyType)
    {
        /* Scalars */
        case kABStringProperty:
        case kABRealProperty:
        case kABIntegerProperty:
        case kABDateProperty:
            length = 1;
            headerNames = new macabfield *[1];
            headerNames[0] = new macabfield;
            headerNames[0]->value = _propertyName;
            headerNames[0]->type = _propertyType;
            break;

        /* Multi-scalars */
        case kABMultiIntegerProperty:
        case kABMultiDateProperty:
        case kABMultiStringProperty:
        case kABMultiRealProperty:
        case kABMultiDataProperty:
            /* For non-scalars, we can only get more information if the property
             * actually exists.
             */
            if(_propertyValue != nullptr)
            {
            sal_Int32 i;

            sal_Int32 multiLength = ABMultiValueCount(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)));
            CFStringRef multiLabel, localizedMultiLabel;
            OUString multiLabelString;
            OUString multiPropertyString;
            OUString headerNameString;
            ABPropertyType multiType = static_cast<ABPropertyType>(ABMultiValuePropertyType(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue))) - 0x100);

            length = multiLength;
            headerNames = new macabfield *[multiLength];
            multiPropertyString = CFStringToOUString(_propertyName);

            /* Go through each element, and - since each element is a scalar -
             * just create a new macabfield for it.
             */
            for(i = 0; i < multiLength; i++)
            {
                multiLabel = ABMultiValueCopyLabelAtIndex(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)), i);
                localizedMultiLabel = ABCopyLocalizedPropertyOrLabel(multiLabel);
                multiLabelString = CFStringToOUString(localizedMultiLabel);
                CFRelease(multiLabel);
                CFRelease(localizedMultiLabel);
                headerNameString = multiPropertyString + ": " + fixLabel(multiLabelString);
                headerNames[i] = new macabfield;
                headerNames[i]->value = OUStringToCFString(headerNameString);
                headerNames[i]->type = multiType;
            }
            }
            break;

        /* Multi-array or dictionary */
        case kABMultiArrayProperty:
        case kABMultiDictionaryProperty:
            /* For non-scalars, we can only get more information if the property
             * actually exists.
             */
            if(_propertyValue != nullptr)
            {
                sal_Int32 i,j,k;

                // Total number of multi-array or multi-dictionary elements.
                sal_Int32 multiLengthFirstLevel = ABMultiValueCount(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)));

                /* Total length, including the length of each element (e.g., if
                 * this multi-dictionary contains three dictionaries, and each
                 * dictionary has four elements, this variable will be twelve,
                 * whereas multiLengthFirstLevel will be three.
                 */
                sal_Int32 multiLengthSecondLevel = 0;

                CFStringRef multiLabel, localizedMultiLabel;
                CFTypeRef multiValue;
                OUString multiLabelString;
                OUString multiPropertyString;
                std::vector<std::unique_ptr<MacabHeader>> multiHeaders;
                ABPropertyType multiType = static_cast<ABPropertyType>(ABMultiValuePropertyType(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue))) - 0x100);

                multiPropertyString = CFStringToOUString(_propertyName);

                /* Go through each element - since each element can really
                 * contain anything, we run this method again on each element
                 * and store the resulting MacabHeader (in the multiHeaders
                 * array). Then, all we'll have to do is combine the MacabHeaders
                 * into a single one.
                 */
                for(i = 0; i < multiLengthFirstLevel; i++)
                {
                    /* label */
                    multiLabel = ABMultiValueCopyLabelAtIndex(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)), i);
                    multiValue = ABMultiValueCopyValueAtIndex(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)), i);
                    std::unique_ptr<MacabHeader> hdr;
                    if(multiValue && multiLabel)
                    {
                        localizedMultiLabel = ABCopyLocalizedPropertyOrLabel(multiLabel);
                        multiLabelString = multiPropertyString + ": " + fixLabel(CFStringToOUString(localizedMultiLabel));
                        CFRelease(multiLabel);
                        CFRelease(localizedMultiLabel);
                        multiLabel = OUStringToCFString(multiLabelString);
                        hdr.reset(createHeaderForProperty(multiType, multiValue, multiLabel));
                        if (!hdr)
                            hdr = std::make_unique<MacabHeader>();
                        multiLengthSecondLevel += hdr->getSize();
                    }
                    else
                    {
                        hdr = std::make_unique<MacabHeader>();
                    }
                    if(multiValue)
                        CFRelease(multiValue);
                    if(multiLabel)
                        CFRelease(multiLabel);
                    multiHeaders.push_back(std::move(hdr));
                }

                /* We now have enough information to create our final MacabHeader.
                 * We go through each field of each header and add it to the
                 * headerNames array (which is what is used below to construct
                 * the MacabHeader we return).
                 */
                length = multiLengthSecondLevel;
                headerNames = new macabfield *[multiLengthSecondLevel];

                for(i = 0, j = 0, k = 0; i < multiLengthSecondLevel; i++,k++)
                {
                    while(multiHeaders[j]->getSize() == k)
                    {
                        j++;
                        k = 0;
                    }

                    headerNames[i] = multiHeaders[j]->copy(k);
                }
            }
            break;

        /* Dictionary */
        case kABDictionaryProperty:
            /* For non-scalars, we can only get more information if the property
             * actually exists.
             */
            if(_propertyValue != nullptr)
            {
            /* Assume all keys are strings */
            sal_Int32 numRecords = static_cast<sal_Int32>(CFDictionaryGetCount(static_cast<CFDictionaryRef>(_propertyValue)));

            /* The only method for getting info out of a CFDictionary, of both
             * keys and values, is to all of them all at once, so these
             * variables will hold them.
             */
            CFStringRef *dictKeys;
            CFTypeRef *dictValues;

            sal_Int32 i,j,k;
            OUString dictKeyString, propertyNameString;
            ABPropertyType dictType;
            MacabHeader **dictHeaders = new MacabHeader *[numRecords];
            OUString dictLabelString;
            CFStringRef dictLabel, localizedDictKey;

            /* Get the keys and values */
            dictKeys = static_cast<CFStringRef *>(malloc(sizeof(CFStringRef)*numRecords));
            dictValues = static_cast<CFTypeRef *>(malloc(sizeof(CFTypeRef)*numRecords));
            CFDictionaryGetKeysAndValues(static_cast<CFDictionaryRef>(_propertyValue), reinterpret_cast<const void **>(dictKeys), dictValues);

            propertyNameString = CFStringToOUString(_propertyName);

            length = 0;
            /* Go through each element - assuming that the key is a string but
             * that the value could be anything. Since the value could be
             * anything, we can't assume that it is scalar (it could even be
             * another dictionary), so we attempt to get its type using
             * the method getABTypeFromCFType and then run this method
             * recursively on that element, storing the MacabHeader that
             * results. Then, we just combine all of the MacabHeaders into
             * one.
             */
            for(i = 0; i < numRecords; i++)
            {
                dictType = getABTypeFromCFType( CFGetTypeID(dictValues[i]) );
                localizedDictKey = ABCopyLocalizedPropertyOrLabel(dictKeys[i]);
                dictKeyString = CFStringToOUString(localizedDictKey);
                dictLabelString = propertyNameString + ": " + fixLabel(dictKeyString);
                dictLabel = OUStringToCFString(dictLabelString);
                dictHeaders[i] = createHeaderForProperty(dictType, dictValues[i], dictLabel);
                if (!dictHeaders[i])
                    dictHeaders[i] = new MacabHeader();
                length += dictHeaders[i]->getSize();
                CFRelease(dictLabel);
                CFRelease(localizedDictKey);
            }

            /* Combine all of the macabfields in each MacabHeader into the
             * headerNames array, which (at the end of this method) is used
             * to create the MacabHeader that is returned.
             */
            headerNames = new macabfield *[length];
            for(i = 0, j = 0, k = 0; i < length; i++,k++)
            {
                while(dictHeaders[j]->getSize() == k)
                {
                    j++;
                    k = 0;
                }

                headerNames[i] = dictHeaders[j]->copy(k);
            }

            for(i = 0; i < numRecords; i++)
                delete dictHeaders[i];

            delete [] dictHeaders;
            free(dictKeys);
            free(dictValues);
            }
            break;

        /* Array */
        case kABArrayProperty:
            /* For non-scalars, we can only get more information if the property
             * actually exists.
             */
            if(_propertyValue != nullptr)
            {
                sal_Int32 arrLength = static_cast<sal_Int32>(CFArrayGetCount(static_cast<CFArrayRef>(_propertyValue)));
                sal_Int32 i,j,k;
                CFTypeRef arrValue;
                ABPropertyType arrType;
                std::vector<std::unique_ptr<MacabHeader>> arrHeaders;
                OUString propertyNameString = CFStringToOUString(_propertyName);
                OUString arrLabelString;
                CFStringRef arrLabel;

                length = 0;
                /* Go through each element - since the elements here do not have
                 * unique keys like the ones in dictionaries, we create a unique
                 * key out of the id of the element in the array (the first
                 * element gets a 0 plopped onto the end of it, the second a 1...
                 * As with dictionaries, the elements could be anything, including
                 * another array, so we have to run this method recursively on
                 * each element, storing the resulting MacabHeader into an array,
                 * which we then combine into one MacabHeader that is returned.
                 */
                for(i = 0; i < arrLength; i++)
                {
                    arrValue = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(_propertyValue), i);
                    arrType = getABTypeFromCFType( CFGetTypeID(arrValue) );
                    arrLabelString = propertyNameString + OUString::number(i);
                    arrLabel = OUStringToCFString(arrLabelString);
                    auto hdr = std::unique_ptr<MacabHeader>(createHeaderForProperty(arrType, arrValue, arrLabel));
                    if (!hdr)
                        hdr = std::make_unique<MacabHeader>();
                    length += hdr->getSize();
                    CFRelease(arrLabel);
                    arrHeaders.push_back(std::move(hdr));
                }

                headerNames = new macabfield *[length];
                for(i = 0, j = 0, k = 0; i < length; i++,k++)
                {
                    while(arrHeaders[j]->getSize() == k)
                    {
                        j++;
                        k = 0;
                    }

                    headerNames[i] = arrHeaders[j]->copy(k);
                }
            }
            break;

            default:
                break;

    }

    /* If we succeeded at adding elements to the headerNames array, then
     * length will no longer be 0. If it is, create a new MacabHeader
     * out of the headerNames (after weeding out duplicate headers), and
     * then return the result. If the length is still 0, return NULL: we
     * failed to create a MacabHeader out of this property.
     */
    if(length != 0)
    {
        manageDuplicateHeaders(headerNames, length);
        MacabHeader *headerResult = new MacabHeader(length, headerNames);
        for(sal_Int32 i = 0; i < length; ++i)
            delete headerNames[i];
        delete [] headerNames;
        return headerResult;
    }
    else
        return nullptr;
}


/* Create a MacabRecord out of an ABRecord, using a given MacabHeader and
 * the record's type. We go through each property for this record type
 * then process it much like we processed the header (above), with two
 * exceptions: if we come upon something not in the header, we ignore it
 * (it's something we don't want to add), and once we find a corresponding
 * location in the header, we store the property and the property type in
 * a macabfield. (For the header, we stored the property type and the name
 * of the property as a CFString.)
 */
MacabRecord *MacabRecords::createMacabRecord(const ABRecordRef _abrecord, const MacabHeader *_header, const CFStringRef _recordType) const
{
    /* The new record that we will create... */
    MacabRecord *macabRecord = new MacabRecord(_header->getSize());

    CFArrayRef recordProperties = ABCopyArrayOfPropertiesForRecordType(addressBook, _recordType);
    sal_Int32 numProperties = static_cast<sal_Int32>(CFArrayGetCount(recordProperties));

    sal_Int32 i;

    CFTypeRef propertyValue;
    ABPropertyType propertyType;

    CFStringRef propertyName, localizedPropertyName;
    OUString propertyNameString;
    for(i = 0; i < numProperties; i++)
    {
        propertyName = static_cast<CFStringRef>(CFArrayGetValueAtIndex(recordProperties, i));
        localizedPropertyName = ABCopyLocalizedPropertyOrLabel(propertyName);
        propertyNameString = CFStringToOUString(localizedPropertyName);
        CFRelease(localizedPropertyName);

        /* Get the property's value */
        propertyValue = ABRecordCopyValue(_abrecord,propertyName);
        if(propertyValue != nullptr)
        {
            propertyType = ABTypeOfProperty(addressBook, _recordType, propertyName);
            if(propertyType != kABErrorInProperty)
                insertPropertyIntoMacabRecord(propertyType, macabRecord, _header, propertyNameString, propertyValue);

            CFRelease(propertyValue);
        }
    }
    CFRelease(recordProperties);
    return macabRecord;
}


/* Inserts a given property into a MacabRecord. This method calls another
 * method by the same name after getting the property type (it only
 * receives the property value). It is called when we aren't given the
 * property's type already.
 */
void MacabRecords::insertPropertyIntoMacabRecord(MacabRecord *_abrecord, const MacabHeader *_header, const OUString& _propertyName, const CFTypeRef _propertyValue) const
{
    CFTypeID cf_type = CFGetTypeID(_propertyValue);
    ABPropertyType ab_type = getABTypeFromCFType( cf_type );

    if(ab_type != kABErrorInProperty)
        insertPropertyIntoMacabRecord(ab_type, _abrecord, _header, _propertyName, _propertyValue);
}


/* Inserts a given property into a MacabRecord. This method is recursive
 * because properties can contain many sub-properties.
 */
void MacabRecords::insertPropertyIntoMacabRecord(const ABPropertyType _propertyType, MacabRecord *_abrecord, const MacabHeader *_header, const OUString& _propertyName, const CFTypeRef _propertyValue) const
{
    /* If there is no value, return */
    if(_propertyValue == nullptr)
        return;

    /* The main switch statement */
    switch(_propertyType)
    {
        /* Scalars */
        case kABStringProperty:
        case kABRealProperty:
        case kABIntegerProperty:
        case kABDateProperty:
        {
            /* Only scalars actually insert a property into the MacabRecord.
             * In all other cases, this method is called recursively until a
             * scalar type, an error, or an unknown type are found.
             * Because of that, the following checks only occur for this type.
             * We store whether we have successfully placed this property
             * into the MacabRecord (or whether an unrecoverable error occurred).
             * Then, we try over and over again to place the property into the
             * record. There are three possible results:
             * 1) Success!
             * 2) There is already a property stored at the column of this name,
             * in which case we have a duplicate header (see the method
             * manageDuplicateHeaders()). If that is the case, we add an ID
             * to the end of the column name in the same format as we do in
             * manageDuplicateHeaders() and try again.
             * 3) No column of this name exists in the header. In this case,
             * there is nothing we can do: we have failed to place this
             * property into the record.
             */
            bool bPlaced = false;
            OUString columnName = _propertyName;
            sal_Int32 i = 1;

            // A big safeguard to prevent two fields from having the same name.
            while(!bPlaced)
            {
                sal_Int32 columnNumber = _header->getColumnNumber(columnName);
                bPlaced = true;
                if(columnNumber != -1)
                {
                    // collision! A property already exists here!
                    if(_abrecord->get(columnNumber) != nullptr)
                    {
                        bPlaced = false;
                        i++;
                        columnName = _propertyName + " (" + OUString::number(i) + ")";
                    }

                    // success!
                    else
                    {
                        _abrecord->insertAtColumn(_propertyValue, _propertyType, columnNumber);
                    }
                }
            }
        }
        break;

        /* Array */
        case kABArrayProperty:
            {
                /* An array is basically just a list of anything, so all we do
                 * is go through the array, and rerun this method recursively
                 * on each element.
                 */
                sal_Int32 arrLength = static_cast<sal_Int32>(CFArrayGetCount(static_cast<CFArrayRef>(_propertyValue)));
                sal_Int32 i;
                OUString newPropertyName;

                /* Going through each element... */
                for(i = 0; i < arrLength; i++)
                {
                    const void *arrValue = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(_propertyValue), i);
                    newPropertyName = _propertyName + OUString::number(i);
                    insertPropertyIntoMacabRecord(_abrecord, _header, newPropertyName, arrValue);
                    CFRelease(arrValue);
                }

            }
            break;

        /* Dictionary */
        case kABDictionaryProperty:
            {
                /* A dictionary is basically a hashmap. Technically, it can
                 * hold any object as a key and any object as a value.
                 * For our case, we assume that the key is a string (so that
                 * we can use the key to get the column name and match it against
                 * the header), but we don't assume anything about the value, so
                 * we run this method recursively (or, rather, we run the version
                 * of this method for when we don't know the object's type) until
                 * we hit a scalar value.
                 */

                sal_Int32 numRecords = static_cast<sal_Int32>(CFDictionaryGetCount(static_cast<CFDictionaryRef>(_propertyValue)));
                OUString dictKeyString;
                sal_Int32 i;
                OUString newPropertyName;

                /* Unfortunately, the only way to get both keys and values out
                 * of a dictionary in Carbon is to get them all at once, so we
                 * do that.
                 */
                CFStringRef *dictKeys;
                CFStringRef localizedDictKey;
                CFTypeRef *dictValues;
                dictKeys = static_cast<CFStringRef *>(malloc(sizeof(CFStringRef)*numRecords));
                dictValues = static_cast<CFTypeRef *>(malloc(sizeof(CFTypeRef)*numRecords));
                CFDictionaryGetKeysAndValues(static_cast<CFDictionaryRef>(_propertyValue), reinterpret_cast<const void **>(dictKeys), dictValues);

                /* Going through each element... */
                for(i = 0; i < numRecords; i++)
                {
                    localizedDictKey = ABCopyLocalizedPropertyOrLabel(dictKeys[i]);
                    dictKeyString = CFStringToOUString(localizedDictKey);
                    CFRelease(localizedDictKey);
                    newPropertyName = _propertyName + ": " + fixLabel(dictKeyString);
                    insertPropertyIntoMacabRecord(_abrecord, _header, newPropertyName, dictValues[i]);
                }

                free(dictKeys);
                free(dictValues);
            }
            break;

        /* Multivalue */
        case kABMultiIntegerProperty:
        case kABMultiDateProperty:
        case kABMultiStringProperty:
        case kABMultiRealProperty:
        case kABMultiDataProperty:
        case kABMultiDictionaryProperty:
        case kABMultiArrayProperty:
            {
                /* All scalar multivalues are handled in the same way. Each element
                 * is a label and a value. All labels are strings
                 * (kABStringProperty), and all values have the same type
                 * (which is the type of the multivalue minus 255, or as
                 * Carbon's list of property types has it, minus 0x100.
                 * We just get the correct type, then go through each element
                 * and get the label and value and print them in a list.
                 */

                sal_Int32 i;
                sal_Int32 multiLength = ABMultiValueCount(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)));
                CFStringRef multiLabel, localizedMultiLabel;
                CFTypeRef multiValue;
                OUString multiLabelString, newPropertyName;
                ABPropertyType multiType = static_cast<ABPropertyType>(ABMultiValuePropertyType(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue))) - 0x100);

                /* Go through each element... */
                for(i = 0; i < multiLength; i++)
                {
                    /* Label and value */
                    multiLabel = ABMultiValueCopyLabelAtIndex(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)), i);
                    multiValue = ABMultiValueCopyValueAtIndex(static_cast<ABMutableMultiValueRef>(const_cast<void *>(_propertyValue)), i);

                    localizedMultiLabel = ABCopyLocalizedPropertyOrLabel(multiLabel);
                    multiLabelString = CFStringToOUString(localizedMultiLabel);
                    newPropertyName = _propertyName + ": " + fixLabel(multiLabelString);
                    insertPropertyIntoMacabRecord(multiType, _abrecord, _header, newPropertyName, multiValue);

                    /* free our variables */
                    CFRelease(multiLabel);
                    CFRelease(localizedMultiLabel);
                    CFRelease(multiValue);
                }
            }
            break;

        /* Unhandled types */
        case kABErrorInProperty:
        case kABDataProperty:
        default:
            /* An error, as far as I have seen, only shows up as a type
             * returned by a function for dictionaries when the dictionary
             * holds many types of values. Since we do not use that function,
             * it shouldn't come up. I have yet to see the kABDataProperty,
             * and I am not sure how to represent it as a string anyway,
             * since it appears to just be a bunch of bytes. Assumably, if
             * these bytes made up a string, the type would be
             * kABStringProperty. I think that this is used when we are not
             * sure what the type is (e.g., it could be a string or a number).
             * That being the case, I still don't know how to represent it.
             * And, default should never come up, since we've exhausted all
             * of the possible types for ABPropertyType, but... just in case.
             */
            break;
    }

}


ABPropertyType MacabRecords::getABTypeFromCFType(const CFTypeID cf_type ) const
{
    for(auto const & i: lcl_CFTypes)
    {
        /* A match! */
        if(i.cf == cf_type)
        {
            return static_cast<ABPropertyType>(i.ab);
        }
    }
    return kABErrorInProperty;
}


sal_Int32 MacabRecords::size() const
{
    return currentRecord;
}


MacabRecords *MacabRecords::begin()
{
    return this;
}


MacabRecords::iterator::iterator ()
{
}


MacabRecords::iterator::~iterator ()
{
}


MacabRecords::iterator& MacabRecords::iterator::operator= (MacabRecords *_records)
{
    id = 0;
    records = _records;
    return *this;
}


void MacabRecords::iterator::operator++ ()
{
    id++;
}


bool MacabRecords::iterator::operator!= (const sal_Int32 i) const
{
    return(id != i);
}


bool MacabRecords::iterator::operator== (const sal_Int32 i) const
{
    return(id == i);
}


MacabRecord *MacabRecords::iterator::operator* () const
{
    return records->getRecord(id);
}


sal_Int32 MacabRecords::end() const
{
    return currentRecord;
}


void MacabRecords::swap(const sal_Int32 _id1, const sal_Int32 _id2)
{
    MacabRecord *swapRecord = records[_id1];

    records[_id1] = records[_id2];
    records[_id2] = swapRecord;
}


void MacabRecords::setName(const OUString& _sName)
{
    m_sName = _sName;
}


OUString const & MacabRecords::getName() const
{
    return m_sName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
