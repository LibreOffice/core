/*************************************************************************
 *
 *  $RCSfile: collect.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_COLLECT_HXX
#define SC_COLLECT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include <limits.h>

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

class Collection : public DataObject
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


class SortedCollection : public Collection
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
                        StrData(const StrData& rData) :aStr(rData.aStr) {}
    virtual DataObject* Clone() const;
    const String&       GetString() const { return aStr; }
    // SetString nur, wenn StrData nicht in StrCollection ist! !!!
    // z.B. fuer Searcher
    void                SetString( const String& rNew ) { aStr = rNew; }
};

//------------------------------------------------------------------------

class SvStream;

class StrCollection : public SortedCollection
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

            TypedStrData( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
                            BOOL bAllStrings );

            TypedStrData( const TypedStrData& rCpy )
                : aStrValue(rCpy.aStrValue),
                  nValue(rCpy.nValue),
                  nStrType(rCpy.nStrType) {}

    virtual DataObject* Clone() const;

    BOOL                IsStrData() const { return nStrType != 0; }
    const String&       GetString() const { return aStrValue; }
    double              GetValue () const { return nValue; }

private:
    friend class TypedStrCollection;
    friend class PivotStrCollection;

    String  aStrValue;
    double  nValue;
    USHORT  nStrType;           // 0 = Value
};

class TypedStrCollection : public SortedCollection
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


