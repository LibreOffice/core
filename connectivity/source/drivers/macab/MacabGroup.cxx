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


#include "MacabGroup.hxx"
#include "MacabRecords.hxx"
#include "macabutilities.hxx"

using namespace connectivity::macab;


/* A MacabGroup is basically a MacabRecords with a different constructor.
 * It only exists as a different entity for clarification purposes (a group
 * is its own entity in the Mac OS X Address Book) and because its
 * construction is so unique (it is based on an already existent
 * MacabRecords of the entire address book).
 */
MacabGroup::MacabGroup(const ABAddressBookRef _addressBook, const MacabRecords *_allRecords, const ABGroupRef _xGroup)
    : MacabRecords(_addressBook)
{
    sal_Int32 i, j, nAllRecordsSize;
    CFArrayRef xGroupMembers = ABGroupCopyArrayOfAllMembers(_xGroup);
    ABPersonRef xPerson;
    CFStringRef sGroupMemberUID;
    bool bFound;
    macabfield *xRecordField;

    // Set the group's name (stored in MacabRecords as m_sName)
    CFStringRef sGroupName;
    sGroupName = static_cast<CFStringRef>(ABRecordCopyValue(_xGroup, kABGroupNameProperty));
    m_sName = CFStringToOUString(sGroupName);
    CFRelease(sGroupName);

    // The _group's_ records (remember MacabGroup inherits from MacabRecords)
    recordsSize = (sal_Int32) CFArrayGetCount(xGroupMembers);
    records = new MacabRecord *[recordsSize];
    setHeader(_allRecords->getHeader());

    /* Go through each record in the group and try to find that record's UID
     * in the MacabRecords that was passed in. If it is found, add that
     * record to the group. Otherwise, report an error. (All records should
     * exist in the MacabRecords that was passed in.)
     */
    nAllRecordsSize = _allRecords->size();
    for(i = 0; i < recordsSize; i++)
    {
        xPerson = static_cast<ABPersonRef>(const_cast<void *>(CFArrayGetValueAtIndex(xGroupMembers,i)));
        if(xPerson != nullptr)
        {
            sGroupMemberUID = static_cast<CFStringRef>(ABRecordCopyValue(xPerson, kABUIDProperty));
            if(sGroupMemberUID != nullptr)
            {
                bFound = false;
                for(j = 0; j < nAllRecordsSize; j++)
                {
                    xRecordField = _allRecords->getField(j,CFStringToOUString(kABUIDProperty));
                    if(xRecordField != nullptr && xRecordField->value != nullptr)
                    {
                        if(CFEqual(xRecordField->value, sGroupMemberUID))
                        {
                            /* Found the matching UID! Insert into the group... */
                            insertRecord(_allRecords->getRecord(j));
                            bFound = true;
                            break;
                        }
                    }
                }
                OSL_ENSURE(bFound, "MacabGroup::MacabGroup : Could not find group member based on UID!\n");
                CFRelease(sGroupMemberUID);
            }
        }
    }

    CFRelease(xGroupMembers);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
