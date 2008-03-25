/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConceptList.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:18:44 $
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
#ifndef _XMLSEARCH_UTIL_CONCEPTLIST_HXX_
#define _XMLSEARCH_UTIL_CONCEPTLIST_HXX_

#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#include <qe/DocGenerator.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#include <util/Decompressor.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_COMPRESSORITERATOR_HXX_
#include <util/CompressorIterator.hxx>
#endif


namespace xmlsearch {

    namespace util {


        class CompressorIterator;



        class ConceptList
            : public CompressorIterator,
              public xmlsearch::qe::NonnegativeIntegerGenerator
        {
        public:

            ConceptList( sal_Int8* array,sal_Int32 arrl,sal_Int32 index )
                : k_( array[index] ),
                  value_( 0 ),
                  list_( arrl,array,index + 1 )  { }

            void value( sal_Int32 val ) { value_ += val; }

            sal_Int32 first() throw( xmlsearch::excep::XmlSearchException )
            {
                value_ = 0;
                sal_Int32 ret = list_.readNext( k_,this ) ? value_ : END;
                return ret;
            }

            sal_Int32 next() throw( xmlsearch::excep::XmlSearchException )
            {
                return list_.readNext( k_,this) ? value_ : END;
            }

        private:

            sal_Int32             k_,value_;
            ByteArrayDecompressor list_;

        };

    }

}


#endif
