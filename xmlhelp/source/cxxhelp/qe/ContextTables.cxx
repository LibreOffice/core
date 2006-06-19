/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextTables.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:40:57 $
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

#ifndef _rtl_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _XMLSEARCH_QE_CONTEXTTABLES_HXX_
#include <qe/ContextTables.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_BYTEARRAYDECOMPRESSOR_HXX_
#include <util/Decompressor.hxx>
#endif

using namespace xmlsearch;
using namespace xmlsearch::qe;



Tables::Tables( ContextTables* p )
    : initialWordsCached_( new sal_Int32[ initialWordsCachedL_ = p->initialWordsL_ ] ),
      destsCached_( new sal_Int32[ destsCachedL_ = p->destsL_ ] ),
      linkTypesCached_( new sal_Int32[ linkTypesCachedL_ = p->linkTypesL_ ] ),
      seqNumbersCached_( new sal_Int32[ seqNumbersCachedL_ = p->seqNumbersL_ ] )
{
    rtl_copyMemory( (void*)initialWordsCached_,
                    (void*)p->initialWords_,
                    sizeof(sal_Int32) * p->initialWordsL_ );

    rtl_copyMemory( (void*)destsCached_,
                    (void*)p->dests_,
                    sizeof(sal_Int32) * p->destsL_ );

    rtl_copyMemory( (void*)linkTypesCached_,
                    (void*)p->linkTypes_,
                    sizeof(sal_Int32) * p->linkTypesL_ );

    rtl_copyMemory( (void*)seqNumbersCached_,
                    (void*)p->seqNumbers_,
                    sizeof(sal_Int32) * p->seqNumbersL_ );
}



Tables::~Tables()
{
    delete[] seqNumbersCached_;
    delete[] linkTypesCached_;
    delete[] destsCached_;
    delete[] initialWordsCached_;
}



void Tables::setTables( ContextTables* p )
{
    delete[] p->initialWords_;
    p->initialWordsL_          = initialWordsCachedL_;
    p->initialWords_           = initialWordsCached_;
    initialWordsCached_ = 0;

    delete[] p->dests_;
    p->destsL_                 = destsCachedL_;
    p->dests_                  = destsCached_;
    destsCached_ = 0;

    delete[] p->linkTypes_;
    p->linkTypesL_             = linkTypesCachedL_;
    p->linkTypes_              = linkTypesCached_;
    linkTypesCached_ = 0;

    delete[] p->seqNumbers_;
    p->seqNumbersL_             = seqNumbersCachedL_;
    p->seqNumbers_              = seqNumbersCached_;
    seqNumbersCached_ = 0;

    p->nTextNodes_              = initialWordsCachedL_;
}




ContextTables::ContextTables( const std::vector< sal_Int32 >& offsets,
                              sal_Int32 contextDataL,sal_Int8 *contextData,
                              sal_Int32 linkNamesL,rtl::OUString *linkNames )
    : lastDocNo_( -1 ),
      initialWordsL_( 0 ),
      destsL_( 0 ),
      linkTypesL_( 0 ),
      seqNumbersL_( 0 ),
      markersL_( 0 ),
      initialWords_( 0 ),
      dests_( 0 ),
      linkTypes_( 0 ),
      seqNumbers_( 0 ),
      markers_( 0 ),
      contextDataL_( contextDataL ),
      contextData_( contextData ),
      linkNamesL_( linkNamesL ),
      linkNames_( linkNames ),
      cache_( offsets.size() ),
      kTable_( 5 ),
      auxArray_( 4096 ),
      offsets_( offsets )
{
    for( sal_uInt32 i = 0; i < offsets_.size(); ++i )
        cache_[i] = 0;
}



ContextTables::~ContextTables()
{
    delete[] markers_;
    delete[] seqNumbers_;
    delete[] linkTypes_;
    delete[] dests_;
    delete[] initialWords_;

    for( sal_uInt32 i = 0; i < cache_.size(); ++i )
        delete cache_[i];
}



void ContextTables::setMicroindex( sal_Int32 docNo ) throw( excep::XmlSearchException )
{
    if( docNo != lastDocNo_ )
    { // check if we need to do anything
        if( cache_[ docNo ] )
            cache_[ docNo ]->setTables( this );
        else
        {
            sal_Int32 offset = offsets_[ docNo ];
            sal_Int32 k0 = contextData_[ offset ] & 0xFF;
            util::ByteArrayDecompressor compr( contextDataL_,contextData_,offset + 1 );
            kTable_.clear();
            compr.decode( k0,kTable_ );
            // decompress initialWords into auxiliary array
            auxArray_.clear();
            compr.ascDecode( kTable_[0],auxArray_ ); // _initialWords

            delete[] initialWords_;
            initialWords_ = new sal_Int32[ initialWordsL_ = auxArray_.size() ];
            sal_Int32 k;
            for( k = 0; k < initialWordsL_; ++k )    //?opt
                initialWords_[k] = auxArray_[k];

            nTextNodes_ = initialWordsL_;
            // decompress destinations into auxiliary array
            auxArray_.clear();
            compr.decode( kTable_[1],auxArray_ ); // _dests
            auxArray_.push_back( -1 );          // sentinel, root

            delete[] dests_;
            dests_ = new sal_Int32[ destsL_ = auxArray_.size() ];
            for( k = 0; k < destsL_; ++k )    //?opt
                dests_[k] = auxArray_[k];

            delete[] linkTypes_;
            linkTypes_ = new sal_Int32[ linkTypesL_ = destsL_ - nTextNodes_ - 1 ];
            compr.decode( kTable_[2],linkTypes_ );

            delete[] seqNumbers_;
            seqNumbers_ = new sal_Int32[ seqNumbersL_ = destsL_ - 1 ];
            compr.decode( kTable_[ 3 ],seqNumbers_ );

            cache_[docNo] = new Tables( this );
        }

        lastDocNo_ = docNo;
        delete[] markers_;
        markers_ = new sal_Int32[ markersL_ = destsL_ ];
    }
    initialWordsIndex_ = 0;
}



sal_Int32 ContextTables::parentContext( sal_Int32 context)
{
    return dests_[ context ];
}


rtl::OUString ContextTables::linkName( sal_Int32 context )
{
    return linkNames_[ linkTypes_[context ] ];
}


sal_Int32 ContextTables::linkCode( const rtl::OUString& linkName_ )
{
    for( sal_Int32 i = 0; i < linkNamesL_; ++i )
        if( linkName_ == linkNames_[i] )
            return i;
    return -1;          // when not found
}


bool* ContextTables::getIgnoredElementsSet( sal_Int32& len,
                                            const sal_Int32 ignoredElementsL,
                                            const rtl::OUString* ignoredElements )
{
    bool *result = 0;
    if( ignoredElements && ignoredElementsL > 0 )
    {
        for( sal_Int32 i = 0; i < ignoredElementsL; ++i )
        {
            sal_Int32 code = linkCode( ignoredElements[i] );
            if( code > -1 )
            {
                if( ! result )
                    result = new bool[ len = linkNamesL_ ];

                result[ code ] = true;
            }
        }
    }
    return result;
}



bool ContextTables::notIgnored( sal_Int32 ctx,
                                sal_Int32 ignoredElementsL,bool* ignoredElements )
{
    (void)ignoredElementsL;

    do
    {
        if( ignoredElements[ linkTypes_[ ctx ] ] )
            return false;
    }
    while( ( ctx = dests_[ ctx ] ) > -1 ); // parentContext 'hand inlined'
    return true;
}


/*
 *  starting with ctx and going up the ancestry tree look for the first
 *  context with the given linkCode
 */

sal_Int32 ContextTables::firstParentWithCode( const sal_Int32 pos,const sal_Int32 linkCode_ )
{
    sal_Int32 ctx = dests_[ wordContextLin(pos) ]; // first parent of text node
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    while( linkTypes_[ ctx - shift ] != linkCode_ )
        if( ( ctx = dests_[ ctx ] ) == limit )
            return -1;
    return ctx;
}



/*
 * starting with ctx and going up the ancestry tree look for the first
 * context with the given linkCode and given parent code
 */

sal_Int32 ContextTables::firstParentWithCode2( sal_Int32 pos,const sal_Int32 linkCode_,const sal_Int32 parentCode)
{
    sal_Int32 ctx = dests_[ wordContextLin( pos ) ]; // first parent of text node
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    for( sal_Int32 parent = dests_[ctx]; parent < limit; parent = dests_[ parent ] )
        if( linkTypes_[ parent - shift ] == parentCode && linkTypes_[ ctx - shift ] == linkCode_ )
            return ctx;
        else
            ctx = parent;
    return -1;
}


/*
 * starting with ctx and going up the ancestry tree look for the first
 * context with the given linkCode and given ancestor code
 */

sal_Int32 ContextTables::firstParentWithCode3( sal_Int32 pos,sal_Int32 linkCode_,sal_Int32 ancestorCode )
{
    sal_Int32 ctx = dests_[ wordContextLin( pos ) ];
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    // find first instance of linkCode
    while( ctx < limit && linkTypes_[ ctx - shift ] != linkCode_ )
        ctx = dests_[ ctx ];
    if( ctx < limit )       // found linkCode, check ancestry
        for( sal_Int32 ancestor = dests_[ctx];
             ancestor < limit;
             ancestor = dests_[ancestor])
            if (linkTypes_[ancestor - shift] == ancestorCode) // ancestor confirmed
                return ctx;     // match found, return successful ctx
    return -1;          // match NOT found
}


/*
 * starting with ctx and going up the ancestry tree look for the first
 * context with any of the given linkCode
 */

sal_Int32 ContextTables::firstParentWithCode4(sal_Int32 pos, sal_Int32 linkCodesL,sal_Int32* linkCodes)
{
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    for (sal_Int32 ctx = dests_[wordContextLin(pos)]; ctx < limit; ctx = dests_[ctx])
    {
        const sal_Int32 code = linkTypes_[ctx - shift];
        for (sal_Int32 i = 0; i < linkCodesL; i++)
            if (code == linkCodes[i])
                return ctx;
    }
    return -1;
}


/*
 * starting with ctx and going up the ancestry tree look for the first
 * context with the given path
 */

sal_Int32 ContextTables::firstParentWithCode5(sal_Int32 pos,sal_Int32 pathCodesL,sal_Int32* pathCodes)
{
    const sal_Int32 lastCode = pathCodes[ pathCodesL - 1 ];
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    sal_Int32 ctx = dests_[wordContextLin(pos)];

 SEARCH:
    for(sal_Int32 parent = dests_[ctx];
        parent < limit;
        parent = dests_[parent])
        if( linkTypes_[ctx - shift] == lastCode )
        { // initial match
            // try to match the entire path
            for(sal_Int32 i = pathCodesL - 2, parent2 = parent; i >= 0; i--)
                if (linkTypes_[parent2 - shift] != pathCodes[i]) // match failure
                    goto SEARCH;    // try to match higher
                else if ((parent2 = dests_[parent2]) == limit)
                    return -1;
            return ctx;
        }
        else
            ctx = parent;
    return -1;
}


/*
 * starting with ctx and going up the ancestry tree look for the first
 * context with the given linkCode
 */

sal_Int32 ContextTables::firstParentWithCode7( const sal_Int32 pos,const sal_Int32 linkCode_,const sal_Int32 seq)
{
    sal_Int32 ctx = dests_[ wordContextLin(pos) ]; // first parent of text node
    const sal_Int32 shift = nTextNodes_;
    const sal_Int32 limit = destsL_ - 1;
    while (linkTypes_[ctx - shift] != linkCode_ || seqNumbers_[ctx] != seq)
        if ((ctx = dests_[ctx]) == limit)
            return -1;
    return ctx;
}


bool ContextTables::isGoverning(sal_Int32 context)
{
    return linkName(context).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("TITLE") ) != 0;
}


void ContextTables::resetContextSearch()
{
    initialWordsIndex_ = 0;
}


sal_Int32 ContextTables::wordContextLin(sal_Int32 wordNumber)
{
    for (sal_Int32 i = initialWordsIndex_; i < nTextNodes_; ++i )
        if (initialWords_[i] > wordNumber)
        {   // first such i
            // - 1 if wordNumbers can be the same
            initialWordsIndex_ = i; // cached to speed up next search
            return i - 1;
        }
    return nTextNodes_ - 1;
}


//  void ContextTables::appendSegment( sal_Int32 context,rtl::OUStringBuffer& result )
//  {
//    result.append( context < nTextNodes_ ?
//           rtl::OUString::createFromAscii( "text()" ) :
//           linkNames_[ linkTypes_[ context - nTextNodes_ ] ] );
//    result.append(sal_Unicode( '[' ) );
//    result.append( seqNumbers_[ context ] );
//    result.append(sal_Unicode( "]/" ) );
//  }


//  /*
//   * XPath (forking) location of the hit
//   */

//  void ContextTables::hitLocation( sal_Int32 termsL,rtl::OUString* terms,
//                   sal_Int32 matchesL,sal_Int32* matches,
//                   StringBuffer& result )
//  {
//    const sal_Int32 N = termsL;
//    std::vector< sal_Int32 > stacks( N );
//    sal_Int32* wordNumbers = new sal_Int32[N];
//    std::vector< sal_Int32 > stack;
//    sal_Int32 lastInitialWordIndex = -1;
//    sal_Int32 pattern = 0,context = 0,nPopped = 0,pathStart = 0,pathEnd = 0;
//    for( sal_Int32 i = 0,marker = 1; i < N; i++,marker <<= 1 )
//      if ( terms[i] )
//        {
//      const sal_Int32 wordNumber = matches[i*2 + 1];
//      const sal_Int32 initialWordIndex = findIndexBin(wordNumber);
//      wordNumbers[i] = wordNumber - initialWords_[initialWordIndex] + 1;
//      if( initialWordIndex == lastInitialWordIndex )           // save work
//        ;                                          // do nothing, path will be reused
//      else
//        {
//          pattern |= marker;
//          stack = stacks[i] = new IntegerArray();

//          context = initialWordIndex;
//          do
//            {
//          const sal_Int32 parent = dests_[context];
//          if( parent != -1 )
//            {
//              stack.add( context );
//              markers_[context] |= marker;
//              context = parent;
//            }
//          else
//            break;
//            }
//          while( true );
//          lastInitialWordIndex = initialWordIndex;
//        }
//        }

//    // find and output common path
//    // process first non-missing match

//    sal_Int32 i = 0, marker = 1, nMissing = 0;

//    // find first non-missing matching term
//    // there has to be at least one if the hit was built
//    // count potential leading missing terms to output appropriate elements
//    // before outputting elements for matches

//    for ( ; i < N; i++,marker <<= 1 )
//      if (terms[i] != null)
//        {
//      result.append( rtl::OUString::createFromAscii( "<Matches path=\"" ) );
//      stack = stacks[i];
//      while (stack.size() > 0)
//        {
//          context = stack.popLast();
//          if ( markers_[context] == pattern )
//            {
//          markers_[context] = 0;
//          appendSegment( context,result ); // belongs to common
//          context = -1;   // used
//          ++nPopped;
//            }
//          else
//            break;
//        }
//      // end of 'matches' && common path
//      result.append("\">");
//      // output elements for any leading missingTerms
//      while (--nMissing >= 0)
//        result.append("<MissingTerm/>");

//      result.append("<Match term=\"");
//      result.append(terms[i]);
//      result.append("\" path=\"");
//      pathStart = result.getLength();
//      if (context != -1)
//        {
//          appendSegment(context, result);
//          markers_[context] = 0;
//        }
//      while (stack.size() > 0 )
//        {
//          context = stack.popLast();
//          appendSegment(context, result);
//          markers_[context] = 0;
//        }

//      pathEnd = result.length();

//      result.append("\" tokenNumber=\"");
//      result.append(wordNumbers[i]);
//      result.append("]\"/>");

//      break;          // just the first non-zero
//        }
//      else
//        ++nMissing;       // only count leading missing terms

//    // process the remaining matches
//    for (i++, marker <<= 1 ; i < N; i++, marker <<= 1)
//      if (terms[i] != null) {
//        result.append("<Match term=\"");
//        result.append(terms[i]);
//        result.append("\" path=\"");
//        stack = stacks[i];
//        if (stack == null)    // reuse previously generated path
//      result.append(result.substring(pathStart, pathEnd));
//        else {
//      stack.pop(nPopped);
//      pathStart = result.length();
//      while (stack.cardinality() > 0) {
//        context = stack.popLast();
//        appendSegment(context, result);
//        _markers[context] = 0;
//      }
//      pathEnd = result.length();
//        }
//        result.append("\" tokenNumber=\"");
//        result.append(wordNumbers[i]);
//        result.append("]\"/>");
//      }
//      else
//        result.append("<MissingTerm/>");
//    result.append("</Matches>");
//  }


//  /*
//   * QueryHitData is initialized in the caller
//   * this function fills the commonPath for all matching terms
//   * and relative paths for the individual terms
//   */

//  void ContextTables::hitLocation(String[] terms, sal_Int32[] matches, QueryHitData data) {
//      StringBuffer buffer = new StringBuffer(512);
//      const sal_Int32 N = terms.length;
//      IntegerArray[] stacks = new IntegerArray[N];
//      sal_Int32[] wordNumbers = new sal_Int32[N];
//      IntegerArray stack;
//      sal_Int32 lastInitialWordIndex = -1;
//      sal_Int32 pattern = 0, nPopped = 0, pathStart = 0, pathEnd = 0;
//      for (sal_Int32 i = 0, marker = 1; i < N; i++, marker <<= 1)
//        if (terms[i] != null) {
//      const sal_Int32 wordNumber = matches[i*2 + 1];
//      const sal_Int32 initialWordIndex = findIndexBin(wordNumber);
//      wordNumbers[i] = wordNumber - _initialWords[initialWordIndex] + 1;
//      if (initialWordIndex == lastInitialWordIndex) // save work
//        ;         // do nothing, path will be reused
//      else {
//        pattern |= marker;
//        stack = stacks[i] = new IntegerArray();
//        for (sal_Int32 ctx = initialWordIndex;;) {
//          const sal_Int32 parent = _dests[ctx];
//          if (parent != -1) {
//            stack.add(ctx);
//            _markers[ctx] |= marker;
//            ctx = parent;
//          }
//          else
//            break;
//        }
//        lastInitialWordIndex = initialWordIndex;
//      }
//        }
//      // find and output common path
//      // process first match
//      StringBuffer path = new StringBuffer(256);
//      String previousPath = null; // we may be copying subpaths from it
//      sal_Int32 i = 0, marker = 1;
//      for ( ; i < N; i++, marker <<= 1)
//        if (terms[i] != null) {
//      sal_Int32 context = 0;
//      stack = stacks[i];
//      while (stack.cardinality() > 0) {
//        context = stack.popLast();
//        if (_markers[context] == pattern) {
//          _markers[context] = 0;
//          appendSegment(context, path); // belongs to common
//          context = -1;   // used
//          ++nPopped;
//        }
//        else
//          break;
//      }
//      data.setCommonPath(path.toString());
//      // end of 'matches' && common path
//      path.setLength(0);      // will now be used for relative paths
//      pathStart = 0;
//      if (context != -1) {
//        appendSegment(context, path);
//        _markers[context] = 0;
//      }
//      while (stack.cardinality() > 0) {
//        context = stack.popLast();
//        appendSegment(context, path);
//        _markers[context] = 0;
//      }
//      pathEnd = path.length();
//      data.setMatchLocation(i, previousPath = path.toString(), wordNumbers[i]);
//      break;          // just the first non-zero
//        }

//      // process the remaining matches
//      for (i++, marker <<= 1 ; i < N; i++, marker <<= 1)
//        if (terms[i] != null) {
//      path.setLength(0);
//      stack = stacks[i];
//      if (stack == null)  // reuse previously generated path
//        path.append(previousPath.substring(pathStart, pathEnd));
//      else {
//        stack.pop(nPopped);
//        pathStart = path.length();
//        while (stack.cardinality() > 0) {
//          const sal_Int32 context = stack.popLast();
//          appendSegment(context, path);
//          _markers[context] = 0;
//        }
//        pathEnd = path.length();
//      }
//      data.setMatchLocation(i, previousPath = path.toString(), wordNumbers[i]);
//        }
//    }

//    private sal_Int32 ContextTables::findIndexBin(const sal_Int32 wordNumber) {
//      sal_Int32 i = 0, j = _nTextNodes - 1;
//      while (i <= j) {
//        const sal_Int32 k = (i + j) >>> 1;
//        if (_initialWords[k] < wordNumber)
//      i = k + 1;
//        else if (_initialWords[k] > wordNumber)
//      j = k - 1;
//        else
//      return k;
//      }
//      return i - 1;
//    }

  /*
    public void addGoverningFiller(int query, RoleFiller rf, int parent) {
    // !!! for now assume just one query
    GoverningContext gc = null;
    if (_governingContexts[parent] == null) {
    // find parent governing context
    for (int c = _dests[parent]; ; c = _dests[c])
    if (_governingContexts[c] != null || c == 0) {
    //    System.out.println("parent found at " + c);
    gc = new GoverningContext(c, rf);
    break;
    }
    }
    else
    gc = new GoverningContext(_governingContexts[parent], rf);
    _governingContexts[parent] = gc;
    }
    */








