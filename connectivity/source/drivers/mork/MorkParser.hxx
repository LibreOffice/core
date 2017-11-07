/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MORKPARSER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MORKPARSER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <string>
#include <map>
#include <set>
#include <vector>

#include "dllapi.h"

// Types

typedef std::map< int, std::string > MorkDict;
typedef std::map< int, int > MorkCells;               // ColumnId : ValueId
struct MorkRowMap { typedef std::map< int, MorkCells > Map; Map map; }; // Row id
struct RowScopeMap { typedef std::map< int, MorkRowMap > Map; Map map; }; // Row scope
struct MorkTableMap { typedef std::map< int, RowScopeMap > Map; Map map; }; // Table id
struct TableScopeMap { typedef std::map< int, MorkTableMap > Map; Map map; }; // Table Scope

// Error codes
enum MorkErrors
{
    NoError = 0,
    FailedToOpen,
    DefectedFormat
};


/// Class MorkParser

class LO_DLLPUBLIC_MORK MorkParser final
{
public:

    explicit MorkParser();

    /// Open and parse mork file

    bool open( const std::string &path );

    /// Returns all tables of specified scope

    MorkTableMap *getTables( int tableScope );

    /// Returns all rows under specified scope

    static MorkRowMap *getRows( int rowScope, RowScopeMap *table );

    /// Return value of specified value oid

    std::string const &getValue( int oid );

    /// Return value of specified column oid

    std::string const &getColumn( int oid );

    void retrieveLists(std::set<std::string>& lists);
    void getRecordKeysForListTable(std::string const & listName, std::set<int>& records);

    void dump();

    // All lists
    std::vector<OUString> lists_;

private: // Members

    void initVars();

    static bool isWhiteSpace( char c );
    char nextChar();

    static void parseScopeId( const std::string &TextId, int *Id, int *Scope );
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

private: // Data

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
    int defaultTableId_;

    // Indicates entity is being parsed
    enum class NP { Columns, Values, Rows } nowParsing_;

    MorkParser(const MorkParser &) = delete;
    MorkParser &operator=(const MorkParser &) = delete;

};

#endif // __MorkParser_h__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
