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
                static macabfield *createMacabField(const OUString& _newFieldString, const ABPropertyType _abtype);
                static OUString fieldToString(const macabfield *_aField);

        };
    }
}

#endif // _CONNECTIVITY_MACAB_RECORD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
