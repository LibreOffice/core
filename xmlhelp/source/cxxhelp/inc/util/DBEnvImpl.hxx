/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DBEnvImpl.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _XMLSEARCH_DB_DBENVIMPL_HXX_
#define _XMLSEARCH_DB_DBENVIMPL_HXX_
#endif


#include <rtl/ustring.hxx>
#include <db/DBEnv.hxx>
#include <util/RandomAccessStream.hxx>
#include <db/Block.hxx>
#include <util/IndexAccessor.hxx>

namespace xmlsearch {

    namespace util {


        class DBEnvImpl
            : public xmlsearch::db::DBEnv
        {
        public:

            DBEnvImpl( const IndexAccessor& indexDir );
            virtual ~DBEnvImpl();

            virtual sal_Int32 getEntryHeaderLen() const;
            virtual sal_Int32 getBlockCount() const;
            virtual sal_Int32 getMaximumBlockCount() const;
            virtual sal_Int32 getDataLen() const;
            virtual sal_Int32 getBlockLen() const;
            virtual void read( sal_Int32 blNum,xmlsearch::db::Block*& block ) const;
            virtual void write( sal_Int32 blNum,xmlsearch::db::Block* block );


        private:

            xmlsearch::util::RandomAccessStream*   file_;
        };


    }

}
