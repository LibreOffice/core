/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XmlIndex.hxx,v $
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
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#define _XMLSEARCH_QE_XMLINDEX_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>
#include <qe/ContextTables.hxx>
#include <db/BtreeDict.hxx>
#include <util/IndexAccessor.hxx>


namespace xmlsearch {

    namespace qe {


        class QueryHit;
        class QueryHitData;
        class NonnegativeIntegerGenerator;


        class XmlIndex
        {
        public:

            XmlIndex( const rtl::OUString& indexDir )
                throw( xmlsearch::excep::IOException );

            ~XmlIndex();

            ContextTables* getContextInfo() { return contextTables_; }


            sal_Int32 fetch( const rtl::OUString& key ) const throw( xmlsearch::excep::XmlSearchException )
            {
                return dict_.fetch( key );
            }

            rtl::OUString fetch( sal_Int32 key ) const throw( xmlsearch::excep::XmlSearchException )
            {
                return dict_.fetch( key );
            }

            std::vector< sal_Int32 > withPrefix( const rtl::OUString& prefix ) const throw( xmlsearch::excep::XmlSearchException )
            {
                return dict_.withPrefix( prefix );
            }

            bool occursInText( sal_Int32 concept );

            void reset();

            void resetContextSearch() { contextTables_->resetContextSearch(); }

            NonnegativeIntegerGenerator* getDocumentIterator( sal_Int32 concept );

            rtl::OUString documentName( sal_Int32 docNumber ) throw( xmlsearch::excep::XmlSearchException );

            QueryHitData* hitToData( QueryHit* );

            sal_Int8* getPositions( sal_Int32&,sal_Int32 docNo ) throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 getDocumentIndex( sal_Int32 docNo ) { return microIndexOffsets_[ docNo ] - currentBatchOffset_; }


        private:

            sal_Int32 currentBatchOffset_,maxDocNumberInCache_;

            xmlsearch::util::IndexAccessor indexAccessor_;
            xmlsearch::util::RandomAccessStream* positionsFile_;
            xmlsearch::db::BtreeDict  dict_;

            ContextTables* contextTables_;

            sal_Int32 allListsL_;
            sal_Int8* allLists_;

            bool allInCache_;
            sal_Int32 positionsL_;
            sal_Int8  *positions_;

            sal_Int32 contextsDataL_;
            sal_Int8 *contextsData_;

            sal_Int32 linkNamesL_;
            rtl::OUString* linkNames_;

            std::vector< sal_Int32 >  concepts_,offsets_,documents_,microIndexOffsets_,titles_,contextsOffsets_;

            void readMicroindexes( sal_Int32 docNo ) throw( xmlsearch::excep::IOException );
        };


    }

}



#endif

