/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: searchdemo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:20:27 $
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
#include <rtl/string.hxx>
#include <osl/file.hxx>
#include <db/Block.hxx>
#include <db/Schema.hxx>
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


// This is the main line of programming


//    Indexccessor indexAcc( rtl::OUString::createFromAscii( "/home/ab106281/work/index" ) );
//    Schema schema( indexAcc,false );
//    BtreeDictParameters params( schema,
//                    rtl::OUString::createFromAscii( "DICTIONARY" ) );

//    blu();

//    std::vector< rtl::OUString > terms( 1 );
//    terms[0] = rtl::OUString::createFromAscii( "text" );
//    QueryStatement statement( 100,terms,rtl::OUString::createFromAscii( "" ) );
//    QueryProcessor queryProcessor( 0 );
//    QueryResults* results = queryProcessor.processQuery( statement );



  // rtl::OString bluber = rtl::OString( bla.getStr(),bla.getLength(),RTL_TEXTENCODING_UTF8 );
  // cout << bluber.getStr() << endl;



/*
  public static void main(String[] args) {
  try {
  Schema schema = new Schema( new IndexAccessor( args[0] ), false);
  BtreeDictParameters params = new BtreeDictParameters(schema, "DICTIONARY");
  BtreeDict source = new BtreeDict(params);
  source.test();
  }
  catch (Exception e) {
  System.err.println(e);
  e.printStackTrace();
  }
  }
*/
