/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabGroup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:52:23 $
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
#include "precompiled_connectivity.hxx"

#include "MacabGroup.hxx"

#ifndef _CONNECTIVITY_MACAB_RECORDS_HXX_
#include "MacabRecords.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_UTILITIES_HXX_
#include "macabutilities.hxx"
#endif

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

