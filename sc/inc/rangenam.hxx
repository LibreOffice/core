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

#ifndef SC_RANGENAM_HXX
#define SC_RANGENAM_HXX

#include "global.hxx" // -> enum UpdateRefMode
#include "address.hxx"
#include "formula/grammar.hxx"
#include "scdllapi.h"

#include <map>
#include <vector>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_map.hpp>

class ScDocument;
class ScTokenArray;

namespace sc {
    struct RefUpdateContext;
    struct RefUpdateInsertTabContext;
}

typedef sal_uInt16 RangeType;

#define RT_NAME             ((RangeType)0x0000)
#define RT_DATABASE         ((RangeType)0x0001)
#define RT_CRITERIA         ((RangeType)0x0002)
#define RT_PRINTAREA        ((RangeType)0x0004)
#define RT_COLHEADER        ((RangeType)0x0008)
#define RT_ROWHEADER        ((RangeType)0x0010)
#define RT_ABSAREA          ((RangeType)0x0020)
#define RT_REFAREA          ((RangeType)0x0040)
#define RT_ABSPOS           ((RangeType)0x0080)
#define RT_SHARED           ((RangeType)0x0100)
#define RT_SHAREDMOD        ((RangeType)0x0200)

class ScRangeData
{
private:
    OUString   aName;
    OUString   aUpperName;     // #i62977# for faster searching (aName is never modified after ctor)
    ScTokenArray*   pCode;
    ScAddress       aPos;
    RangeType       eType;
    ScDocument*     pDoc;
    formula::FormulaGrammar::Grammar    eTempGrammar;   // needed for unresolved XML compiles
    sal_uInt16      nIndex;
    bool            bModified;          // is set/cleared by UpdateReference

    // max row and column to use for wrapping of references.  If -1 use the
    // application's default.
    SCROW           mnMaxRow;
    SCCOL           mnMaxCol;

    void CompileRangeData( const OUString& rSymbol, bool bSetError );
    void InitCode();
public:

    enum TabRefUpdateMode { Insert = 1, Delete = 2, Move = 3 };

    typedef ::std::map<sal_uInt16, sal_uInt16> IndexMap;

    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const OUString& rName,
                                 const OUString& rSymbol,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME,
                                 const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const OUString& rName,
                                 const ScTokenArray& rArr,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME );
    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const OUString& rName,
                                 const ScAddress& rTarget );
                                // rTarget is ABSPOS jump label
                    ScRangeData(const ScRangeData& rScRangeData, ScDocument* pDocument = NULL);

    SC_DLLPUBLIC ~ScRangeData();

    bool            operator== (const ScRangeData& rData) const;

    void            GetName( OUString& rName ) const  { rName = aName; }
    const OUString&   GetName( void ) const           { return aName; }
    const OUString&   GetUpperName( void ) const      { return aUpperName; }
    ScAddress       GetPos() const                  { return aPos; }
    // The index has to be unique. If index=0 a new index value is assigned.
    void            SetIndex( sal_uInt16 nInd )         { nIndex = nInd; }
    sal_uInt16    GetIndex() const                { return nIndex; }
    ScTokenArray*   GetCode()                       { return pCode; }
    SC_DLLPUBLIC void   SetCode( ScTokenArray& );
    const ScTokenArray* GetCode() const             { return pCode; }
    SC_DLLPUBLIC sal_uInt16 GetErrCode() const;
    bool            HasReferences() const;
    void            SetDocument( ScDocument* pDocument){ pDoc = pDocument; }
    ScDocument*     GetDocument() const             { return pDoc; }
    void            SetType( RangeType nType )      { eType = nType; }
    void            AddType( RangeType nType )      { eType = eType|nType; }
    RangeType       GetType() const                 { return eType; }
    bool            HasType( RangeType nType ) const;
    sal_uInt32      GetUnoType() const;
    SC_DLLPUBLIC void GetSymbol( String& rSymbol, const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    SC_DLLPUBLIC void GetSymbol( OUString& rSymbol, const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    SC_DLLPUBLIC void GetSymbol( OUString& rSymbol, const ScAddress& rPos, const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            UpdateSymbol( OUStringBuffer& rBuffer, const ScAddress&,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT );

    /**
     * @param nLocalTab sheet index where this name belongs, or -1 for global
     *                  name.
     */
    void UpdateReference( sc::RefUpdateContext& rCxt, SCTAB nLocalTab = -1 );
    bool            IsModified() const              { return bModified; }

    SC_DLLPUBLIC void           GuessPosition();

    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    SC_DLLPUBLIC bool           IsReference( ScRange& rRef ) const;
    bool                        IsReference( ScRange& rRef, const ScAddress& rPos ) const;
    SC_DLLPUBLIC bool           IsValidReference( ScRange& rRef ) const;
    bool                        IsRangeAtBlock( const ScRange& ) const;

    void UpdateTabRef(SCTAB nOldTable, TabRefUpdateMode eMode, SCTAB nNewTable, SCTAB nNewSheets);
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt, SCTAB nLocalTab = -1 );

    void            ValidateTabRefs();

    static void     MakeValidName( String& rName );
    SC_DLLPUBLIC static bool        IsNameValid( const String& rName, ScDocument* pDoc );

    SC_DLLPUBLIC void SetMaxRow(SCROW nRow);
    SCROW GetMaxRow() const;
    SC_DLLPUBLIC void SetMaxCol(SCCOL nCol);
    SCCOL GetMaxCol() const;

    void            CompileUnresolvedXML();
};

inline bool ScRangeData::HasType( RangeType nType ) const
{
    return ( ( eType & nType ) == nType );
}

extern "C" int SAL_CALL ScRangeData_QsortNameCompare( const void*, const void* );

bool operator< (const ScRangeData& left, const ScRangeData& right);

class ScRangeName
{
private:
    typedef std::vector<ScRangeData*> IndexDataType;
    typedef ::boost::ptr_map<OUString, ScRangeData> DataType;
    DataType maData;
    IndexDataType maIndexToData;

public:
    /// Map that manages stored ScRangeName instances.
    typedef ::boost::ptr_map<SCTAB, ScRangeName>  TabNameMap;
    /// Map that stores non-managed pointers to ScRangeName instances.
    typedef ::std::map<SCTAB, const ScRangeName*> TabNameCopyMap;

    typedef DataType::const_iterator const_iterator;
    typedef DataType::iterator iterator;

    ScRangeName();
    SC_DLLPUBLIC ScRangeName(const ScRangeName& r);

    SC_DLLPUBLIC const ScRangeData* findByRange(const ScRange& rRange) const;
    SC_DLLPUBLIC ScRangeData* findByUpperName(const OUString& rName);
    SC_DLLPUBLIC const ScRangeData* findByUpperName(const OUString& rName) const;
    SC_DLLPUBLIC ScRangeData* findByIndex(sal_uInt16 i) const;
    void UpdateReference( sc::RefUpdateContext& rCxt, SCTAB nLocalTab = -1 );
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt, SCTAB nLocalTab = -1 );
    void UpdateTabRef(SCTAB nTable, ScRangeData::TabRefUpdateMode eMode, SCTAB nNewTable = 0, SCTAB nNewSheets = 1);
    void UpdateTranspose(const ScRange& rSource, const ScAddress& rDest);
    void UpdateGrow(const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY);

    /** Compile those names that couldn't be resolved during loading and
        inserting because they may have referred a name that was inserted later.
     */
    void CompileUnresolvedXML();

    SC_DLLPUBLIC const_iterator begin() const;
    SC_DLLPUBLIC const_iterator end() const;
    SC_DLLPUBLIC iterator begin();
    SC_DLLPUBLIC iterator end();
    SC_DLLPUBLIC size_t size() const;
    bool empty() const;

    /** Insert object into set.
        @ATTENTION: The underlying ::boost::ptr_set_adapter::insert(p) takes
        ownership of p and if it can't insert it deletes the object! So, if
        this insert here returns false the object where p pointed to is gone!
     */
    SC_DLLPUBLIC bool insert(ScRangeData* p);

    void erase(const ScRangeData& r);
    void erase(const OUString& rName);

    /**
     * Erase by iterator position.  Note that this method doesn't check for
     * iterator's validity.  The caller must make sure that the iterator is
     * valid.
     */
    void erase(const iterator& itr);
    void clear();
    bool operator== (const ScRangeName& r) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
