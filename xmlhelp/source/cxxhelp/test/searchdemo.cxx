/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    OString bluber = OString( bla.getStr(),bla.getLength(),RTL_TEXTENCODING_UTF8 );
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
