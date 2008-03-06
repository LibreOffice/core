/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditio.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:15:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_CONDITIO_HXX
#define SC_CONDITIO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef SC_GRAMMAR_HXX
#include "grammar.hxx"
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

class ScBaseCell;
class ScFormulaCell;
class ScTokenArray;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
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

class ScConditionEntry
{
                                        // gespeicherte Daten:
    ScConditionMode     eOp;
    USHORT              nOptions;
    double              nVal1;          // eingegeben oder berechnet
    double              nVal2;
    String              aStrVal1;       // eingegeben oder berechnet
    String              aStrVal2;
    ScGrammar::Grammar  eTempGrammar;   // grammar to be used on (re)compilation, e.g. in XML import
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
                        const ScGrammar::Grammar eGrammar, BOOL bTextToReal );
    void    Interpret( const ScAddress& rPos );

    BOOL    IsValid( double nArg ) const;
    BOOL    IsValidStr( const String& rArg ) const;

protected:
            ScConditionEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
    void    StoreCondition(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

public:
            ScConditionEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const ScGrammar::Grammar eGrammar );
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
                                    const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT ) const;

    ScTokenArray*   CreateTokenArry( USHORT nPos ) const;

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void            SourceChanged( const ScAddress& rChanged );

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
    ScDocument*     GetDocument() const     { return pDoc; }
};

//
//  einzelner Eintrag fuer bedingte Formatierung
//

class ScConditionalFormat;

class ScCondFormatEntry : public ScConditionEntry
{
    String                  aStyleName;
    ScConditionalFormat*    pParent;

    using ScConditionEntry::operator==;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle,
                                const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
            ScCondFormatEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
    virtual ~ScCondFormatEntry();

    void            SetParent( ScConditionalFormat* pNew )  { pParent = pNew; }

    void            Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    int             operator== ( const ScCondFormatEntry& r ) const;

    const String&   GetStyle() const        { return aStyleName; }

protected:
    virtual void    DataChanged( const ScRange* pModified ) const;
};

//
//  komplette bedingte Formatierung
//

class ScConditionalFormat
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
            ScConditionalFormat(SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument);
            ~ScConditionalFormat();

    // echte Kopie der Formeln (fuer Ref-Undo / zwischen Dokumenten)
    ScConditionalFormat* Clone(ScDocument* pNewDoc = NULL) const;

    void            Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    void            AddEntry( const ScCondFormatEntry& rNew );

    BOOL            IsEmpty() const         { return (nEntryCount == 0); }
    USHORT          Count() const           { return nEntryCount; }

    void            CompileAll();
    void            CompileXML();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

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

    void    Load( SvStream& rStream, ScDocument* pDocument );
    void    Store( SvStream& rStream ) const;
    void    ResetUsed();

    void    CompileAll();
    void    CompileXML();
    void    UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void    SourceChanged( const ScAddress& rAddr );

    BOOL    operator==( const ScConditionalFormatList& r ) const;       // fuer Ref-Undo
};

#endif


