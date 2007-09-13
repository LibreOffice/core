/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabRecords.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:53:49 $
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

#ifndef _CONNECTIVITY_MACAB_RECORDS_HXX_
#define _CONNECTIVITY_MACAB_RECORDS_HXX_

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#include "MacabRecord.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#include "MacabHeader.hxx"
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

namespace connectivity
{
    namespace macab
    {
        /* This struct is for converting CF types to AB types (Core Foundation
         * types to Address Book types).
         */
        struct lcl_CFType {
            sal_Int32 cf;
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
                ::rtl::OUString m_sName;

                /* For converting CF types to AB types */
                sal_Int32 lcl_CFTypesLength;
                lcl_CFType *lcl_CFTypes;

                /* For required properties */
                CFStringRef *requiredProperties;
                sal_Int32 numRequiredProperties;

            private:
                /* All of the private methods are for creating a MacabHeader or a
                 * MacabRecord. They are used by the initialize method that goes
                 * about filling a MacabRecords using all of the records in the
                 * Mac OS X Address Book.
                 */
                void bootstrap_CF_types();
                void bootstrap_requiredProperties();
                MacabHeader *createHeaderForProperty(const ABRecordRef _record, const CFStringRef _propertyName, const CFStringRef _recordType, const sal_Bool _isPropertyRequired) const;
                MacabHeader *createHeaderForProperty(const ABPropertyType _propertyType, const CFTypeRef _propertyValue, const CFStringRef _propertyName) const;
                void manageDuplicateHeaders(macabfield **_headerNames, const sal_Int32 _length) const;
                ABPropertyType getABTypeFromCFType(const CFTypeID cf_type ) const;
                void insertPropertyIntoMacabRecord(MacabRecord *_abrecord, const MacabHeader *_header, const ::rtl::OUString  _propertyName, const CFTypeRef _propertyValue) const;
                void insertPropertyIntoMacabRecord(const ABPropertyType _propertyType, MacabRecord *_abrecord, const MacabHeader *_header, const ::rtl::OUString  _propertyName, const CFTypeRef _propertyValue) const;
            public:
                MacabRecords(const ABAddressBookRef _addressBook, MacabHeader *_header, MacabRecord **_records, sal_Int32 _numRecords);
                MacabRecords(const MacabRecords *_copy);
                MacabRecords(const ABAddressBookRef _addressBook);
                ~MacabRecords();

                void initialize();

                void setHeader(MacabHeader *_header);
                MacabHeader *getHeader() const;

                void setName(const ::rtl::OUString _sName);
                ::rtl::OUString getName() const;

                MacabRecord *insertRecord(MacabRecord *_newRecord, const sal_Int32 _location);
                void insertRecord(MacabRecord *_newRecord);
                MacabRecord *getRecord(const sal_Int32 _location) const;
                void swap(const sal_Int32 _id1, const sal_Int32 _id2);

                macabfield *getField(const sal_Int32 _recordNumber, const sal_Int32 _columnNumber) const;
                macabfield *getField(const sal_Int32 _recordNumber, const ::rtl::OUString  _columnName) const;
                sal_Int32 getFieldNumber(const ::rtl::OUString  _columnName) const;

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
                        void operator= (MacabRecords *_records);
                        iterator();
                        ~iterator();
                        void operator++ ();
                        sal_Bool operator!= (const sal_Int32 i) const;
                        sal_Bool operator== (const sal_Int32 i) const;
                        MacabRecord *operator* () const;
                };

        };
    }
}

#endif // _CONNECTIVITY_MACAB_RECORDS_HXX_
