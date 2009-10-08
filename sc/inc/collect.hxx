/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: collect.hxx,v $
 * $Revision: 1.6.32.4 $
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

#ifndef SC_COLLECT_HXX
#define SC_COLLECT_HXX

#include "address.hxx"
#include <tools/string.hxx>

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif
#include "scdllapi.h"

#define MAXCOLLECTIONSIZE       16384
#define MAXDELTA                1024
#define SCPOS_INVALID           USHRT_MAX

#define SC_STRTYPE_VALUE        0
#define SC_STRTYPE_STANDARD     1

class ScDocument;

class DataObject
{
public:
                            DataObject() {}
    virtual                 ~DataObject();
    virtual DataObject*     Clone() const = 0;
};

class SC_DLLPUBLIC Collection : public DataObject
{
protected:
    USHORT          nCount;
    USHORT          nLimit;
    USHORT          nDelta;
    DataObject**    pItems;
public:
                        Collection(USHORT nLim = 4, USHORT nDel = 4);
                        Collection(const Collection& rCollection);
    virtual             ~Collection();

    virtual DataObject* Clone() const;

            void        AtFree(USHORT nIndex);
            void        Free(DataObject* pDataObject);
            void        FreeAll();

            BOOL        AtInsert(USHORT nIndex, DataObject* pDataObject);
    virtual BOOL        Insert(DataObject* pDataObject);

            DataObject* At(USHORT nIndex) const;
    virtual USHORT      IndexOf(DataObject* pDataObject) const;
            USHORT      GetCount() const { return nCount; }

            DataObject* operator[]( const USHORT nIndex) const {return At(nIndex);}
            Collection& operator=( const Collection& rCol );
};


class SC_DLLPUBLIC SortedCollection : public Collection
{
private:
    BOOL    bDuplicates;
protected:
                        // fuer StrCollection Load/Store
            void        SetDups( BOOL bVal ) { bDuplicates = bVal; }
            BOOL        IsDups() const { return bDuplicates; }
public:
                        SortedCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE);
                        SortedCollection(const SortedCollection& rSortedCollection) :
                            Collection(rSortedCollection),
                            bDuplicates(rSortedCollection.bDuplicates) {}

    virtual USHORT      IndexOf(DataObject* pDataObject) const;
    virtual short       Compare(DataObject* pKey1, DataObject* pKey2) const = 0;
    virtual BOOL        IsEqual(DataObject* pKey1, DataObject* pKey2) const;
            BOOL        Search(DataObject* pDataObject, USHORT& rIndex) const;
    virtual BOOL        Insert(DataObject* pDataObject);
    virtual BOOL        InsertPos(DataObject* pDataObject, USHORT& nIndex);

            BOOL        operator==(const SortedCollection& rCmp) const;
};



//------------------------------------------------------------------------
class StrData : public DataObject
{
friend class StrCollection;
    String aStr;
public:
                        StrData(const String& rStr) : aStr(rStr) {}
                        StrData(const StrData& rData) : DataObject(), aStr(rData.aStr) {}
    virtual DataObject* Clone() const;
    const String&       GetString() const { return aStr; }
    // SetString nur, wenn StrData nicht in StrCollection ist! !!!
    // z.B. fuer Searcher
    void                SetString( const String& rNew ) { aStr = rNew; }
};

//------------------------------------------------------------------------

class SvStream;

class SC_DLLPUBLIC StrCollection : public SortedCollection
{
public:
    StrCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) :
                        SortedCollection    ( nLim, nDel, bDup ) {}
    StrCollection(const StrCollection& rStrCollection) :
                        SortedCollection    ( rStrCollection ) {}

    virtual DataObject* Clone() const;
            StrData*    operator[]( const USHORT nIndex) const {return (StrData*)At(nIndex);}
    virtual short       Compare(DataObject* pKey1, DataObject* pKey2) const;

            void        Load( SvStream& );
            void        Store( SvStream& ) const;
};

//------------------------------------------------------------------------
// TypedStrCollection: wie StrCollection, nur, dass Zahlen vor Strings
//                     sortiert werden

class TypedStrData : public DataObject
{
public:
            TypedStrData( const String& rStr, double nVal = 0.0,
                          USHORT nType = SC_STRTYPE_STANDARD )
                : aStrValue(rStr),
                  nValue(nVal),
                  nStrType(nType) {}

//UNUSED2008-05  TypedStrData( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
//UNUSED2008-05                  BOOL bAllStrings );

            TypedStrData( const TypedStrData& rCpy )
                : DataObject(),
                  aStrValue(rCpy.aStrValue),
                  nValue(rCpy.nValue),
                  nStrType(rCpy.nStrType) {}

    virtual DataObject* Clone() const;

    BOOL                IsStrData() const { return nStrType != 0; }
    const String&       GetString() const { return aStrValue; }
    double              GetValue () const { return nValue; }

private:
    friend class TypedStrCollection;
#if OLD_PIVOT_IMPLEMENTATION
    friend class PivotStrCollection;
#endif

    String  aStrValue;
    double  nValue;
    USHORT  nStrType;           // 0 = Value
};

class SC_DLLPUBLIC TypedStrCollection : public SortedCollection
{
private:
    BOOL    bCaseSensitive;

public:
            TypedStrCollection( USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE )
                : SortedCollection( nLim, nDel, bDup ) { bCaseSensitive = FALSE; }

            TypedStrCollection( const TypedStrCollection& rCpy )
                : SortedCollection( rCpy ) { bCaseSensitive = rCpy.bCaseSensitive; }

    virtual DataObject*     Clone() const;
    virtual short           Compare( DataObject* pKey1, DataObject* pKey2 ) const;

    TypedStrData*   operator[]( const USHORT nIndex) const
                        { return (TypedStrData*)At(nIndex); }

    void    SetCaseSensitive( BOOL bSet )       { bCaseSensitive = bSet; }

    BOOL    FindText( const String& rStart, String& rResult, USHORT& rPos, BOOL bBack ) const;
    BOOL    GetExactMatch( String& rString ) const;
};

#endif
