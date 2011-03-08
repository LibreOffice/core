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

#ifndef SC_CONDITIO_HXX
#define SC_CONDITIO_HXX

#include "global.hxx"
#include "address.hxx"
#include "formula/grammar.hxx"
#include <svl/svarray.hxx>
#include "scdllapi.h"

class ScBaseCell;
class ScFormulaCell;
class ScTokenArray;
class ScRangeList;


#define SC_COND_GROW 16

//  nOptions Flags
#define SC_COND_NOBLANKS    1


            // Reihenfolge von ScConditionMode wie ScQueryOp,
            // damit einmal zusammengefasst werden kann:

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
    SC_COND_DIRECT,
    SC_COND_NONE
};

enum ScConditionValType
{
    SC_VAL_VALUE,
    SC_VAL_STRING,
    SC_VAL_FORMULA
};

class SC_DLLPUBLIC ScConditionEntry
{
                                        // gespeicherte Daten:
    ScConditionMode     eOp;
    USHORT              nOptions;
    double              nVal1;          // eingegeben oder berechnet
    double              nVal2;
    String              aStrVal1;       // eingegeben oder berechnet
    String              aStrVal2;
    String              aStrNmsp1;      // namespace to be used on (re)compilation, e.g. in XML import
    String              aStrNmsp2;      // namespace to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar1;  // grammar to be used on (re)compilation, e.g. in XML import
    formula::FormulaGrammar::Grammar eTempGrammar2;  // grammar to be used on (re)compilation, e.g. in XML import
    BOOL                bIsStr1;        // um auch leere Strings zu erkennen
    BOOL                bIsStr2;
    ScTokenArray*       pFormula1;      // eingegebene Formel
    ScTokenArray*       pFormula2;
    ScAddress           aSrcPos;        // source position for formulas
                                        // temporary data:
    String              aSrcString;     // formula source position as text during XML import
    ScFormulaCell*      pFCell1;
    ScFormulaCell*      pFCell2;
    ScDocument*         pDoc;
    BOOL                bRelRef1;
    BOOL                bRelRef2;
    BOOL                bFirstRun;

    void    MakeCells( const ScAddress& rPos );
    void    Compile( const String& rExpr1, const String& rExpr2,
                        const String& rExprNmsp1, const String& rExprNmsp2,
                        formula::FormulaGrammar::Grammar eGrammar1,
                        formula::FormulaGrammar::Grammar eGrammar2,
                        BOOL bTextToReal );
    void    Interpret( const ScAddress& rPos );

    BOOL    IsValid( double nArg ) const;
    BOOL    IsValidStr( const String& rArg ) const;

public:
            ScConditionEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rExprNmsp1, const String& rExprNmsp2,
                                formula::FormulaGrammar::Grammar eGrammar1,
                                formula::FormulaGrammar::Grammar eGrammar2 );
            ScConditionEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScConditionEntry( const ScConditionEntry& r );  // flache Kopie der Formeln
            // echte Kopie der Formeln (fuer Ref-Undo):
            ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r );
    virtual ~ScConditionEntry();

    int             operator== ( const ScConditionEntry& r ) const;

    BOOL            IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

    ScConditionMode GetOperation() const        { return eOp; }
    BOOL            IsIgnoreBlank() const       { return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void            SetIgnoreBlank(BOOL bSet);
    ScAddress       GetSrcPos() const           { return aSrcPos; }

    ScAddress       GetValidSrcPos() const;     // adjusted to allow textual representation of expressions

    void            SetSrcString( const String& rNew );     // for XML import

    void            SetFormula1( const ScTokenArray& rArray );
    void            SetFormula2( const ScTokenArray& rArray );

    String          GetExpression( const ScAddress& rCursor, USHORT nPos, ULONG nNumFmt = 0,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    ScTokenArray*   CreateTokenArry( USHORT nPos ) const;

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void            SourceChanged( const ScAddress& rChanged );

    bool            MarkUsedExternalReferences() const;

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
    ScDocument*     GetDocument() const     { return pDoc; }
};

//
//  einzelner Eintrag fuer bedingte Formatierung
//

class ScConditionalFormat;

class SC_DLLPUBLIC ScCondFormatEntry : public ScConditionEntry
{
    String                  aStyleName;
    ScConditionalFormat*    pParent;

    using ScConditionEntry::operator==;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle,
                                const String& rExprNmsp1 = EMPTY_STRING,
                                const String& rExprNmsp2 = EMPTY_STRING,
                                formula::FormulaGrammar::Grammar eGrammar1 = formula::FormulaGrammar::GRAM_DEFAULT,
                                formula::FormulaGrammar::Grammar eGrammar2 = formula::FormulaGrammar::GRAM_DEFAULT );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
    virtual ~ScCondFormatEntry();

    void            SetParent( ScConditionalFormat* pNew )  { pParent = pNew; }

    int             operator== ( const ScCondFormatEntry& r ) const;

    const String&   GetStyle() const        { return aStyleName; }
    void            UpdateStyleName(const String& rNew)  { aStyleName=rNew; }

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
};

//
//  komplette bedingte Formatierung
//

class SC_DLLPUBLIC ScConditionalFormat
{
    ScDocument*         pDoc;
    ScRangeList*        pAreas;             // Bereiche fuer Paint
    sal_uInt32          nKey;               // Index in Attributen
    ScCondFormatEntry** ppEntries;
    USHORT              nEntryCount;
    BOOL                bIsUsed;            // temporaer beim Speichern

public:
            ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument);
            ScConditionalFormat(const ScConditionalFormat& r);
            ~ScConditionalFormat();

    // echte Kopie der Formeln (fuer Ref-Undo / zwischen Dokumenten)
    ScConditionalFormat* Clone(ScDocument* pNewDoc = NULL) const;

    void            AddEntry( const ScCondFormatEntry& rNew );

    BOOL            IsEmpty() const         { return (nEntryCount == 0); }
    USHORT          Count() const           { return nEntryCount; }

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );
    void            RenameCellStyle( const String& rOld, const String& rNew );

    void            SourceChanged( const ScAddress& rAddr );

    const ScCondFormatEntry* GetEntry( USHORT nPos ) const;

    const String&   GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const;

    BOOL            EqualEntries( const ScConditionalFormat& r ) const;

    void            DoRepaint( const ScRange* pModified );
    void            InvalidateArea();

    sal_uInt32      GetKey() const          { return nKey; }
    void            SetKey(sal_uInt32 nNew) { nKey = nNew; }    // nur wenn nicht eingefuegt!

    void            SetUsed(BOOL bSet)      { bIsUsed = bSet; }
    BOOL            IsUsed() const          { return bIsUsed; }

    bool            MarkUsedExternalReferences() const;

    //  sortiert (per PTRARR) nach Index
    //  operator== nur fuer die Sortierung
    BOOL operator ==( const ScConditionalFormat& r ) const  { return nKey == r.nKey; }
    BOOL operator < ( const ScConditionalFormat& r ) const  { return nKey <  r.nKey; }
};

//
//  Liste der Bereiche und Formate:
//

typedef ScConditionalFormat* ScConditionalFormatPtr;

SV_DECL_PTRARR_SORT(ScConditionalFormats_Impl, ScConditionalFormatPtr,
                        SC_COND_GROW, SC_COND_GROW)

class ScConditionalFormatList : public ScConditionalFormats_Impl
{
public:
        ScConditionalFormatList() {}
        ScConditionalFormatList(const ScConditionalFormatList& rList);
        ScConditionalFormatList(ScDocument* pNewDoc, const ScConditionalFormatList& rList);
        ~ScConditionalFormatList() {}

    void    InsertNew( ScConditionalFormat* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScConditionalFormat* GetFormat( sal_uInt32 nKey );

    void    CompileAll();
    void    CompileXML();
    void    UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void    RenameCellStyle( const String& rOld, const String& rNew );
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void    SourceChanged( const ScAddress& rAddr );

    /** Temporarily during save, returns RefManager's decision whether ALL
     *  references are marked now. */
    bool    MarkUsedExternalReferences() const;

    BOOL    operator==( const ScConditionalFormatList& r ) const;       // fuer Ref-Undo
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
