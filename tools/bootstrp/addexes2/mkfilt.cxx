/*************************************************************************
 *
 *  $RCSfile: mkfilt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:01 $
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

#include <stdio.h>

#include <tools/string.hxx>
#include <tools/list.hxx>

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
    char c;
    while ( (c = fgetc( pIn )) != EOF )
        fputc( c, pOut );
}

#define LINE_LEN 2048

class ByteStringList;

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

DECLARE_LIST( ByteStringList, MkLine * );

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
    delete pTnrLst;
    delete pLst;
}

ByteString MkFilter::aTnr="$(TNR)";

void MkFilter::Filter()
{
    char aLineBuf[LINE_LEN];
    int nState = 0;
    BOOL bNew = TRUE;

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

            pLst->Insert( pMkLine, LIST_APPEND );
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
                pLst->Insert( p_MkLine, LIST_APPEND );
                nState = 0;
                bInTnrList = FALSE;
            }
            ByteString *pStr = new ByteString( aLineBuf );
            pMkLine->aLine = *pStr;
            pMkLine->bOut = FALSE;

            if ( bInTnrList )
                pTnrLst->Insert( pMkLine, LIST_APPEND );
        }
        else
            /* Zeilen ignorieren */;
    }   // End Of File
    fprintf( stderr, "\n" );

    // das File wieder ausgegeben
    ULONG nLines = pLst->Count();
    for ( ULONG j=0; j<nLines; j++ )
    {
        MkLine *pLine = pLst->GetObject( j );
        if ( pLine->bHier )
        {
            // die List n - Mal abarbeiten
            for ( USHORT n=1; n<11; n++)
            {
                ULONG nCount = pLine->pPrivateTnrLst->Count();
                for ( ULONG i=0; i<nCount; i++ )
                {
                    MkLine *pMkLine = pLine->pPrivateTnrLst->GetObject(i);
                    ByteString aLine = pMkLine->aLine;
                    if ( pMkLine->bOut );
                    {
                        while( aLine.SearchAndReplace( aTnr, ByteString::CreateFromInt32( n )) != (USHORT)-1 );
                        fputs( aLine.GetBuffer(), pOut );
                        fprintf( stderr, "o" );
                    }
                }
            }
            if ( pLine->pPrivateTnrLst != NULL )
                delete pLine->pPrivateTnrLst;
            pLine->pPrivateTnrLst = NULL;
        }
        if ( pLine->bOut )
                fputs(pLine->aLine.GetBuffer(), pOut );
    }
    fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
    int nRet = 0;

    TextFilter *pFlt = new MkFilter();
    pFlt->Execute();
    delete pFlt;

    return nRet;
}
