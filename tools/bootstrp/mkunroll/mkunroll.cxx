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


#include <stdio.h>

#include <../../inc/tools/string.hxx>
#include <vector>

class TextFilter
{
protected:
    FILE            *pIn, *pOut;
    virtual void    Filter();
public:
                    TextFilter( rtl::OString aInFile = "stdin",
                        rtl::OString aOutFile = "stdout" );
    virtual         ~TextFilter();

    virtual void    Execute();
};

TextFilter::TextFilter( rtl::OString aInFile, rtl::OString aOutFile )
{
    if ( aInFile == "stdin" )
        pIn = stdin;
    else
        if (( pIn = fopen( aInFile.getStr(), "r" )) == NULL )
            printf( "Can't read %s\n", aInFile.getStr() );

    if ( aOutFile == "stdout" )
        pOut = stdout;
    else
        if (( pOut = fopen( aOutFile.getStr(), "w" )) == NULL )
            printf( "Can't write %s\n", aOutFile.getStr() );
}

TextFilter::~TextFilter()
{
    fclose( pOut );
    fclose( pIn );
}

void TextFilter::Execute()
{
    Filter();
}

void TextFilter::Filter()
{
    int c;
    while ( (c = fgetc( pIn )) != EOF )
        fputc( c, pOut );
}

#define LINE_LEN 2048

class MkLine;
typedef ::std::vector< MkLine* > ByteStringList;

class MkLine
{
public:
    rtl::OString aLine;
    ByteStringList*     pPrivateTnrLst;
    sal_Bool            bOut;
    sal_Bool            bHier;

                    MkLine();
};

MkLine::MkLine()
{
    bOut = sal_False;
    bHier = sal_False;
    pPrivateTnrLst = NULL;
}


class MkFilter : public TextFilter
{
    static rtl::OString aTnr;
    ByteStringList      *pLst;
    ByteStringList      *pTnrLst;
protected:
    virtual void    Filter();
public:
                    MkFilter( rtl::OString aInFile = "stdin", rtl::OString aOutFile = "stdout");
                    ~MkFilter();
};

MkFilter::MkFilter( rtl::OString aInFile, rtl::OString aOutFile ) :
    TextFilter( aInFile, aOutFile )
{
    pLst = new ByteStringList;
    pTnrLst = new ByteStringList;
}

MkFilter::~MkFilter()
{
    for ( size_t i = 0, n = pTnrLst->size(); i < n; ++i ) {
        delete (*pTnrLst)[ i ];
    }
    delete pTnrLst;
    for ( size_t i = 0, n = pLst->size(); i < n; ++i ) {
        delete (*pLst)[ i ];
    }
    delete pLst;
}

rtl::OString MkFilter::aTnr(RTL_CONSTASCII_STRINGPARAM("$(TNR)"));

void MkFilter::Filter()
{
    char aLineBuf[LINE_LEN];
    int nState = 0;

    while(( fgets(aLineBuf, LINE_LEN, pIn)) != NULL )
    {
        rtl::OString aLine(aLineBuf);
        if (aLine.indexOf(rtl::OString(RTL_CONSTASCII_STRINGPARAM("mkfilter1"))) != -1)
        {
            // surpress lines
            fprintf( stderr, "mkfilter1\n" );
            nState = 0;
        }
        else if (aLine.indexOf(rtl::OString(RTL_CONSTASCII_STRINGPARAM("unroll begin"))) != -1)
        {
            // Print lines while replacing $(TNR) with int n
            fprintf( stderr, "\nunroll begin\n" );
            nState = 1;
        }

        if ( nState == 0  )
        {
            fprintf( stderr, "." );
            MkLine *pMkLine = new MkLine();
            pMkLine->aLine = aLineBuf;
            pMkLine->bOut = sal_False;

            pLst->push_back( pMkLine );
        }
        else if ( nState == 1 )
        {
            sal_Bool bInTnrList = sal_True;
            fprintf( stderr, ":" );
            MkLine *pMkLine = new MkLine();
            if (aLine.indexOf(rtl::OString(RTL_CONSTASCII_STRINGPARAM("unroll end"))) != -1)
            {
                fprintf( stderr, ";\nunroll end\n" );
                MkLine *p_MkLine = new MkLine();
                p_MkLine->bHier = sal_True;
                p_MkLine->aLine = rtl::OString(RTL_CONSTASCII_STRINGPARAM(
                    "# do not delete this line === mkfilter3i\n"));
                p_MkLine->bOut = sal_False;
                p_MkLine->pPrivateTnrLst = pTnrLst;
                pTnrLst = new ByteStringList();
                pLst->push_back( p_MkLine );
                nState = 0;
                bInTnrList = sal_False;
            }
            pMkLine->aLine = rtl::OString(aLineBuf);
            pMkLine->bOut = sal_False;

            if ( bInTnrList )
                pTnrLst->push_back( pMkLine );
        }
        else {
            /* Skip these lines */;
        }
    }   // End Of File
    fprintf( stderr, "\n" );

    // Output file again
    size_t nLines = pLst->size();
    for ( size_t j=0; j<nLines; j++ )
    {
        MkLine *pLine = (*pLst)[ j ];
        if ( pLine->bHier )
        {
            // Iterate list n times
            for ( sal_uInt16 n=1; n<11; n++)
            {
                size_t nCount = pLine->pPrivateTnrLst->size();
                for ( size_t i=0; i<nCount; i++ )
                {
                    MkLine *pMkLine = (*pLine->pPrivateTnrLst)[ i ];
                    rtl::OString aLine = pMkLine->aLine.replaceAll(aTnr, rtl::OString::valueOf(static_cast<sal_Int32>(n)));
                    fputs( aLine.getStr(), pOut );
                    fprintf( stderr, "o" );
                }
            }
            if ( pLine->pPrivateTnrLst != NULL ) {
                for ( size_t i = 0, n = pLine->pPrivateTnrLst->size(); i < n; ++i ) {
                    delete (*pLine->pPrivateTnrLst)[ i ];
                }
                delete pLine->pPrivateTnrLst;
            }
            pLine->pPrivateTnrLst = NULL;
        }
        if ( pLine->bOut )
            fputs(pLine->aLine.getStr(), pOut );
    }
    fprintf( stderr, "\n" );
}

int main()
{
    int nRet = 0;

    TextFilter *pFlt = new MkFilter();
    pFlt->Execute();
    delete pFlt;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
