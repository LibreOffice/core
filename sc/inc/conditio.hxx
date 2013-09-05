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

#ifndef SC_CONDITIO_HXX
#define SC_CONDITIO_HXX

#include "global.hxx"
#include "address.hxx"
#include "formula/grammar.hxx"
#include "scdllapi.h"
#include "rangelst.hxx"

#include <rtl/math.hxx>
#include <tools/date.hxx>

#include <map>

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

class ScFormulaCell;
class ScTokenArray;
struct ScRefCellValue;

namespace sc {

struct RefUpdateContext;
struct RefUpdateInsertTabContext;
struct RefUpdateDeleteTabContext;
struct RefUpdateMoveTabContext;

}

//  nOptions Flags
#define SC_COND_NOBLANKS    1

enum ScConditionMode
{
    SC_COND_EQUAL,
    SC_COND_LESS,
    SC_COND_GREATER,
    SC_COND_EQLESS,
    SC_COND_EQGREATER,
    SC_COND_NOTEQUAL,
    SC_COND_BETWEEN,
    SC_COND_NOTBETWEEN,
    SC_COND_DUPLICATE,
    SC_COND_NOTDUPLICATE,
    SC_COND_DIRECT,
    SC_COND_TOP10,
    SC_COND_BOTTOM10,
    SC_COND_TOP_PERCENT,
    SC_COND_BOTTOM_PERCENT,
    SC_COND_ABOVE_AVERAGE,
    SC_COND_BELOW_AVERAGE,
    SC_COND_ABOVE_EQUAL_AVERAGE,
    SC_COND_BELOW_EQUAL_AVERAGE,
    SC_COND_ERROR,
    SC_COND_NOERROR,
    SC_COND_BEGINS_WITH,
    SC_COND_ENDS_WITH,
    SC_COND_CONTAINS_TEXT,
    SC_COND_NOT_CONTAINS_TEXT,
    SC_COND_NONE
};

class ScConditionalFormat;
struct ScDataBarInfo;
struct ScIconSetInfo;

namespace condformat
{

enum ScFormatEntryType
{
    CONDITION,
    COLORSCALE,
    DATABAR,
    ICONSET,
    DATE
};

}

struct ScCondFormatData
{
    ScCondFormatData():
        pColorScale(NULL),
        pDataBar(NULL),
        pIconSet(NULL) {}

    Color* pColorScale;
    ScDataBarInfo* pDataBar;
    ScIconSetInfo* pIconSet;
    OUString aStyleName;
};

class SC_DLLPUBLIC ScFormatEntry
{
public:
    ScFormatEntry(ScDocument* pDoc);
    virtual ~ScFormatEntry() {}

    virtual condformat::ScFormatEntryType GetType() const = 0;
    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) = 0;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) = 0;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) = 0;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) = 0;

    virtual ScFormatEntry* Clone( ScDocument* pDoc = NULL ) const = 0;

    virtual void SetParent( ScConditionalFormat* pNew ) = 0;

    bool operator==( const ScFormatEntry& ) const;

    virtual void startRendering();
    virtual void endRendering();
protected:
    ScDocument* mpDoc;

};

class approx_less : public std::binary_function<double, double, bool>
{
public:
    bool operator() (double nVal1, double nVal2) const
    {
        if(nVal1 < nVal2 && !rtl::math::approxEqual(nVal1, nVal2))
            return true;

        return false;
    }
};

class SC_DLLPUBLIC ScConditionEntry : public ScFormatEntry
{
                                        // stored data:
    ScConditionMode     eOp;
    sal_uInt16              nOptions;
    double              nVal1;          // input or calculated
    double              nVal2;
    OUString              aStrVal1;       // input or calculated
    OUString              aStrVal2;
    OUString              aStrNmsp1;      // namespace to be used on (re)compilation, e.g. in XML import
    OUString              aStrNmsp2;      // namespace to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar1;  // grammar to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar2;  // grammar to be used on (re)compilation, e.g. in XML import
    bool                bIsStr1;        // for recognition of empty strings
    bool                bIsStr2;
    ScTokenArray*       pFormula1;      // entered formula
    ScTokenArray*       pFormula2;
    ScAddress           aSrcPos;        // source position for formulas
                                        // temporary data:
    OUString              aSrcString;     // formula source position as text during XML import
    ScFormulaCell*      pFCell1;
    ScFormulaCell*      pFCell2;
    bool                bRelRef1;
    bool                bRelRef2;
    bool                bFirstRun;

    void    MakeCells( const ScAddress& rPos );
    void    Compile( const OUString& rExpr1, const OUString& rExpr2,
                        const OUString& rExprNmsp1, const OUString& rExprNmsp2,
                        formula::FormulaGrammar::Grammar eGrammar1,
                        formula::FormulaGrammar::Grammar eGrammar2,
                        bool bTextToReal );
    void    Interpret( const ScAddress& rPos );

    bool    IsValid( double nArg, const ScAddress& rPos ) const;
    bool    IsValidStr( const OUString& rArg, const ScAddress& rPos ) const;

public:
            ScConditionEntry( ScConditionMode eOper,
                                const OUString& rExpr1, const OUString& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rExprNmsp1, const OUString& rExprNmsp2,
                                formula::FormulaGrammar::Grammar eGrammar1,
                                formula::FormulaGrammar::Grammar eGrammar2 );
            ScConditionEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScConditionEntry( const ScConditionEntry& r );  // flat copy of formulas
            // true copy of formulas (for Ref-Undo):
            ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r );
    virtual ~ScConditionEntry();

    int             operator== ( const ScConditionEntry& r ) const;

    virtual void SetParent( ScConditionalFormat* pNew )  { pCondFormat = pNew; }

    bool IsCellValid( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    ScConditionMode GetOperation() const        { return eOp; }
    bool            IsIgnoreBlank() const       { return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void            SetIgnoreBlank(bool bSet);
    ScAddress       GetSrcPos() const           { return aSrcPos; }

    ScAddress       GetValidSrcPos() const;     // adjusted to allow textual representation of expressions

    void            SetSrcString( const OUString& rNew );     // for XML import

    void            SetFormula1( const ScTokenArray& rArray );
    void            SetFormula2( const ScTokenArray& rArray );

    OUString          GetExpression( const ScAddress& rCursor, sal_uInt16 nPos, sal_uLong nNumFmt = 0,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    ScTokenArray*   CreateTokenArry( sal_uInt16 nPos ) const;

    void            CompileAll();
    void            CompileXML();
    virtual void UpdateReference( sc::RefUpdateContext& rCxt );
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt );

    void            SourceChanged( const ScAddress& rChanged );

    bool            MarkUsedExternalReferences() const;

    virtual condformat::ScFormatEntryType GetType() const { return condformat::CONDITION; }

    virtual ScFormatEntry* Clone(ScDocument* pDoc = NULL) const;

    static ScConditionMode GetModeFromApi(sal_Int32 nOperator);

    virtual void endRendering();
    virtual void startRendering();

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
    ScDocument*     GetDocument() const     { return mpDoc; }
    ScConditionalFormat*    pCondFormat;

private:

    bool IsDuplicate(double nArg, const OUString& rStr) const;
    bool IsTopNElement( double nArg ) const;
    bool IsTopNPercent( double nArg ) const;
    bool IsBottomNElement( double nArg ) const;
    bool IsBottomNPercent( double nArg ) const;
    bool IsAboveAverage( double nArg, bool bEqual ) const;
    bool IsBelowAverage( double nArg, bool bEqual ) const;

    bool IsError( const ScAddress& rPos ) const;

    void FillCache() const;

    struct ScConditionEntryCache
    {
        typedef std::map<OUString, sal_Int32> StringCacheType;
        StringCacheType maStrings;
        typedef std::map<double, sal_Int32, approx_less> ValueCacheType;
        ValueCacheType maValues;

        // cache them for easier access
        size_t nValueItems;

        ScConditionEntryCache():
            nValueItems(0) {}
    };

    mutable boost::scoped_ptr<ScConditionEntryCache> mpCache;
};

//
//  single entry for conditional formatting
//

class SC_DLLPUBLIC ScCondFormatEntry : public ScConditionEntry
{
    OUString                  aStyleName;

    using ScConditionEntry::operator==;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const OUString& rExpr1, const OUString& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rStyle,
                                const OUString& rExprNmsp1 = EMPTY_STRING,
                                const OUString& rExprNmsp2 = EMPTY_STRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
    virtual ~ScCondFormatEntry();

    int             operator== ( const ScCondFormatEntry& r ) const;

    const OUString&   GetStyle() const        { return aStyleName; }
    void            UpdateStyleName(const OUString& rNew)  { aStyleName=rNew; }
    virtual ScFormatEntry* Clone(ScDocument* pDoc) const;

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
};

namespace condformat {

enum ScCondFormatDateType
{
    TODAY,
    YESTERDAY,
    TOMORROW,
    LAST7DAYS,
    THISWEEK,
    LASTWEEK,
    NEXTWEEK,
    THISMONTH,
    LASTMONTH,
    NEXTMONTH,
    THISYEAR,
    LASTYEAR,
    NEXTYEAR
};

}

class SC_DLLPUBLIC ScCondDateFormatEntry : public ScFormatEntry
{
public:
    ScCondDateFormatEntry(ScDocument* pDoc);
    ScCondDateFormatEntry(ScDocument* pDoc, const ScCondDateFormatEntry& rEntry);

    bool IsValid( const ScAddress& rPos ) const;

    void SetDateType(condformat::ScCondFormatDateType eType);
    condformat::ScCondFormatDateType GetDateType() const;

    const OUString& GetStyleName() const;
    void SetStyleName( const OUString& rStyleName );

    virtual condformat::ScFormatEntryType GetType() const { return condformat::DATE; }
    virtual void UpdateReference( sc::RefUpdateContext& ) {}
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& ) {}
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& ) {}
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& ) {}

    virtual ScFormatEntry* Clone( ScDocument* pDoc = NULL ) const;

    virtual void SetParent( ScConditionalFormat* ) {}

    bool operator==( const ScFormatEntry& ) const;

    virtual void startRendering();
    virtual void endRendering();

private:
    condformat::ScCondFormatDateType meType;

    mutable boost::scoped_ptr<Date> mpCache;

    OUString maStyleName;
};

//
//  complete conditional formatting
//

class SC_DLLPUBLIC ScConditionalFormat: private boost::noncopyable
{
    ScDocument*         pDoc;
    sal_uInt32          nKey;               // Index in attributes

    typedef boost::ptr_vector<ScFormatEntry> CondFormatContainer;
    CondFormatContainer maEntries;
    ScRangeList maRanges;            // Ranges for conditional format

public:
            ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument);
            ~ScConditionalFormat();

    // true copy of formulas (for Ref-Undo / between documents)
    ScConditionalFormat* Clone(ScDocument* pNewDoc = NULL) const;

    void            AddEntry( ScFormatEntry* pNew );
    void            AddRange( const ScRangeList& rRanges );
    const ScRangeList&  GetRange() const  { return maRanges; }
    // don't use the same name as for the const version
    ScRangeList& GetRangeList() { return maRanges; }

    ScDocument* GetDocument() { return pDoc; }

    bool IsEmpty() const;
    size_t size() const;

    void            CompileAll();
    void            CompileXML();
    void UpdateReference( sc::RefUpdateContext& rCxt, bool bCopyAsMove = false );
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt );

    void InsertRow(SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowStart, SCSIZE nSize);
    void InsertCol(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColStart, SCSIZE nSize);

    void            DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void            RenameCellStyle( const OUString& rOld, const OUString& rNew );

    void            SourceChanged( const ScAddress& rAddr );

    const ScFormatEntry* GetEntry( sal_uInt16 nPos ) const;

    const OUString& GetCellStyle( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    ScCondFormatData GetData( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    bool            EqualEntries( const ScConditionalFormat& r ) const;

    void            DoRepaint( const ScRange* pModified );

    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // only if not inserted!

    bool            MarkUsedExternalReferences() const;

    //  sorted (via PTRARR) by Index
    //  operator== only for sorting
    bool operator ==( const ScConditionalFormat& r ) const  { return nKey == r.nKey; }
    bool operator < ( const ScConditionalFormat& r ) const  { return nKey <  r.nKey; }

    void startRendering();
    void endRendering();
};

//
//  List of areas and formats:
//

class SC_DLLPUBLIC ScConditionalFormatList
{
private:
    typedef boost::ptr_set<ScConditionalFormat> ConditionalFormatContainer;
    ConditionalFormatContainer maConditionalFormats;

    void operator =(ScConditionalFormatList const &) SAL_DELETED_FUNCTION;

public:
    ScConditionalFormatList() {}
    ScConditionalFormatList(const ScConditionalFormatList& rList);
    ScConditionalFormatList(ScDocument* pDoc, const ScConditionalFormatList& rList);
    ~ScConditionalFormatList() {}

    void    InsertNew( ScConditionalFormat* pNew );

    /**
     * Checks that all cond formats have a non empty range.
     * Deletes empty cond formats.
     * @return true if all cond formats were valid
     */
    bool    CheckAllEntries();

    ScConditionalFormat* GetFormat( sal_uInt32 nKey );
    const ScConditionalFormat* GetFormat( sal_uInt32 nKey ) const;

    void    CompileAll();
    void    CompileXML();
    void UpdateReference( sc::RefUpdateContext& rCxt );
    void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt );

    void InsertRow(SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowStart, SCSIZE nSize);
    void InsertCol(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColStart, SCSIZE nSize);

    void    RenameCellStyle( const OUString& rOld, const OUString& rNew );
    void    DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    void    SourceChanged( const ScAddress& rAddr );

    bool    operator==( const ScConditionalFormatList& r ) const;       // for Ref-Undo

    typedef ConditionalFormatContainer::iterator iterator;
    typedef ConditionalFormatContainer::const_iterator const_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;

    void erase(sal_uLong nIndex);

    void startRendering();
    void endRendering();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
