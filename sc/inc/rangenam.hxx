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

#ifndef SC_RANGENAM_HXX
#define SC_RANGENAM_HXX

#include "global.hxx" // -> enum UpdateRefMode
#include "address.hxx"
#include "collect.hxx"
#include "formula/grammar.hxx"
#include "scdllapi.h"

#include <map>

//------------------------------------------------------------------------

class ScDocument;

namespace rtl {
    class OUStringBuffer;
}


//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

class ScTokenArray;

class ScRangeData : public ScDataObject
{
private:
    String          aName;
    String          aUpperName;         // #i62977# for faster searching (aName is never modified after ctor)
    ScTokenArray*   pCode;
    ScAddress       aPos;
    RangeType       eType;
    ScDocument*     pDoc;
    sal_uInt16          nIndex;
    sal_Bool            bModified;          // wird bei UpdateReference gesetzt/geloescht

    // max row and column to use for wrapping of references.  If -1 use the
    // application's default.
    SCROW           mnMaxRow;
    SCCOL           mnMaxCol;

    friend class ScRangeName;
    ScRangeData( sal_uInt16 nIndex );
public:
    typedef ::std::map<sal_uInt16, sal_uInt16> IndexMap;

    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const String& rSymbol,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME,
                                 const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScTokenArray& rArr,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME );
    SC_DLLPUBLIC                ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScAddress& rTarget );
                                // rTarget ist ABSPOS Sprungmarke
                    ScRangeData(const ScRangeData& rScRangeData);

    SC_DLLPUBLIC virtual        ~ScRangeData();


    virtual ScDataObject* Clone() const;

    sal_Bool            operator== (const ScRangeData& rData) const;

    void            GetName( String& rName ) const  { rName = aName; }
    const String&   GetName( void ) const           { return aName; }
    const String&   GetUpperName( void ) const      { return aUpperName; }
    ScAddress       GetPos() const                  { return aPos; }
    // Der Index muss eindeutig sein. Ist er 0, wird ein neuer Index vergeben
    void            SetIndex( sal_uInt16 nInd )         { nIndex = nInd; }
    sal_uInt16    GetIndex() const                { return nIndex; }
    ScTokenArray*   GetCode()                       { return pCode; }
    sal_uInt16          GetErrCode();
    sal_Bool            HasReferences() const;
    void            SetDocument( ScDocument* pDocument){ pDoc = pDocument; }
    ScDocument*     GetDocument() const             { return pDoc; }
    void            SetType( RangeType nType )      { eType = nType; }
    void            AddType( RangeType nType )      { eType = eType|nType; }
    RangeType       GetType() const                 { return eType; }
    sal_Bool            HasType( RangeType nType ) const;
    SC_DLLPUBLIC void           GetSymbol( String& rSymbol, const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            UpdateSymbol( rtl::OUStringBuffer& rBuffer, const ScAddress&,
                                    const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT );
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                             const ScRange& r,
                             SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    sal_Bool            IsModified() const              { return bModified; }

    SC_DLLPUBLIC void           GuessPosition();

    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    SC_DLLPUBLIC sal_Bool           IsReference( ScRange& rRef ) const;
    sal_Bool            IsReference( ScRange& rRef, const ScAddress& rPos ) const;
    sal_Bool            IsValidReference( ScRange& rRef ) const;

//UNUSED2009-05 sal_Bool            IsRangeAtCursor( const ScAddress&, sal_Bool bStartOnly ) const;
    sal_Bool            IsRangeAtBlock( const ScRange& ) const;

    void            UpdateTabRef(SCTAB nOldTable, sal_uInt16 nFlag, SCTAB nNewTable);
    void            TransferTabRef( SCTAB nOldTab, SCTAB nNewTab );

    void            ValidateTabRefs();

    void            ReplaceRangeNamesInUse( const IndexMap& rMap );

    static void     MakeValidName( String& rName );
    SC_DLLPUBLIC static sal_Bool        IsNameValid( const String& rName, ScDocument* pDoc );

    SC_DLLPUBLIC void SetMaxRow(SCROW nRow);
    SCROW GetMaxRow() const;
    SC_DLLPUBLIC void SetMaxCol(SCCOL nCol);
    SCCOL GetMaxCol() const;
};

inline sal_Bool ScRangeData::HasType( RangeType nType ) const
{
    return ( ( eType & nType ) == nType );
}

extern "C" int SAL_CALL ScRangeData_QsortNameCompare( const void*, const void* );

#if defined( ICC ) && defined( OS2 )
    static int _Optlink  ICCQsortNameCompare( const void* a, const void* b)
                            { return ScRangeData_QsortNameCompare(a,b); }
#endif

//------------------------------------------------------------------------

class ScRangeName : public ScSortedCollection
{
private:
    ScDocument* pDoc;
    sal_uInt16 nSharedMaxIndex;

    using ScSortedCollection::Clone;    // calcwarnings: shouldn't be used

public:
    ScRangeName(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False,
                ScDocument* pDocument = NULL) :
        ScSortedCollection  ( nLim, nDel, bDup ),
        pDoc                ( pDocument ),
        nSharedMaxIndex     ( 1 ) {}            // darf nicht 0 sein!!

    ScRangeName(const ScRangeName& rScRangeName, ScDocument* pDocument);

    virtual ScDataObject*     Clone(ScDocument* pDocP) const
                             { return new ScRangeName(*this, pDocP); }
    ScRangeData*            operator[]( const sal_uInt16 nIndex) const
                             { return (ScRangeData*)At(nIndex); }
    virtual short           Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
    virtual sal_Bool            IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;

//UNUSED2009-05 ScRangeData*            GetRangeAtCursor( const ScAddress&, sal_Bool bStartOnly ) const;
    SC_DLLPUBLIC ScRangeData*           GetRangeAtBlock( const ScRange& ) const;

    SC_DLLPUBLIC sal_Bool                   SearchName( const String& rName, sal_uInt16& rPos ) const;
                            // SearchNameUpper must be called with an upper-case search string
    sal_Bool                    SearchNameUpper( const String& rUpperName, sal_uInt16& rPos ) const;
    void                    UpdateReference(UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                    UpdateTabRef(SCTAB nTable, sal_uInt16 nFlag, SCTAB nNewTable = 0);
    void                    UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void                    UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );
    virtual sal_Bool            Insert(ScDataObject* pScDataObject);
    SC_DLLPUBLIC ScRangeData*           FindIndex(sal_uInt16 nIndex);
    sal_uInt16                  GetSharedMaxIndex()             { return nSharedMaxIndex; }
    void                    SetSharedMaxIndex(sal_uInt16 nInd)  { nSharedMaxIndex = nInd; }
    sal_uInt16                  GetEntryIndex();
};

#endif

