/*************************************************************************
 *
 *  $RCSfile: XmlIndex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-06 14:48:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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


using namespace xmlsearch;
using namespace xmlsearch::qe;


// extern sal_Int32 getInteger_( const sal_Int8* );


XmlIndex::XmlIndex( const rtl::OUString& indexDir )
    : indexAccessor_( indexDir ),
      dict_( indexAccessor_ ),
      documents_( 0 ),
      concepts_( 0 ),
      allLists_( 0 ),
      allListsL_( 0 ),
      positionsL_( 0 ),
      positions_( 0 ),
      contextsDataL_( 0 ),
      contextsData_( 0 ),
      contextTables_( 0 ),
      currentBatchOffset_( 0 ),
      maxDocNumberInCache_( -1 )
{
    // reading DOCS
    {
        allListsL_ = indexAccessor_.readByteArray( allLists_,
                                                   rtl::OUString::createFromAscii("DOCS") ); // reading DOCS
    }

    // reading CONTEXTS
    {
        contextsDataL_ = indexAccessor_.readByteArray( contextsData_,
                                                       rtl::OUString::createFromAscii("CONTEXTS") ); // reading CONTEXTS
    }

    // reading POSITIONS
    {
        positionsFile_ = indexAccessor_.getStream( rtl::OUString::createFromAscii( "POSITIONS" ),
                                                   rtl::OUString::createFromAscii( "r" ) );

        //!!! temporary: better than fixed large value, worse than 'intelligent' size mgt
        if( allInCache_ = true )   // yes, intended
        {
            reset();
            positions_ = new sal_Int8[ positionsL_ = positionsFile_->length() ];
            positionsFile_->readBytes( positions_,positionsL_ );
        }
    }


    // reading DOCS.TAB
    {
        util::RandomAccessStream* in = indexAccessor_.getStream( rtl::OUString::createFromAscii( "DOCS.TAB" ),
                                                                 rtl::OUString::createFromAscii( "r" ) );
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

    // reading OFFSETS
    {
        util::RandomAccessStream* in = indexAccessor_.getStream( rtl::OUString::createFromAscii( "OFFSETS" ),
                                                                 rtl::OUString::createFromAscii( "r" ) );
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

    // Hard coding linknames ( object serialization is hard to undo )
    {
        linkNames_ = new rtl::OUString[ linkNamesL_ = 8 ];
        linkNames_[0] = rtl::OUString::createFromAscii( "help:link" );
        linkNames_[1] = rtl::OUString::createFromAscii( "help:help-text" );
        linkNames_[2] = rtl::OUString::createFromAscii( "text:p" );
        linkNames_[3] = rtl::OUString::createFromAscii( "text:span" );
        linkNames_[4] = rtl::OUString::createFromAscii( "headingheading" );
        linkNames_[5] = rtl::OUString::createFromAscii( "office:body" );
        linkNames_[6] = rtl::OUString::createFromAscii( "help:to-be-embedded" );
        linkNames_[7] = rtl::OUString::createFromAscii( "office:document" );
    }


    {
        contextTables_ = new ContextTables( contextsOffsets_,
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
    maxDocNumberInCache_ = ( allInCache_ ? ( microIndexOffsets_.size() - 1 ) : sal_Int32( -1 ) );
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


#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif

QueryHitData* XmlIndex::hitToData( QueryHit* hit )
{
    sal_Int32 termsL,matchesL;
    sal_Int32 *matches = hit->getMatches( matchesL );
    rtl::OUString *terms = new rtl::OUString[ termsL = matchesL >>/*>*/ 1 ];
    for( sal_Int32 i = 0; i < termsL; ++i )
        if( matches[ i << 1 ] > 0 )
            terms[i] = fetch( matches[i << 1] );

    sal_Int32 document = hit->getDocument();
    QueryHitData *res = new QueryHitData( hit->getPenalty(),
                                          documentName( document ),
                                          termsL,terms );
    contextTables_->setMicroindex( document );
    contextTables_->resetContextSearch();
    return res;
}


