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

#include <bf_svtools/svarray.hxx>
namespace binfilter {

class ScBaseCell;
class ScFormulaCell;
class ScTokenArray;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class ScRangeList;


#define SC_COND_GROW 16

//	nOptions Flags
#define SC_COND_NOBLANKS	1


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
    ScConditionMode		eOp;
    USHORT				nOptions;
    double				nVal1;			// eingegeben oder berechnet
    double				nVal2;
    String				aStrVal1;		// eingegeben oder berechnet
    String				aStrVal2;
    BOOL				bIsStr1;		// um auch leere Strings zu erkennen
    BOOL				bIsStr2;
    ScTokenArray*		pFormula1;		// eingegebene Formel
    ScTokenArray*		pFormula2;
    ScAddress			aSrcPos;		// Quell-Adresse fuer Formeln
                                        // temporaere Daten:
    ScFormulaCell*		pFCell1;
    ScFormulaCell*		pFCell2;
    ScDocument*			pDoc;
    BOOL				bRelRef1;
    BOOL				bRelRef2;
    BOOL				bFirstRun;

    void	MakeCells( const ScAddress& rPos );
    void	Compile( const String& rExpr1, const String& rExpr2, BOOL bEnglish,
                        BOOL bCompileXML, BOOL bTextToReal );
    void	Interpret( const ScAddress& rPos );

    BOOL	IsValid( double nArg ) const;

protected:
            ScConditionEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
    void	StoreCondition(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

public:
            ScConditionEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                BOOL bCompileEnglish, BOOL bCompileXML );
            ScConditionEntry( const ScConditionEntry& r );	// flache Kopie der Formeln
            // echte Kopie der Formeln (fuer Ref-Undo):
            ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r );
            ~ScConditionEntry();

    int				operator== ( const ScConditionEntry& r ) const;

    BOOL			IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

    ScConditionMode	GetOperation() const		{ return eOp; }
    BOOL			IsIgnoreBlank() const		{ return ( nOptions & SC_COND_NOBLANKS ) == 0; }
    void			SetIgnoreBlank(BOOL bSet);
    ScAddress		GetSrcPos() const			{ return aSrcPos; }

    String			GetExpression( const ScAddress& rCursor, USHORT nPos, ULONG nNumFmt = 0,
                                    BOOL bEnglish = FALSE, BOOL bCompileXML = FALSE,
                                    BOOL bTextToReal = FALSE ) const;


    void			CompileXML();
    void			UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );

    void			SourceChanged( const ScAddress& rChanged );

protected:
    virtual void	DataChanged( const ScRange* pModified ) const;
    ScDocument*		GetDocument() const		{ return pDoc; }
};

//
//	einzelner Eintrag fuer bedingte Formatierung
//

class ScConditionalFormat;

class ScCondFormatEntry : public ScConditionEntry
{
    String					aStyleName;
    ScConditionalFormat*	pParent;

public:
            ScCondFormatEntry( ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                const String& rStyle,
                                BOOL bCompileEnglish = FALSE, BOOL bCompileXML = FALSE );
            ScCondFormatEntry( const ScCondFormatEntry& r );
            ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r );
            ScCondFormatEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
            ~ScCondFormatEntry();

    void			SetParent( ScConditionalFormat* pNew )	{ pParent = pNew; }

    void			Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    int				operator== ( const ScCondFormatEntry& r ) const;

    const String&	GetStyle() const		{ return aStyleName; }

protected:
    virtual void	DataChanged( const ScRange* pModified ) const;
};

//
//	komplette bedingte Formatierung
//

class ScConditionalFormat
{
    ScDocument*			pDoc;
    ScRangeList*		pAreas;				// Bereiche fuer Paint
    sal_uInt32			nKey;				// Index in Attributen
    ScCondFormatEntry**	ppEntries;
    USHORT				nEntryCount;
    BOOL				bIsUsed;			// temporaer beim Speichern

public:
            ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument);
            ScConditionalFormat(const ScConditionalFormat& r);
            ScConditionalFormat(SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument);
            ~ScConditionalFormat();

    // echte Kopie der Formeln (fuer Ref-Undo / zwischen Dokumenten)
    ScConditionalFormat* Clone(ScDocument* pNewDoc = NULL) const;

    void			Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    void			AddEntry( const ScCondFormatEntry& rNew );

    BOOL			IsEmpty() const			{ return (nEntryCount == 0); }
    USHORT			Count() const			{ return nEntryCount; }

    void			CompileXML();
    void			UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );

    void			SourceChanged( const ScAddress& rAddr );

    const ScCondFormatEntry* GetEntry( USHORT nPos ) const;

    const String&	GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const;

    BOOL			EqualEntries( const ScConditionalFormat& r ) const;

    void			DoRepaint( const ScRange* pModified );
    void			InvalidateArea();

    sal_uInt32		GetKey() const			{ return nKey; }
    void			SetKey(sal_uInt32 nNew)	{ nKey = nNew; }	// nur wenn nicht eingefuegt!

    void			SetUsed(BOOL bSet)		{ bIsUsed = bSet; }
    BOOL			IsUsed() const			{ return bIsUsed; }

    //	sortiert (per PTRARR) nach Index
    //	operator== nur fuer die Sortierung
    BOOL operator ==( const ScConditionalFormat& r ) const	{ return nKey == r.nKey; }
    BOOL operator < ( const ScConditionalFormat& r ) const	{ return nKey <  r.nKey; }
};

//
//	Liste der Bereiche und Formate:
//

typedef ScConditionalFormat* ScConditionalFormatPtr;

SV_DECL_PTRARR_SORT(ScConditionalFormats_Impl, ScConditionalFormatPtr,
                        SC_COND_GROW, SC_COND_GROW)//STRIP008 ;

class ScConditionalFormatList : public ScConditionalFormats_Impl
{
public:
        ScConditionalFormatList() {}
        ScConditionalFormatList(const ScConditionalFormatList& rList);
        ~ScConditionalFormatList() {}

    void	InsertNew( ScConditionalFormat* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScConditionalFormat* GetFormat( sal_uInt32 nKey );

    void	Load( SvStream& rStream, ScDocument* pDocument );
    void	Store( SvStream& rStream ) const;
    void	ResetUsed();

    void	CompileXML();
    void	UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );

    void	SourceChanged( const ScAddress& rAddr );

};

} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
