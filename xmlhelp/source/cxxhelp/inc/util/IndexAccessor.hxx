/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IndexAccessor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:19:23 $
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
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#define _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_


#ifndef _RTL_USTRING_HXX_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif

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
                    if( dirName.getStr()[dirName.getLength()-1] != sal_Unicode( '/' ) )
                        dirName_ = ( dirName + rtl::OUString::createFromAscii( "/" ) ) ;
                    else
                        dirName_ = dirName;
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
