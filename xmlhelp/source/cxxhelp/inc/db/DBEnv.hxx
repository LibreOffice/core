/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DBEnv.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:16:15 $
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
