/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DBEnvImpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:18:57 $
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
#ifndef _XMLSEARCH_DB_DBENVIMPL_HXX_
#define _XMLSEARCH_DB_DBENVIMPL_HXX_
#endif


#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_DB_DBENV_HXX_
#include <db/DBEnv.hxx>
#endif
#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif
#ifndef _XMLSEARCH_DB_BLOCK_HXX_
#include <db/Block.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#include <util/IndexAccessor.hxx>
#endif

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
