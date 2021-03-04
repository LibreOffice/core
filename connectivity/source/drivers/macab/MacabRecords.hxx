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

#pragma once

#include <sal/config.h>

#include <string_view>
#include <vector>

#include "MacabRecord.hxx"
#include "MacabHeader.hxx"

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>
#include <com/sun/star/util/DateTime.hpp>

namespace connectivity::macab
{
        /* This struct is for converting CF types to AB types (Core Foundation
         * types to Address Book types).
         */
        struct lcl_CFType {
            CFTypeID cf;
            sal_Int32 ab;
        };

        class MacabRecords{
            protected:
                /* MacabRecords is, at its core, a table of macabfields, in the
                 * form of a header and a list of objects of type MacabRecord.
                 * It also has a unique name that refers to the name of the table.
                 */
                sal_Int32 recordsSize;
                sal_Int32 currentRecord;
                CFStringRef recordType;
                MacabHeader *header;
                MacabRecord **records;
                ABAddressBookRef addressBook;
                OUString m_sName;

                /* For converting CF types to AB types */
                std::vector<lcl_CFType> lcl_CFTypes;

                /* For required properties */
                std::vector<CFStringRef> requiredProperties;

            private:
                /* All of the private methods are for creating a MacabHeader or a
                 * MacabRecord. They are used by the initialize method that goes
                 * about filling a MacabRecords using all of the records in the
                 * macOS Address Book.
                 */
                void bootstrap_CF_types();
                void bootstrap_requiredProperties();
                MacabHeader *createHeaderForProperty(const ABRecordRef _record, const CFStringRef _propertyName, const CFStringRef _recordType, const bool _isPropertyRequired) const;
                MacabHeader *createHeaderForProperty(const ABPropertyType _propertyType, const CFTypeRef _propertyValue, const CFStringRef _propertyName) const;
                ABPropertyType getABTypeFromCFType(const CFTypeID cf_type ) const;
                void insertPropertyIntoMacabRecord(MacabRecord *_abrecord, const MacabHeader *_header, const OUString& _propertyName, const CFTypeRef _propertyValue) const;
                void insertPropertyIntoMacabRecord(const ABPropertyType _propertyType, MacabRecord *_abrecord, const MacabHeader *_header, const OUString& _propertyName, const CFTypeRef _propertyValue) const;
            public:
                MacabRecords(const ABAddressBookRef _addressBook, MacabHeader *_header, MacabRecord **_records, sal_Int32 _numRecords);
                explicit MacabRecords(const MacabRecords *_copy);
                explicit MacabRecords(const ABAddressBookRef _addressBook);
                ~MacabRecords();

                void initialize();

                void setHeader(MacabHeader *_header);
                MacabHeader *getHeader() const;

                void setName(const OUString& _sName);
                OUString const & getName() const;

                MacabRecord *insertRecord(MacabRecord *_newRecord, const sal_Int32 _location);
                void insertRecord(MacabRecord *_newRecord);
                MacabRecord *getRecord(const sal_Int32 _location) const;
                void swap(const sal_Int32 _id1, const sal_Int32 _id2);

                macabfield *getField(const sal_Int32 _recordNumber, const sal_Int32 _columnNumber) const;
                macabfield *getField(const sal_Int32 _recordNumber, std::u16string_view _columnName) const;
                sal_Int32 getFieldNumber(std::u16string_view _columnName) const;

                sal_Int32 size() const;

                MacabHeader *createHeaderForRecordType(const CFArrayRef _records, const CFStringRef _recordType) const;
                MacabRecord *createMacabRecord(const ABRecordRef _abrecord, const MacabHeader *_header, const CFStringRef _recordType) const;

                MacabRecords *begin();
                sal_Int32 end() const;
                class iterator{
                    protected:
                        MacabRecords *records;
                    public:
                        sal_Int32 id;
                        iterator& operator= (MacabRecords *_records);
                        iterator();
                        ~iterator();
                        void operator++ ();
                        bool operator!= (const sal_Int32 i) const;
                        bool operator== (const sal_Int32 i) const;
                        MacabRecord *operator* () const;
                };

        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
