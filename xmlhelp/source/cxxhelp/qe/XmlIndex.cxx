/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XmlIndex.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-13 14:19:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <qe/XmlIndex.hxx>
#endif
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#include <qe/DocGenerator.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_CONCEPTLIST_HXX_
#include <util/ConceptList.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_RANDOMACCESSSTREAM_HXX_
#include <util/RandomAccessStream.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#include <util/Decompressor.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif

using namespace xmlsearch;
using namespace xmlsearch::excep;
using namespace xmlsearch::qe;


// extern sal_Int32 getInteger_( const sal_Int8* );


XmlIndex::XmlIndex( const rtl::OUString& indexDir )
    throw( IOException )
    : currentBatchOffset_( 0 ),
      maxDocNumberInCache_( -1 ),
      indexAccessor_( indexDir ),
      dict_( indexAccessor_ ),
      contextTables_( 0 ),
      allListsL_( 0 ),
      allLists_( 0 ),
      positionsL_( 0 ),
      positions_( 0 ),
      contextsDataL_( 0 ),
      contextsData_( 0 ),
      concepts_( 0 ),
      documents_( 0 )
{
    // reading DOCS
    try
    {
        allListsL_ = indexAccessor_.readByteArray( allLists_,
                                                   rtl::OUString::createFromAscii("DOCS") ); // reading DOCS
    }
    catch( IOException )
    {
        OSL_ENSURE( allLists_ != 0, "XmlIndex::XmlIndex -> cannot open DOCS/docs" );
        throw;
    }

    // reading CONTEXTS
    try
    {
        contextsDataL_ = indexAccessor_.readByteArray( contextsData_,
                                                       rtl::OUString::createFromAscii("CONTEXTS") ); // reading CONTEXTS
    }
    catch( IOException )
    {
        OSL_ENSURE( allLists_ != 0, "XmlIndex::XmlIndex -> cannot open CONTEXTS/contexts" );
        delete[] allLists_;
        throw;
    }

    // reading POSITIONS
    {
        positionsFile_ = indexAccessor_.getStream( rtl::OUString::createFromAscii( "POSITIONS" ),
                                                   rtl::OUString::createFromAscii( "r" ) );

        OSL_ENSURE( positionsFile_ != 0, "XmlIndex::XmlIndex -> cannot open POSITIONS/positions" );

        if( positionsFile_ )
        {
            //!!! temporary: better than fixed large value, worse than 'intelligent' size mgt
            allInCache_ = true;
            if( allInCache_ )   // yes, intended
            {
                reset();
                positions_ = new sal_Int8[ positionsL_ = positionsFile_->length() ];
                positionsFile_->readBytes( positions_,positionsL_ );
            }
        }
        else
        {
            delete[] allLists_;
            delete[] contextsData_;
            throw IOException( rtl::OUString::createFromAscii( "XmlIndex::XmlIndex -> no POSITIONS/positions") );
        }
    }


    // reading DOCS.TAB
    {
        util::RandomAccessStream* in = indexAccessor_.getStream( rtl::OUString::createFromAscii( "DOCS.TAB" ),
                                                                 rtl::OUString::createFromAscii( "r" ) );

        if( in )
        {
            sal_Int8 a[4];
            a[0] = a[1] = a[2] = 0;
            in->readBytes( &a[3],1 );
            sal_Int32 k1 = ::getInteger_( a );
            util::StreamDecompressor sddocs( in );
            sddocs.ascDecode( k1,concepts_ );
            in->readBytes( &a[3],1 );
            sal_Int32 k2 = ::getInteger_( a );
            offsets_.push_back( 0 );
            util::StreamDecompressor sdoffsets( in );
            sdoffsets.ascDecode( k2,offsets_ );
            delete in;
        }
        else
        {
            delete[] allLists_;
            delete[] contextsData_;
            delete[] positions_;
            delete positionsFile_;
            throw IOException( rtl::OUString::createFromAscii( "XmlIndex::XmlIndex -> no DOCS.TAB/docs.tab") );
        }
    }

    // reading OFFSETS
    {
        util::RandomAccessStream* in = indexAccessor_.getStream( rtl::OUString::createFromAscii( "OFFSETS" ),
                                                                 rtl::OUString::createFromAscii( "r" ) );
        if( in )
        {
            sal_Int8 a[4];
            a[0] = a[1] = a[2] = 0;
            in->readBytes( &a[3],1 );
            sal_Int32 k1 = ::getInteger_( a );
            util::StreamDecompressor sddocs( in );
            sddocs.decode( k1,documents_ );
            in->readBytes( &a[3],1 );
            sal_Int32 k2 = ::getInteger_( a );
            util::StreamDecompressor sdoffsets( in );
            sdoffsets.ascDecode( k2,microIndexOffsets_ );
            in->readBytes( &a[3],1 );
            sal_Int32 k3 = ::getInteger_( a );
            util::StreamDecompressor sdtitles( in );
            sdtitles.decode( k3,titles_ );

            in->readBytes( &a[3],1 );
            sal_Int32 k4 = ::getInteger_( a );
            //     contextsOffsets_ = new IntegerArray(_documents.cardinality() + 1);
            util::StreamDecompressor co(in);
            //    _contextsOffsets.add(0);  // first, trivial offset
            co.ascDecode( k4,contextsOffsets_ );
            delete in;
        }
        else
        {
            delete[] allLists_;
            delete[] contextsData_;
            delete[] positions_;
            delete positionsFile_;
            throw IOException( rtl::OUString::createFromAscii( "XmlIndex::XmlIndex -> no OFFSETS/offsets") );
        }
    }

    // reading linknames
    {
        util::RandomAccessStream* in =
            indexAccessor_.getStream( rtl::OUString::createFromAscii( "LINKNAMES" ),
                                      rtl::OUString::createFromAscii( "r" ) );
        if( ! in )
        {
            delete[] allLists_;
            delete[] contextsData_;
            delete[] positions_;
            delete positionsFile_;
            throw IOException(
                rtl::OUString::createFromAscii( "BtreeDict::BtreeDict -> no LINKNAMES/linknames" ) );
        }

        sal_Int32 len = in->length();
        char* bff = new char[ 1 + len ], *bff1 = new char[ 1 + len ];
        bff[ len ] = 0;
        in->readBytes( reinterpret_cast<sal_Int8*>( bff ),len );
        delete in;

        // Now the buffer must be densified.
        int i,len1 = 0;
        for( i = 0; i < len; ++i )
        {
            if( bff[i] )
                bff1[ len1++ ] = bff[i];
        }
        bff1[len1] = 0;
        delete[] bff;
        rtl::OString aStr( bff1 );  // build a string from the densified buffer;
        delete[] bff1;

//      // Now determine the order
// #define NAMECOUNT 16
// #define UNREACHABLEPLACE 100000;
//      /**
//       *  The available names cannot be determined from LINKNAMES at current,
//       *  because LINKNAMES is a serialized Java-object
//       *  Always update LINKNAMES if index.xsl or default.xsl are modified.
//       */
//      rtl::OString LN[NAMECOUNT];
//      LN[0] = "text:span";
//      LN[1] = "help:help-text";
//      LN[2] = "help:to-be-embedded";
//      LN[3] = "headingheading";
//      LN[4] = "office:body";
//      LN[5] = "text:p";
//      LN[6] = "office:document";
//      LN[7] = "help:link";
//      LN[8] = "help:key-word";
//      LN[9] = "table:table";
//      LN[10] = "table:table-header-row";
//      LN[11] = "table:table-row";
//      LN[12] = "table:table-cell";
//      LN[13] = "text:unordered-list";
//      LN[14] = "text:ordered-list";
//      LN[15] = "text:list-item";
        // Now determine the order

#define NAMECOUNT 16
#define UNREACHABLEPLACE 100000;
        /**
         *  The available names cannot be determined from LINKNAMES at current,
         *  because LINKNAMES is a serialized Java-object
         *  Always update LINKNAMES if index.xsl or default.xsl are modified.
         */

        rtl::OString LN[NAMECOUNT];
        LN[0] = "helpdocument";
        LN[1] = "body";
        LN[2] = "title";
        LN[3] = "table";
        LN[4] = "tablecell";
        LN[5] = "tablerow";
        LN[6] = "list";
        LN[7] = "listitem";
        LN[8] = "item";
        LN[9] = "emph";
        LN[10] = "paragraph";
        LN[11] = "section";
        LN[12] = "bookmark";
        LN[13] = "bookmark_value";
        LN[14] = "ahelp";
        LN[15] = "link";

        // Determine index in file
        int idx[NAMECOUNT];
        /*int*/ linkNamesL_ = NAMECOUNT;
        for( i = 0; i < NAMECOUNT; ++i )
            if( ( idx[i] = aStr.indexOf( LN[i] ) ) == -1 ) {
                idx[i] = UNREACHABLEPLACE;
                --linkNamesL_;
            }

        linkNames_ = new rtl::OUString[linkNamesL_];
        for( i = 0; i < linkNamesL_; ++i ) {
            // TODO what happens to first if we never hit Place?
            int first = 0;
            int Place = UNREACHABLEPLACE; // This is the defintely last place
            for( int j = 0; j < NAMECOUNT; ++j )
            {
                if( idx[j] < Place )
                    Place = idx[first = j];
            }
            idx[first] = UNREACHABLEPLACE;
            linkNames_[i] = rtl::OUString( LN[first].getStr(),LN[first].getLength(),RTL_TEXTENCODING_UTF8 );
        }

#undef NAMECOUNT
#undef UNREACHABLEPLACE
    }  // end linknames


    {
        contextTables_ = new ContextTables(contextsOffsets_,
                                           contextsDataL_,contextsData_,
                                           linkNamesL_,linkNames_ );
    }
}


XmlIndex::~XmlIndex()
{
    delete[] allLists_;
    delete[] contextsData_;
    delete[] linkNames_;
    delete[] positions_;
    delete positionsFile_;
    delete contextTables_;
}



void XmlIndex::reset()
{
    maxDocNumberInCache_ = allInCache_ ? static_cast<sal_Int32>(microIndexOffsets_.size()) - 1 : -1;
}


sal_Int32 binarySearch( const std::vector<sal_Int32>& arr,sal_Int32 value )
{
    sal_Int32 i = 0, j = arr.size(), k;
    while (i <= j)
        if (arr[k = (i + j)/2] < value)
            i = k + 1;
        else if (value < arr[k])
            j = k - 1;
        else
            return k;
    return -1;
}


NonnegativeIntegerGenerator* XmlIndex::getDocumentIterator( sal_Int32 concept )
{
    sal_Int32 index = binarySearch( concepts_,concept );

    if( index >= 0 )
        return new util::ConceptList( allLists_,allListsL_,offsets_[index] );
    else
        return 0;
}


bool XmlIndex::occursInText( sal_Int32 concept )
{
    return binarySearch( concepts_,concept) >= 0;
}


sal_Int8* XmlIndex::getPositions( sal_Int32& len,sal_Int32 docNo ) throw( excep::XmlSearchException )
{
    contextTables_->setMicroindex( docNo );
    if( docNo > maxDocNumberInCache_ )
        readMicroindexes( docNo );

    len = positionsL_;
    return positions_;
}


rtl::OUString XmlIndex::documentName( sal_Int32 docNumber ) throw( excep::XmlSearchException )
{
    if( docNumber < 0 || documents_.size() <= sal_uInt32( docNumber ) )
    {
        rtl::OUString message = rtl::OUString::createFromAscii( "XmlIndex::documentName -> " );
        throw excep::XmlSearchException( message );
    }

    return dict_.fetch( documents_[ docNumber ] );
}




void XmlIndex::readMicroindexes( sal_Int32 docNo ) throw( xmlsearch::excep::IOException )
{
    currentBatchOffset_ = microIndexOffsets_[docNo];
    sal_Int32 offsetLimit = currentBatchOffset_ + positionsL_;
    sal_Int32 upTo = 0, nextDoc = docNo;
    sal_Int32 lastOffset = 0;

    do
    {
        if( ++nextDoc == sal_Int32( microIndexOffsets_.size() ) )
            lastOffset = sal_Int32( positionsFile_->length() );
        else if( microIndexOffsets_[ nextDoc ] > offsetLimit )
            lastOffset = microIndexOffsets_[ nextDoc ];
    }
    while( lastOffset == 0 );

    if( lastOffset > offsetLimit )
    {
        upTo = microIndexOffsets_[ nextDoc - 1 ];
        maxDocNumberInCache_ = nextDoc - 2;
    }
    else
    {
        upTo = lastOffset;
        maxDocNumberInCache_ = nextDoc - 1;
    }

    if( maxDocNumberInCache_ < docNo )
    { // cache too small
        // for current microindex
        // System.out.println("expanding cache to " + _positionsCacheSize);
        delete[] positions_;
        positions_ = new sal_Int8[ positionsL_ = lastOffset - currentBatchOffset_ ];
        readMicroindexes( docNo );
        return;
    }

    positionsFile_->seek( currentBatchOffset_ );
    positionsFile_->readBytes( positions_,upTo - currentBatchOffset_ );
}


QueryHitData* XmlIndex::hitToData( QueryHit* hit )
{
    sal_Int32 termsL,matchesL;
    sal_Int32 *matches = hit->getMatches( matchesL );
    rtl::OUString *terms = new rtl::OUString[ termsL = matchesL >>/*>*/ 1 ];
    for( sal_Int32 i = 0; i < termsL; ++i )
    {
        sal_Int32 aInt = ( i << 1 );
        if( 0 <= aInt && aInt < matchesL )
        {
            sal_Int32 match = matches[ aInt ];
            if( match > 0 )
                try
                {
                    terms[i] = fetch( match );
                }
                catch( const excep::XmlSearchException& )
                {
                }
        }
    }

    sal_Int32 document = hit->getDocument();
    QueryHitData *res = new QueryHitData( hit->getPenalty(),
                                          documentName( document ),
                                          termsL,terms );
    contextTables_->setMicroindex( document );
    contextTables_->resetContextSearch();
    return res;
}


