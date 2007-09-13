/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabRecord.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:53:30 $
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

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#define _CONNECTIVITY_MACAB_RECORD_HXX_

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

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
