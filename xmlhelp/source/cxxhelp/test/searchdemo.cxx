/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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



void print_rtl_OUString( const OUString bla )
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
        OUString installDir("//./e|/index/");
        QueryProcessor queryProcessor( installDir );

        std::vector<OUString> Query(2);
        Query[0] = "text*";
        Query[1] = "abbildung";
        OUString Scope = "headingheading";
        int HitCount = 40;

         QueryStatement queryStatement( HitCount,Query,Scope );
        queryResults = queryProcessor.processQuery( queryStatement );

        OUString translations[2];
        translations[0] = "#HLP#";
        translations[1] = "vnd.sun.star.help://";

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
