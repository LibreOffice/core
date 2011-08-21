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

#include "MacabAddressBook.hxx"
#include "MacabRecords.hxx"
#include "MacabGroup.hxx"

#include <vector>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>
#include "connectivity/CommonTools.hxx"

using namespace connectivity::macab;
using namespace ::com::sun::star::uno;

// -----------------------------------------------------------------------------
MacabAddressBook::MacabAddressBook( )
{
    m_aAddressBook = ABGetSharedAddressBook();
    m_xMacabRecords = NULL;
    m_bRetrievedGroups = sal_False;
}

// -----------------------------------------------------------------------------
MacabAddressBook::~MacabAddressBook()
{
    if(m_xMacabRecords != NULL)
    {
        delete m_xMacabRecords;
        m_xMacabRecords = NULL;
    }

    if(!m_xMacabGroups.empty())
    {
        ::std::vector<MacabGroup *>::iterator iter, end;
        iter = m_xMacabGroups.begin();
        end = m_xMacabGroups.end();
        for( ; iter != end; ++iter)
            delete (*iter);
    }

    m_bRetrievedGroups = sal_False;
}

// -----------------------------------------------------------------------------
/* Get the address book's default table name. This is the table name that
 * refers to the table containing _all_ records in the address book.
 */
const ::rtl::OUString & MacabAddressBook::getDefaultTableName()
{
    /* This string probably needs to be localized. */
    static const ::rtl::OUString aDefaultTableName
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Address Book")));

    return aDefaultTableName;
}

// -----------------------------------------------------------------------------
MacabRecords *MacabAddressBook::getMacabRecords()
{
    /* If the MacabRecords don't exist, create them. */
    if(m_xMacabRecords == NULL)
    {
        m_xMacabRecords = new MacabRecords(m_aAddressBook);
        m_xMacabRecords->setName(getDefaultTableName());
        m_xMacabRecords->initialize();
    }

    return m_xMacabRecords;
}

// -----------------------------------------------------------------------------
/* Get the MacabRecords for a given name: either a group name or the
 * default table name.
 */
MacabRecords *MacabAddressBook::getMacabRecords(const ::rtl::OUString _tableName)
{
    if(_tableName == getDefaultTableName())
    {
        return getMacabRecords();
    }
    else
    {
        return getMacabGroup(_tableName);
    }
}

// -----------------------------------------------------------------------------
MacabRecords *MacabAddressBook::getMacabRecordsMatch(const ::rtl::OUString _tableName)
{
    if(match(_tableName, getDefaultTableName(), '\0'))
    {
        return getMacabRecords();
    }

    return getMacabGroupMatch(_tableName);
}

// -----------------------------------------------------------------------------
::std::vector<MacabGroup *> MacabAddressBook::getMacabGroups()
{
    /* If the MacabGroups haven't been created yet, create them. */
    if(m_bRetrievedGroups == sal_False)
    {
        /* If the MacabRecords haven't been created yet, create them. */
        if(m_xMacabRecords == NULL)
        {
            m_xMacabRecords = new MacabRecords(m_aAddressBook);
            m_xMacabRecords->setName(getDefaultTableName());
            m_xMacabRecords->initialize();
        }

        CFArrayRef allGroups = ABCopyArrayOfAllGroups(m_aAddressBook);
        sal_Int32 nGroups = CFArrayGetCount(allGroups);
        m_xMacabGroups = ::std::vector<MacabGroup *>(nGroups);

        sal_Int32 i;
        ABGroupRef xGroup;

        /* Go through each group and create a MacabGroup out of it. */
        for(i = 0; i < nGroups; i++)
        {
            xGroup = (ABGroupRef) CFArrayGetValueAtIndex(allGroups, i);
            m_xMacabGroups[i] = new MacabGroup(m_aAddressBook, m_xMacabRecords, xGroup);
        }

        CFRelease(allGroups);

        /* Manage duplicates. */
        manageDuplicateGroups(m_xMacabGroups);
        m_bRetrievedGroups = sal_True;
    }

    return m_xMacabGroups;
}

// -----------------------------------------------------------------------------
MacabGroup *MacabAddressBook::getMacabGroup(::rtl::OUString _groupName)
{
    // initialize groups if not already initialized
    if(m_bRetrievedGroups == sal_False)
        getMacabGroups();

    sal_Int32 nGroups = m_xMacabGroups.size();
    sal_Int32 i;

    for(i = 0; i < nGroups; i++)
    {
        if(m_xMacabGroups[i] != NULL)
        {
            if(m_xMacabGroups[i]->getName() == _groupName)
            {
                return m_xMacabGroups[i];
            }
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------
MacabGroup *MacabAddressBook::getMacabGroupMatch(::rtl::OUString _groupName)
{
    // initialize groups if not already initialized
    if(m_bRetrievedGroups == sal_False)
        getMacabGroups();

    sal_Int32 nGroups = m_xMacabGroups.size();
    sal_Int32 i;

    for(i = 0; i < nGroups; i++)
    {
        if(m_xMacabGroups[i] != NULL)
        {
            if(match(m_xMacabGroups[i]->getName(), _groupName, '\0'))
            {
                return m_xMacabGroups[i];
            }
        }
    }

    return NULL;
}

// -------------------------------------------------------------------------
void MacabAddressBook::manageDuplicateGroups(::std::vector<MacabGroup *> _xGroups) const
{
    /* If we have two cases of groups, say, family, this makes it:
     * family
     * family (2)
     */
    ::std::vector<MacabGroup *>::reverse_iterator iter1, iter2;
    sal_Int32 count;

    for(iter1 = _xGroups.rbegin(); iter1 != _xGroups.rend(); ++iter1)
    {
        /* If the name matches the default table name, there is already
         * (obviously) a conflict. So, start the count of groups with this
         * name at 2 instead of 1.
         */
        if( (*iter1)->getName() == getDefaultTableName() )
            count = 2;
        else
            count = 1;

        iter2 = iter1;
        for( ++iter2; iter2 != _xGroups.rend(); ++iter2)
        {
            if( (*iter1)->getName() == (*iter2)->getName() )
            {
                count++;
            }
        }

        // duplicate!
        if(count != 1)
        {
            ::rtl::OUString sName = (*iter1)->getName();
            sName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (")) +
                ::rtl::OUString::valueOf(count) +
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            (*iter1)->setName(sName);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
