/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RandomAccessStream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:19:34 $
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
#ifndef _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#define _XMLEARCH_UTIL_RANDOMACCESSSTREAM_HXX_

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

namespace xmlsearch {

    namespace util {


        class RandomAccessStream
        {
        public:

            virtual ~RandomAccessStream() { };

            // The calle is responsible for allocating the buffer
            virtual void seek( sal_Int32 ) = 0;
            virtual sal_Int32 readBytes( sal_Int8*,sal_Int32 ) = 0;
            virtual void writeBytes( sal_Int8*, sal_Int32 ) = 0;
            virtual sal_Int32 length() = 0;
            virtual void close() = 0;


        protected:

            enum OPENFLAG { Read = OpenFlag_Read,
                            Write = OpenFlag_Write,
                            Create = OpenFlag_Create };

        };


    }
}

#endif
