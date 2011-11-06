/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

class SC_DLLPUBLIC ScDataObject
{
public:
                            ScDataObject() {}
    virtual    ~ScDataObject();
    virtual    ScDataObject*       Clone() const = 0;
};

class SC_DLLPUBLIC ScCollection : public ScDataObject
{
protected:
    sal_uInt16          nCount;
    sal_uInt16          nLimit;
    sal_uInt16          nDelta;
    ScDataObject**  pItems;
public:
    ScCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
    ScCollection(const ScCollection& rCollection);
    virtual             ~ScCollection();

    virtual ScDataObject*   Clone() const;

    void        AtFree(sal_uInt16 nIndex);
    void        Free(ScDataObject* pScDataObject);
    void        FreeAll();

    sal_Bool        AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject);
    virtual sal_Bool        Insert(ScDataObject* pScDataObject);

    ScDataObject*   At(sal_uInt16 nIndex) const;
    virtual sal_uInt16      IndexOf(ScDataObject* pScDataObject) const;
    sal_uInt16 GetCount() const;

            ScDataObject* operator[]( const sal_uInt16 nIndex) const {return At(nIndex);}
            ScCollection&   operator=( const ScCollection& rCol );
};


class SC_DLLPUBLIC  ScSortedCollection : public ScCollection
{
private:
    sal_Bool    bDuplicates;
protected:
                        // fuer ScStrCollection Load/Store
            void        SetDups( sal_Bool bVal ) { bDuplicates = bVal; }
            sal_Bool        IsDups() const { return bDuplicates; }
public:
    ScSortedCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False);
    ScSortedCollection(const ScSortedCollection& rScSortedCollection) :
                            ScCollection(rScSortedCollection),
                            bDuplicates(rScSortedCollection.bDuplicates) {}

    virtual sal_uInt16      IndexOf(ScDataObject* pScDataObject) const;
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const = 0;
    virtual sal_Bool        IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;
    sal_Bool        Search(ScDataObject* pScDataObject, sal_uInt16& rIndex) const;
    virtual sal_Bool        Insert(ScDataObject* pScDataObject);
    virtual sal_Bool        InsertPos(ScDataObject* pScDataObject, sal_uInt16& nIndex);

    sal_Bool        operator==(const ScSortedCollection& rCmp) const;
};



//------------------------------------------------------------------------
class StrData : public ScDataObject
{
friend class ScStrCollection;
    String aStr;
public:
                        StrData(const String& rStr) : aStr(rStr) {}
                        StrData(const StrData& rData) : ScDataObject(), aStr(rData.aStr) {}
    virtual ScDataObject*   Clone() const;
    const String&       GetString() const { return aStr; }
    // SetString nur, wenn StrData nicht in ScStrCollection ist! !!!
    // z.B. fuer Searcher
    void                SetString( const String& rNew ) { aStr = rNew; }
};

//------------------------------------------------------------------------

class SvStream;

class SC_DLLPUBLIC ScStrCollection : public ScSortedCollection
{
public:
    ScStrCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False) :
                        ScSortedCollection  ( nLim, nDel, bDup ) {}
    ScStrCollection(const ScStrCollection& rScStrCollection) :
                        ScSortedCollection  ( rScStrCollection ) {}

    virtual ScDataObject*   Clone() const;
            StrData*    operator[]( const sal_uInt16 nIndex) const {return (StrData*)At(nIndex);}
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
};

//------------------------------------------------------------------------
// TypedScStrCollection: wie ScStrCollection, nur, dass Zahlen vor Strings
//                     sortiert werden

class TypedStrData : public ScDataObject
{
public:
            TypedStrData( const String& rStr, double nVal = 0.0,
                          sal_uInt16 nType = SC_STRTYPE_STANDARD )
                : aStrValue(rStr),
                  nValue(nVal),
                  nStrType(nType) {}

//UNUSED2008-05  TypedStrData( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
//UNUSED2008-05                  sal_Bool bAllStrings );

            TypedStrData( const TypedStrData& rCpy )
                : ScDataObject(),
                  aStrValue(rCpy.aStrValue),
                  nValue(rCpy.nValue),
                  nStrType(rCpy.nStrType) {}

    virtual ScDataObject*   Clone() const;

    sal_Bool                IsStrData() const { return nStrType != 0; }
    const String&       GetString() const { return aStrValue; }
    double              GetValue () const { return nValue; }

private:
    friend class TypedScStrCollection;

    String  aStrValue;
    double  nValue;
    sal_uInt16  nStrType;           // 0 = Value
};

class SC_DLLPUBLIC TypedScStrCollection : public ScSortedCollection
{
private:
    sal_Bool    bCaseSensitive;

public:
    TypedScStrCollection( sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False );

    TypedScStrCollection( const TypedScStrCollection& rCpy )
        : ScSortedCollection( rCpy ) { bCaseSensitive = rCpy.bCaseSensitive; }
    ~TypedScStrCollection();

    virtual ScDataObject*       Clone() const;
    virtual short           Compare( ScDataObject* pKey1, ScDataObject* pKey2 ) const;

    TypedStrData*   operator[]( const sal_uInt16 nIndex) const;

    void    SetCaseSensitive( sal_Bool bSet );

    sal_Bool    FindText( const String& rStart, String& rResult, sal_uInt16& rPos, sal_Bool bBack ) const;
    sal_Bool    GetExactMatch( String& rString ) const;
};

#endif
