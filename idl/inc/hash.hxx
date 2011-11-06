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



#ifndef _HASH_HXX
#define _HASH_HXX



#include <tools/ref.hxx>
#include <tools/string.hxx>

/****************** H a s h - T a b l e **********************************/
class SvHashTable
{
    sal_uInt32       nMax;                 // size of hash-tabel
    sal_uInt32       nFill;                // elements in hash-tabel
    sal_uInt32       lAsk;                 // Anzahl der Anfragen
    sal_uInt32       lTry;                 // Anzahl der Versuche
protected:
    sal_Bool        Test_Insert( const void *, sal_Bool bInsert, sal_uInt32 * pInsertPos );

                            // compare element with entry
    virtual StringCompare   Compare( const void * , sal_uInt32 ) const = 0;
                            // get hash value from subclass
    virtual sal_uInt32          HashFunc( const void * ) const = 0;
public:
                SvHashTable( sal_uInt32 nMaxEntries );
                virtual ~SvHashTable();

    sal_uInt32      GetMax() const { return nMax; }

    virtual sal_Bool            IsEntry( sal_uInt32 ) const = 0;
};

/************** S t r i n g H a s h T a b l e E n t r y ******************/
class SvStringHashTable;
class SvStringHashEntry : public SvRefBase
{
friend class SvStringHashTable;
    ByteString  aName;
    sal_uInt32  nHashId;
    sal_uLong   nValue;
    sal_Bool    bHasId;
public:
                    SvStringHashEntry() : bHasId( sal_False ) {;}
                    SvStringHashEntry( const ByteString & rName, sal_uInt32 nIdx )
                        : aName( rName )
                        , nHashId( nIdx )
                        , nValue( 0 )
                        , bHasId( sal_True ) {}
                    ~SvStringHashEntry();

    const ByteString &  GetName() const { return aName; }
    sal_Bool            HasId() const { return bHasId; }
    sal_uInt32          GetId() const { return nHashId; }

    void            SetValue( sal_uLong n ) { nValue = n; }
    sal_uLong           GetValue() const { return nValue; }

    sal_Bool            operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    sal_Bool            operator != ( const SvStringHashEntry & rRef )
                    { return ! operator == ( rRef ); }
    SvStringHashEntry & operator = ( const SvStringHashEntry & rRef )
        { SvRefBase::operator=( rRef );
          aName   = rRef.aName;
          nHashId = rRef.nHashId;
          nValue  = rRef.nValue;
          bHasId  = rRef.bHasId;
          return *this;
        }
};

SV_DECL_IMPL_REF(SvStringHashEntry)

/****************** S t r i n g H a s h T a b l e ************************/
DECLARE_LIST(SvStringHashList,SvStringHashEntry *)

class SvStringHashTable : public SvHashTable
{
    SvStringHashEntry * pEntries;
protected:
    virtual sal_uInt32          HashFunc( const void * pElement ) const;
    virtual StringCompare   Compare( const void * pElement, sal_uInt32 nIndex ) const;
public:
            SvStringHashTable( sal_uInt32 nMaxEntries );   // max size of hash-tabel
            virtual ~SvStringHashTable();

    ByteString          GetNearString( const ByteString & rName ) const;
    virtual sal_Bool    IsEntry( sal_uInt32 nIndex ) const;

    sal_Bool    Insert( const ByteString & rStr, sal_uInt32 * pHash ); // insert string
    sal_Bool    Test( const ByteString & rStr, sal_uInt32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( sal_uInt32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( sal_uInt32 nPos ) const
            { return pEntries[ nPos ]; }

    void    FillHashList( SvStringHashList * rList ) const;
};

#endif // _RSCHASH_HXX
