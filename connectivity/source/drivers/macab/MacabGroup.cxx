/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
