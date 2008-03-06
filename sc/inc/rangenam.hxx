/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rangenam.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:18:07 $
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

#ifndef SC_RANGENAM_HXX
#define SC_RANGENAM_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx" // -> enum UpdateRefMode
#endif
#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif
#ifndef SC_GRAMMAR_HXX
#include "grammar.hxx"
#endif

//------------------------------------------------------------------------

class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;

namespace rtl {
    class OUStringBuffer;
}


//------------------------------------------------------------------------

typedef USHORT RangeType;

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
class ScIndexMap;

class ScRangeData : public DataObject
{
private:
    String          aName;
    String          aUpperName;         // #i62977# for faster searching (aName is never modified after ctor)
    ScTokenArray*   pCode;
    ScAddress       aPos;
    RangeType       eType;
    ScDocument*     pDoc;
    USHORT          nIndex;
    BOOL            bModified;          // wird bei UpdateReference gesetzt/geloescht

    friend class ScRangeName;
    ScRangeData( USHORT nIndex );
public:
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const String& rSymbol,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME,
                                 const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScTokenArray& rArr,
                                 const ScAddress& rAdr = ScAddress(),
                                 RangeType nType = RT_NAME );
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScAddress& rTarget );
                                // rTarget ist ABSPOS Sprungmarke
                    ScRangeData(const ScRangeData& rScRangeData);

    virtual         ~ScRangeData();


    virtual DataObject* Clone() const;

    BOOL            operator== (const ScRangeData& rData) const;

    void            GetName( String& rName ) const  { rName = aName; }
    const String&   GetName( void ) const           { return aName; }
    const String&   GetUpperName( void ) const      { return aUpperName; }
    ScAddress       GetPos() const                  { return aPos; }
    // Der Index muss eindeutig sein. Ist er 0, wird ein neuer Index vergeben
    void            SetIndex( USHORT nInd )         { nIndex = nInd; }
    const USHORT    GetIndex() const                { return nIndex; }
    ScTokenArray*   GetCode()                       { return pCode; }
    USHORT          GetErrCode();
    BOOL            HasReferences() const;
    void            SetDocument( ScDocument* pDocument){ pDoc = pDocument; }
    ScDocument*     GetDocument() const             { return pDoc; }
    void            SetType( RangeType nType )      { eType = nType; }
    void            AddType( RangeType nType )      { eType = eType|nType; }
    RangeType       GetType() const                 { return eType; }
    BOOL            HasType( RangeType nType ) const;
    void            GetSymbol( String& rSymbol, const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT ) const;
    void            UpdateSymbol( String& rSymbol, const ScAddress&,
                                    const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );
    void            UpdateSymbol( rtl::OUStringBuffer& rBuffer, const ScAddress&,
                                    const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                             const ScRange& r,
                             SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    BOOL            IsModified() const              { return bModified; }

    void            GuessPosition();

    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    BOOL            IsReference( ScRange& rRef ) const;
    BOOL            IsReference( ScRange& rRef, const ScAddress& rPos ) const;
    BOOL            IsValidReference( ScRange& rRef ) const;

    BOOL            IsRangeAtCursor( const ScAddress&, BOOL bStartOnly ) const;
    BOOL            IsRangeAtBlock( const ScRange& ) const;

    void            UpdateTabRef(SCTAB nOldTable, USHORT nFlag, SCTAB nNewTable);
    void            TransferTabRef( SCTAB nOldTab, SCTAB nNewTab );

    void            ValidateTabRefs();

    void            ReplaceRangeNamesInUse( const ScIndexMap& rMap );

    BOOL            IsBeyond( SCROW nMaxRow ) const;

    static void     MakeValidName( String& rName );
    SC_DLLPUBLIC static BOOL        IsNameValid( const String& rName, ScDocument* pDoc );
};

inline BOOL ScRangeData::HasType( RangeType nType ) const
{
    return ( ( eType & nType ) == nType );
}

extern "C" int
#ifdef WNT
__cdecl
#endif
ScRangeData_QsortNameCompare( const void*, const void* );

#if defined( ICC ) && defined( OS2 )
    static int _Optlink  ICCQsortNameCompare( const void* a, const void* b)
                            { return ScRangeData_QsortNameCompare(a,b); }
#endif

//------------------------------------------------------------------------

class ScRangeName : public SortedCollection
{
private:
    ScDocument* pDoc;
    USHORT nSharedMaxIndex;

    using SortedCollection::Clone;    // calcwarnings: shouldn't be used

public:
    ScRangeName(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE,
                ScDocument* pDocument = NULL) :
        SortedCollection    ( nLim, nDel, bDup ),
        pDoc                ( pDocument ),
        nSharedMaxIndex     ( 1 ) {}            // darf nicht 0 sein!!

    ScRangeName(const ScRangeName& rScRangeName, ScDocument* pDocument);

    virtual DataObject*     Clone(ScDocument* pDocP) const
                             { return new ScRangeName(*this, pDocP); }
    ScRangeData*            operator[]( const USHORT nIndex) const
                             { return (ScRangeData*)At(nIndex); }
    virtual short           Compare(DataObject* pKey1, DataObject* pKey2) const;
    virtual BOOL            IsEqual(DataObject* pKey1, DataObject* pKey2) const;

    ScRangeData*            GetRangeAtCursor( const ScAddress&, BOOL bStartOnly ) const;
    ScRangeData*            GetRangeAtBlock( const ScRange& ) const;

    BOOL                    SearchName( const String& rName, USHORT& rPos ) const;
                            // SearchNameUpper must be called with an upper-case search string
    BOOL                    SearchNameUpper( const String& rUpperName, USHORT& rPos ) const;
    void                    UpdateReference(UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                    UpdateTabRef(SCTAB nTable, USHORT nFlag, SCTAB nNewTable = 0);
    void                    UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void                    UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );
    virtual BOOL            Insert(DataObject* pDataObject);
    ScRangeData*            FindIndex(USHORT nIndex);
    USHORT                  GetSharedMaxIndex()             { return nSharedMaxIndex; }
    void                    SetSharedMaxIndex(USHORT nInd)  { nSharedMaxIndex = nInd; }
    USHORT                  GetEntryIndex();
};

#endif

