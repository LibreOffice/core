/*************************************************************************
 *
 *  $RCSfile: conditio.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-01 17:27:40 $
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

#ifndef SC_CONDITIO_HXX
#define SC_CONDITIO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
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
    BOOL                bIsStr1;        // um auch leere Strings zu erkennen
    BOOL                bIsStr2;
    ScTokenArray*       pFormula1;      // eingegebene Formel
    ScTokenArray*       pFormula2;
    ScAddress           aSrcPos;        // Quell-Adresse fuer Formeln
                                        // temporaere Daten:
    ScFormulaCell*      pFCell1;
    ScFormulaCell*      pFCell2;
    ScDocument*         pDoc;
    BOOL                bRelRef1;
    BOOL                bRelRef2;
    BOOL                bFirstRun;

    void    MakeCells( const ScAddress& rPos );
    void    Compile( const String& rExpr1, const String& rExpr2, BOOL bEnglish, BOOL bCompileXML );
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
                                BOOL bCompileEnglish, BOOL bCompileXML );
            ScConditionEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScConditionEntry( const ScConditionEntry& r );  // flache Kopie der Formeln
            // echte Kopie der Formeln (fuer Ref-Undo):
            ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r );
            ~ScConditionEntry();

    int             operator== ( const ScConditionEntry& r ) const;

    BOOL            IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

    ScConditionMode GetOperation() const        { return eOp; }
    BOOL            IsIgnoreBlank() const       { return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void            SetIgnoreBlank(BOOL bSet);
    ScAddress       GetSrcPos() const           { return aSrcPos; }

    String          GetExpression( const ScAddress& rCursor, USHORT nPos, ULONG nNumFmt = 0,
                                    BOOL bEnglish = FALSE, BOOL bCompileXML = FALSE ) const;

    ScTokenArray*   CreateTokenArry( USHORT nPos ) const;

    void            CompileAll();
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );
    void            UpdateMoveTab( USHORT nOldPos, USHORT nNewPos );

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

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle,
                                BOOL bCompileEnglish = FALSE, BOOL bCompileXML = FALSE );
            ScCondFormatEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
            ScCondFormatEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
            ~ScCondFormatEntry();

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
    ULONG               nKey;               // Index in Attributen
    ScCondFormatEntry** ppEntries;
    USHORT              nEntryCount;
    BOOL                bIsUsed;            // temporaer beim Speichern

public:
            ScConditionalFormat(ULONG nNewKey, ScDocument* pDocument);
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
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );
    void            UpdateMoveTab( USHORT nOldPos, USHORT nNewPos );

    void            SourceChanged( const ScAddress& rAddr );

    const ScCondFormatEntry* GetEntry( USHORT nPos ) const;

    const String&   GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const;

    BOOL            EqualEntries( const ScConditionalFormat& r ) const;

    void            DoRepaint( const ScRange* pModified );
    void            InvalidateArea();

    ULONG           GetKey() const          { return nKey; }
    void            SetKey(ULONG nNew)      { nKey = nNew; }    // nur wenn nicht eingefuegt!

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
                        SC_COND_GROW, SC_COND_GROW);

class ScConditionalFormatList : public ScConditionalFormats_Impl
{
public:
        ScConditionalFormatList() {}
        ScConditionalFormatList(const ScConditionalFormatList& rList);
        ScConditionalFormatList(ScDocument* pNewDoc, const ScConditionalFormatList& rList);
        ~ScConditionalFormatList() {}

    void    InsertNew( ScConditionalFormat* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScConditionalFormat* GetFormat( ULONG nKey );

    void    Load( SvStream& rStream, ScDocument* pDocument );
    void    Store( SvStream& rStream ) const;
    void    ResetUsed();

    void    CompileAll();
    void    UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );
    void    UpdateMoveTab( USHORT nOldPos, USHORT nNewPos );

    void    SourceChanged( const ScAddress& rAddr );

    BOOL    operator==( const ScConditionalFormatList& r ) const;       // fuer Ref-Undo
};

#endif


