/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabHeader.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:52:51 $
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

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#define _CONNECTIVITY_MACAB_HEADER_HXX_

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#include "MacabRecord.hxx"
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

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
                        void operator= (MacabHeader *_record);
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
