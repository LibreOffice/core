/*************************************************************************
 *
 *  $RCSfile: gi_parse.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2001-06-12 14:38:15 $
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

#include <gi_parse.hxx>

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <gilacces.hxx>


using namespace std;


const char * C_sLineEnd = "\r\n";



inline void
WriteStr( ostream & o_rOut, const Simstr & i_rStr )
{
    o_rOut.write( i_rStr.str(), i_rStr.l() );
}

inline void
WriteStr( ostream & o_rOut, const char * i_rStr )
{
    o_rOut.write( i_rStr, strlen(i_rStr) );
}

inline void
GenericInfo_Parser::SetError( E_Error i_eError )
{
     eErrorCode = i_eError;
    nErrorLine = nCurLine;
}


GenericInfo_Parser::GenericInfo_Parser()
    :   sCurParsePosition(""),
        nCurLine(0),
        nLevel(0),
        bGoon(false),
        // sCurComment,
        eErrorCode(ok),
        nErrorLine(0),
        pResult(0),
        pResource(0)
{
}

GenericInfo_Parser::~GenericInfo_Parser()
{
}

bool
GenericInfo_Parser::LoadList( GenericInfoList_Builder &  o_rResult,
                              const Simstr &             i_sSourceFileName )
{
    ifstream  aFile( i_sSourceFileName.str() );
    if ( aFile.fail() )
    {
        SetError(cannot_open);
        return false;
    }

    ResetState(o_rResult);

    for ( ReadLine(aFile); bGoon; ReadLine(aFile) )
    {
        bool bOk = InterpretLine();
        if ( !bOk)
        {
            SetError(syntax_error);
            break;
        }
    }

    if ( nLevel > 0 && eErrorCode == ok)
    {
        SetError(unexpected_eof);
    }
    else if ( nLevel < 0 )
    {
        SetError(unexpected_list_end);
    }

    aFile.close();
    return eErrorCode == ok;
}

bool
GenericInfo_Parser::SaveList( const Simstr &            i_rOutputFile,
                              GenericInfoList_Browser & io_rListBrowser )
{
    ofstream  aFile( i_rOutputFile.str() );
    if ( aFile.fail() )
    {
        SetError(cannot_open);
        return false;
    }

    ResetState(io_rListBrowser);

    WriteList(aFile);

    aFile.close();
    return eErrorCode == ok;
}

void
GenericInfo_Parser::ResetState( GenericInfoList_Builder & io_rResult )
{
    sCurParsePosition = "";
    nCurLine = 0;
    nLevel = 0;
    bGoon = true;
    sCurComment = "";
    eErrorCode = ok;
    nErrorLine = 0;
    pResult = &io_rResult;
    pResource = 0;
}

void
GenericInfo_Parser::ResetState( GenericInfoList_Browser & io_rSrc )
{
    sCurParsePosition = "";
    nCurLine = 0;
    nLevel = 0;
    bGoon = false;
    sCurComment = "";
    eErrorCode = ok;
    nErrorLine = 0;
    pResult = 0;
    pResource = &io_rSrc;
}


void
GenericInfo_Parser::ReadLine( istream & i_rSrc )
{
    const int nInputSize = 32000;
    static char sInput[nInputSize];

    i_rSrc.get(sInput, nInputSize);
    UINT32 nGot = UINT32(i_rSrc.gcount());
    if (nGot == 0 && i_rSrc.eof())
    {
        bGoon = false;
        return;
    }

    nCurLine++;

#if 0
    if ( sInput[ nGot-1 ] == '\r' )
        sInput[ nGot-1 ] = '\0';
#endif
    i_rSrc.get();

    for ( sCurParsePosition = &sInput[0]; *sCurParsePosition > 0 && *sCurParsePosition <= 32; ++sCurParsePosition );
    for ( char * sEnd = const_cast< char* >(strchr(sCurParsePosition,'\0'));
          sEnd != sCurParsePosition
                ? *(sEnd-1) <= 32
                : false;
          --sEnd )
    {
        *(sEnd-1) = '\0';
    }
}

bool
GenericInfo_Parser::InterpretLine()
{
    switch ( ClassifyLine() )
    {
        case lt_key:        ReadKey();
                            break;
        case lt_open_list:  PushLevel_Read();
                            break;
        case lt_close_list: PopLevel_Read();
                            break;
        case lt_comment:    AddCurLine2CurComment();
                            break;
        case lt_empty:      AddCurLine2CurComment();
                            break;
        default:
                            return false;
    }
    return true;
}

GenericInfo_Parser::E_LineType
GenericInfo_Parser::ClassifyLine()
{
    switch ( *sCurParsePosition )
    {
         case '{':   return lt_open_list;
        case '}':   return lt_close_list;
        case '#':   return lt_comment;
        case '\0':   return lt_empty;
    }

    return lt_key;
}

void
GenericInfo_Parser::ReadKey()
{
    const char * pSearch = sCurParsePosition;

    for ( ; *pSearch > 32; ++pSearch );
    UINT32 nKeyLength = pSearch - sCurParsePosition;

    for ( ; *pSearch <= 32 && *pSearch > '\0'; ++pSearch );

    pResult->AddKey( sCurParsePosition,     nKeyLength,
                     pSearch,               strlen(pSearch),
                     sCurComment.str(),     sCurComment.l()
                   );
    sCurComment = "";
}

void
GenericInfo_Parser::PushLevel_Read()
{
    nLevel++;
    pResult->OpenList();
}

void
GenericInfo_Parser::PopLevel_Read()
{
    nLevel--;
    pResult->CloseList();
}

void
GenericInfo_Parser::AddCurLine2CurComment()
{
    sCurComment +=  sCurParsePosition;
    sCurComment +=  C_sLineEnd;
}

void
GenericInfo_Parser::WriteList( ostream & o_rFile )
{
    static char sBuffer[32000];

    for ( bGoon = pResource->Start_CurList();
          bGoon;
          bGoon = pResource->NextOf_CurList() )
    {
        pResource->Get_CurComment(&sBuffer[0]);
        WriteComment(o_rFile,sBuffer);

        pResource->Get_CurKey(&sBuffer[0]);
        WriteKey(o_rFile,sBuffer);

        pResource->Get_CurValue(&sBuffer[0]);
        WriteValue(o_rFile,sBuffer);

        if ( pResource->HasSubList_CurKey() )
        {
            PushLevel_Write();

/*
            WriteIndentation();
            o_rFile.write("{",1);
            o_rFile.write(C_sLineEnd, C_nLineEndLength);
*/
            WriteList(o_rFile);

/*
            WriteIndentation();
            o_rFile.write("}",1);
            o_rFile.write(C_sLineEnd, C_nLineEndLength);
*/
            PopLevel_Write();
        }
    }   // end for
}

void
GenericInfo_Parser::PushLevel_Write()
{
    nLevel++;
    pResource->Push_CurList();
}

void
GenericInfo_Parser::PopLevel_Write()
{
    nLevel--;
    pResource->Pop_CurList();
}

void
GenericInfo_Parser::WriteComment( ostream &    o_rFile,
                                  const char *  i_sStr )
{
    WriteStr( o_rFile, i_sStr );
    if ( i_sStr[ strlen(i_sStr)-1 ] != '\n' )
        WriteStr( o_rFile, C_sLineEnd );
}

void
GenericInfo_Parser::WriteKey( ostream &    o_rFile,
                              const char *  i_sStr )
{
    WriteIndentation(o_rFile);
    WriteStr( o_rFile, i_sStr );
}

void
GenericInfo_Parser::WriteValue( ostream &    o_rFile,
                                const char *  i_sStr )
{
    if ( i_sStr != 0 ? strlen(i_sStr) > 0 : false )
    {
         WriteStr(o_rFile," ");
         WriteStr(o_rFile,i_sStr);
    }

    WriteStr(o_rFile,C_sLineEnd);
}

void
GenericInfo_Parser::WriteIndentation( ostream &    o_rFile )
{
    const int nIndentBound = 60;

    static const char sIndentation[nIndentBound+1] =
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

    if ( nLevel == 0 )
        return;

    if ( nLevel <= nIndentBound )
        o_rFile.write( sIndentation, nLevel );
    else
    {
        INT16 iLevel = nLevel;
        for ( ; iLevel > nIndentBound; iLevel-=nIndentBound )
            o_rFile.write( sIndentation, nIndentBound );
        o_rFile.write( sIndentation, iLevel );
    }
}



