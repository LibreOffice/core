/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CColumns.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:58:49 $
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

#ifndef _CONNECTIVITY_CALC_COLUMNS_HXX_
#define _CONNECTIVITY_CALC_COLUMNS_HXX_

#ifndef _CONNECTIVITY_FILE_COLUMNS_HXX_
#include "file/FColumns.hxx"
#endif

namespace connectivity
{
    namespace calc
    {
        class OCalcColumns : public file::OColumns
        {
        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        public:
            OCalcColumns(file::OFileTable* _pTable,
                            ::osl::Mutex& _rMutex,
                            const TStringVector &_rVector
                         ) : file::OColumns(_pTable,_rMutex,_rVector)
            {}

        };
    }
}

#endif // _CONNECTIVITY_CALC_COLUMNS_HXX_

