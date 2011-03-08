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

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#define _CONNECTIVITY_MACAB_HEADER_HXX_

#include "MacabRecord.hxx"
#include <cppuhelper/compbase3.hxx>

namespace connectivity
{
    namespace macab
    {
        class MacabHeader: public MacabRecord{
            protected:
                macabfield **sortRecord(sal_Int32 _start, sal_Int32 _length);
            public:
                MacabHeader();
                MacabHeader(const sal_Int32 _size, macabfield **_fields);
                virtual ~MacabHeader();
                void operator+= (const MacabHeader *r);
                ::rtl::OUString getString(const sal_Int32 i) const;
                void sortRecord();
                sal_Int32 getColumnNumber(const ::rtl::OUString s) const;

                static sal_Int32 compareFields(const macabfield *_field1, const macabfield *_field2);

                MacabHeader *begin();
                sal_Int32 end() const;
                class iterator{
                    protected:
                        sal_Int32 id;
                        MacabHeader *record;
                    public:
                        iterator& operator= (MacabHeader *_record);
                        iterator();
                        ~iterator();
                        void operator++ ();
                        sal_Bool operator!= (const sal_Int32 i) const;
                        sal_Bool operator== (const sal_Int32 i) const;
                        macabfield *operator* () const;
                };
        };
    }
}

#endif // _CONNECTIVITY_MACAB_HEADER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
