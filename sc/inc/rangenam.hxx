/*************************************************************************
 *
 *  $RCSfile: rangenam.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-06 13:44:06 $
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

#ifndef SC_RANGENAM_HXX
#define SC_RANGENAM_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx" // -> enum UpdateRefMode
#endif
#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

//------------------------------------------------------------------------

class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;

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
#if defined( ICC ) && defined( OS2 )
    friend static int _Optlink   ICCQsortNameCompare( const void* a, const void* b);
#endif
private:
    String          aName;
    ScTokenArray*   pCode;
    ScAddress       aPos;
    RangeType       eType;
    ScDocument*     pDoc;
    USHORT          nIndex;
    USHORT          nExportIndex;
    BOOL            bModified;          // wird bei UpdateReference gesetzt/geloescht

    friend class ScRangeName;
    ScRangeData( USHORT nIndex );
public:
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const String& rSymbol,
                                 USHORT nCol = 0,
                                 USHORT nRow = 0,
                                 USHORT nTab = 0,
                                 RangeType nType = RT_NAME,
                                 BOOL bEnglish = FALSE );
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScTokenArray& rArr,
                                 USHORT nCol = 0,
                                 USHORT nRow = 0,
                                 USHORT nTab = 0,
                                 RangeType nType = RT_NAME );
                    ScRangeData( ScDocument* pDoc,
                                 const String& rName,
                                 const ScAddress& rTarget );
                                // rTarget ist ABSPOS Sprungmarke
                    ScRangeData(const ScRangeData& rScRangeData);
                    ScRangeData( SvStream& rStream,
                                 ScMultipleReadHeader& rHdr,
                                 USHORT nVer );

    virtual         ~ScRangeData();


    virtual DataObject* Clone() const;

    BOOL            Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

    BOOL            operator== (const ScRangeData& rData) const;

    void            GetName( String& rName ) const  { rName = aName; }
    const String&   GetName( void ) const           { return aName; }
    ScAddress       GetPos() const                  { return aPos; }
    // Der Index muss eindeutig sein. Ist er 0, wird ein neuer Index vergeben
    void            SetIndex( USHORT nInd )         { nIndex = nExportIndex = nInd; }
    const USHORT    GetIndex()                      { return nIndex; }
    void            SetExportIndex( USHORT nInd )   { nExportIndex = nInd; }
    const USHORT    GetExportIndex()                { return nExportIndex; }
    ScTokenArray*   GetCode()                       { return pCode; }
    USHORT          GetErrCode();
    BOOL            HasReferences() const;
    void            SetDocument( ScDocument* pDocument){ pDoc = pDocument; }
    ScDocument*     GetDocument() const             { return pDoc; }
    void            SetType( RangeType nType )      { eType = nType; }
    void            AddType( RangeType nType )      { eType = eType|nType; }
    RangeType       GetType() const                 { return eType; }
    BOOL            HasType( RangeType nType ) const;
    void            GetSymbol(String& rSymbol) const;
    void            GetEnglishSymbol(String& rSymbol, BOOL bCompileXML = FALSE) const;
    void            UpdateSymbol( String& rSymbol, const ScAddress&,
                                    BOOL bEnglish = FALSE, BOOL bCompileXML = FALSE );
    void            UpdateReference( UpdateRefMode eUpdateRefMode,
                             const ScRange& r,
                             short nDx, short nDy, short nDz );
    BOOL            IsModified() const              { return bModified; }

    void            GuessPosition();

    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void            UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    BOOL            IsReference( ScRange& rRef ) const;

    BOOL            IsRangeAtCursor( const ScAddress&, BOOL bStartOnly ) const;
    BOOL            IsRangeAtBlock( const ScRange& ) const;

    void            UpdateTabRef(USHORT nOldTable, USHORT nFlag, USHORT nNewTable);
    void            TransferTabRef( USHORT nOldTab, USHORT nNewTab );

    void            ReplaceRangeNamesInUse( const ScIndexMap& rMap );

    BOOL            IsBeyond( USHORT nMaxRow ) const;

    static void     MakeValidName( String& rName );
    static BOOL     IsNameValid( const String& rName, ScDocument* pDoc );
#ifdef WNT
    static int __cdecl  QsortNameCompare( const void*, const void* );
#else
    static int      QsortNameCompare( const void*, const void* );
#endif
};

inline BOOL ScRangeData::HasType( RangeType nType ) const
{
    return ( ( eType & nType ) == nType );
}

#if defined( ICC ) && defined( OS2 )
    static int _Optlink  ICCQsortNameCompare( const void* a, const void* b)
                            { ScRangeData::QsortNameCompare(a,b); }
#endif

//------------------------------------------------------------------------

class ScRangeName : public SortedCollection
{
private:
    ScDocument* pDoc;
    USHORT nSharedMaxIndex;
public:
    ScRangeName(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE,
                ScDocument* pDocument = NULL) :
        SortedCollection    ( nLim, nDel, bDup ),
        pDoc                ( pDocument ),
        nSharedMaxIndex     ( 1 ) {}            // darf nicht 0 sein!!

    ScRangeName(const ScRangeName& rScRangeName, ScDocument* pDocument);

    virtual DataObject*     Clone(ScDocument* pDoc) const
                             { return new ScRangeName(*this, pDoc); }
    ScRangeData*            operator[]( const USHORT nIndex) const
                             { return (ScRangeData*)At(nIndex); }
    virtual short           Compare(DataObject* pKey1, DataObject* pKey2) const;
    virtual BOOL            IsEqual(DataObject* pKey1, DataObject* pKey2) const;

    ScRangeData*            GetRangeAtCursor( const ScAddress&, BOOL bStartOnly ) const;
    ScRangeData*            GetRangeAtBlock( const ScRange& ) const;

    BOOL                    Load( SvStream& rStream, USHORT nVer );
    BOOL                    Store( SvStream& rStream ) const;
    BOOL                    SearchName( const String& rName, USHORT& rPos ) const;
    void                    UpdateReference(UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange,
                                short nDx, short nDy, short nDz );
    void                    UpdateTabRef(USHORT nTable, USHORT nFlag, USHORT nNewTable = 0);
    void                    UpdateTranspose( const ScRange& rSource, const ScAddress& rDest );
    void                    UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );
    virtual BOOL            Insert(DataObject* pDataObject);
    ScRangeData*            FindIndex(USHORT nIndex);
    USHORT                  GetSharedMaxIndex()             { return nSharedMaxIndex; }
    void                    SetSharedMaxIndex(USHORT nInd)  { nSharedMaxIndex = nInd; }
    USHORT                  GetEntryIndex();
};

#endif

