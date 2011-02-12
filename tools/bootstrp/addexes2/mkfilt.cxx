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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdio.h>

#include <../../inc/tools/string.hxx>
#include <vector>

class TextFilter
{
protected:
    FILE            *pIn, *pOut;
    virtual void    Filter();
public:
                    TextFilter( ByteString aInFile = "stdin",
                        ByteString aOutFile = "stdout" );
    virtual         ~TextFilter();

    virtual void    Execute();
};

TextFilter::TextFilter( ByteString aInFile, ByteString aOutFile )
{
    if ( aInFile == "stdin" )
        pIn = stdin;
    else
        if (( pIn = fopen( aInFile.GetBuffer(), "r" )) == NULL )
            printf( "Can't read %s\n", aInFile.GetBuffer() );

    if ( aOutFile == "stdout" )
        pOut = stdout;
    else
        if (( pOut = fopen( aOutFile.GetBuffer(), "w" )) == NULL )
            printf( "Can't write %s\n", aOutFile.GetBuffer() );
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
    ByteString          aLine;
    ByteStringList*     pPrivateTnrLst;
    BOOL            bOut;
    BOOL            bHier;

                    MkLine();
};

MkLine::MkLine()
{
    bOut = FALSE;
    bHier = FALSE;
    pPrivateTnrLst = NULL;
}


class MkFilter : public TextFilter
{
    static ByteString   aTnr;
    ByteStringList      *pLst;
    ByteStringList      *pTnrLst;
protected:
    virtual void    Filter();
public:
                    MkFilter( ByteString aInFile = "stdin", ByteString aOutFile = "stdout");
                    ~MkFilter();
};

MkFilter::MkFilter( ByteString aInFile, ByteString aOutFile ) :
    TextFilter( aInFile, aOutFile )
{
    pLst = new ByteStringList;
    pTnrLst = new ByteStringList;
}

MkFilter::~MkFilter()
{
    for ( size_t i = 0, n = pLst->size(); i < n; ++i ) {
        delete (*pTnrLst)[ i ];
    }
    delete pTnrLst;
    for ( size_t i = 0, n = pLst->size(); i < n; ++i ) {
        delete (*pLst)[ i ];
    }
    delete pLst;
}

ByteString MkFilter::aTnr="$(TNR)";

void MkFilter::Filter()
{
    char aLineBuf[LINE_LEN];
    int nState = 0;

    while(( fgets(aLineBuf, LINE_LEN, pIn)) != NULL )
    {
        ByteString aLine( aLineBuf );
        //fprintf(stderr, "aLine :%s\n", aLine.GetBuffer());
        if ( aLine.Search("mkfilter1" ) != STRING_NOTFOUND )
        {
            // Zeilen unterdruecken
            fprintf( stderr, "mkfilter1\n" );
            nState = 0;
        }
        else if ( aLine.Search("unroll begin" ) != STRING_NOTFOUND )
        {
            // Zeilen raus schreiben mit ersetzen von $(TNR) nach int n
            fprintf( stderr, "\nunroll begin\n" );
            nState = 1;
        }
        ;

        if ( nState == 0  )
        {
            fprintf( stderr, "." );
            MkLine *pMkLine = new MkLine();
            ByteString *pStr = new ByteString( aLineBuf );
            pMkLine->aLine = *pStr;
            pMkLine->bOut = FALSE;

            pLst->push_back( pMkLine );
        }
        else if ( nState == 1 )
        {
            BOOL bInTnrList = TRUE;
            fprintf( stderr, ":" );
            MkLine *pMkLine = new MkLine();
            if ( aLine.Search("unroll end") != STRING_NOTFOUND )
            {
                fprintf( stderr, ";\nunroll end\n" );
                MkLine *p_MkLine = new MkLine();
                p_MkLine->bHier = TRUE;
                ByteString *pByteString = new ByteString("# do not delete this line === mkfilter3i\n");
                p_MkLine->aLine = *pByteString;
                p_MkLine->bOut = FALSE;
                p_MkLine->pPrivateTnrLst = pTnrLst;
                pTnrLst = new ByteStringList();
                pLst->push_back( p_MkLine );
                nState = 0;
                bInTnrList = FALSE;
            }
            ByteString *pStr = new ByteString( aLineBuf );
            pMkLine->aLine = *pStr;
            pMkLine->bOut = FALSE;

            if ( bInTnrList )
                pTnrLst->push_back( pMkLine );
        }
        else {
            /* Zeilen ignorieren */;
        }
    }   // End Of File
    fprintf( stderr, "\n" );

    // das File wieder ausgegeben
    size_t nLines = pLst->size();
    for ( size_t j=0; j<nLines; j++ )
    {
        MkLine *pLine = (*pLst)[ j ];
        if ( pLine->bHier )
        {
            // die List n - Mal abarbeiten
            for ( USHORT n=1; n<11; n++)
            {
                size_t nCount = pLine->pPrivateTnrLst->size();
                for ( size_t i=0; i<nCount; i++ )
                {
                    MkLine *pMkLine = (*pLine->pPrivateTnrLst)[ i ];
                    ByteString aLine = pMkLine->aLine;
                    while( aLine.SearchAndReplace( aTnr, ByteString::CreateFromInt32( n )) != (USHORT)-1 ) ;
                    fputs( aLine.GetBuffer(), pOut );
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
                fputs(pLine->aLine.GetBuffer(), pOut );
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
