/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <rtl/string.hxx>
#include <osl/file.hxx>
#include <db/Block.hxx>
#include <db/BtreeDictParameters.hxx>
#include <db/BtreeDict.hxx>
#include <util/RandomAccessStream.hxx>
#include <db/DBEnv.hxx>
#include <qe/QueryProcessor.hxx>

#ifdef ABIDEBUG
#include <abidebug.hxx>
#endif


using namespace xmlsearch::util;
using namespace xmlsearch::db;
using namespace xmlsearch::qe;



extern RandomAccessStream* theFile();



void print_rtl_OUString( const rtl::OUString bla )
{
    rtl::OString bluber = rtl::OString( bla.getStr(),bla.getLength(),RTL_TEXTENCODING_UTF8 );
    char* bluberChr = new char[ 1+bluber.getLength() ];
    const sal_Char* jux = bluber.getStr();

    for( int i = 0; i < bluber.getLength(); ++i )
        bluberChr[i] = jux[i];

    bluberChr[ bluber.getLength() ] = 0;
    printf( "%s\n",bluberChr );
    delete[] bluberChr;
}


extern void bla();
extern void blu();


int main( int argc,char* argv[] )
{

    QueryResults* queryResults = 0;

    try
    {
        rtl::OUString installDir = rtl::OUString::createFromAscii( "//./e|/index/" );
        QueryProcessor queryProcessor( installDir );

        std::vector<rtl::OUString> Query(2);
        Query[0] = rtl::OUString::createFromAscii( "text*" );
        Query[1] = rtl::OUString::createFromAscii( "abbildung" );
        rtl::OUString Scope = rtl::OUString::createFromAscii( "headingheading" );
        int HitCount = 40;

         QueryStatement queryStatement( HitCount,Query,Scope );
        queryResults = queryProcessor.processQuery( queryStatement );

        rtl::OUString translations[2];
        translations[0] = rtl::OUString::createFromAscii( "#HLP#" );
        translations[1] = rtl::OUString::createFromAscii( "vnd.sun.star.help://" );

        PrefixTranslator* translator =  PrefixTranslator::makePrefixTranslator( translations,2 );

        QueryHitIterator* it = queryResults->makeQueryHitIterator();
        sal_Int32 j = 0;
        while( j < 10 && it->next() )
        {
            printf( "Ergebnis %2d    ",j );
            QueryHitData* qhd = it->getHit( translator );
            printf( "Penalty = %10.4f    ",qhd->getPenalty() );
            print_rtl_OUString( qhd->getDocument() );
            ++j;
        }

        delete it;
    }
    catch( ... )
    {
        printf( "catched exception" );
        throw;
    }
    return 0;
}
