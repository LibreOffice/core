/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "MacabGroup.hxx"
#include "MacabRecords.hxx"
#include "macabutilities.hxx"

using namespace connectivity::macab;

// -------------------------------------------------------------------------
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
    sal_Bool bFound;
    macabfield *xRecordField;

    // Set the group's name (stored in MacabRecords as m_sName)
    CFStringRef sGroupName;
    sGroupName = (CFStringRef) ABRecordCopyValue(_xGroup, kABGroupNameProperty);
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
        xPerson = (ABPersonRef) CFArrayGetValueAtIndex(xGroupMembers,i);
        if(xPerson != NULL)
        {
            sGroupMemberUID = (CFStringRef) ABRecordCopyValue(xPerson, kABUIDProperty);
            if(sGroupMemberUID != NULL)
            {
                bFound = sal_False;
                for(j = 0; j < nAllRecordsSize; j++)
                {
                    xRecordField = _allRecords->getField(j,CFStringToOUString(kABUIDProperty));
                    if(xRecordField != NULL && xRecordField->value != NULL)
                    {
                        if(CFEqual(xRecordField->value, sGroupMemberUID))
                        {
                            /* Found the matching UID! Insert into the group... */
                            insertRecord(_allRecords->getRecord(j));
                            bFound = sal_True;
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

