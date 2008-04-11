/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DBEnv.hxx,v $
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
#ifndef _XMLSEARCH_DB_DBENV_HXX_
#define _XMLSEARCH_DB_DBENV_HXX_

namespace xmlsearch {

    namespace db {


        class Block;


        // environment class for a DBEnv

        class DBEnv
        {
        public:

            virtual sal_Int32 getEntryHeaderLen() const = 0;

            virtual sal_Int32 getBlockCount() const = 0;

            virtual sal_Int32 getMaximumBlockCount() const = 0;

            virtual sal_Int32 getDataLen() const = 0;

            virtual sal_Int32 getBlockLen() const = 0;

            virtual void read( sal_Int32 block,Block*& ) const = 0;

            virtual void write( sal_Int32,Block* ) = 0;
        };


    }

}


#endif
