/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XmlIndex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:18:21 $
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
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#define _XMLSEARCH_QE_XMLINDEX_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_USTRING_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLSEARCH_QE_CONTEXTTABLES_HXX_
#include <qe/ContextTables.hxx>
#endif
#ifndef _XMLSEARCH_DB_BTREEDICT_HXX_
#include <db/BtreeDict.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_INDEXACCESSOR_HXX_
#include <util/IndexAccessor.hxx>
#endif


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

