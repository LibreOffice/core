/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IndexAccessor.hxx,v $
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
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#define _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_


#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>

namespace xmlsearch {

    namespace util {

        class RandomAccessStream;

        class IndexAccessor
        {
        public:

            enum ACCESSKIND { READ,
                              WRITE,
                              READWRITE };


            IndexAccessor( const rtl::OUString& dirName )
            {
                if( dirName.getLength() )
                {
                    if( dirName.getStr()[dirName.getLength()-1] != sal_Unicode( '/' ) )
                        dirName_ = ( dirName + rtl::OUString::createFromAscii( "/" ) ) ;
                    else
                        dirName_ = dirName;
                }
            }


            ~IndexAccessor()
            {
            }


            IndexAccessor( const IndexAccessor& accessor )
            {
                dirName_ = accessor.dirName_;
            }


            RandomAccessStream* getStream( const rtl::OUString& fileName,
                                           const rtl::OUString& how ) const;


            sal_Int32 readByteArray( sal_Int8*& data, const rtl::OUString& fileName )
                throw( xmlsearch::excep::IOException );

        private:

            rtl::OUString  dirName_;

        };

    }
}


#endif
