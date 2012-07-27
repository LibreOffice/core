/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2006, ScalingWeb.com
 * All rights reserved.
 *
 * Redistribution and use of this software in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other
 *   materials provided with the distribution.
 *
 * * Neither the name of ScalingWeb.com nor the names of its
 *   contributors may be used to endorse or promote products
 *   derived from this software without specific prior
 *   written permission of ScalingWeb.com.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MorkParser.hxx"
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <stdexcept>
#include <fstream>
#include <iostream>

std::string g_Empty = "";

// Mork header of supported format version
const char *MorkMagicHeader = "// <!-- <mdb:mork:z v=\"1.4\"/> -->";

const char *MorkDictColumnMeta = "<(a=c)>";


MorkParser::MorkParser( int DefaultScope ) :
    columns_(),
    values_(),
    mork_(),
    currentCells_(0),
    error_(NoError),
    morkData_(),
    morkPos_(0),
    nextAddValueId_(0x7fffffff),
    defaultScope_(DefaultScope),
    nowParsing_(NPValues)
{
}

bool MorkParser::open( const std::string &path )
{
    initVars();
    std::string line;
    std::ifstream infile(path.c_str(), std::ios_base::in);
    if(!infile.is_open())
    {
        error_ = FailedToOpen;
        return false;
    }

    while (getline(infile, line, '\n'))
    {
        morkData_.append(line);
        morkData_.append("\n");
    }

    // Parse mork
    return parse();
}

inline MorkErrors MorkParser::error()
{
    return error_;
}

void MorkParser::initVars()
{
    error_ = NoError;
    morkPos_ = 0;
    nowParsing_ = NPValues;
    currentCells_ = 0;
    nextAddValueId_ = 0x7fffffff;
}

bool MorkParser::parse()
{
    bool Result = true;
    char cur = 0;

    // Run over mork chars and parse each term
    cur = nextChar();

    int i = 0;

    while ( Result && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            i++;
            // Figure out what a term
            switch ( cur )
            {
            case '<':
                // Dict
                Result = parseDict();
                break;
            case '/':
                // Comment
                Result = parseComment();
                break;
            case '{':
                Result = parseTable();
                // Table
                break;
            case '[':
                Result = parseRow( 0, 0 );
                // Row
                break;
            case '@':
                Result = parseGroup();
                // Group
                break;
            default:
                error_ = DefectedFormat;
                Result = false;
                break;
            }
        }

        // Get next char
        cur = nextChar();
    }

    return Result;
}

bool MorkParser::isWhiteSpace( char c )
{
    switch ( c )
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '\f':
        return true;
    default:
        return false;
    }
}

inline char MorkParser::nextChar()
{
    char cur = 0;


    if ( morkPos_ < morkData_.length() )
    {
        cur = morkData_[ morkPos_ ];
        morkPos_++;
    }

    if ( !cur )
    {
        cur = 0;
    }

    return cur;
}

bool MorkParser::parseDict()
{
    char cur = nextChar();
    bool Result = true;
    nowParsing_ = NPValues;

    while ( Result && cur != '>' && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            switch ( cur )
            {
            case '<':
            {

                if ( morkData_.substr( morkPos_ - 1, strlen( MorkDictColumnMeta ) ) == MorkDictColumnMeta )
                {
                    nowParsing_ = NPColumns;
                    morkPos_ += strlen( MorkDictColumnMeta ) - 1;
                }


                break;
            }
            case '(':
                Result = parseCell();
                break;
            case '/':
                Result = parseComment();
                break;

            }
        }

        cur = nextChar();
    }

    return Result;
}

inline bool MorkParser::parseComment()
{
    char cur = nextChar();
    if ( '/' != cur ) return false;

    while ( cur != '\r' && cur != '\n' && cur )
    {
        cur = nextChar();
    }

    return true;
}

bool MorkParser::parseCell()
{
    bool Result = true;
    bool bValueOid = false;
    bool bColumn = true;
    int Corners = 0;

    // Column = Value
    std::string Column;
    std::string Text;
    Column.reserve( 4 );
    Text.reserve( 32 );

    char cur = nextChar();

    // Process cell start with column (bColumn == true)
    while ( Result && cur != ')' && cur )
    {
        switch ( cur )
        {
        case '^':
            // Oids
            Corners++;
            if ( 1 == Corners )
            {
            }
            else if ( 2 == Corners )
            {
                bColumn = false;
                bValueOid = true;
            }
            else
            {
                Text += cur;
            }

            break;
        case '=':
            // From column to value
            if ( bColumn )
            {
                bColumn = false;
            }
            else
            {
                Text += cur;
            }
            break;
        case '\\':
        {
            // Get next two chars
            char NextChar= nextChar();
            if ( '\r' != NextChar && '\n' != NextChar )
            {
                Text += NextChar;
            }
            else nextChar();
        }
        break;
        case '$':
        {
            // Get next two chars
            std::string HexChar;
            HexChar += nextChar();
            HexChar += nextChar();
            Text += (char)strtoul(HexChar.c_str(), 0, 16);
        }
        break;
        default:
            // Just a char
            if ( bColumn )
            {
                Column += cur;
            }
            else
            {
                Text += cur;
            }
            break;
        }

        cur = nextChar();
    }

    // Apply column and text
    int ColumnId = strtoul(Column.c_str(), 0, 16);

    if ( NPRows != nowParsing_ )
    {
        // Dicts
        if ( "" != Text )
        {
            if ( nowParsing_ == NPColumns )
            {
                columns_[ ColumnId ] = Text;
            }
            else
            {
                values_[ ColumnId ] = Text;
            }
        }
    }
    else
    {
        if ( "" != Text )
        {
            // Rows
            //int ValueId = string( Text.c_str() ).toInt( 0, 16 );
            int ValueId = strtoul(Text.c_str(), 0, 16);

            if ( bValueOid  )
            {
                ( *currentCells_ )[ ColumnId ] = ValueId;
            }
            else
            {
                nextAddValueId_--;
                values_[ nextAddValueId_ ] = Text;
                ( *currentCells_ )[ ColumnId ] = nextAddValueId_;
            }
        }
    }

    return Result;
}

bool MorkParser::parseTable()
{
    bool Result = true;
    std::string TextId;
    int Id = 0, Scope = 0;

    char cur = nextChar();

    // Get id
    while ( cur != '{' && cur != '[' && cur != '}' && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            TextId += cur;
        }

        cur = nextChar();
    }

    parseScopeId( TextId, &Id, &Scope );

    // Parse the table
    while ( Result && cur != '}' && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            switch ( cur )
            {
            case '{':
                Result = parseMeta( '}' );
                break;
            case '[':
                Result = parseRow( Id, Scope );
                break;
            case '-':
            case '+':
                break;
            default:
            {
                std::string JustId;
                while ( !isWhiteSpace( cur ) && cur )
                {
                    JustId += cur;
                    cur = nextChar();

                    if ( cur == '}' )
                    {
                        return Result;
                    }
                }

                int JustIdNum = 0, JustScopeNum = 0;
                parseScopeId( JustId, &JustIdNum, &JustScopeNum );

                setCurrentRow( Scope, Id, JustScopeNum, JustIdNum );
            }
            break;
            }
        }

        cur = nextChar();
    }

    return Result;
}

void MorkParser::parseScopeId( const std::string &TextId, int *Id, int *Scope )
{
    int Pos = 0;

    if ( ( Pos = TextId.find( ':' ) ) >= 0 )
    {
        std::string tId = TextId.substr( 0, Pos );
        std::string tSc = TextId.substr( Pos + 1, TextId.length() - Pos );

        if ( tSc.length() > 1 && '^' == tSc[ 0 ] )
        {
            // Delete '^'
            tSc.erase( 0, 1 );
        }

        *Id = strtoul(tId.c_str(), 0, 16);

        *Scope = strtoul(tSc.c_str(), 0, 16);
    }
    else
    {
        *Id = strtoul(TextId.c_str(), 0, 16);
    }
}

inline void MorkParser::setCurrentRow( int TableScope, int TableId, int RowScope, int RowId )
{
    if ( !RowScope )
    {
        RowScope = defaultScope_;
    }

    if ( !TableScope )
    {
        TableScope = defaultScope_;
    }

    currentCells_ = &( mork_[ abs( TableScope ) ][ abs( TableId ) ][ abs( RowScope ) ][ abs( RowId ) ] );
}

bool MorkParser::parseRow( int TableId, int TableScope )
{
    bool Result = true;
    std::string TextId;
    int Id = 0, Scope = 0;
    nowParsing_ = NPRows;

    char cur = nextChar();

    // Get id
    while ( cur != '(' && cur != ']' && cur != '[' && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            TextId += cur;
        }

        cur = nextChar();
    }

    parseScopeId( TextId, &Id, &Scope );
    setCurrentRow( TableScope, TableId, Scope, Id );

    // Parse the row
    while ( Result && cur != ']' && cur )
    {
        if ( !isWhiteSpace( cur ) )
        {
            switch ( cur )
            {
            case '(':
                Result = parseCell();
                break;
            case '[':
                Result = parseMeta( ']' );
                break;
            default:
                Result = false;
                break;
            }
        }

        cur = nextChar();
    }

    return Result;
}

bool MorkParser::parseGroup()
{
    return parseMeta( '@' );
}

bool MorkParser::parseMeta( char c )
{
    char cur = nextChar();

    while ( cur != c && cur )
    {
        cur = nextChar();
    }

    return true;
}

MorkTableMap *MorkParser::getTables( int TableScope )
{
    TableScopeMap::iterator iter;
    iter = mork_.find( TableScope );

    if ( iter == mork_.end() )
    {
        return 0;
    }

    return &iter->second;
}

MorkRowMap *MorkParser::getRows( int RowScope, RowScopeMap *table )
{
    RowScopeMap::iterator iter;
    iter = table->find( RowScope );

    if ( iter == table->end() )
    {
        return 0;
    }

    return &iter->second;
}

std::string &MorkParser::getValue( int oid )
{
    MorkDict::iterator foundIter = values_.find( oid );

    if ( values_.end() == foundIter )
    {
        return g_Empty;
    }

    return foundIter->second;
}

std::string &MorkParser::getColumn( int oid )
{
    MorkDict::iterator foundIter = columns_.find( oid );

    if ( columns_.end() == foundIter )
    {
        return g_Empty;
    }

    return foundIter->second;
}

void MorkParser::dumpColumns()
{
    for ( MorkDict::iterator cellsIter = columns_.begin();
          cellsIter != columns_.end(); cellsIter++ )
    {
        char buffer[20];
        sprintf( buffer, "%d", cellsIter->first );
        std::string value = cellsIter->second;
        //SAL_INFO("connectivity.mork", "dumpColumns: " << buffer << " => " << value);
        std::cout << "dumpColumns: " << buffer << " => " << value << std::endl;
    }
}

void MorkParser::dumpValues()
{
    for ( MorkDict::iterator cellsIter = values_.begin();
          cellsIter != values_.end(); cellsIter++ )
    {
        char buffer[20];
        sprintf( buffer, "%d", cellsIter->first );
        std::string value = cellsIter->second;
        //SAL_INFO("connectivity.mork", "dumpValues: " << buffer << " => " << value);
        std::cout << "dumpValues: " << buffer << " => " << value << std::endl;
    }
}
