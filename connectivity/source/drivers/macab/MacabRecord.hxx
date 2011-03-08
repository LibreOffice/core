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

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#define _CONNECTIVITY_MACAB_RECORD_HXX_

#include <cppuhelper/compbase3.hxx>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>

namespace connectivity
{
    namespace macab
    {
        /* a MacabRecord is at root a list of macabfields (which is just
         * something to hold both a CFTypeRef (a CoreFoundation object) and
         * its Address Book type.
         */
        struct macabfield
        {
            CFTypeRef value;
            ABPropertyType type;
        };

        class MacabRecord{
            protected:
                sal_Int32 size;
                macabfield **fields;
            protected:
                void releaseFields();
            public:
                MacabRecord();
                MacabRecord(const sal_Int32 _size);
                virtual ~MacabRecord();
                void insertAtColumn (CFTypeRef _value, ABPropertyType _type, const sal_Int32 _column);
                sal_Bool contains(const macabfield *_field) const;
                sal_Bool contains(const CFTypeRef _value) const;
                sal_Int32 getSize() const;
                macabfield *copy(const sal_Int32 i) const;
                macabfield *get(const sal_Int32 i) const;

                static sal_Int32 compareFields(const macabfield *_field1, const macabfield *_field2);
                static macabfield *createMacabField(const ::rtl::OUString _newFieldString, const ABPropertyType _abtype);
                static ::rtl::OUString fieldToString(const macabfield *_aField);

        };
    }
}

#endif // _CONNECTIVITY_MACAB_RECORD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
