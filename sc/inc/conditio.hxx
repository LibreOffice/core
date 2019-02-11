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

#ifndef INCLUDED_SC_INC_CONDITIO_HXX
#define INCLUDED_SC_INC_CONDITIO_HXX

#include "global.hxx"
#include <tools/solar.h>
#include <tools/color.hxx>
#include "address.hxx"
#include <formula/grammar.hxx>
#include "scdllapi.h"
#include "rangelst.hxx"

#include <svl/listener.hxx>

#include <comphelper/stl_types.hxx>
#include <com/sun/star/sheet/ConditionOperator.hpp>

#include <rtl/math.hxx>
#include <tools/date.hxx>
#include <tools/link.hxx>

#include <boost/optional.hpp>
#include <map>
#include <memory>
#include <set>

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

enum class ScConditionMode
{
    Equal,
    Less,
    Greater,
    EqLess,
    EqGreater,
    NotEqual,
    Between,
    NotBetween,
    Duplicate,
    NotDuplicate,
    Direct,
    Top10,
    Bottom10,
    TopPercent,
    BottomPercent,
    AboveAverage,
    BelowAverage,
    AboveEqualAverage,
    BelowEqualAverage,
    Error,
    NoError,
    BeginsWith,
    EndsWith,
    ContainsText,
    NotContainsText,
    NONE
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScConditionMode& rMode)
{
    switch (rMode)
    {
    case ScConditionMode::Equal:
        stream << "EQUAL";
        break;
    case ScConditionMode::Less:
        stream << "LESS";
        break;
    case ScConditionMode::Greater:
        stream << "GREATER";
        break;
    case ScConditionMode::EqLess:
        stream << "EQLESS";
        break;
    case ScConditionMode::EqGreater:
        stream << "EQGREATER";
        break;
    case ScConditionMode::NotEqual:
        stream << "NOTEQUAL";
        break;
    case ScConditionMode::Between:
        stream << "BETWEEN";
        break;
    case ScConditionMode::NotBetween:
        stream << "NOTBETWEEN";
        break;
    case ScConditionMode::Duplicate:
        stream << "DUPLICATE";
        break;
    case ScConditionMode::NotDuplicate:
        stream << "NOTDUPLICATE";
        break;
    case ScConditionMode::Direct:
        stream << "DIRECT";
        break;
    case ScConditionMode::Top10:
        stream << "TOP10";
        break;
    case ScConditionMode::Bottom10:
        stream << "BOTTOM10";
        break;
    case ScConditionMode::TopPercent:
        stream << "TOPPERCENT";
        break;
    case ScConditionMode::BottomPercent:
        stream << "BOTTOMPERCENT";
        break;
    case ScConditionMode::AboveAverage:
        stream << "ABOVEAVERAGE";
        break;
    case ScConditionMode::BelowAverage:
        stream << "BELOWAVERAGE";
        break;
    case ScConditionMode::AboveEqualAverage:
        stream << "ABOVEEQUALAVERAGE";
        break;
    case ScConditionMode::BelowEqualAverage:
        stream << "BELOWEQUALAVERAGE";
        break;
    case ScConditionMode::Error:
        stream << "ERROR";
        break;
    case ScConditionMode::NoError:
        stream << "NOERROR";
        break;
    case ScConditionMode::BeginsWith:
        stream << "BEGINSWITH";
        break;
    case ScConditionMode::EndsWith:
        stream << "ENDSWITH";
        break;
    case ScConditionMode::ContainsText:
        stream << "CONTAINSTEXT";
        break;
    case ScConditionMode::NotContainsText:
        stream << "NOTCONTAINSTEXT";
        break;
    case ScConditionMode::NONE:
        stream << "NONE";
        break;
    default:
        stream << "?(" << static_cast<int>(rMode) << ")";
        break;
    }

    return stream;
}

class ScFormulaListener : public SvtListener
{
private:
    mutable bool mbDirty;
    ScDocument* mpDoc;
    std::function<void()> maCallbackFunction;

    void startListening(const ScTokenArray* pTokens, const ScRange& rPos);
    void startListening(const ScRangeList& rPos);

public:
    explicit ScFormulaListener(ScFormulaCell* pCell);
    explicit ScFormulaListener(ScDocument* pDoc);
    explicit ScFormulaListener(ScDocument* pDoc, const ScRangeList& rRange);
    virtual ~ScFormulaListener() override;

    void Notify( const SfxHint& rHint ) override;

    bool NeedsRepaint() const;

    void addTokenArray(const ScTokenArray* pTokens, const ScRange& rRange);
    void stopListening();
    void setCallback(const std::function<void()>& aCallbackFunction);

};

class ScConditionalFormat;
struct ScDataBarInfo;
struct ScIconSetInfo;

struct ScCondFormatData
{
    ScCondFormatData();
    ScCondFormatData(ScCondFormatData&&);
    ~ScCondFormatData();

    boost::optional<Color> mxColorScale;
    std::unique_ptr<ScDataBarInfo> pDataBar;
    std::unique_ptr<ScIconSetInfo> pIconSet;
    OUString aStyleName;
};

class SC_DLLPUBLIC ScFormatEntry
{
public:
    ScFormatEntry(ScDocument* pDoc);
    virtual ~ScFormatEntry() {}

    enum class Type
    {
        Condition,
        Colorscale,
        Databar,
        Iconset,
        Date
    };

    virtual Type GetType() const = 0;
    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) = 0;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) = 0;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) = 0;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) = 0;

    virtual ScFormatEntry* Clone( ScDocument* pDoc ) const = 0;

    virtual void SetParent( ScConditionalFormat* pNew ) = 0;

    bool operator==( const ScFormatEntry& ) const;
    virtual bool IsEqual( const ScFormatEntry&, bool bIgnoreSrcPos ) const;

    virtual void startRendering();
    virtual void endRendering();
protected:
    ScDocument* mpDoc;

};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScFormatEntry::Type& rType)
{
    switch (rType)
    {
    case ScFormatEntry::Type::Condition:
        stream << "Condition";
        break;
    case ScFormatEntry::Type::Colorscale:
        stream << "Colorscale";
        break;
    case ScFormatEntry::Type::Databar:
        stream << "Databar";
        break;
    case ScFormatEntry::Type::Iconset:
        stream << "Iconset";
        break;
    case ScFormatEntry::Type::Date:
        stream << "Data";
        break;
    default:
        stream << "?(" << static_cast<int>(rType) << ")";
        break;
    }
    return stream;
}

class approx_less
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
    const OUString      aStrNmsp1;      // namespace to be used on (re)compilation, e.g. in XML import
    const OUString      aStrNmsp2;      // namespace to be used on (re)compilation, e.g. in XML import
    const formula::FormulaGrammar::Grammar eTempGrammar1;  // grammar to be used on (re)compilation, e.g. in XML import
    const formula::FormulaGrammar::Grammar eTempGrammar2;  // grammar to be used on (re)compilation, e.g. in XML import
    bool                bIsStr1;        // for recognition of empty strings
    bool                bIsStr2;
    std::unique_ptr<ScTokenArray> pFormula1;      // entered formula
    std::unique_ptr<ScTokenArray> pFormula2;
    ScAddress           aSrcPos;        // source position for formulas
                                        // temporary data:
    OUString              aSrcString;     // formula source position as text during XML import
    std::unique_ptr<ScFormulaCell>  pFCell1;
    std::unique_ptr<ScFormulaCell>  pFCell2;
    bool                bRelRef1;
    bool                bRelRef2;
    bool                bFirstRun;
    std::unique_ptr<ScFormulaListener> mpListener;

    static void SimplifyCompiledFormula( std::unique_ptr<ScTokenArray>& rFormula,
                                     double& rVal,
                                     bool& rIsStr,
                                     OUString& rStrVal );

    void    MakeCells( const ScAddress& rPos );
    void    Compile( const OUString& rExpr1, const OUString& rExpr2,
                        const OUString& rExprNmsp1, const OUString& rExprNmsp2,
                        formula::FormulaGrammar::Grammar eGrammar1,
                        formula::FormulaGrammar::Grammar eGrammar2,
                        bool bTextToReal );
    void    Interpret( const ScAddress& rPos );

    bool    IsValid( double nArg, const ScAddress& rPos ) const;
    bool    IsValidStr( const OUString& rArg, const ScAddress& rPos ) const;
    void    StartListening();

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
    virtual ~ScConditionEntry() override;

    bool            IsEqual( const ScFormatEntry& r, bool bIgnoreSrcPos ) const override;

    virtual void SetParent( ScConditionalFormat* pNew ) override;

    bool IsCellValid( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    ScConditionMode GetOperation() const        { return eOp; }
    void SetOperation(ScConditionMode eMode);
    bool            IsIgnoreBlank() const       { return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void            SetIgnoreBlank(bool bSet);
    const OUString& GetSrcString() const         { return aSrcString; }
    const ScAddress& GetSrcPos() const           { return aSrcPos; }

    ScAddress       GetValidSrcPos() const;     // adjusted to allow textual representation of expressions

    void            SetSrcString( const OUString& rNew );     // for XML import

    void            SetFormula1( const ScTokenArray& rArray );
    void            SetFormula2( const ScTokenArray& rArray );

    OUString          GetExpression( const ScAddress& rCursor, sal_uInt16 nPos, sal_uInt32 nNumFmt = 0,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

                    /** Create a flat copy using ScTokenArray copy-ctor with
                        shared tokens. */
    ScTokenArray*   CreateFlatCopiedTokenArray( sal_uInt16 nPos ) const;

    void            CompileAll();
    void            CompileXML();
    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) override;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) override;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) override;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) override;

    bool            MarkUsedExternalReferences() const;

    virtual Type GetType() const override { return Type::Condition; }

    virtual ScFormatEntry* Clone(ScDocument* pDoc) const override;

    static ScConditionMode GetModeFromApi(css::sheet::ConditionOperator nOperator);

    virtual void endRendering() override;
    virtual void startRendering() override;

    bool NeedsRepaint() const;
    void CalcAll();

protected:
    virtual void    DataChanged() const;
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

    mutable std::unique_ptr<ScConditionEntryCache> mpCache;
};

//  single condition entry for conditional formatting
class SC_DLLPUBLIC ScCondFormatEntry : public ScConditionEntry
{
    OUString                  aStyleName;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const OUString& rExpr1, const OUString& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rStyle,
                                const OUString& rExprNmsp1 = EMPTY_OUSTRING,
                                const OUString& rExprNmsp2 = EMPTY_OUSTRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const OUString& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
    virtual ~ScCondFormatEntry() override;

    bool            IsEqual( const ScFormatEntry& r, bool bIgnoreSrcPos ) const override;

    const OUString&   GetStyle() const        { return aStyleName; }
    void            UpdateStyleName(const OUString& rNew)  { aStyleName=rNew; }
    virtual ScFormatEntry* Clone(ScDocument* pDoc) const override;

protected:
    virtual void    DataChanged() const override;
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
    condformat::ScCondFormatDateType GetDateType() const { return meType;}

    const OUString& GetStyleName() const { return maStyleName;}
    void SetStyleName( const OUString& rStyleName );

    virtual Type GetType() const override { return Type::Date; }
    virtual void UpdateReference( sc::RefUpdateContext& ) override {}
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& ) override {}
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& ) override {}
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& ) override {}

    virtual ScFormatEntry* Clone( ScDocument* pDoc ) const override;

    virtual void SetParent( ScConditionalFormat* ) override {}

    virtual void startRendering() override;
    virtual void endRendering() override;

private:
    condformat::ScCondFormatDateType meType;

    mutable std::unique_ptr<Date> mpCache;

    OUString maStyleName;
};

//  complete conditional formatting
class SC_DLLPUBLIC ScConditionalFormat
{
    ScDocument*         pDoc;
    sal_uInt32          nKey;               // Index in attributes

    std::vector<std::unique_ptr<ScFormatEntry>> maEntries;
    ScRangeList maRanges;            // Ranges for conditional format

public:
    ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument);
    ~ScConditionalFormat();
     ScConditionalFormat(const ScConditionalFormat&) = delete;
     const ScConditionalFormat& operator=(const ScConditionalFormat&) = delete;

    // true copy of formulas (for Ref-Undo / between documents)
    std::unique_ptr<ScConditionalFormat> Clone(ScDocument* pNewDoc = nullptr) const;

    void            AddEntry( ScFormatEntry* pNew );
    void RemoveEntry(size_t nIndex);
    void            SetRange( const ScRangeList& rRanges );
    const ScRangeList&  GetRange() const  { return maRanges; }
    // don't use the same name as for the const version
    ScRangeList& GetRangeList() { return maRanges; }

    bool IsEmpty() const;
    size_t size() const;

    ScDocument* GetDocument();

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

    const ScFormatEntry* GetEntry( sal_uInt16 nPos ) const;

    const OUString& GetCellStyle( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    ScCondFormatData GetData( ScRefCellValue& rCell, const ScAddress& rPos ) const;

    bool            EqualEntries( const ScConditionalFormat& r, bool bIgnoreSrcPos = false ) const;

    void            DoRepaint();

    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // only if not inserted!

    bool            MarkUsedExternalReferences() const;

    //  sorted (via std::set) by Index
    bool operator < ( const ScConditionalFormat& r ) const  { return nKey <  r.nKey; }

    void startRendering();
    void endRendering();

    // Forced recalculation for formulas
    void CalcAll();
};

struct CompareScConditionalFormat
{
    using is_transparent = void;
    bool operator()(std::unique_ptr<ScConditionalFormat> const& lhs,
                    std::unique_ptr<ScConditionalFormat> const& rhs) const
    {
        return (*lhs) < (*rhs);
    }
    bool operator()(sal_uInt32 nKey, std::unique_ptr<ScConditionalFormat> const& rpFormat) const
    {
        return nKey < rpFormat->GetKey();
    }
    bool operator()(std::unique_ptr<ScConditionalFormat> const& rpFormat, sal_uInt32 nKey) const
    {
        return rpFormat->GetKey() < nKey;
    }
};

//  List of all conditional formats in a sheet
class SC_DLLPUBLIC ScConditionalFormatList
{
private:
    typedef std::set<std::unique_ptr<ScConditionalFormat>,
                CompareScConditionalFormat> ConditionalFormatContainer;
    ConditionalFormatContainer m_ConditionalFormats;

    void operator =(ScConditionalFormatList const &) = delete;

public:
    ScConditionalFormatList() {}
    ScConditionalFormatList(const ScConditionalFormatList& rList);
    ScConditionalFormatList(ScDocument* pDoc, const ScConditionalFormatList& rList);

    void    InsertNew( std::unique_ptr<ScConditionalFormat> pNew );

    /**
     * Checks that all cond formats have a non empty range.
     * Deletes empty cond formats. Optionally call rLink
     * on the empty format before deleting it.
     * @return true if all cond formats were valid
     */
    bool    CheckAllEntries(const Link<ScConditionalFormat*,void>& rLink = Link<ScConditionalFormat*,void>());

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

    typedef ConditionalFormatContainer::iterator iterator;
    typedef ConditionalFormatContainer::const_iterator const_iterator;

    ScRangeList GetCombinedRange() const;

    void RemoveFromDocument(ScDocument* pDoc) const;
    void AddToDocument(ScDocument* pDoc) const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;
    bool empty() const;

    void erase(sal_uLong nIndex);
    void clear();

    void startRendering();
    void endRendering();

    sal_uInt32 getMaxKey() const;

    /// Forced recalculation of formulas
    void CalcAll();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
