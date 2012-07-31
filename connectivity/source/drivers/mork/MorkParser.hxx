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

#ifndef _MORK_PARSER_HXX_
#define _MORK_PARSER_HXX_

#include <sal/types.h>

#include <string>
#include <map>

#include "dllapi.h"

// Types

typedef std::map< int, std::string > MorkDict;
typedef std::map< int, int > MorkCells;               // ColumnId : ValueId
typedef std::map< int, MorkCells > MorkRowMap;        // Row id
typedef std::map< int, MorkRowMap > RowScopeMap;      // Row scope
typedef std::map< int, RowScopeMap > MorkTableMap;    // Table id
typedef std::map< int, MorkTableMap > TableScopeMap;  // Table Scope

// Error codes
enum MorkErrors
{
    NoError = 0,
    FailedToOpen,
    UnsupportedVersion,
    DefectedFormat
};

// Mork term types
enum MorkTerm
{
    NoneTerm = 0,
    DictTerm,
    GroupTerm,
    TableTerm,
    RowTerm,
    CellTerm,
    CommentTerm,
    LiteralTerm
};


/// Class MorkParser

class LO_DLLPUBLIC_MORK MorkParser
{
public:

    MorkParser( int defaultScope = 0x80 );

    ///
    /// Open and parse mork file

    bool open( const std::string &path );

    ///
    /// Return error status

    MorkErrors error();

    ///
    /// Returns all tables of specified scope

    MorkTableMap *getTables( int tableScope );

    ///
    /// Rerturns all rows under specified scope

    MorkRowMap *getRows( int rowScope, RowScopeMap *table );

    ///
    /// Return value of specified value oid

    std::string &getValue( int oid );

    ///
    /// Return value of specified column oid

    std::string &getColumn( int oid );

    void dump();

protected: // Members

    void initVars();

    bool isWhiteSpace( char c );
    char nextChar();

    void parseScopeId( const std::string &TextId, int *Id, int *Scope );
    void setCurrentRow( int TableScope, int TableId, int RowScope, int RowId );

    // Parse methods
    bool parse();
    bool parseDict();
    bool parseComment();
    bool parseCell();
    bool parseTable();
    bool parseMeta( char c );
    bool parseRow( int TableId, int TableScope );
    bool parseGroup();

protected: // Data

    // Columns in mork means value names
    MorkDict columns_;
    MorkDict values_;

    // All mork file data
    TableScopeMap mork_;
    MorkCells *currentCells_;

    // Error status of last operation
    MorkErrors error_;

    // All Mork data
    std::string morkData_;

    unsigned morkPos_;
    int nextAddValueId_;
    int defaultScope_;
    int defaultListScope_;
    int defaultTableId_;

    // Indicates intity is being parsed
    enum { NPColumns, NPValues, NPRows } nowParsing_;

private:
    MorkParser(const MorkParser &);
    MorkParser &operator=(const MorkParser &);

};

#endif // __MorkParser_h__

