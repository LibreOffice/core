/*************************************************************************
 *
 *  $RCSfile: searchdemo.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-08 12:08:05 $
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



//  ostream& operator<<( ostream& out,const rtl::OUString& bla )
//  {
//    out << bla.getLength() << endl;
//    rtl::OString bluber = rtl::OString( bla.getStr(),bla.getLength(),RTL_TEXTENCODING_UTF8 );
//    char* bluberChr = new char[ 1+bluber.getLength() ];
//    const sal_Char* jux = bluber.getStr();

//    for( int i = 0; i < bluber.getLength(); ++i )
//      bluberChr[i] = jux[i];

//    bluberChr[ bluber.getLength() ] = 0;
//    return out << bluberChr;
//  }


extern void bla();
extern void blu();


int main( int argc,char* argv[] )
{

    QueryResults* queryResults = 0;

    try
    {
        rtl::OUString installDir = rtl::OUString::createFromAscii( "//./home/ab106281/work/index" );
        QueryProcessor queryProcessor( installDir );

        std::vector<rtl::OUString> Query(1);
        Query[0] = ( rtl::OUString::createFromAscii( "text*" ) );
        rtl::OUString Scope = rtl::OUString::createFromAscii( "" );
        int HitCount = 100;

        QueryStatement queryStatement( HitCount,Query,Scope );
        queryResults = queryProcessor.processQuery( queryStatement );

        rtl::OUString translations[2];
        translations[0] = rtl::OUString::createFromAscii( "#HLP#" );
        translations[1] = rtl::OUString::createFromAscii( "vnd.sun.star.help://" );

        PrefixTranslator* translator =  PrefixTranslator::makePrefixTranslator( translations,2 );

        QueryHitIterator* it = queryResults->makeQueryHitIterator();
        while( it->next() )
            ; // cout << it->getHit( translator )->getDocument() << endl;

        delete it;
    }
    catch( ... )
    {

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
